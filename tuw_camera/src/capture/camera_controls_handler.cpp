#include "tuw_camera/camera_controls_handler.hpp"

namespace tuw_camera
{

template<libcamera::ControlType CT>
struct control_data_type;
template<>
struct control_data_type<libcamera::ControlTypeBool>
{
  using type = bool;
};
template<>
struct control_data_type<libcamera::ControlTypeByte>
{
  using type = uint8_t;
};
template<>
struct control_data_type<libcamera::ControlTypeInteger32>
{
  using type = int32_t;
};
template<>
struct control_data_type<libcamera::ControlTypeInteger64>
{
  using type = int64_t;
};
template<>
struct control_data_type<libcamera::ControlTypeFloat>
{
  using type = float;
};
template<>
struct control_data_type<libcamera::ControlTypeString>
{
  using type = std::string_view;
};
template<>
struct control_data_type<libcamera::ControlTypeRectangle>
{
  using type = libcamera::Rectangle;
};
template<>
struct control_data_type<libcamera::ControlTypeSize>
{
  using type = libcamera::Size;
};

template<libcamera::ControlType CT>
struct control_param_type
{
  using type = typename control_data_type<CT>::type;
};
template<>
struct control_param_type<libcamera::ControlTypeRectangle>
{
  using type = std::vector<double>;
};
template<>
struct control_param_type<libcamera::ControlTypeSize>
{
  using type = std::vector<double>;
};

template<libcamera::ControlType CT>
struct control_value_converter
{
  using data_type = typename control_data_type<CT>::type;
  using param_type = data_type;

  static data_type from_parameter(const param_type & param) {return param;}

  static param_type to_parameter(const data_type & data) {return data;}
};

template<>
struct control_value_converter<libcamera::ControlTypeString>
{
  using data_type = typename control_data_type<libcamera::ControlTypeString>::type;
  using param_type = std::string;

  static data_type from_parameter(const param_type & param) {return data_type(param);}

  static param_type to_parameter(const data_type & data) {return param_type(data);}
};

template<>
struct control_value_converter<libcamera::ControlTypeRectangle>
{
  using data_type = typename control_data_type<libcamera::ControlTypeRectangle>::type;
  using param_type = std::vector<int64_t>;

  static data_type from_parameter(const param_type & param)
  {
    if (param.size() != 4) {
      throw std::domain_error("Rectangle control type needs 4 values.");
    }
    if (param[2] < 0 || param[3] < 0) {
      throw std::domain_error("Rectangle control type needs positive width and height.");
    }
    return data_type(param[0], param[1], param[2], param[3]);
  }

  static param_type to_parameter(const data_type & data) {return {data.x, data.y, data.width, data.height};}
};

template<>
struct control_value_converter<libcamera::ControlTypeSize>
{
  using data_type = typename control_data_type<libcamera::ControlTypeSize>::type;
  using param_type = std::vector<int64_t>;

  static data_type from_parameter(const param_type & param)
  {
    if (param.size() != 2) {
      throw std::domain_error("Size control type needs 2 values.");
    }
    if (param[0] < 0 || param[1] < 0) {
      throw std::domain_error("Size control type needs positive width and height.");
    }
    return data_type(param[0], param[1]);
  }

  static param_type to_parameter(const data_type & data) {return {data.width, data.height};}
};

template<libcamera::ControlType CT>
libcamera::ControlValue conversion_func(const rclcpp::Parameter & param)
{
  using converter = control_value_converter<CT>;
  return converter::from_parameter(param.get_value<typename converter::param_type>());
}

#define HANDLE_CONTROL_TYPE(T) \
    case libcamera::ControlType ## T: \
      handle_parameter<libcamera::ControlType ## T>(node, ctrl_id, ctrl_info); \
      break;

CameraControlsHandler::CameraControlsHandler(rclcpp::Node * node, const libcamera::ControlInfoMap & controls)
: logger(node->get_logger())
{
  current_controls = libcamera::ControlList(controls);

  for (const auto & [ctrl_id, ctrl_info] : controls) {
    RCLCPP_INFO(node->get_logger(), "Camera supported control: %s Type: %d Info: %s", ctrl_id->name().c_str(), ctrl_id->type(), ctrl_info.toString().c_str());

    if (ctrl_info.min().isArray() || ctrl_info.max().isArray() || ctrl_info.def().isArray()) {
      RCLCPP_WARN(node->get_logger(), "Skipping array-valued control '%s' (not supported)", ctrl_id->name().c_str());
      continue;
    }

    switch (ctrl_id->type()) {
    HANDLE_CONTROL_TYPE(Bool);
    HANDLE_CONTROL_TYPE(Byte);
    HANDLE_CONTROL_TYPE(Integer32);
    HANDLE_CONTROL_TYPE(Integer64);
    HANDLE_CONTROL_TYPE(Float);
    HANDLE_CONTROL_TYPE(String);
    HANDLE_CONTROL_TYPE(Rectangle);
    HANDLE_CONTROL_TYPE(Size);
      default:
        throw std::runtime_error("ControlType " + std::to_string(ctrl_id->type()) + " is not known.");
    }
  }

  parameter_cb_handle = node->add_on_set_parameters_callback(std::bind(&CameraControlsHandler::parameter_callback, this, std::placeholders::_1));
}

template<libcamera::ControlType CT>
void CameraControlsHandler::handle_parameter(rclcpp::Node * node, const libcamera::ControlId * ctrl_id, const libcamera::ControlInfo & ctrl_info)
{
  using converter = control_value_converter<CT>;

  const std::string PARAM_PREFIX = "controls.";
  std::string param_id = PARAM_PREFIX + ctrl_id->name();
  typename converter::data_type def_value{};
  if (!ctrl_info.def().isNone()) {
    def_value = ctrl_info.def().get<typename converter::data_type>();
  }
  auto def_param = converter::to_parameter(def_value);
  auto param = node->declare_parameter<typename converter::param_type>(param_id, def_param);

  if (param != def_param) {
    current_controls.set(ctrl_id->id(), converter::from_parameter(param));
  }
  converters[param_id] = {ctrl_id, &conversion_func<CT>};
}

rcl_interfaces::msg::SetParametersResult CameraControlsHandler::parameter_callback(const std::vector<rclcpp::Parameter> & parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  bool updates = false;

  {
    std::lock_guard<std::mutex> lock(controls_mutex);
    for (const auto & param : parameters) {
      const auto entry = converters.find(param.get_name());
      if (entry != converters.end()) {
        RCLCPP_DEBUG(logger, "Updating control from parameter '%s'", entry->first.c_str());
        auto [ctrl_id, conversion] = entry->second;
        current_controls.set(ctrl_id->id(), conversion(param));
        updates = true;
      }
    }
  }

  if (updates) {
    update_seq++;
  }

  result.successful = true;
  return result;
}

void CameraControlsHandler::update_request(libcamera::ControlList & controls, uint32_t * req_update_seq, bool force)
{
  uint32_t observed_update_seq = update_seq;

  if (force || *req_update_seq != observed_update_seq) {
    std::lock_guard<std::mutex> lock(controls_mutex);
    RCLCPP_INFO(logger, "Applying update control values to request: ");
    for (const auto & [id, value] : current_controls) {
      RCLCPP_INFO(logger, "Control: %s Value: %s", current_controls.idMap()->at(id)->name().c_str(), value.toString().c_str());
      controls.set(id, value);
    }
    *req_update_seq = observed_update_seq;
  }
}

}  // namespace tuw_camera

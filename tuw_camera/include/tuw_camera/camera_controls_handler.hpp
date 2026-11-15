#ifndef TUW_CAMERA__CAMERA_CONTROLS_HANDLER_HPP_
#define TUW_CAMERA__CAMERA_CONTROLS_HANDLER_HPP_

#include <libcamera/libcamera.h>

#include <atomic>
#include <rclcpp/node.hpp>

namespace tuw_camera
{

class CameraControlsHandler
{
public:
  CameraControlsHandler(rclcpp::Node * node, const libcamera::ControlInfoMap & controls);
  void update_request(libcamera::ControlList & controls, uint32_t * req_update_seq, bool force = false);

private:
  using ConversionFunc = libcamera::ControlValue (*)(const rclcpp::Parameter &);
  using Converter = std::pair<const libcamera::ControlId *, ConversionFunc>;

  template<libcamera::ControlType ControlType>
  void handle_parameter(rclcpp::Node * node, const libcamera::ControlId * ctrl_id, const libcamera::ControlInfo & ctrl_info);

  std::unordered_map<std::string, Converter> converters;
  libcamera::ControlList current_controls;
  std::atomic<uint32_t> update_seq = 0;
  std::mutex controls_mutex;

  rcl_interfaces::msg::SetParametersResult parameter_callback(const std::vector<rclcpp::Parameter> & parameters);
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr parameter_cb_handle;

  rclcpp::Logger logger;
};

}  // namespace tuw_camera

#endif

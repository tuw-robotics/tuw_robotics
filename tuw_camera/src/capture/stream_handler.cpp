#include "tuw_camera/stream_handler.hpp"

#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "tuw_camera/convert.hpp"

namespace tuw_camera
{

std::string topic_name(const Params::Streams::MapStreamRoles & params, std::string suffix = "")
{
  std::string topic;
  if (!params.sub_topic.empty()) {
    topic += params.sub_topic + "/";
  }
  topic += params.topic;
  if (!suffix.empty()) {
    topic += "/" + suffix;
  }
  return topic;
}

template<typename TConvert>
class StreamHandlerImpl;

template<>
class StreamHandlerImpl<ConvertDirect>: public StreamHandler
{
public:
  StreamHandlerImpl(rclcpp::Node * node, const libcamera::StreamConfiguration & stream_cfg, const Params::Streams::MapStreamRoles & params, const ConvertDirect & conversion)
  {
    width = stream_cfg.size.width;
    height = stream_cfg.size.height;
    stride = stream_cfg.stride;
    ros_format = conversion.ros_format;

    publisher = node->create_publisher<sensor_msgs::msg::Image>(topic_name(params), rclcpp::SensorDataQoS());

    RCLCPP_INFO(node->get_logger(), "StreamHandlerImpl<ConvertDirect> publishing on topic: %s", publisher->get_topic_name());
  }

  void publish_buffer(const BufferContext & ctx, const std_msgs::msg::Header & header) override
  {
    // Create image message
    auto img_msg = std::make_unique<sensor_msgs::msg::Image>();

    // Copy into image message
    img_msg->data.assign(ctx.data().begin(), ctx.data().end());

    // Set metadata values
    img_msg->encoding = ros_format;
    img_msg->width = width;
    img_msg->height = height;
    img_msg->step = stride;
    img_msg->is_bigendian = false;
    img_msg->header = header;

    publisher->publish(std::move(img_msg));
  }

private:
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher;
  std::string ros_format;
  uint32_t width, height, stride;
};

template<>
class StreamHandlerImpl<ConvertCompressed>: public StreamHandler
{
public:
  StreamHandlerImpl(rclcpp::Node * node, const libcamera::StreamConfiguration & stream_cfg, const Params::Streams::MapStreamRoles & params, const ConvertCompressed & conversion)
  {
    width = stream_cfg.size.width;
    height = stream_cfg.size.height;
    ros_format = conversion.ros_format;

    publisher = node->create_publisher<sensor_msgs::msg::CompressedImage>(topic_name(params, "compressed"), rclcpp::SensorDataQoS());

    RCLCPP_INFO(node->get_logger(), "StreamHandlerImpl<ConvertCompressed> publishing on topic: %s", publisher->get_topic_name());
  }

  void publish_buffer(const BufferContext & ctx, const std_msgs::msg::Header & header) override
  {
    // Create image message
    auto img_msg = std::make_unique<sensor_msgs::msg::CompressedImage>();

    // Copy into image message
    img_msg->data.assign(ctx.data().begin(), ctx.data().end());

    // Set metadata values
    img_msg->format = ros_format;
    img_msg->header = header;

    publisher->publish(std::move(img_msg));
  }

private:
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr publisher;
  std::string ros_format;
  uint32_t width, height;
};

template<>
class StreamHandlerImpl<ConvertDecode>: public StreamHandler
{
public:
  StreamHandlerImpl(rclcpp::Node * node, const libcamera::StreamConfiguration & stream_cfg, const Params::Streams::MapStreamRoles & params, const ConvertDecode & conversion)
  {
    if (conversion.ros_format != sensor_msgs::image_encodings::BGR8) {
      throw std::runtime_error("Unsupported error for this stream handler");
    }

    width = stream_cfg.size.width;
    height = stream_cfg.size.height;
    decompressed_size = width * height * CHANNELS;
    stride = width * CHANNELS;

    publisher = node->create_publisher<sensor_msgs::msg::Image>(topic_name(params), rclcpp::SensorDataQoS());

    RCLCPP_INFO(node->get_logger(), "StreamHandlerImpl<ConvertDecode> publishing on topic: %s", publisher->get_topic_name());
  }

  void publish_buffer(const BufferContext & ctx, const std_msgs::msg::Header & header) override
  {
    // Create image message
    auto img_msg = std::make_unique<sensor_msgs::msg::Image>();

    // Decode into image message
    img_msg->data.resize(decompressed_size);
    cv::Mat dst{(int)height, (int)width, CV_8UC3, img_msg->data.data()};
    cv::imdecode(ctx.mat(), cv::ImreadModes::IMREAD_COLOR, &dst);
    assert(dst.data == img_msg->data.data());

    // Set metadata values
    img_msg->encoding = sensor_msgs::image_encodings::BGR8;
    img_msg->width = dst.cols;
    img_msg->height = dst.rows;
    img_msg->step = stride;
    img_msg->is_bigendian = false;
    img_msg->header = header;

    publisher->publish(std::move(img_msg));
  }

private:
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher;
  uint32_t width, height, stride, decompressed_size;
  static constexpr uint32_t CHANNELS = 3;
};

template<>
class StreamHandlerImpl<ConvertLuma>: public StreamHandler
{
public:
  StreamHandlerImpl(rclcpp::Node * node, const libcamera::StreamConfiguration & stream_cfg, const Params::Streams::MapStreamRoles & params, const ConvertLuma & conversion)
  {
    if (conversion.ros_format != sensor_msgs::image_encodings::MONO8) {
      throw std::runtime_error("Unsupported format for this stream handler");
    }

    width = stream_cfg.size.width;
    height = stream_cfg.size.height;
    // stride of the first (Y) plane
    stride = stream_cfg.stride;
    luma_size = static_cast<size_t>(stride) * height;

    publisher = node->create_publisher<sensor_msgs::msg::Image>(topic_name(params), rclcpp::SensorDataQoS());

    RCLCPP_INFO(node->get_logger(), "StreamHandlerImpl<ConvertLuma> publishing on topic: %s", publisher->get_topic_name());
  }

  void publish_buffer(const BufferContext & ctx, const std_msgs::msg::Header & header) override
  {
    auto data = ctx.data();
    if (data.size() < luma_size) {
      throw std::runtime_error("Buffer smaller than expected Y plane.");
    }

    auto img_msg = std::make_unique<sensor_msgs::msg::Image>();

    // The Y plane is the first plane of the buffer
    img_msg->data.assign(data.begin(), data.begin() + luma_size);

    img_msg->encoding = sensor_msgs::image_encodings::MONO8;
    img_msg->width = width;
    img_msg->height = height;
    img_msg->step = stride;
    img_msg->is_bigendian = false;
    img_msg->header = header;

    publisher->publish(std::move(img_msg));
  }

private:
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher;
  uint32_t width, height, stride;
  size_t luma_size;
};

template<>
class StreamHandlerImpl<ConvertPackedLuma>: public StreamHandler
{
public:
  StreamHandlerImpl(rclcpp::Node * node, const libcamera::StreamConfiguration & stream_cfg, const Params::Streams::MapStreamRoles & params, const ConvertPackedLuma & conversion)
  {
    if (conversion.ros_format != sensor_msgs::image_encodings::MONO8) {
      throw std::runtime_error("Unsupported format for this stream handler");
    }

    width = stream_cfg.size.width;
    height = stream_cfg.size.height;
    // packed 4:2:2 -> 2 bytes per pixel
    stride = stream_cfg.stride;
    y_offset = conversion.y_offset;
    if (stride < width * BYTES_PER_PIXEL) {
      throw std::runtime_error("Stride smaller than packed row size.");
    }

    publisher = node->create_publisher<sensor_msgs::msg::Image>(topic_name(params), rclcpp::SensorDataQoS());

    RCLCPP_INFO(node->get_logger(), "StreamHandlerImpl<ConvertPackedLuma> publishing on topic: %s", publisher->get_topic_name());
  }

  void publish_buffer(const BufferContext & ctx, const std_msgs::msg::Header & header) override
  {
    auto data = ctx.data();
    // last row only needs width * BYTES_PER_PIXEL bytes, not full stride
    const size_t required = static_cast<size_t>(stride) * (height - 1) + static_cast<size_t>(width) * BYTES_PER_PIXEL;
    if (data.size() < required) {
      throw std::runtime_error("Buffer smaller than expected image.");
    }

    auto img_msg = std::make_unique<sensor_msgs::msg::Image>();
    img_msg->data.resize(static_cast<size_t>(width) * height);

    const uint8_t * src = data.data();
    uint8_t * dst = img_msg->data.data();
    for (uint32_t row = 0; row < height; ++row) {
      const uint8_t * src_row = src + static_cast<size_t>(row) * stride + y_offset;
      uint8_t * dst_row = dst + static_cast<size_t>(row) * width;
      for (uint32_t x = 0; x < width; ++x) {
        dst_row[x] = src_row[BYTES_PER_PIXEL * x];
      }
    }

    img_msg->encoding = sensor_msgs::image_encodings::MONO8;
    img_msg->width = width;
    img_msg->height = height;
    img_msg->step = width;
    img_msg->is_bigendian = false;
    img_msg->header = header;

    publisher->publish(std::move(img_msg));
  }

private:
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher;
  uint32_t width, height, stride, y_offset;
  static constexpr uint32_t BYTES_PER_PIXEL = 2;
};

struct StreamFactory
{
  template<typename TConvert>
  StreamHandler::Ptr operator()(const TConvert & convert)
  {
    return std::make_unique<StreamHandlerImpl<TConvert>>(node, config, params, convert);
  }

  rclcpp::Node * node;
  const libcamera::StreamConfiguration & config;
  const Params::Streams::MapStreamRoles & params;
};

StreamHandler::Ptr create_stream_handler(rclcpp::Node * node, const libcamera::StreamConfiguration & stream_cfg, const Params::Streams::MapStreamRoles & params)
{
  StreamFactory factory{node, stream_cfg, params};

  auto mapping = get_format_mapping(stream_cfg.pixelFormat, params.target_format);
  if (!mapping) {
    auto target_formats = get_target_formats(stream_cfg.pixelFormat);

    std::stringstream error;
    if (params.target_format.empty()) {
      error << "No conversion available for pixel format: " << stream_cfg.pixelFormat;
    } else {
      error << "Target format '" << params.target_format << "' is not available for pixel format: " << stream_cfg.pixelFormat;
    }
    if (target_formats.empty()) {
      error << ". No target formats are supported for this pixel "
        "format.";
    } else {
      error << ". Supported target formats: ";
      for (size_t i = 0; i < target_formats.size(); ++i) {
        if (i > 0) {
          error << ", ";
        }
        error << target_formats[i];
      }
    }
    throw std::runtime_error(error.str());
  }

  return std::visit(factory, *mapping);
}

}  // namespace tuw_camera

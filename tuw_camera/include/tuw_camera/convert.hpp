#ifndef TUW_CAMERA__LIBCAMERA_CONVERT_HPP_
#define TUW_CAMERA__LIBCAMERA_CONVERT_HPP_

#include <libcamera/libcamera.h>

#include <optional>
#include <sensor_msgs/image_encodings.hpp>
#include <unordered_map>
#include <variant>
#include <vector>

namespace tuw_camera
{

struct ConvertCommon
{
  std::string ros_format;
};

struct ConvertDirect : ConvertCommon
{
};
struct ConvertCompressed : ConvertCommon
{
};
struct ConvertDecode : ConvertCommon
{
};
// Publish only the luma (Y) plane of a planar/semi-planar YUV format as mono8
struct ConvertLuma : ConvertCommon
{
};
// Extract the luma (Y) samples of a packed YUV 4:2:2 format (YUYV, UYVY) as
// mono8. Every second byte of a row is a Y sample, starting at y_offset.
struct ConvertPackedLuma : ConvertCommon
{
  uint32_t y_offset;
};

using FormatMapping = std::variant<ConvertDirect, ConvertCompressed, ConvertDecode, ConvertLuma, ConvertPackedLuma>;

std::optional<FormatMapping> get_format_mapping(libcamera::PixelFormat fmt, std::string target_format = "");

std::vector<std::string> get_target_formats(libcamera::PixelFormat fmt);

}   // namespace tuw_camera

#endif  // TUW_CAMERA__LIBCAMERA_CONVERT_HPP_

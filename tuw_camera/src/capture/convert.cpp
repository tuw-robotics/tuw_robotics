#include "tuw_camera/convert.hpp"

#include "opencv2/opencv.hpp"

template<>
struct std::hash<libcamera::PixelFormat>
{
  std::size_t operator()(const libcamera::PixelFormat & k) const noexcept {return std::hash<uint64_t>{}(k.fourcc());}
};

namespace tuw_camera
{

namespace lc = libcamera::formats;
namespace ros = sensor_msgs::image_encodings;

std::pair<libcamera::PixelFormat, FormatMapping> direct(libcamera::PixelFormat lc, const char * ros) {return {lc, ConvertDirect{ros}};}

std::pair<libcamera::PixelFormat, FormatMapping> decode(libcamera::PixelFormat lc, const char * ros) {return {lc, ConvertDecode{ros}};}

std::pair<libcamera::PixelFormat, FormatMapping> luma(libcamera::PixelFormat lc, const char * ros) {return {lc, ConvertLuma{ros}};}

std::pair<libcamera::PixelFormat, FormatMapping> packed_luma(libcamera::PixelFormat lc, const char * ros, uint32_t y_offset) {return {lc, ConvertPackedLuma{{ros}, y_offset}};}

std::pair<libcamera::PixelFormat, FormatMapping> compressed(libcamera::PixelFormat lc, const char * ros) {return {lc, ConvertCompressed{ros}};}

// libcamera formats interpret channels as one little endian integer:
// -> format XYZ8 is actually stored as ZYX8
const std::unordered_multimap<libcamera::PixelFormat, FormatMapping> FORMAT_MAP{
  // Mono
  direct(lc::R8, ros::MONO8),
  direct(lc::R16, ros::MONO16),
  // Color 8 bit
  direct(lc::RGB888, ros::BGR8),
  direct(lc::BGR888, ros::RGB8),
  direct(lc::ABGR8888, ros::RGBA8),
  direct(lc::ARGB8888, ros::BGRA16),
  // Color 16 bit
  direct(lc::RGB161616, ros::BGR16),
  direct(lc::BGR161616, ros::RGB16),
  // Compressed formats
  decode(lc::MJPEG, ros::BGR8),
  compressed(lc::MJPEG, "jpeg"),
  // Packed YCbCr
  direct(lc::UYVY, ros::YUV422),
  direct(lc::YUYV, ros::YUYV),
  // Packed YCbCr: luma samples only -> mono8
  packed_luma(lc::YUYV, ros::MONO8, 0),  // Y0 U0 Y1 V0
  packed_luma(lc::UYVY, ros::MONO8, 1),  // U0 Y0 V0 Y1
  // Planar / semi-planar YCbCr: luma plane only -> mono8
  luma(lc::YUV420, ros::MONO8),
  luma(lc::YVU420, ros::MONO8),
  luma(lc::NV12, ros::MONO8),
  luma(lc::NV21, ros::MONO8),

};

std::optional<FormatMapping> get_format_mapping(libcamera::PixelFormat fmt, std::string target_format)
{
  auto [start, end] = FORMAT_MAP.equal_range(fmt);
  auto mapping = std::find_if(start, end, [target_format](const auto & entry) {
        std::string ros_format = std::visit([](auto && conversion) {return conversion.ros_format;}, entry.second);
        return target_format.empty() || ros_format == target_format;
  });

  if (mapping == end) {
    return std::nullopt;
  } else {
    return mapping->second;
  }
}

std::vector<std::string> get_target_formats(libcamera::PixelFormat fmt)
{
  auto [start, end] = FORMAT_MAP.equal_range(fmt);
  std::vector<std::string> target_formats;
  for (auto it = start; it != end; ++it) {
    target_formats.push_back(std::visit([](auto && conversion) {return conversion.ros_format;}, it->second));
  }
  return target_formats;
}

}  // namespace tuw_camera

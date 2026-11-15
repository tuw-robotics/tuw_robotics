#ifndef TUW_CAMERA__BUFFER_CONTEXT_HPP_
#define TUW_CAMERA__BUFFER_CONTEXT_HPP_

#include <libcamera/libcamera.h>
#include <sys/mman.h>

#include <opencv2/core/mat.hpp>
#include <span>
#include <vector>

namespace tuw_camera
{

class BufferContext
{
public:
  // Disable copy
  BufferContext(const BufferContext &) = delete;
  BufferContext & operator=(const BufferContext &) = delete;
  BufferContext(libcamera::FrameBuffer * buffer, size_t stream_index);
  ~BufferContext();

  BufferContext(BufferContext && other);
  BufferContext & operator=(BufferContext && other);

  std::span<unsigned char> data() const;
  cv::Mat mat() const;
  size_t stream_idx() const;

private:
  void * mem;
  unsigned int size;
  size_t stream_index;
};

}  // namespace tuw_camera

#endif  // TUW_CAMERA__BUFFER_CONTEXT_HPP_

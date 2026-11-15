#include "tuw_camera/buffer_context.hpp"

namespace tuw_camera
{

BufferContext::BufferContext(libcamera::FrameBuffer * buffer, size_t stream_index)
: stream_index(stream_index)
{
  int fd = buffer->planes().front().fd.get();
  size = 0;
  for (const auto & plane : buffer->planes()) {
    if (fd != plane.fd.get()) {
      throw std::runtime_error("Planes have differing file descriptors.");
    }
    if (plane.offset == libcamera::FrameBuffer::Plane::kInvalidOffset) {
      throw std::runtime_error("Plane has invalid offset.");
    }
    size = std::max(size, plane.offset + plane.length);
  }

  mem = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
  if (mem == MAP_FAILED) {
    mem = nullptr;
    size = 0;
    throw std::runtime_error("Failed to map memory.");
  }
}

BufferContext::~BufferContext()
{
  if (mem) {
    munmap(mem, size);
    mem = nullptr;
    size = 0;
  }
}

BufferContext::BufferContext(BufferContext && other) {*this = std::move(other);}

BufferContext & BufferContext::operator=(BufferContext && other)
{
  mem = other.mem;
  size = other.size;
  stream_index = other.stream_index;
  other.mem = nullptr;
  other.size = 0;
  other.stream_index = 0;
  return *this;
}

std::span<unsigned char> BufferContext::data() const {return {(unsigned char *)mem, size};}

cv::Mat BufferContext::mat() const {return {1, (int)size, CV_8UC1, mem};}

size_t BufferContext::stream_idx() const {return stream_index;}

}   // namespace tuw_camera

#ifndef TUW_CAMERA__REQUEST_CONTEXT_HPP_
#define TUW_CAMERA__REQUEST_CONTEXT_HPP_

#include <libcamera/request.h>

#include <camera_info_manager/camera_info_manager.hpp>
#include <rclcpp/guard_condition.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/waitable.hpp>

#include "tuw_camera/camera_controls_handler.hpp"
#include "tuw_camera/stream_handler.hpp"

namespace tuw_camera
{

class RequestHandler : public rclcpp::Waitable
{
public:
  using FramePublishedCallback = std::function<void(rclcpp::Time)>;
  RequestHandler(rclcpp::Node * node, std::shared_ptr<libcamera::Camera> camera, size_t num_requests, std::string frame_id);

  void add_to_wait_set(rcl_wait_set_t & wait_set) override;
  bool is_ready(const rcl_wait_set_t & wait_set) override;
  std::shared_ptr<void> take_data() override;
  std::shared_ptr<void> take_data_by_entity_id(size_t id) override;
  void execute(const std::shared_ptr<void> & data) override;
  size_t get_number_of_ready_guard_conditions() override;
  void set_on_ready_callback(std::function<void(size_t, int)> callback) override;
  void clear_on_ready_callback() override;
  std::vector<std::shared_ptr<rclcpp::TimerBase>> get_timers() const override;

  void add_stream(libcamera::Stream * stream, std::unique_ptr<StreamHandler> stream_handler, libcamera::FrameBufferAllocator & allocator);
  void set_frame_publish_callback(FramePublishedCallback callback);
  void start();
  void handle(libcamera::Request * request);

private:
  struct RequestContext
  {
    RequestContext(size_t idx, std::shared_ptr<libcamera::Camera> camera);

    std::unique_ptr<libcamera::Request> request;
    uint32_t control_seq{};
    bool waiting{};
  };

  rclcpp::GuardCondition::SharedPtr gc;
  rclcpp::Logger logger;
  rclcpp::Clock::SharedPtr clock;
  std::chrono::nanoseconds time_offset;
  std::shared_ptr<libcamera::Camera> camera;
  std::string frame_id;

  std::vector<std::unique_ptr<libcamera::Request>> requests;
  std::vector<BufferContext> buffer_ctx;
  std::vector<RequestContext> request_ctx;
  std::vector<std::unique_ptr<StreamHandler>> stream_handlers;
  std::unique_ptr<CameraControlsHandler> controls_handler;
  FramePublishedCallback callback;

  std::atomic<size_t> waiting_requests = 0;
};

}  // namespace tuw_camera

#endif  // TUW_CAMERA__REQUEST_CONTEXT_HPP_

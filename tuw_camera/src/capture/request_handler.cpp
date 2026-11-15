#include "tuw_camera/request_handler.hpp"
#include "rcutils/logging_macros.h"
#include <thread>
#include <chrono>

namespace tuw_camera {

RequestHandler::RequestContext::RequestContext(
    size_t idx, std::shared_ptr<libcamera::Camera> camera) {
    request = camera->createRequest(idx);
    if (!request) {
        throw std::runtime_error("Failed to create request");
    }
}

RequestHandler::RequestHandler(rclcpp::Node *node,
                               std::shared_ptr<libcamera::Camera> camera,
                               size_t num_requests, std::string frame_id)
    : logger(node->get_logger()), clock(node->get_clock()), camera(camera),
      frame_id(frame_id) {
    static_assert(std::numeric_limits<size_t>::max() <=
                  std::numeric_limits<uint64_t>::max());

    gc = std::make_shared<rclcpp::GuardCondition>(
        node->get_node_base_interface()->get_context());
    controls_handler =
        std::make_unique<CameraControlsHandler>(node, camera->controls());

    for (size_t request_idx = 0; request_idx < num_requests; request_idx++) {
        auto &ctx = request_ctx.emplace_back(request_idx, camera);
        // Initialize request controls
        controls_handler->update_request(ctx.request->controls(),
                                         &ctx.control_seq, true);
    }

    // The buffer timestamps are based on ktime_get_ns() in the kernel driver which is passed onto libcamera:
    // https://github.com/raspberrypi/linux/blob/rpi-6.6.y/drivers/media/platform/bcm2835/bcm2835-unicam.c#L1010
    // ktime_get_ns returns monotonic time: https://docs.kernel.org/core-api/timekeeping.html
    // The user-space equivalent is clock_gettime(CLOCK_MONOTONIC),
    // in c++ this is abstracted by std::chrono::steady_clock.
    // So we need to determine the offset between std::chrono::steady_clock and ROS time.
    std::chrono::nanoseconds steady = std::chrono::steady_clock::now().time_since_epoch();
    std::chrono::nanoseconds ros_time{node->get_clock()->now().nanoseconds()};
    time_offset = ros_time - steady;
}

void RequestHandler::add_stream(libcamera::Stream *stream,
                                std::unique_ptr<StreamHandler> stream_handler,
                                libcamera::FrameBufferAllocator &allocator) {
    size_t stream_idx = stream_handlers.size();
    for (size_t request_idx = 0; request_idx < request_ctx.size();
         request_idx++) {
        const auto &buffer = allocator.buffers(stream).at(request_idx);

        // Add buffer to context
        size_t buffer_idx = buffer_ctx.size();
        buffer->setCookie(buffer_idx);
        buffer_ctx.emplace_back(buffer.get(), stream_idx);

        // Add buffer to request
        if (request_ctx.at(request_idx)
                .request->addBuffer(stream, buffer.get()) < 0) {
            throw std::runtime_error("Failed to add buffer to request");
        }
    }
    stream_handlers.push_back(std::move(stream_handler));
}

void RequestHandler::set_frame_publish_callback(
    FramePublishedCallback callback) {
    this->callback = callback;
}

void RequestHandler::start() {
    for (const auto &ctx : request_ctx) {
        camera->queueRequest(ctx.request.get());
    }
}

void RequestHandler::handle(libcamera::Request *request) {
    if (request->status() == libcamera::Request::RequestCancelled)
        return;

    RCLCPP_DEBUG_STREAM(logger, "Received completed request "
                                    << request->cookie() << " on thread: "
                                    << std::this_thread::get_id());

    auto &ctx = request_ctx.at(request->cookie());
    if (ctx.waiting)
        throw std::runtime_error("assertion failed: request not processed");
    ctx.waiting = true;
    waiting_requests++;
    gc->trigger();
}

void RequestHandler::execute(const std::shared_ptr<void> &data) {
    (void)data;
    RCLCPP_DEBUG(logger, "Execute begin: waiting = %ld",
                 waiting_requests.load());
    for (auto &ctx : request_ctx) {
        if (ctx.waiting) {
            waiting_requests--;

            RCLCPP_DEBUG_STREAM(logger, "Handle completed request "
                                            << ctx.request->cookie()
                                            << " on thread: "
                                            << std::this_thread::get_id());

            std_msgs::msg::Header header;
            header.frame_id = frame_id;
            for (auto [stream, buffer] : ctx.request->buffers()) {
                int64_t timestamp = time_offset.count() + static_cast<int64_t>(buffer->metadata().timestamp);
                header.stamp = rclcpp::Time(timestamp);
                auto &ctx = buffer_ctx.at(buffer->cookie());
                stream_handlers.at(ctx.stream_idx())
                    ->publish_buffer(ctx, header);
            }

            int64_t delay_ms = (clock->now() - rclcpp::Time(header.stamp)).nanoseconds() / 1'000'000;
            RCLCPP_DEBUG_STREAM(logger, "Published completed request "
                                            << ctx.request->cookie()
                                            << " with processing delay "
                                            << delay_ms << "ms");

            if (callback) {
                callback(header.stamp);
            }

            ctx.request->reuse(libcamera::Request::ReuseBuffers);
            controls_handler->update_request(ctx.request->controls(),
                                             &ctx.control_seq, false);
            ctx.waiting = false;
            camera->queueRequest(ctx.request.get());
        }
    }
    RCLCPP_DEBUG(logger, "Execute end: waiting = %ld", waiting_requests.load());
}

void RequestHandler::add_to_wait_set(rcl_wait_set_t &wait_set) {
    gc->add_to_wait_set(wait_set);
}

bool RequestHandler::is_ready(const rcl_wait_set_t &wait_set) {
    (void)wait_set;
    RCLCPP_DEBUG(logger, "is_ready: waiting = %ld", waiting_requests.load());
    return waiting_requests > 0;
}

std::shared_ptr<void> RequestHandler::take_data() { return nullptr; }

std::shared_ptr<void> RequestHandler::take_data_by_entity_id(size_t id) {
    (void)id;
    return nullptr;
}

size_t RequestHandler::get_number_of_ready_guard_conditions() { return 1; }

void RequestHandler::set_on_ready_callback(std::function<void(size_t, int)> callback) {
    (void)callback;
}

void RequestHandler::clear_on_ready_callback() {}

std::vector<std::shared_ptr<rclcpp::TimerBase>> RequestHandler::get_timers() const { return {}; }

} // namespace tuw_camera
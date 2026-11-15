#include <rclcpp/rclcpp.hpp>

#include <image_transport/image_transport.hpp>
#include <image_transport/publisher_plugin.hpp>
#include <pluginlib/class_loader.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace tuw_camera {

using namespace sensor_msgs::msg;

class TransportNode : public rclcpp::Node {
  public:
    TransportNode(const rclcpp::NodeOptions &options)
        : Node("transport", options) {

        pluginlib::ClassLoader<image_transport::PublisherPlugin> loader(
            "image_transport", "image_transport::PublisherPlugin");
        std::vector<std::string> allowlist;

        for (auto lookup_name : loader.getDeclaredClasses()) {
            const std::string suffix = "_pub";
            const auto suffix_idx = lookup_name.size() - suffix.size();
            if (lookup_name.size() >= suffix.size() &&
                lookup_name.substr(suffix_idx) == suffix) {
                lookup_name.erase(suffix_idx);
            }

            if (lookup_name != "image_transport/raw") {
                allowlist.emplace_back(lookup_name);
            }
        }

        std::string topic = this->get_node_base_interface()->resolve_topic_or_service_name("image", false);
        std::string ns = this->get_effective_namespace();
        std::string transport_param_name = topic;
        if (transport_param_name.compare(0, ns.size(), ns) == 0) {
            transport_param_name.erase(0, ns.size());
        }
        std::replace(transport_param_name.begin(), transport_param_name.end(), '/', '.');
        if (transport_param_name.front() == '.') {
            transport_param_name.erase(0, 1);
        }
        transport_param_name.append(".enable_pub_plugins");

        declare_parameter<std::vector<std::string>>(transport_param_name, allowlist);

        auto pub = image_transport::create_publisher(
            *this, topic, rclcpp::QoS(rmw_qos_profile_default.depth));
        sub = create_subscription<Image>(
            "image", rclcpp::SensorDataQoS(),
            [pub](Image::ConstSharedPtr img) { pub.publish(img); });
    }

  private:
    rclcpp::SubscriptionBase::SharedPtr sub;
};

}; // namespace tuw_camera

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(tuw_camera::TransportNode)
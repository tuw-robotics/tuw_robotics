#ifndef TUW_GAMEPAD__GAMEPAD_HPP_
#define TUW_GAMEPAD__GAMEPAD_HPP_

#include <memory>
#include <thread>
#include <string>
#include <tuw/node.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>

namespace tuw
{
  class Joystick;
}

namespace tuw_gamepad
{
  class Gamepad : public tuw::Node
  {
  public:
    Gamepad(const std::string &node_name);

  private:
    const std::string topic_name_cmd_to_provide_{"cmd_vel"}; /// topic name to provide twist commands

    /// publisher
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr pub_cmd_vel_stamped_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_cmd_vel_;

    geometry_msgs::msg::TwistStamped current_cmd_;

    // timer for loop_rate
    rclcpp::TimerBase::SharedPtr timer_;

    // callbacks
    void on_timer();

    std::shared_ptr<tuw::Joystick> joystick_;

    double rate_;              /// dynamic parameter: Publishing rate in [hz]
    std::string frame_id_;     /// static parameter: Frame used in TwistStamped messages
    std::string device_;        /// static parameter: Joystick device default "/dev/input/js0"
    bool use_stamped_velocity_;  /// static parameter: On true it uses TwistStamped messages, otherwise Twist!
    int button_deadman_;       /// static parameter: deadmean button id
    double scale_linear_;      /// static parameter: scale in linear  [m/s]
    double scale_angular_;     /// static parameter: scale in rotation  [rad/s]
    int axis_linear_x_;        /// static parameter: axis id of x velocity
    int axis_angular_x_;       /// static parameter: axis id of x rotation
    int axis_linear_y_;        /// static parameter: axis id of y velocity
    int axis_angular_y_;       /// static parameter: axis id of y rotation
    int axis_linear_z_;        /// static parameter: axis id of z velocity
    int axis_angular_z_;       /// static parameter: axis id of z rotation

    void declare_parameters();      // declare parameters
    void read_static_parameters();  // ready the static parameters
    bool read_dynamic_parameters(); // ready the dynamic parameters and returns true on changes
  };
}
#endif // TUW_GAMEPAD__TUW_GAMEPAD_HPP_

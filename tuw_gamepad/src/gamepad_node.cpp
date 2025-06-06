#include <filesystem>

#include "tuw_gamepad/gamepad_node.hpp"
#include "tuw_gamepad/joystick.hpp"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using namespace tuw_gamepad;

Gamepad::Gamepad(const std::string & node_name)
: Node(node_name)
{
  declare_parameters();
  read_static_parameters();
  read_dynamic_parameters();

  if (use_stamped_velocity_) {
    RCLCPP_INFO(this->get_logger(), "stamped velocities");
    pub_cmd_vel_stamped_ =
      this->create_publisher<geometry_msgs::msg::TwistStamped>(topic_name_cmd_to_provide_, 10);
  } else {
    RCLCPP_INFO(this->get_logger(), "unstamped velocities");
    pub_cmd_vel_ = this->create_publisher<geometry_msgs::msg::Twist>(topic_name_cmd_to_provide_,
      10);
  }

  joystick_ = std::make_shared<tuw::Joystick>(device_);
  std::string info;
  if (joystick_->init(info) == tuw::Joystick::SUCCESS) {
    joystick_->start();
    int ms = 1.0 / rate_ * 1000.0;
    timer_ = create_wall_timer(std::chrono::milliseconds(ms), std::bind(&Gamepad::on_timer, this));
    RCLCPP_INFO(this->get_logger(), "\n%s", info.c_str());
  } else {
    RCLCPP_ERROR(this->get_logger(), "\n%s", info.c_str());
  }
}

void Gamepad::on_timer()
{
  // RCLCPP_INFO(this->get_logger(), "on_timer");
  static std::vector<bool> last_buttons;   /// used to identify changes
  static std::vector<double> last_axes;    /// used to identify changes
  bool state_changed = false;              /// true if something changed
  current_cmd_.header.frame_id = frame_id_;
  current_cmd_.header.stamp = this->get_clock()->now();
  if (read_dynamic_parameters() == true) {
    RCLCPP_INFO(this->get_logger(), "update timer");
    int ms = 1.0 / rate_ * 1000.0;
    timer_ = create_wall_timer(std::chrono::milliseconds(ms), std::bind(&Gamepad::on_timer, this));
  }
  std::vector<bool> buttons = joystick_->get_buttons_values();
  std::vector<double> axes = joystick_->get_axes_values();
  state_changed = (buttons != last_buttons) || (axes != last_axes);
  last_buttons = buttons;
  last_axes = axes;

  current_cmd_.twist.linear.x = 0.0;
  current_cmd_.twist.linear.y = 0.0;
  current_cmd_.twist.linear.z = 0.0;
  current_cmd_.twist.angular.x = 0.0;
  current_cmd_.twist.angular.y = 0.0;
  current_cmd_.twist.angular.z = 0.0;
  if (buttons[button_deadman_]) {
    if (axis_linear_x_ >= 0) {
      current_cmd_.twist.linear.x = axes[axis_linear_x_] * scale_linear_;
    }
    if (axis_linear_y_ >= 0) {
      current_cmd_.twist.linear.y = axes[axis_linear_y_] * scale_linear_;
    }
    if (axis_linear_z_ >= 0) {
      current_cmd_.twist.linear.z = axes[axis_linear_z_] * scale_linear_;
    }

    if (axis_angular_x_ >= 0) {
      current_cmd_.twist.angular.x = axes[axis_angular_x_] * scale_angular_;
    }
    if (axis_angular_y_ >= 0) {
      current_cmd_.twist.angular.y = axes[axis_angular_y_] * scale_angular_;
    }
    if (axis_angular_z_ >= 0) {
      current_cmd_.twist.angular.z = axes[axis_angular_z_] * scale_angular_;
    }
  }
  if (debug_ & state_changed) {
    RCLCPP_INFO(this->get_logger(), "\n%s", joystick_->state().c_str());
  }

  if (use_stamped_velocity_) {
    pub_cmd_vel_stamped_->publish(current_cmd_);
  } else {
    pub_cmd_vel_->publish(current_cmd_.twist);
  }

  last_buttons = buttons;
  last_axes = axes;
}

void Gamepad::declare_parameters()
{
  declare_parameters_with_description("rate", 10.0,
                                      "Publishing rate in [hz]",
                                      1.0, 10000.0, 0.1);
  declare_parameters_with_description("frame_id", "base_link",
    "Frame used in TwistStamped messages");
  declare_parameters_with_description("device", "/dev/input/js0", "Joystick device");
  declare_parameters_with_description("use_stamped_velocity", false,
    "On true it uses TwistStamped messages, otherwise Twist!");
  declare_parameters_with_description("debug", false, "On true it prints changes on the joystick");

  declare_parameters_with_description("button_deadman", 4, "button id deadman", 0, 8, 1);
  declare_parameters_with_description("lx", 4, "axis linear x  (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("ax", -1, "axis angular x (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("ly", -1, "axis linear y  (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("ay", -1, "axis angular y (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("lz", -1, "axis linear z  (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("az", 0, "axis angular z (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("scale_linear", -1.0, "scale linear", -10.0, 10.0, 0.1);
  declare_parameters_with_description("scale_angular", -1.0, "scale roation", -10.0, 10.0, 0.1);
}

bool Gamepad::read_dynamic_parameters()
{
  static bool first_call = true;                            /// varible to identify the first time the fnc was called to init all variables
  static rclcpp::Time last_call = this->get_clock()->now(); /// time on the last check
  rclcpp::Time current_call = this->get_clock()->now();     /// current time
  bool changes = false;                                     /// used to identify changes

  if (first_call || ((current_call - last_call)).seconds() > 1.0) {
    update_parameter_and_log("rate", rate_, changes, first_call);
    update_parameter_and_log("button_deadman", button_deadman_, changes, first_call);
    update_parameter_and_log("lx", axis_linear_x_, changes, first_call);
    update_parameter_and_log("ax", axis_angular_x_, changes, first_call);
    update_parameter_and_log("ly", axis_linear_y_, changes, first_call);
    update_parameter_and_log("ay", axis_angular_y_, changes, first_call);
    update_parameter_and_log("lz", axis_linear_z_, changes, first_call);
    update_parameter_and_log("az", axis_angular_z_, changes, first_call);
    update_parameter_and_log("scale_linear", scale_linear_, changes, first_call);
    update_parameter_and_log("scale_angular", scale_angular_, changes, first_call);
  }
  first_call = false;
  return changes;
}

void Gamepad::read_static_parameters()
{
  get_parameter_and_log("frame_id", frame_id_);
  get_parameter_and_log("use_stamped_velocity", use_stamped_velocity_);
  get_parameter_and_log("device", device_);
  get_parameter_and_log("debug", debug_);
}

#include <filesystem>

#include "tuw_gamepad/gamepad_node.hpp"
#include "tuw_gamepad/joystick.hpp"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using namespace tuw_gamepad;

Gamepad::Gamepad(const std::string &node_name) : Node(node_name)
{
  declare_parameters();
  read_static_parameters();
  read_dynamic_parameters();

  if (use_stamped_velocity_)
  {
    pub_cmd_vel_stamped_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(topic_name_cmd_to_provide_, 10);
  }
  else
  {
    pub_cmd_vel_ = this->create_publisher<geometry_msgs::msg::Twist>(topic_name_cmd_to_provide_, 10);
  }

  joystick_ = std::make_shared<tuw::Joystick>(device_);
  std::string info;
  if (joystick_->init(info) == tuw::Joystick::SUCCESS)
  {
    joystick_->start();
    int ms = 1.0 / rate_ * 1000.0;
    timer_ = create_wall_timer(std::chrono::milliseconds(ms), std::bind(&Gamepad::on_timer, this));
    RCLCPP_INFO(this->get_logger(), "\n%s", info.c_str());
  }
  else
  {
    RCLCPP_ERROR(this->get_logger(), "\n%s", info.c_str());
  }
}

void Gamepad::on_timer()
{
  // RCLCPP_INFO(this->get_logger(), "on_timer");
  current_cmd_.header.frame_id = "base_link";
  current_cmd_.header.stamp = this->get_clock()->now();
  RCLCPP_INFO(this->get_logger(), "\n%s", joystick_->state().c_str());
  if (read_dynamic_parameters() == true)
  {
    RCLCPP_INFO(this->get_logger(), "update timer");
    int ms = 1.0 / rate_ * 1000.0;
    timer_ = create_wall_timer(std::chrono::milliseconds(ms), std::bind(&Gamepad::on_timer, this));
  }
  std::vector<bool> buttons = joystick_->get_buttons_values();
  std::vector<double> axes = joystick_->get_axes_values();
  current_cmd_.twist.linear.x = 0.0;
  current_cmd_.twist.linear.y = 0.0;
  current_cmd_.twist.linear.z = 0.0;
  current_cmd_.twist.angular.x = 0.0;
  current_cmd_.twist.angular.y = 0.0;
  current_cmd_.twist.angular.z = 0.0;
  if ( buttons[ button_deadman_])
  {
    if (axis_vx_ >= 0)
      current_cmd_.twist.linear.x = axes[axis_vx_] * scale_v_;
    if (axis_vy_ >= 0)
      current_cmd_.twist.linear.y = axes[axis_vy_] * scale_v_;
    if (axis_vz_ >= 0)
      current_cmd_.twist.linear.z = axes[axis_vz_] * scale_v_;

    if (axis_rx_ >= 0)
      current_cmd_.twist.angular.x = axes[axis_rx_] * scale_r_;
    if (axis_ry_ >= 0)
      current_cmd_.twist.angular.y = axes[axis_ry_] * scale_r_;
    if (axis_rz_ >= 0)
      current_cmd_.twist.angular.z = axes[axis_rz_] * scale_r_;
  }

  if (use_stamped_velocity_)
    pub_cmd_vel_stamped_->publish(current_cmd_);
  else
    pub_cmd_vel_->publish(current_cmd_.twist);
}

void Gamepad::declare_parameters()
{
  declare_parameters_with_description("rate", 10.0,
                                      "Publishing rate in [hz]",
                                      1.0, 10000.0, 0.1);
  declare_parameters_with_description("frame_id", "", "Frame used in TwistStamped messages");
  declare_parameters_with_description("device", "/dev/input/js0", "Joystick device");
  declare_parameters_with_description("use_stamped_velocity", false, "On true it uses TwistStamped messages, otherwise Twist!");

  declare_parameters_with_description("button_deadman", 4, "button id deadman", 0, 8, 1);
  declare_parameters_with_description("axis_vx", 4, "axis id along x  (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("axis_rx", -1, "axis id around x (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("axis_vy", -1, "axis id along y  (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("axis_ry", -1, "axis id around y (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("axis_vz", -1, "axis id along z  (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("axis_rz", 0, "axis id around z (-1 means not used)", -1, 8, 1);
  declare_parameters_with_description("scale_v", 1.0, "scale linear", -10.0, 10.0, 0.1);
  declare_parameters_with_description("scale_r", 1.0, "scale roation", -10.0, 10.0, 0.1);
}

bool Gamepad::read_dynamic_parameters()
{
  static bool first_call = true;                            /// varible to identify the first time the fnc was called to init all variables
  static rclcpp::Time last_call = this->get_clock()->now(); /// time on the last check
  rclcpp::Time current_call = this->get_clock()->now();     /// current time
  bool changes = false;                                     /// used to identify changes

  if (first_call || ((current_call - last_call)).seconds() > 1.0)
  {
    update_parameter_and_log("rate", rate_, changes, first_call);
    update_parameter_and_log("button_deadman", button_deadman_, changes, first_call);
    update_parameter_and_log("axis_vx", axis_vx_, changes, first_call);
    update_parameter_and_log("axis_rx", axis_rx_, changes, first_call);
    update_parameter_and_log("axis_vy", axis_vy_, changes, first_call);
    update_parameter_and_log("axis_ry", axis_ry_, changes, first_call);
    update_parameter_and_log("axis_vz", axis_vz_, changes, first_call);
    update_parameter_and_log("axis_rz", axis_rz_, changes, first_call);
    update_parameter_and_log("scale_v", scale_v_, changes, first_call);
    update_parameter_and_log("scale_r", scale_r_, changes, first_call);
  }
  first_call = false;
  return changes;
}

void Gamepad::read_static_parameters()
{
  get_parameter_and_log("frame_id", frame_id_);
  get_parameter_and_log("use_stamped_velocity", use_stamped_velocity_);
  get_parameter_and_log("device", device_);
}

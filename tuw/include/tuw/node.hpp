/*
Copyright (c) 2024, Markus Bader
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of TU Wien nor the names of its contributors may be
   used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TUW__TUW_NODE_HPP_
#define TUW__TUW_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
namespace tuw
{
class Node : public rclcpp::Node
{
private:
  bool new_parameters_avaliable_{true};  /// Value to check if there are new parameters to apply

public:
  using rclcpp::Node::Node;

  /**
         * Function to check if there are new parameters to apply
         * @param true if there are paremeter changes
         */
  bool parameters_changed() {return new_parameters_avaliable_;}

  /**
         * function to signale that the new paremeters are applied
         */
  void parameters_applied() {new_parameters_avaliable_ = false;}

  /**
         * function to signale that there are new paremeters
         */
  void parameters_has_changed() {new_parameters_avaliable_ = true;}

  /**
         * function to update parameters on changes and prints a log message if the value was updated
         * @param name name of the shared parameter
         * @param value value to store the parameter content
         * @param flag_changed will be set on true of the parameter changed or the value was read the first time
         * @param force_update forces a value update on ture
         */
  void update_parameter_and_log(
    const char * name, double & value, bool & flag_changed, bool force_update)
  {
    double tmp = std::numeric_limits<double>::quiet_NaN();
    this->get_parameter<double>(name, tmp);
    if (force_update || (tmp != value)) {
      flag_changed = true;
      RCLCPP_INFO(this->get_logger(), "%s: %f", name, tmp);
    }
    value = tmp;
  }

  /**
         * function to update parameters on changes and prints a log message if the value was updated
         * @param name name of the shared parameter
         * @param value value to store the parameter content
         * @param flag_changed will be set on true of the parameter changed or the value was read the first time
         * @param force_update forces a value update on ture
         */
  void update_parameter_and_log(
    const char * name, bool & value, bool & flag_changed, bool force_update)
  {
    bool tmp = false;
    this->get_parameter<bool>(name, tmp);
    if (force_update || (tmp != value)) {
      flag_changed = true;
      RCLCPP_INFO(this->get_logger(), "%s: %s", name, (value ? "true" : "false"));
    }
    value = tmp;
  }

  /**
         * function to update parameters on changes and prints a log message if the value was updated
         * @param name name of the shared parameter
         * @param value value to store the parameter content
         * @param flag_changed will be set on true of the parameter changed or the value was read the first time
         * @param force_update forces a value update on ture
         */
  void update_parameter_and_log(
    const char * name, int & value, bool & flag_changed, bool force_update)
  {
    int tmp = 0;
    this->get_parameter<int>(name, tmp);
    if (force_update || (tmp != value)) {
      flag_changed = true;
      RCLCPP_INFO(this->get_logger(), "%s: %4d", name, tmp);
    }
    value = tmp;
  }
  /**
         * function to read a parameter and prints a log message
         * @param name name of the shared parameter
         * @param value value to store the parameter content
         */
  void get_parameter_and_log(const char * name, bool & value)
  {
    this->get_parameter<bool>(name, value);
    RCLCPP_INFO(this->get_logger(), "%s: %s", name, (value ? "true" : "false"));
  }
  /**
         * function to read a parameter and prints a log message
         * @param name name of the shared parameter
         * @param value value to store the parameter content
         */
  void get_parameter_and_log(const char * name, int & value)
  {
    this->get_parameter<int>(name, value);
    RCLCPP_INFO(this->get_logger(), "%s: %4d", name, value);
  }
  /**
         * function to read a parameter and prints a log message
         * @param name name of the shared parameter
         * @param value value to store the parameter content
         */
  void get_parameter_and_log(const char * name, double & value)
  {
    this->get_parameter<double>(name, value);
    RCLCPP_INFO(this->get_logger(), "%s: %f", name, value);
  }
  /**
         * function to read a parameter and prints a log message
         * @param name name of the shared parameter
         * @param value value to store the parameter content
         */
  void get_parameter_and_log(const char * name, std::string & value)
  {
    this->get_parameter<std::string>(name, value);
    RCLCPP_INFO(this->get_logger(), "%s: %s", name, value.c_str());
  }

  /**
         * declares parameters with description and defines a default value
         * @param name name of the shared parameter
         * @param default_value default_value
         * @param description description
         */
  void declare_parameters_with_description(
    const char * name, const char * default_value, const char * description)
  {
    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};
    descriptor.description = description;
    this->declare_parameter<std::string>(name, default_value, descriptor);
  }

  /**
         * declares parameters with description and defines a default value
         * @param name name of the shared parameter
         * @param default_value default_value
         * @param description description
         */
  void declare_parameters_with_description(
    const char * name, bool default_value, const char * description)
  {
    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};
    descriptor.description = description;
    this->declare_parameter<bool>(name, default_value, descriptor);
  }
  /**
         * declares parameters with description and defines a default value
         * @param name name of the shared parameter
         * @param default_value default_value
         * @param description description
         */
  void declare_parameters_with_description(
    const char * name, double default_value, const char * description)
  {
    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};
    descriptor.description = description;
    this->declare_parameter<double>(name, default_value, descriptor);
  }
  /**
         * declares parameters with description and defines a default value
         * @param name name of the shared parameter
         * @param default_value default_value
         * @param description description
         * @param min min value
         * @param max max value
         * @param step_size step size
         */
  void declare_parameters_with_description(
    const char * name, double default_value, const char * description, double min, double max,
    double step_size = 0.01)
  {
    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};
    rcl_interfaces::msg::FloatingPointRange range;
    range.set__from_value(min).set__to_value(max).set__step(step_size);
    descriptor.floating_point_range = {range};
    descriptor.description = description;
    this->declare_parameter<double>(name, default_value, descriptor);
  }
  /**
         * declares parameters with description and defines a default value
         * @param name name of the shared parameter
         * @param default_value default_value
         * @param description description
         */
  void declare_parameters_with_description(
    const char * name, int default_value, const char * description)
  {
    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};
    descriptor.description = description;
    this->declare_parameter<int>(name, default_value, descriptor);
  }
  /**
         * declares parameters with description and defines a default value
         * @param name name of the shared parameter
         * @param default_value default_value
         * @param description description
         * @param min min value
         * @param max max value
         * @param step_size step size
         */
  void declare_parameters_with_description(
    const char * name, int default_value, const char * description, int min, int max,
    int step_size = 1)
  {
    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};
    rcl_interfaces::msg::IntegerRange range;
    range.set__from_value(min).set__to_value(max).set__step(step_size);
    descriptor.integer_range = {range};
    descriptor.description = description;
    this->declare_parameter<int>(name, default_value, descriptor);
  }
};
}  // namespace tuw
#endif  // TUW__TUW_NODE_HPP_

#include <json/json.h>

#include <tuw_json/json.hpp>
#include <tuw_std_msgs/parameter_array_json.hpp>

#include "gtest/gtest.h"

TEST(json, parameter)
{
  std::string filename = "/tmp/parameter.json";

  std::string name = "length";
  double value = 22.9;
  tuw_std_msgs::Parameter src(name, value);
  tuw_json::write(filename, "parameter", tuw_json::toJson(src));

  tuw_std_msgs::Parameter des;
  tuw_json::fromJson(tuw_json::read(filename, "parameter"), des);
  ASSERT_EQ(des, src);
  double tolerance = 0.001;
  ASSERT_NEAR(des.get<double>(), value, tolerance);
}

TEST(json, parameter_array)
{
  std::string filename = "/tmp/parameter_array.json";
  std::vector<std::string> names = {"length", "width"};
  std::vector<double> values = {22.9, 44.3};
  tuw_std_msgs::ParameterArray src(names, values);
  tuw_json::write(filename, "parameter_array", tuw_json::toJson(src));

  tuw_std_msgs::ParameterArray des;
  tuw_json::fromJson(tuw_json::read(filename, "parameter_array"), des);
  ASSERT_EQ(des, src);
}

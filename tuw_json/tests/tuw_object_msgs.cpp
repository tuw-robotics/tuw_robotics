#include <json/json.h>

#include <tuw_geometry_msgs/pose.hpp>
#include <tuw_geometry_msgs/pose_json.hpp>
#include <tuw_json/json.hpp>
#include <tuw_object_msgs/shape_json.hpp>
#include <tuw_std_msgs/parameter_array_json.hpp>

#include "gtest/gtest.h"

TEST(json, tuw_shapeshape)
{
  std::string filename = "/tmp/shape.json";
  tuw_geometry_msgs::Pose pose(10.0, 20.0, 30.0);
  tuw_object_msgs::Shape shape(2, 3);
  shape.poses.push_back(pose);
  shape.params =
    tuw_std_msgs::ParameterArray({"length", "width"}, std::vector<double>({22.9, 44.3}));
  tuw_json::write(filename, "shape", tuw_json::toJson(shape));
}

TEST(json, tuw_object_msgs)
{
  /*
  using namespace tuw_std_msgs;
  using namespace tuw_object_msgs;
  using namespace geographic_msgs;
  std::string filename = "/tmp/tuw_object_msgs.json";
  ObjectPoint opoint0(GeoPoint(563938.78, 5183454.78, 296));
  opoint0.parameters.push_back(Parameter("enflation_radius", 0.3));
  opoint0.parameters.push_back(Parameter("bondary_radius", 2.0));
  ObjectPoint opoint1(GeoPoint(563923.96, 5183458.61, 299));
  opoint1.parameters.push_back(Parameter("enflation_radius", 0.3));
  opoint1.parameters.push_back(Parameter("bondary_radius", 2.0));
  tuw_object_msgs::Object obj0(0, tuw_object_msgs::msg::Object::TYPE_PLANT_WINE_ROW);
  obj0.points = {opoint0, opoint1};
  ObjectPoint opoint2(GeoPoint(563887.05, 5183464.3, 305));
  opoint2.parameters.push_back(Parameter("enflation_radius", 0.3));
  opoint2.parameters.push_back(Parameter("bondary_radius", 2.0));
  ObjectPoint opoint3(GeoPoint(563887.32, 5183467.21, 305));
  opoint3.parameters.push_back(Parameter("enflation_radius", 0.3));
  opoint3.parameters.push_back(Parameter("bondary_radius", 2.0));

  tuw_object_msgs::Object obj1(1, tuw_object_msgs::msg::Object::TYPE_PLANT_WINE_ROW);
  obj1.points = {opoint2, opoint3};
  obj1.parameters.push_back(Parameter("ending", 4.2));

  tuw_object_msgs::ObjectMap src;
  src.header.frame_id = "map";
  src.objects = {obj0, obj1};
  tuw_json::write(filename, "objects", tuw_json::toJson(src));

  tuw_object_msgs::ObjectMap des;
  tuw_json::fromJson(tuw_json::read(filename, "objects"), des);
  ASSERT_EQ(des, src);
  */
}

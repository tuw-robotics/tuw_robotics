#include <tuw_graph_msgs/graph.hpp>
#include <tuw_graph_msgs/graph_json.hpp>
#include <tuw_json/json.hpp>
#include <tuw_object_map_msgs/objects_json.hpp>

#include "gtest/gtest.h"

TEST(json, tuw_geometry_msgs)
{
  std::string filename = "/tmp/tuw_json.json";
  tuw_geometry_msgs::Pose pose0(0, 0, 0);
  tuw_geometry_msgs::Pose pose1(1, 0, 0);
  tuw_geometry_msgs::Pose pose2(2, 0, 0);
  tuw_geometry_msgs::Pose pose3(2, 1, 0);
  tuw_geometry_msgs::Pose pose4(3, 1, 0);
  tuw_geometry_msgs::Pose pose5(5, 5, 0);
  tuw_graph_msgs::Node node0(0, pose0);
  tuw_graph_msgs::Node node1(1, pose4);
  tuw_graph_msgs::Node node2(2, pose5);
  tuw_graph_msgs::Edge edge0(1, true, 0.2, 0, 1);
  edge0.path = {pose1, pose2, pose3};
  edge0.flags.push_back(tuw_graph_msgs::msg::Edge::FLAG_OPEN);
  tuw_graph_msgs::Edge edge1(2, false, 0.3, 1, 2);
  tuw_graph_msgs::Graph graph0("r0_map", tuw_geometry_msgs::Pose(-1, -2, 0));
  graph0.edges = {edge0, edge1};
  graph0.nodes = {node0, node1, node2};
  tuw_json::write(filename, "graph", tuw_json::toJson(graph0));

  tuw_graph_msgs::Graph graph1;
  tuw_json::fromJson(tuw_json::read(filename, "graph"), graph1);
  ASSERT_EQ(graph0, graph1);
  ASSERT_EQ(graph1.nodes[0].id, node0.id);
  ASSERT_EQ(graph1.nodes[1].id, node1.id);
  ASSERT_NE(graph1.nodes[1].id, node0.id);
  ASSERT_EQ(graph1.edges[0], edge0);
  ASSERT_EQ(graph1.edges[1], edge1);
  ASSERT_NE(graph1.edges[1], edge0);
}
TEST(json, tuw_object_map_msgs)
{
  std::string filename = "/tmp/tuw_object_map_msgs.json";
  tuw_geometry_msgs::Point pose0(563938.78, 5183454.78, 296);
  tuw_geometry_msgs::Point pose1(563923.96, 5183458.61, 299);
  tuw_object_map_msgs::Object obj0(0, tuw_object_map_msgs::msg::Object::TYPE_PLANT_WINE_ROW);
  obj0.map_points = {pose0, pose1};
  obj0.enflation_radius = {0.3, 0.3};
  obj0.bondary_radius = {2.0, 2.0};
  tuw_geometry_msgs::Point pose2(563887.05, 5183464.3, 305);
  tuw_geometry_msgs::Point pose3(563887.32, 5183467.21, 305);
  tuw_object_map_msgs::Object obj1(1, tuw_object_map_msgs::msg::Object::TYPE_PLANT_WINE_ROW);
  obj1.map_points = {pose2, pose3};
  obj1.enflation_radius = {0.3, 0.3};
  obj1.bondary_radius = {2.0, 2.0};

  tuw_object_map_msgs::Objects map;
  map.header.frame_id = "map";
  map.objects = {obj0, obj1};
  tuw_json::write(filename, "objects", tuw_json::toJson(map));
}

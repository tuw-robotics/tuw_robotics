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

#include "tuw_graph/graph_display.hpp"

#include <OgreSceneNode.h>

#include <memory>

#include "rviz_common/display_context.hpp"
#include "rviz_common/frame_manager_iface.hpp"
#include "rviz_common/interaction/selection_manager.hpp"
#include "rviz_common/logging.hpp"
#include "rviz_common/properties/color_property.hpp"
#include "rviz_common/properties/enum_property.hpp"
#include "rviz_common/properties/float_property.hpp"
#include "rviz_common/properties/quaternion_property.hpp"
#include "rviz_common/properties/vector_property.hpp"
#include "rviz_common/validate_floats.hpp"
#include "rviz_rendering/objects/arrow.hpp"
#include "rviz_rendering/objects/axes.hpp"
#include "rviz_rendering/objects/billboard_line.hpp"
#include "rviz_rendering/objects/line.hpp"
#include "rviz_rendering/objects/movable_text.hpp"
#include "rviz_rendering/objects/shape.hpp"
#include "tuw_graph/graph_display_selection_handler.hpp"

namespace tuw_graph
{
namespace displays
{
GraphDisplay::GraphDisplay()
: origin_axes_(nullptr), pose_valid_(false)
{
  origin_axes_length_property_ = new rviz_common::properties::FloatProperty(
    "Axes Length", 1, "Length of each axis, in meters.", this, SLOT(updateAxisGeometry()));

  origin_axes_radius_property_ = new rviz_common::properties::FloatProperty(
    "Axes Radius", 0.1f, "Radius of each axis, in meters.", this, SLOT(updateAxisGeometry()));

  path_show_property_ = new rviz_common::properties::BoolProperty(
    "Show Path", true, "Shows path between nodes.", this, SLOT(updatePathsVisibility()));
  path_alpha_property_ = new rviz_common::properties::FloatProperty(
    "Path alpha", 1, "Amount of transparency to apply to the path.", this,
    SLOT(updatePathsVisibility()));
  path_alpha_property_->setMin(0);
  path_alpha_property_->setMax(1);
  path_color_property_ = new rviz_common::properties::ColorProperty(
    "Path color", QColor(0, 255, 0), "Color to draw path", this, SLOT(updatePathsColor()));

  edge_arrow_size_property_ = new rviz_common::properties::FloatProperty(
    "Edge Arrow Size", 0.1f, "Size of the the edge arrow. 0 means off.", this,
    SLOT(updateEdgesGeometry()));
  edge_arrow_size_property_->setMin(0);
  edge_arrow_size_property_->setMax(1);
  edge_show_property_ = new rviz_common::properties::BoolProperty(
    "Draw Edge", true, "Draws edges between nodes.", this, SLOT(updateEdgesVisibility()));
  edge_alpha_property_ = new rviz_common::properties::FloatProperty(
    "Edges alpha", 1, "Amount of transparency to apply to the edges.", this,
    SLOT(updateEdgesColor()));
  edge_overlay_offset_property_ = new rviz_common::properties::FloatProperty(
    "Edges overlay offset", 0.1, "Overlay offest for bidirectional edges", this,
    SLOT(updateEdgesGeometry()));
  edge_alpha_property_->setMin(0);
  edge_alpha_property_->setMax(1);
  edge_color_property_ = new rviz_common::properties::ColorProperty(
    "Edges color", QColor(0, 255, 255), "Color to draw edges", this, SLOT(updateEdgesColor()));

  node_alpha_property_ = new rviz_common::properties::FloatProperty(
    "Nodes alpha", 0.5, "Amount of transparency to apply to the nodes.", this,
    SLOT(updateNodesVisibility()));
  node_show_property_ = new rviz_common::properties::BoolProperty(
    "Draw Node", true, "Draws nodes.", this, SLOT(updateNodesVisibility()));
  node_alpha_property_->setMin(0);
  node_alpha_property_->setMax(1);
  node_color_property_ = new rviz_common::properties::ColorProperty(
    "Nodes color", QColor(255, 0, 255), "Color to draw nodes", this, SLOT(updateNodesColor()));
  node_size_property_ = new rviz_common::properties::FloatProperty(
    "Nodes size", 0.15f, "Node size.", this, SLOT(updateNodesGeometry()));
  node_id_color_property_ = new rviz_common::properties::ColorProperty(
    "Nodes ID color", QColor(128, 0, 255), "Color to draw nodes IDs", this,
    SLOT(updateNodesIdColor()));
  node_id_show_property_ = new rviz_common::properties::BoolProperty(
    "Draw Node ID", true, "Draws nodes IDs", this, SLOT(updateNodesIdVisibility()));
}

void GraphDisplay::onInitialize()
{
  MFDClass::onInitialize();
  origin_axes_ = std::make_unique<rviz_rendering::Axes>(
    scene_manager_, scene_node_, origin_axes_length_property_->getFloat(),
    origin_axes_radius_property_->getFloat());
  updateAxesVisibility();
  updateAxisGeometry();
}

GraphDisplay::~GraphDisplay() = default;

void GraphDisplay::onEnable()
{
  MFDClass::onEnable();
  updateAxisGeometry();
  updateAxesVisibility();
  setupSelectionHandler();
}

void GraphDisplay::setupSelectionHandler()
{
  coll_handler_ =
    rviz_common::interaction::createSelectionHandler<GraphDisplaySelectionHandler>(this, context_);
  coll_handler_->addTrackedObjects(origin_axes_->getSceneNode());
}

void GraphDisplay::onDisable()
{
  MFDClass::onDisable();
  coll_handler_.reset();
}

void GraphDisplay::updateAxesVisibility()
{
  if (pose_valid_) {
    origin_axes_->getSceneNode()->setVisible(true);
  }
}
void GraphDisplay::updateAxisGeometry()
{
  origin_axes_->set(
    origin_axes_length_property_->getFloat(), origin_axes_radius_property_->getFloat());
  context_->queueRender();
}

void GraphDisplay::updateNodesGeometry()
{
  float size = node_size_property_->getFloat();
  for (auto & [id, node] : nodes_) {
    node.shape->setScale(Ogre::Vector3(size, size, size));
  }
  context_->queueRender();
}

void GraphDisplay::updateNodesColor()
{
  Ogre::ColourValue color_node = node_color_property_->getOgreColor();
  color_node.a = (node_show_property_->getBool() ? node_alpha_property_->getFloat() : 0.);
  Ogre::ColourValue color_node_id = node_id_color_property_->getOgreColor();
  for (auto & [id, node] : nodes_) {
    node.shape->setColor(color_node);
    node.text->setColor(color_node_id);
  }
}

void GraphDisplay::updateNodesVisibility() {updateNodesColor();}

void GraphDisplay::updateNodesIdColor()
{
  Ogre::ColourValue color_node_id = node_id_color_property_->getOgreColor();
  for (auto & [id, node] : nodes_) {
    node.text->setColor(color_node_id);
  }
}

void GraphDisplay::updateNodesIdVisibility()
{
  bool visible = node_id_show_property_->getBool();
  for (auto & [id, node] : nodes_) {
    node.scene_node->setVisible(visible);
  }
}

void GraphDisplay::updateEdgesGeometry()
{
  float edge_overlay_offset = edge_overlay_offset_property_->getFloat();
  for (auto & [id, edge] : edges_) {
    Ogre::Vector3 diff = edge.end - edge.start;
    Ogre::Vector3 midpoint = edge.start + diff / 2.;
    Ogre::Vector3 unit = diff / diff.length();
    Ogre::Vector3 normal = Ogre::Vector3(-unit.y, unit.x, 0);
    normal.normalise();
    edge.midpoint_offset = midpoint + normal * edge_overlay_offset * diff.length();
    edge.line0->setPoints(edge.start, edge.midpoint_offset);
    edge.line1->setPoints(edge.midpoint_offset, edge.end);
  }
  float edge_arrow_property = edge_arrow_size_property_->getFloat();
  for (auto & [id, edge] : edges_) {
    if (edge_arrow_property > 0.) {
      Ogre::Vector3 diff = edge.end - edge.midpoint_offset;
      Ogre::Vector3 unit = diff / diff.length();
      // Arrow shaft
      Ogre::Vector3 shaft = unit * edge_arrow_property;  // times 2 because its half the edge
      double a = 5. * M_PI / 180.;
      Ogre::Vector3 startL = edge.end - Ogre::Vector3(
        shaft.x * cos(+a) + shaft.y * -sin(+a),
        shaft.x * sin(+a) + shaft.y * cos(+a), shaft.z);
      Ogre::Vector3 startR = edge.end - Ogre::Vector3(
        shaft.x * cos(-a) + shaft.y * -sin(-a),
        shaft.x * sin(-a) + shaft.y * cos(-a), shaft.z);
      edge.arrowL->setPoints(startL, edge.end);
      edge.arrowR->setPoints(startR, edge.end);
    } else {
      edge.arrowL->setPoints(edge.end, edge.end);
      edge.arrowR->setPoints(edge.end, edge.end);
    }
  }
}

void GraphDisplay::updateEdgesColor()
{
  Ogre::ColourValue color_edge = edge_color_property_->getOgreColor();
  color_edge.a = edge_alpha_property_->getFloat();
  Ogre::ColourValue color_path = path_color_property_->getOgreColor();
  color_path.a = path_alpha_property_->getFloat();
  for (auto & [id, edge] : edges_) {
    edge.line0->setColor(color_edge);
    edge.line1->setColor(color_edge);
    edge.arrowL->setColor(color_edge);
    edge.arrowR->setColor(color_edge);
    for (auto & line : edge.path) {
      line->setColor(color_path);
    }
  }
  context_->queueRender();
}

void GraphDisplay::updateEdgesVisibility()
{
  bool draw_edge_lines = edge_show_property_->getBool();
  bool draw_edge_arrows = (draw_edge_lines && (edge_arrow_size_property_->getFloat() > 0.));
  bool draw_path_lines = path_show_property_->getBool();
  for (auto & [id, edge] : edges_) {
    edge.line0->setVisible(draw_edge_lines);
    edge.line1->setVisible(draw_edge_lines);
    edge.arrowL->setVisible(draw_edge_arrows);
    edge.arrowR->setVisible(draw_edge_arrows);
    for (auto & line : edge.path) {
      line->setVisible(draw_path_lines);
    }
  }
}

void GraphDisplay::updatePathsGeometry()
{
  /// nothing do do for the path
}

void GraphDisplay::updatePathsColor()
{
  Ogre::ColourValue color_path = path_color_property_->getOgreColor();
  color_path.a = path_alpha_property_->getFloat();
  for (auto & [id, edge] : edges_) {
    for (auto & line : edge.path) {
      line->setColor(color_path);
    }
  }
  context_->queueRender();
}

void GraphDisplay::updatePathsVisibility()
{
  bool draw_path_lines = path_show_property_->getBool();
  for (auto & [id, edge] : edges_) {
    for (auto & line : edge.path) {
      line->setVisible(draw_path_lines);
    }
  }
}

void GraphDisplay::processMessage(tuw_graph_msgs::msg::Graph::ConstSharedPtr message)
{
  static tuw_graph_msgs::msg::Graph msg;
  /// Prevent double processing of same messages
  if (msg == *message) {
    return;
  }
  msg = *message;

  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
  if (!context_->getFrameManager()->transform(msg.header, msg.origin, position, orientation)) {
    setMissingTransformToFixedFrame(msg.header.frame_id);
    return;
  }
  setTransformOk();
  pose_valid_ = true;

  edges_.clear();
  nodes_.clear();
  for (const auto & node : msg.nodes) {
    const geometry_msgs::msg::Point & p = node.pose.position;
    NodeDisplay node_display;
    node_display.shape = std::make_unique<rviz_rendering::Shape>(
      rviz_rendering::Shape::Cube, scene_manager_, scene_node_);
    node_display.shape->setPosition(Ogre::Vector3(p.x, p.y, p.z));
    node_display.text = std::make_unique<rviz_rendering::MovableText>(
      std::to_string(node.id), "Liberation Sans", 0.1f);
    node_display.text->setTextAlignment(
      rviz_rendering::MovableText::H_CENTER, rviz_rendering::MovableText::V_BELOW);
    node_display.scene_node = scene_node_->createChildSceneNode();
    node_display.scene_node->setPosition(Ogre::Vector3(p.x, p.y, p.z));
    node_display.scene_node->attachObject(node_display.text.get());
    nodes_.insert(std::make_pair(node.id, std::move(node_display)));
  }

  for (const auto & edge : msg.edges) {
    EdgeDisplay edge_display;
    if (auto it = nodes_.find(edge.start); it == nodes_.end()) {
      continue;
    } else {
      edge_display.start = it->second.shape->getPosition();
    }
    if (auto it = nodes_.find(edge.end); it == nodes_.end()) {
      continue;
    } else {
      edge_display.end = it->second.shape->getPosition();
    }
    edge_display.line0 = std::make_unique<rviz_rendering::Line>(scene_manager_, scene_node_);
    edge_display.line1 = std::make_unique<rviz_rendering::Line>(scene_manager_, scene_node_);
    edge_display.arrowL = std::make_unique<rviz_rendering::Line>(scene_manager_, scene_node_);
    edge_display.arrowR = std::make_unique<rviz_rendering::Line>(scene_manager_, scene_node_);

    {
      /// the path starts with the start node and ends with the end node.
      /// edge.start -> ... edge.path ... -> edge.end
      Ogre::Vector3 p0 = edge_display.start, p1;
      for (const auto & pose : edge.path) {
        const geometry_msgs::msg::Point & p = pose.position;
        p1 = Ogre::Vector3(p.x, p.y, p.z);
        auto line = std::make_unique<rviz_rendering::Line>(scene_manager_, scene_node_);
        line->setPoints(p0, p1);
        edge_display.path.push_back(std::move(line));
        p0 = p1;
      }
      auto line = std::make_unique<rviz_rendering::Line>(scene_manager_, scene_node_);
      line->setPoints(p0, edge_display.end);
      edge_display.path.push_back(std::move(line));
    }

    edges_.insert(std::make_pair(edge.id, std::move(edge_display)));
  }

  updateEdgesColor();
  updateEdgesGeometry();
  updateEdgesVisibility();
  updatePathsColor();
  updatePathsGeometry();
  updatePathsVisibility();
  updateNodesColor();
  updateNodesGeometry();
  updateNodesVisibility();
  updateNodesIdColor();
  updateNodesIdVisibility();

  coll_handler_->setMessage(message);

  scene_node_->setPosition(position);
  scene_node_->setOrientation(orientation);

  coll_handler_->setMessage(message);

  context_->queueRender();
}

void GraphDisplay::reset()
{
  MFDClass::reset();
  pose_valid_ = false;
  updateAxesVisibility();
}

}  // namespace displays
}  // namespace tuw_graph

#include <pluginlib/class_list_macros.hpp>  // NOLINT
PLUGINLIB_EXPORT_CLASS(tuw_graph::displays::GraphDisplay, rviz_common::Display)

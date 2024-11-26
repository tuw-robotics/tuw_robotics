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

#include "tuw_shape_array/shape_array_display.hpp"

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
#include "rviz_rendering/objects/billboard_line.hpp"
#include "rviz_rendering/objects/point_cloud.hpp"
#include "rviz_rendering/objects/shape.hpp"
#include "tuw_geometry_msgs/pose.hpp"
#include "tuw_shape_array/shape_array_display_selection_handler.hpp"

namespace tuw_shape_array
{
namespace displays
{

ShapeArrayDisplay::ShapeArrayDisplay()
: map_frame_billboard_line_(nullptr), shape_array_valid_(false)
{
  color_property_ = new rviz_common::properties::ColorProperty(
    "Color", QColor(255, 25, 0), "Color to draw the points.", this, SLOT(updateColorAndAlpha()));

  alpha_property_ = new rviz_common::properties::FloatProperty(
    "Alpha", 1, "Amount of transparency to apply to the points.", this,
    SLOT(updateColorAndAlpha()));
  alpha_property_->setMin(0);
  alpha_property_->setMax(1);

  size_property_ = new rviz_common::properties::FloatProperty(
    "Size", 0.1f, "Size of the Poses, in meters.", this, SLOT(updatePoseSize()));
}
void ShapeArrayDisplay::onInitialize()
{
  MFDClass::onInitialize();
  updateShapeChoice();
  updateColorAndAlpha();
}

ShapeArrayDisplay::~ShapeArrayDisplay() = default;

void ShapeArrayDisplay::onEnable()
{
  MFDClass::onEnable();
  updateShapeVisibility();
  setupSelectionHandler();
}

void ShapeArrayDisplay::setupSelectionHandler()
{
  coll_handler_ =
    rviz_common::interaction::createSelectionHandler<ShapeArrayDisplaySelectionHandler>(
    this, context_);
}

void ShapeArrayDisplay::onDisable()
{
  MFDClass::onDisable();
  coll_handler_.reset();
}

void ShapeArrayDisplay::updateColorAndAlpha()
{
  Ogre::ColourValue color = color_property_->getOgreColor();
  color.a = alpha_property_->getFloat();

  //arrow_->setColor(color);

  for (size_t i = 0; i < points_.size(); i++) {
    for (size_t j = 0; j < points_[i].size(); j++) {
      points_[i][j]->setColor(color);
    }
  }
  context_->queueRender();
}

void ShapeArrayDisplay::updatePoseSize()
{
  float s = size_property_->getFloat();
  for (size_t i = 0; i < points_.size(); i++) {
    for (size_t j = 0; j < points_[i].size(); j++) {
      points_[i][j]->setScale(Ogre::Vector3(s, s, s));
    }
  }
  context_->queueRender();
}

void ShapeArrayDisplay::updateShapeChoice()
{
  updateShapeVisibility();

  context_->queueRender();
}

void ShapeArrayDisplay::updateShapeVisibility() {}

void ShapeArrayDisplay::processMessage(tuw_object_msgs::msg::ShapeArray::ConstSharedPtr message)
{
  for (const auto & shape : message->shapes) {
    if (!rviz_common::validateFloats(shape.poses)) {
      setStatus(
        rviz_common::properties::StatusProperty::Error, "Topic",
        "Message contained invalid floating point values (nans or infs)");
      return;
    }
  }

  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
  tuw_geometry_msgs::Pose origin;
  if (!context_->getFrameManager()->transform(message->header, origin, position, orientation)) {
    setMissingTransformToFixedFrame(message->header.frame_id);
    return;
  }
  setTransformOk();
  Ogre::ColourValue color = color_property_->getOgreColor();
  color.a = alpha_property_->getFloat();
  float s = size_property_->getFloat();

  points_.resize(message->shapes.size());
  line_strips_.resize(message->shapes.size());
  for (size_t i = 0; i < message->shapes.size(); i++) {
    const auto & shape = message->shapes[i];
    for (size_t j = 0; j < shape.poses.size(); j++) {
      const auto p = shape.poses[j].position;
      if (j >= points_[i].size()) {
        points_[i].push_back(
          std::make_unique<rviz_rendering::Shape>(
            rviz_rendering::Shape::Cube, scene_manager_, scene_node_));
      }
      points_[i][j]->setPosition(Ogre::Vector3(p.x, p.y, p.z));
      points_[i][j]->setColor(color);
      points_[i][j]->setScale(Ogre::Vector3(s, s, s));
    }
    if ((shape.type == tuw_object_msgs::msg::Shape::TYPE_MAP) && (shape.poses.size() == 2)) {
      if (!map_frame_billboard_line_) {
        map_frame_billboard_line_ =
          std::make_unique<rviz_rendering::BillboardLine>(scene_manager_, scene_node_);
      }
      map_frame_billboard_line_->clear();
      const auto & p0 = message->shapes[i].poses[0].position;
      const auto & p1 = message->shapes[i].poses[1].position;
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p0.x, p0.y, p0.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p1.x, p0.y, p0.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p1.x, p1.y, p0.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p0.x, p1.y, p0.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p0.x, p0.y, p0.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p0.x, p0.y, p1.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p1.x, p0.y, p1.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p1.x, p1.y, p1.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p0.x, p1.y, p1.z));
      map_frame_billboard_line_->addPoint(Ogre::Vector3(p0.x, p0.y, p1.z));
    }
    if (
      (shape.shape == tuw_object_msgs::msg::Shape::SHAPE_LINE_STRIP) && (shape.poses.size() >= 2))
    {
      if (!line_strips_[i]) {
        line_strips_[i] =
          std::make_unique<rviz_rendering::BillboardLine>(scene_manager_, scene_node_);
      }
      line_strips_[i]->clear();
      for (size_t j = 0; j < shape.poses.size(); j++) {
        const auto p = shape.poses[j].position;
        line_strips_[i]->addPoint(Ogre::Vector3(p.x, p.y, p.z));
      }
    }
  }

  shape_array_valid_ = true;
  updateShapeVisibility();

  scene_node_->setPosition(position);
  scene_node_->setOrientation(orientation);

  coll_handler_->setMessage(message);

  context_->queueRender();
}

void ShapeArrayDisplay::reset()
{
  MFDClass::reset();
  shape_array_valid_ = false;
  updateShapeVisibility();
}

}  // namespace displays
}  // namespace tuw_shape_array

#include <pluginlib/class_list_macros.hpp>  // NOLINT
PLUGINLIB_EXPORT_CLASS(tuw_shape_array::displays::ShapeArrayDisplay, rviz_common::Display)

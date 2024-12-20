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

#include "tuw_shape_array/shape_array_display_selection_handler.hpp"

#include <OgreEntity.h>

#include "rviz_common/display_context.hpp"
#include "rviz_common/interaction/selection_handler.hpp"
#include "rviz_common/msg_conversions.hpp"
#include "rviz_common/properties/enum_property.hpp"
#include "rviz_common/properties/quaternion_property.hpp"
#include "rviz_common/properties/string_property.hpp"
#include "rviz_common/properties/vector_property.hpp"
#include "rviz_rendering/objects/arrow.hpp"
#include "rviz_rendering/objects/axes.hpp"
#include "rviz_rendering/objects/shape.hpp"
#include "tuw_shape_array/shape_array_display.hpp"

namespace tuw_shape_array
{
namespace displays
{

ShapeArrayDisplaySelectionHandler::ShapeArrayDisplaySelectionHandler(
  ShapeArrayDisplay * display, rviz_common::DisplayContext * context)
: SelectionHandler(context),
  display_(display),
  frame_property_(nullptr),
  position_property_(nullptr),
  orientation_property_(nullptr)
{
}

void ShapeArrayDisplaySelectionHandler::createProperties(
  const rviz_common::interaction::Picked & obj, rviz_common::properties::Property * parent_property)
{
  (void)obj;
  rviz_common::properties::Property * cat = new rviz_common::properties::Property(
    "Pose " + display_->getName(), QVariant(), "", parent_property);
  properties_.push_back(cat);

  frame_property_ = new rviz_common::properties::StringProperty("Frame", "", "", cat);
  frame_property_->setReadOnly(true);

  position_property_ =
    new rviz_common::properties::VectorProperty("Position", Ogre::Vector3::ZERO, "", cat);
  position_property_->setReadOnly(true);

  orientation_property_ = new rviz_common::properties::QuaternionProperty(
    "Orientation", Ogre::Quaternion::IDENTITY, "", cat);
  orientation_property_->setReadOnly(true);
}

rviz_common::interaction::V_AABB ShapeArrayDisplaySelectionHandler::getAABBs(
  const rviz_common::interaction::Picked & obj)
{
  (void)obj;
  rviz_common::interaction::V_AABB aabbs;
  if (display_->shape_array_valid_) {
    /** with 'derive_world_bounding_box' set to 'true', the WorldBoundingBox is derived each time.
        setting it to 'false' results in the wire box not properly following the pose arrow, but it
        would be less computationally expensive.
     */
    void();
  }
  return aabbs;
}

void ShapeArrayDisplaySelectionHandler::setMessage(tuw_object_msgs::msg::ShapeArray::ConstSharedPtr)
{
  // properties_.size() should only be > 0 after createProperties()
  // and before destroyProperties(), during which frame_property_,
  // position_property_, and orientation_property_ should be valid
  // pointers.
  /*
  if (properties_.size() > 0) {
    frame_property_->setStdString(message->header.frame_id);
    position_property_->setVector(rviz_common::pointMsgToOgre(message->pose.position));
    orientation_property_->setQuaternion(
      rviz_common::quaternionMsgToOgre(message->pose.orientation));
  }
  */
  void();
}

}  // namespace displays
}  // namespace tuw_shape_array

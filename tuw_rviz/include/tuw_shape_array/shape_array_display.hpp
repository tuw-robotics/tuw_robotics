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

#ifndef TUW_SHAPE_ARRAY__SHAPE_ARRAY_DISPLAY_HPP_
#define TUW_SHAPE_ARRAY__SHAPE_ARRAY_DISPLAY_HPP_

#include <memory>

#include "rviz_common/interaction/forwards.hpp"
#include "rviz_common/message_filter_display.hpp"
#include "tuw_object_msgs/shape_array.hpp"
#include "tuw_rviz/visibility_control.hpp"

namespace rviz_rendering
{
class Shape;
class BillboardLine;
}  // namespace rviz_rendering

namespace rviz_common
{
namespace properties
{
class ColorProperty;
class EnumProperty;
class FloatProperty;
}  // namespace properties
}  // namespace rviz_common

namespace tuw_shape_array
{
namespace displays
{

class ShapeArrayDisplaySelectionHandler;

typedef std::shared_ptr<ShapeArrayDisplaySelectionHandler> ShapeArrayDisplaySelectionHandlerPtr;

/** @brief Accumulates and displays the pose from a tuw_object_msgs::ShapeArray message. */
class TUW_RVIZ_PUBLIC ShapeArrayDisplay
  : public rviz_common::MessageFilterDisplay<tuw_object_msgs::msg::ShapeArray>
{
  Q_OBJECT

public:
  enum Shape
  {
    Arrow,
    Axes,
  };

  ShapeArrayDisplay();

  ~ShapeArrayDisplay() override;
  void onInitialize() override;
  void reset() override;

protected:
  /** @brief Overridden from MessageFilterDisplay to get arrow/axes visibility correct. */
  void onEnable() override;
  void onDisable() override;
  void processMessage(tuw_object_msgs::msg::ShapeArray::ConstSharedPtr message) override;

private Q_SLOTS:
  void updateShapeVisibility();
  void updateColorAndAlpha();
  void updateShapeChoice();
  void updatePoseSize();

private:
  void setupSelectionHandler();

  std::vector<std::vector<std::unique_ptr<rviz_rendering::Shape>>> points_;
  std::vector<std::unique_ptr<rviz_rendering::BillboardLine>> line_strips_;
  std::unique_ptr<rviz_rendering::BillboardLine> map_frame_billboard_line_;
  bool shape_array_valid_;
  ShapeArrayDisplaySelectionHandlerPtr coll_handler_;
  rviz_common::properties::ColorProperty * color_property_;
  rviz_common::properties::FloatProperty * alpha_property_;
  rviz_common::properties::FloatProperty * size_property_;

  friend class ShapeArrayDisplaySelectionHandler;
};

}  // namespace displays
}  // namespace tuw_shape_array

#endif  // TUW_SHAPE_ARRAY__SHAPE_ARRAY_DISPLAY_HPP_

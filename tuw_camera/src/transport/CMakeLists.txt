add_library(tuw_camera_transport_node SHARED
  transport_node.cpp)

target_link_libraries(tuw_camera_transport_node
  ${pluginlib_TARGETS}
  ${image_transport_TARGETS}
  tuw_camera_common)

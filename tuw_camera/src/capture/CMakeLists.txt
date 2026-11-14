add_library(tuw_camera_capture_node SHARED
  capture_node.cpp
  convert.cpp
  stream_handler.cpp
  buffer_context.cpp
  request_handler.cpp
  camera_controls_handler.cpp)

generate_parameter_library(tuw_camera_capture_node_parameters
  capture_node_parameters.yaml)

target_link_libraries(tuw_camera_capture_node
  ${camera_info_manager_TARGETS}
  ${OpenCV_LIBS}
  tuw_camera_common
  tuw_camera_capture_node_parameters
  PkgConfig::LibCamera)

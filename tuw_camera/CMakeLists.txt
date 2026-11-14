cmake_minimum_required(VERSION 3.28)
project(tuw_camera)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(OpenCV REQUIRED)
find_package(image_transport REQUIRED)
find_package(pluginlib REQUIRED)
find_package(camera_info_manager REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(generate_parameter_library REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LibCamera REQUIRED IMPORTED_TARGET libcamera)

# Common flags and dependencies
add_library(tuw_camera_common INTERFACE)
target_include_directories(tuw_camera_common INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(tuw_camera_common INTERFACE c_std_99 cxx_std_17)  # Require C99 and C++17

target_link_libraries(tuw_camera_common INTERFACE
  ${rclcpp_components_TARGETS}
  ${sensor_msgs_TARGETS})

# Include node source
add_subdirectory(src/capture)
add_subdirectory(src/transport)

# Register nodes
rclcpp_components_register_node(tuw_camera_capture_node
  PLUGIN "tuw_camera::CaptureNode"
  EXECUTABLE tuw_camera_capture)

rclcpp_components_register_node(tuw_camera_transport_node
  PLUGIN "tuw_camera::TransportNode"
  EXECUTABLE tuw_camera_transport)

# Export libraries
ament_export_targets(export_tuw_camera)
install(
  TARGETS
  tuw_camera_common
  tuw_camera_capture_node
  tuw_camera_capture_node_parameters
  tuw_camera_transport_node
  EXPORT export_tuw_camera
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

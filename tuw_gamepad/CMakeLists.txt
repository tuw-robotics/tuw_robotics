cmake_minimum_required(VERSION 3.8...3.28)
project(tuw_gamepad)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tuw REQUIRED)

add_library(joystick src/joystick.cpp)
target_include_directories(joystick PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

add_executable(gamepad_node src/gamepad_standalone.cpp src/gamepad_node.cpp)
target_include_directories(gamepad_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_compile_features(gamepad_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_link_libraries(gamepad_node
  rclcpp::rclcpp  
  ${geometry_msgs_TARGETS}
  ${tuw_TARGETS}
  joystick
)

add_executable(joystick_reader src/joystick_reader.cpp)

install(TARGETS gamepad_node joystick_reader
  DESTINATION lib/${PROJECT_NAME})

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

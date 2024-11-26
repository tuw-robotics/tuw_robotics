cmake_minimum_required(VERSION 3.8)
project(tuw_json)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(tuw_graph_msgs REQUIRED)
find_package(tuw_object_map_msgs REQUIRED)
find_package(tuw_object_msgs REQUIRED)
find_package(jsoncpp REQUIRED)

add_library(tuw_json src/json.cpp )
target_compile_features(tuw_json PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_include_directories(tuw_json PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(
  tuw_json
)
target_link_libraries(tuw_json jsoncpp_lib)


install(TARGETS ${PROJECT_NAME} 
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)



add_executable(example_json src/main_json.cpp)
target_include_directories(example_json PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(example_json PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  example_json
  "rclcpp"
)
target_link_libraries(example_json jsoncpp_lib tuw_json)

install(TARGETS example_json
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


  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(unittest 
    tests/unittest.cpp 
    tests/tuw_std_msgs.cpp
    tests/tuw_object_msgs.cpp
  )
  target_include_directories(unittest PUBLIC src/)  # Need to specify directories in order to #include from them in the test
  target_link_libraries(unittest ${PROJECT_NAME})
  ament_target_dependencies(
    unittest
    "tuw_graph_msgs"
    "tuw_object_map_msgs"
    "tuw_object_msgs"
  )
endif()


ament_export_include_directories(
  include
)
ament_export_libraries(
  ${PROJECT_NAME}
)
ament_export_targets(
  export_${PROJECT_NAME}
)

ament_export_dependencies(jsoncpp)

ament_package()
cmake_minimum_required(VERSION 3.8)
project(tuw)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)

install(
  DIRECTORY include/
  DESTINATION include
)


add_library(${PROJECT_NAME}  src/debug.cpp src/timing.cpp src/string.cpp )
target_compile_features(${PROJECT_NAME}  PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_include_directories(${PROJECT_NAME}  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(
  ${PROJECT_NAME} 
)

install(TARGETS ${PROJECT_NAME} 
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
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


find_package(ament_cmake_gtest REQUIRED)
ament_add_gtest(unittest 
  tests/unittest.cpp 
)
target_include_directories(unittest PUBLIC src/)  # Need to specify directories in order to #include from them in the test
target_link_libraries(unittest ${PROJECT_NAME})
ament_target_dependencies(
  unittest
  "std_srvs"
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

ament_package()

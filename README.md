# tuw_robotics
ROS package collection

## packages
- [tuw](./tuw/README.md): Classes and functions which are making life easier in ROS2
- [tuw_json](./tuw_json/README.md): This package allows to read and write tuw_msgs into json files
- [tuw_rviz](./tuw_rviz/README.md): RViz2 plugins for tuw_msgs 
- [tuw_gamepad](./tuw_gamepad/README.md): This package is a simple gamepad/joy node. it works perfectly with an F710 Wireless Gamepad built by Logitech.

## branches on github.com
- __ros2__: this is the stable working branch 
- __ros2-devel__: this is the devel branch and may be unstable
- __humble__: indexed in [rosdistro/humble/distribution.yaml](https://github.com/ros/rosdistro/blob/master/humble/distribution.yaml)
- __jazzy__: indexed in [rosdistro/jazzy/distribution.yaml](https://github.com/ros/rosdistro/blob/master/jazzy/distribution.yaml)
- __rolling__: indexed in [rosdistro/rolling/distribution.yaml](https://github.com/ros/rosdistro/blob/master/rolling/distribution.yaml)

### commands 
```bash
# fix format in this order!!!
ament_clang_format . --reformat
ament_uncrustify . --reformat
# update ros2 branch with ros2-devel
git checkout ros2; git merge ros2-devel; git push origin ros2; git checkout ros2-devel;
```
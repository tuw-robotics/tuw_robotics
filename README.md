# tuw_robotics
ROS2 package collection from TU Wien robotics lab. 

## packages
- [tuw](./tuw/README.md): Classes and functions which are making life easier in ROS2
- [tuw_camera](./tuw_camera/README.md): libcamera based camera driver for USB (UVC) webcams and Raspberry Pi camera modules, with format conversion (e.g. YUYV → mono8), camera_info / calibration support and image_transport republishing.
- [tuw_gamepad](./tuw_gamepad/README.md): This package is a simple gamepad/joy node. it works perfectly with an F710 Wireless Gamepad built by Logitech.
- [tuw_json](./tuw_json/README.md): This package allows to read and write tuw_msgs into json files
- [tuw_rviz](./tuw_rviz/README.md): RViz2 plugins for tuw_msgs 

## developer and AI notes
See [Developer and AI Notes](./Notes.md) for code formatting and how to release the packages with bloom.

## branches on github.com
- __ros2__: this is the stable working branch 
- __ros2-devel__: this is the devel branch and may be unstable
- __humble__: indexed in [rosdistro/humble/distribution.yaml](https://github.com/ros/rosdistro/blob/master/humble/distribution.yaml)
- __jazzy__: indexed in [rosdistro/jazzy/distribution.yaml](https://github.com/ros/rosdistro/blob/master/jazzy/distribution.yaml)
- __rolling__: indexed in [rosdistro/rolling/distribution.yaml](https://github.com/ros/rosdistro/blob/master/rolling/distribution.yaml)

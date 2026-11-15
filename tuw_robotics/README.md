# tuw_robotics

`tuw_robotics` is a metapackage. It contains no code, nodes or libraries.

It exists only so that all packages of the [tuw_robotics](https://github.com/tuw-robotics/tuw_robotics) repository can be installed with a single package. It depends (`exec_depend`) on:

* [tuw](../tuw)
* [tuw_camera](../tuw_camera)
* [tuw_gamepad](../tuw_gamepad)
* [tuw_json](../tuw_json)
* [tuw_rviz](../tuw_rviz)

## Installation

Once the packages are released for your ROS 2 distribution, install them all with:

```
sudo apt install ros-$ROS_DISTRO-tuw-robotics
```

When building from source, build the whole workspace or select this package together with its dependencies:

```
colcon build --packages-up-to tuw_robotics
```

## Notes

* This package is optional. The repository can be indexed or released in a ROS distribution without it. The repository name in `rosdistro` does not have to match any package name.
* If you add a new package to the repository, also add it as an `exec_depend` in [package.xml](package.xml).

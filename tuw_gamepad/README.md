# tuw_gamepad: 
Package containing a node to publish motion commands based on input on a gamepad without joy msgs.

## run
```
# with TwistStamped
ros2 run tuw_gamepad gamepad_node --ros-args -p use_stamped_velocity:=false -p debug:=true
# with TwistStamped
ros2 run tuw_gamepad gamepad_node --ros-args -p use_stamped_velocity:=true -p debug:=true
```
## Arguments
* __rate__: default 10.0, Publishing rate in [hz]
* __frame_id__: default base_link, Frame used in TwistStamped messages
* __device__: default /dev/input/js0, Joystick device
* __use_stamped_velocity__: default false, On true it uses **TwistStamped** messages, otherwise **Twist**!
* __debug__: default false, On true it prints changes on the joystick
* __button_deadman__: default 4, button id deadman
* __lx__: default 4, axis linear x  (-1 means not used)
* __ax__: default -1, axis angular x (-1 means not used)
* __ly__: default -1, axis linear y  (-1 means not used)
* __ay__: default -1, axis angular y (-1 means not used)
* __lz__: default -1, axis linear z  (-1 means not used)
* __az__: default 0, axis angular z (-1 means not used)
* __scale_linear__: default -1.0, scale linear
* __scale_angular__: default -1.0, scale roation

# tuw_camera

ROS 2 nodes for capturing images from a [libcamera](https://libcamera.org/)-supported
camera and republishing them through `image_transport`.

## Supported cameras

Any camera libcamera can open works. In practice that means two groups:

- **USB cameras following the UVC standard** (USB Video Class), on a PC or
  on a Pi, through libcamera's `uvcvideo` pipeline handler. This covers
  almost all webcams (laptop cameras, Logitech, Microsoft, generic USB
  webcams) and many USB board cameras. Tested: the built-in webcam of a
  Dell Precision 3591 laptop
  ([`dell_precision_3591_yuyv_to_mono8.yaml`](config/dell_precision_3591_yuyv_to_mono8.yaml)).
  - One stream per camera, in the formats the camera offers over UVC
    (typically `YUYV` and `MJPEG`, sometimes `NV12` or `GREY` → `R8`).
  - Basic UVC controls (exposure, auto-exposure, brightness, contrast, …)
    as `controls.*` parameters.
  - At 1080p, `YUYV` over USB 2.0 usually reaches only ~5 fps; `MJPEG`
    typically 30 fps.
- **Raspberry Pi camera modules** (CSI ribbon cable) on a Raspberry Pi,
  e.g. Camera Module 3 / 3 Wide (IMX708), Camera Module 2 (IMX219), HQ
  Camera (IMX477) and Global Shutter Camera (IMX296). They use the Pi's own
  libcamera pipeline (with ISP), offer several formats (`RGB888`, `YUV420`,
  `YUYV`, …) and many controls (autofocus, exposure, gain, white balance).
  Tested: Camera Module 3 Wide on a Raspberry Pi 5
  ([`pi_module3_wide_*.yaml`](config)).

**Not supported:** non-UVC USB cameras (e.g. USB3 Vision / GenICam
industrial cameras from Basler, FLIR, IDS, or cameras that need a vendor
driver), and depth/IR streams of depth cameras (RealSense, Kinect). The
RGB stream of such a camera often is UVC and works. Pixel formats missing
from the [conversion table](#supported-pixeltarget-formats) are listed at
startup but can't be published.

To check a camera:

```bash
v4l2-ctl --list-devices                      # listed as /dev/videoX -> V4L2 driver active
v4l2-ctl --list-formats-ext -d /dev/video0   # formats, resolutions, frame rates
cam -l                                       # cameras libcamera can open (libcamera-tools)
```

If a camera is listed by `cam -l`, this node can open it. With several
cameras, select one with the `camera` parameter; the node logs the
available IDs at startup (`Available cameras:`).

## Nodes

The package provides two composable nodes, meant to run together inside one
`component_container`:

- **`CaptureNode`** (executable `camera_capture`) — drives libcamera
  directly, configures one or more streams, and publishes raw or
  compressed `sensor_msgs/Image` messages plus (optionally) `camera_info`.
- **`TransportNode`** (executable `camera_transport`) — subscribes to
  the raw image published by `CaptureNode` and republishes it through all
  available `image_transport` plugins (e.g. `compressed`, `theora`), so
  downstream tools can pick whichever transport they need.

## Launching

The package has two launch files.

| Launch file | Nodes | Processes | Camera parameters |
|---|---|---|---|
| [`demo.launch.py`](launch/demo.launch.py) | `CaptureNode` (`camera_capture`), `TransportNode` (`camera_transport`) | **separate processes**, no rectification | launch arguments |
| [`composed.launch.py`](launch/composed.launch.py) | `CaptureNode`, `TransportNode`, `RectifyNode` (rectification) | all in **one process** (component container, intra-process communication) | YAML file in [`config/`](config) |

### demo.launch.py

```bash
ros2 launch tuw_camera demo.launch.py
```

Should work out of the box, and most settings can be given as launch
arguments (see [Demo launch file with format arguments](#demo-launch-file-with-format-arguments)).
It is slow, though, because the nodes run in separate processes.

### composed.launch.py

In `composed.launch.py` the images are passed between the nodes inside the
process without going through DDS, which is faster and avoids the problems
with large images described in
[Large images and Fast DDS](#large-images-and-fast-dds).

`composed.launch.py` needs a configuration file that matches your camera
(format, resolution, calibration): pick one from [`config/`](config) with
`config:=<file>` or add your own (see
[Configuration files](#configuration-files)).

```bash
ros2 launch tuw_camera composed.launch.py
```

This starts `CaptureNode`, `TransportNode` and an `image_proc` `RectifyNode`
inside one `rclcpp_components` container (`camera_processing_container`)
with intra-process communication enabled between them. The camera
parameters come from a YAML file in [`config/`](config) (see
[Configuration files](#configuration-files)).

| Argument | Default | Description |
|---|---|---|
| `config` | `pi_module3_wide_yuyv_to_mono8.yaml` | Camera configuration file in `tuw_camera/config` |
| `namespace` | `camera` | Namespace of all three nodes and their topics |
| `transport_launch` | `true` | Start `TransportNode` (compressed versions of `<ns>/image`) |
| `rectify_launch` | `true` | Start `RectifyNode` (`<ns>/image` + `<ns>/camera_info` → `<ns>/image_rect`); needs a calibration |

```bash
# Dell Precision 3591 laptop webcam, greyscale, calibrated
ros2 launch tuw_camera composed.launch.py config:=dell_precision_3591_yuyv_to_mono8.yaml

# Pi Camera Module 3 Wide in colour, different namespace
ros2 launch tuw_camera composed.launch.py config:=pi_module3_wide_bgr.yaml namespace:=front_cam

# Capture node only
ros2 launch tuw_camera composed.launch.py transport_launch:=false rectify_launch:=false
```

Run `ros2 launch tuw_camera composed.launch.py --show-args` to list the
arguments. `RectifyNode` needs `image_proc` from `image_pipeline` (built in
`ws00`).

> **Note:** the container name is fixed. Don't run two `composed.launch.py`
> at the same time: the second launch loads its nodes into the first
> container.

Resulting topics (default namespace `camera`):

| Topic | Published by |
|---|---|
| `/camera/image` | `CaptureNode` (raw only) |
| `/camera/camera_info` | `CaptureNode` |
| `/camera/image/compressed`, `/zstd`, `/theora`, … | `TransportNode` |
| `/camera/image_rect` + `/camera/image_rect/compressed`, … | `RectifyNode` (it publishes through `image_transport` itself) |

### Configuration files

| File | Camera | Stream | Calibration |
|---|---|---|---|
| [`pi_module3_wide_bgr.yaml`](config/pi_module3_wide_bgr.yaml) | Pi Camera Module 3 Wide | `RGB888` → `bgr8`, 1536x864 | `pi_module3_wide.yaml` |
| [`pi_module3_wide_yuyv.yaml`](config/pi_module3_wide_yuyv.yaml) | Pi Camera Module 3 Wide | `YUYV` → `yuyv`, 1536x864 | `pi_module3_wide.yaml` |
| [`pi_module3_wide_yuyv_to_mono8.yaml`](config/pi_module3_wide_yuyv_to_mono8.yaml) | Pi Camera Module 3 Wide | `YUYV` → `yuyv`, 1536x864 (`target_format` is not `mono8` yet) | `pi_module3_wide.yaml` |
| [`dell_precision_3591_yuyv_to_mono8.yaml`](config/dell_precision_3591_yuyv_to_mono8.yaml) | Dell Precision 3591 webcam (UVC) | `YUYV` → `mono8`, 1920x1080 | `dell_precision_3591.yaml` |

Calibration files live in [`config/camera_calibration/`](config/camera_calibration).
A calibration is only valid for the resolution it was made at, and its
`camera_name` must match `camera_info_name` in the config, otherwise
`camera_info_manager` warns about a name mismatch.

A config file looks like this (all topic names set explicitly):

```yaml
/**:
  ros__parameters:
    camera_info_name: dell_precision_3591
    camera_info_url: package://tuw_camera/config/camera_calibration/dell_precision_3591.yaml
    camera_info_topic: camera_info
    frame_id: "camera_link"
    stream_roles: ["video"]
    streams:
      video:
        topic: image
        format: YUYV
        target_format: mono8
        width: 1920
        height: 1080
```

`TransportNode` and `RectifyNode` don't read these topic names: both
subscribe to `image` in their namespace. If you rename `topic` or
`camera_info_topic`, remap these nodes accordingly. `RectifyNode` always
looks for `camera_info` next to its image topic.

### Demo launch file with format arguments

[`launch/demo.launch.py`](launch/demo.launch.py) starts `CaptureNode` and
`TransportNode` as the standalone executables `camera_capture` and
`camera_transport` (two processes, no component container, no
`RectifyNode`), sets the camera parameters directly in the launch file (no
YAML file) and exposes the stream format, resolution and camera calibration
as launch arguments:

| Argument | Default | Description |
|---|---|---|
| `format` | `YUYV` | libcamera pixel format (`streams.video.format`) |
| `target_format` | `mono8` | ROS image encoding (`streams.video.target_format`) |
| `resolution` | `""` | `<width>x<height>`, e.g. `1920x1080`. Empty uses the camera default. |
| `namespace` | `camera` | Namespace of both nodes and their topics (`/<namespace>/image`, …) |
| `transport_launch` | `true` | Start `camera_transport` (compressed versions of `/<namespace>/image`) |
| `frame_id` | `camera_link` | `frame_id` in the header of published image/camera_info messages |
| `camera_info_name` | `pi_module3_wide` | Camera name for `camera_info`; must match `camera_name` in the calibration file. Empty disables `camera_info`. |
| `camera_info_url` | `""` | Calibration file (`package://` or `file://` URL). Empty uses camera_info_manager's default `~/.ros/camera_info/<camera_info_name>.yaml`. |

The stream uses the `video` role and is published on `/camera/image`
(compressed versions on `/camera/image/compressed`, … from `TransportNode`),
with `camera_info` on `/camera/camera_info`. Since both nodes run as separate
processes, every image crosses DDS; for large images set the
[Fast DDS profile](#large-images-and-fast-dds). By default no
calibration file from the package is loaded: the node looks for
`~/.ros/camera_info/pi_module3_wide.yaml`, and publishes an uncalibrated
`camera_info` if it doesn't exist. This is also where **COMMIT** in the
calibrator saves a new calibration. To use the packaged calibration
[`config/camera_calibration/pi_module3_wide.yaml`](config/camera_calibration/pi_module3_wide.yaml)
(Camera Module 3 Wide at 1536x864, `camera_name: pi_module3_wide`), pass it
as `camera_info_url` (see the example below). If you change the resolution
you need a matching calibration file.

For a rectified image, or all nodes in one container with intra-process
communication, use [`composed.launch.py`](#launching).

```bash
# Default: YUYV -> mono8, camera default resolution
ros2 launch tuw_camera demo.launch.py

# Colour: RGB888 -> bgr8 (Pi camera modules)
ros2 launch tuw_camera demo.launch.py format:=RGB888 target_format:=bgr8

# Fixed resolution
ros2 launch tuw_camera demo.launch.py resolution:=1280x720

# Colour from a USB webcam: MJPEG decoded to bgr8
ros2 launch tuw_camera demo.launch.py format:=MJPEG target_format:=bgr8 resolution:=1920x1080

# YUYV, no conversion
ros2 launch tuw_camera demo.launch.py format:=YUYV target_format:=yuyv

# Greyscale: only the luma (Y) plane of YUV420 is published as mono8
ros2 launch tuw_camera demo.launch.py format:=YUV420 target_format:=mono8

# Different TF frame
ros2 launch tuw_camera demo.launch.py frame_id:=front_camera_link

# Greyscale (mono8) with the pi_module3_wide calibration, all set explicitly (camera_info_name must match camera_name in the file)
ros2 launch tuw_camera demo.launch.py \
  format:=YUV420 target_format:=mono8 resolution:=1536x864 \
  camera_info_url:=package://tuw_camera/config/camera_calibration/pi_module3_wide.yaml \
  camera_info_name:=pi_module3_wide
```

Run `ros2 launch tuw_camera demo.launch.py --show-args` to list the
arguments. Valid combinations are listed in
[Supported pixel/target formats](#supported-pixeltarget-formats).

> **Note:** on the Raspberry Pi 5 with a colour sensor (e.g. Camera Module 3),
> `format:=R8` does **not** give a greyscale image: libcamera treats `R8` as a
> raw mono-sensor format and falls back to a Bayer format. Use
> `format:=YUV420 target_format:=mono8` instead.

### Running `CaptureNode` standalone

For debugging it can also be run directly, passing parameters via
`--ros-args -p`. Nested parameters (`streams.<role>.*`) are addressed with
dotted names:

```bash
ros2 run tuw_camera camera_capture --ros-args \
  -r __ns:=/camera \
  -p frame_id:=camera_link \
  -p camera_info_name:=pi_module3_wide \
  -p camera_info_url:=package://tuw_camera/config/camera_calibration/pi_module3_wide.yaml \
  -p stream_roles:="[video]" \
  -p streams.video.format:=YUYV \
  -p streams.video.target_format:=yuyv \
  -p streams.video.width:=1536 \
  -p streams.video.height:=864
```

Or load one of the config files:

```bash
ros2 run tuw_camera camera_capture --ros-args \
  -r __ns:=/camera \
  --params-file $(ros2 pkg prefix tuw_camera)/share/tuw_camera/config/dell_precision_3591_yuyv_to_mono8.yaml
```

`-p` arguments placed after `--params-file` override values from the file.

## CaptureNode

### Topics published

| Topic | Type | Condition |
|---|---|---|
| `[<sub_topic>/]<topic>` (default `image`) | `sensor_msgs/msg/Image` | one per configured stream role, if the resolved format maps to a raw ROS encoding |
| `[<sub_topic>/]<topic>/compressed` | `sensor_msgs/msg/CompressedImage` | one per configured stream role, if the resolved format maps to a compressed encoding (e.g. MJPEG → `jpeg`) |
| `<camera_info_topic>` (default `camera_info`) | `sensor_msgs/msg/CameraInfo` | only if `camera_info_name` is non-empty |

### Services

| Service | Type | Condition |
|---|---|---|
| `<ns>/set_camera_info` | `sensor_msgs/srv/SetCameraInfo` | only if `camera_info_name` is non-empty |

Used by `camera_calibration` (**COMMIT** button) to store a new
calibration; it is written to `camera_info_url` (see
[Camera calibration](#camera-calibration)). The service is created in the
node namespace, e.g. `/camera/set_camera_info`.

### Startup log

After `Selected camera: …` the node logs every pixel format and resolution
the camera supports for each configured stream role, together with the
`target_format` values it can be converted to, e.g.:

```
Supported formats for role 'video':
- MJPEG: 640x360 640x480 1280x720 1280x960 1920x1080 -> target_format: jpeg, bgr8
- YUYV: 160x120 320x180 320x240 640x360 640x480 1280x720 1920x1080 -> target_format: mono8, yuyv
```

Native RGB/BGR formats show up as `RGB888` (→ `bgr8`) / `BGR888` (→ `rgb8`),
native greyscale as `R8` (→ `mono8`). USB webcams usually offer only
`YUYV` and `MJPEG`.

### Parameters

Defined via `generate_parameter_library` in
[`capture_node_parameters.yaml`](src/capture/capture_node_parameters.yaml).
All parameters below are `read_only` (set at startup, not runtime-changeable).

| Parameter | Type | Default | Description |
|---|---|---|---|
| `camera` | string | `""` | libcamera camera ID to use. Empty selects the first camera libcamera enumerates. |
| `camera_info_name` | string | `""` | Camera name used to look up/publish `camera_info`. Empty disables the `camera_info` topic entirely. |
| `camera_info_url` | string | `""` | URL of the camera calibration file. Empty uses camera_info_manager's default (`file:///<home>/.ros/camera_info/<camera_info_name>.yaml`). |
| `camera_info_topic` | string | `camera_info` | Topic for `camera_info`. Relative names are resolved in the node namespace. Must not be empty. |
| `frame_id` | string | `""` | `frame_id` set in the header of published image/camera_info messages. |
| `stream_roles` | string array | `["video"]` | Which libcamera stream roles to configure and publish. Must be non-empty, unique, and a subset of `raw`, `viewfinder`, `still`, `video`. Each entry gets a corresponding `streams.<role>` group and its own topic. |
| `streams.<role>.width` | int | `0` | Requested stream width in pixels. `0` leaves it to libcamera's default for the role. |
| `streams.<role>.height` | int | `0` | Requested stream height in pixels. `0` leaves it to libcamera's default for the role. |
| `streams.<role>.format` | string | `""` | Requested libcamera pixel format (e.g. `YUYV`, `RGB888`, `MJPEG`), parsed via `libcamera::PixelFormat::fromString`. Empty leaves it to libcamera's default for the role. |
| `streams.<role>.topic` | string | `image` | Image topic name for this stream. Relative names are resolved in the node namespace. Must not be empty. |
| `streams.<role>.sub_topic` | string | `""` | Topic prefix for this stream's image topic(s), e.g. `video` → `video/image`. Empty publishes directly on `<topic>`. |
| `streams.<role>.target_format` | string | `""` | Which ROS encoding to convert the captured pixel format into (see [Supported pixel/target formats](#supported-pixeltarget-formats) below). Empty picks the first match for the pixel format. |

`<role>` is one of the entries listed in `stream_roles` (e.g. `streams.video.width`).

If the requested stream configuration is invalid or can't be matched to a
known ROS encoding, the node logs the pixel formats libcamera actually
supports for that role, and/or the ROS target formats available for the
requested pixel format, before throwing at startup.

> **Tip:** to find out which pixel formats and resolutions your camera
> actually supports before setting `streams.<role>.format`/`width`/`height`,
> check the node's [startup log](#startup-log), or use
> `rpicam-hello --list-cameras` (see
> [`docs/raspberry_pi.md`](../../../../docs/raspberry_pi.md)) or
> `v4l2-ctl --list-formats-ext -d <device>`. `v4l2-ctl` also shows the frame
> rate per resolution (e.g. the Dell webcam: YUYV 1920x1080 only at 5 fps,
> MJPEG 1920x1080 at 30 fps).

#### Camera controls (`controls.*`)

In addition to the static parameters above, `CaptureNode` inspects the
camera's supported libcamera controls at startup (e.g. `AeEnable`,
`ExposureTime`, `AnalogueGain`, `Brightness`, `Contrast`, `Saturation`,
`AwbMode`, ...) and declares one ROS parameter per control, named
`controls.<ControlName>`, seeded with that control's libcamera default.
These *are* settable at runtime via `ros2 param set` and take effect on the
next capture request. The exact set of available controls depends on the
camera/sensor; run the node and check its startup log
(`Camera supported control: ...`) or `ros2 param list` for the definitive
list on your hardware.

Array-valued controls (e.g. `AfWindows`, `FrameDurationLimits`) are not
currently supported and are skipped (logged as a warning) rather than
exposed as parameters.

### Supported pixel/target formats

The mapping between libcamera pixel formats and ROS image encodings is
defined in [`convert.cpp`](src/capture/convert.cpp):

| libcamera `streams.<role>.format` | ROS `streams.<role>.target_format` | Message type |
|---|---|---|
| `R8` | `mono8` (mono sensors only; on a colour sensor use `YUV420` below) | `Image` |
| `R16` | `mono16` | `Image` |
| `RGB888` | `bgr8` | `Image` |
| `BGR888` | `rgb8` | `Image` |
| `ABGR8888` | `rgba8` | `Image` |
| `ARGB8888` | `bgra16` | `Image` |
| `RGB161616` | `bgr16` | `Image` |
| `BGR161616` | `rgb16` | `Image` |
| `MJPEG` | `bgr8` (decoded) | `Image` |
| `MJPEG` | `jpeg` (passthrough) | `CompressedImage` |
| `UYVY` | `yuv422` | `Image` |
| `YUYV` | `yuyv` | `Image` |
| `YUYV`, `UYVY` | `mono8` (luma samples of the packed format) | `Image` |
| `YUV420`, `YVU420`, `NV12`, `NV21` | `mono8` (luma plane only) | `Image` |

A given `format` may map to more than one `target_format` (see `MJPEG`
above); `target_format` disambiguates which one to use. Leaving
`target_format` empty picks the first matching entry for that pixel format.

The greyscale conversions are cheap: for planar formats the Y plane is
copied as one block, for packed `YUYV`/`UYVY` every second byte of a row is
taken (no colour math). `YUYV` → `bgr8` is not supported; for colour from a
USB webcam use `MJPEG` → `bgr8`.

> **Note:** the ROS image encoding `yuv422_yuy2` is deprecated (see
> `sensor_msgs/image_encodings.hpp`) and is no longer emitted by this
> package; use `yuyv` instead as the `target_format` for the `YUYV` pixel
> format above.

## TransportNode

Subscribes to the `image` topic (in its namespace) and republishes it
through every discovered `image_transport` publisher plugin other than
`raw` (since `CaptureNode` already publishes the raw image directly). The
set of enabled plugins is exposed as the standard `image_transport`
parameter `<resolved topic, dots for slashes>.enable_pub_plugins`, e.g. for
the default `composed.launch.py` setup this is `image.enable_pub_plugins`.

It only handles the `image` topic. `image_rect` gets its compressed
versions from `RectifyNode` itself. To compress a different topic, remap
`image` for this node (e.g. `remappings=[('image', 'image_rect')]`).

## Viewing the image

```bash
# raw
ros2 run image_view image_view --ros-args -r image:=/camera/image

# compressed (needs TransportNode, or image_rect)
ros2 run image_view image_view --ros-args -r image:=/camera/image -p image_transport:=compressed
```

Keep the remap on the base topic; `image_view` appends `/compressed` itself.

## Camera calibration

Uses `camera_calibration` with a checkerboard (here 9x6 inner corners,
25 mm squares):

```bash
ros2 run camera_calibration cameracalibrator --size 9x6 --square 0.025 \
  --ros-args -r image:=/camera/image
```

The calibrator calls `camera/set_camera_info`, which matches the default
namespace. For another namespace add
`-r camera/set_camera_info:=/<ns>/set_camera_info`.

- **COMMIT** sends the calibration to `CaptureNode` through
  `set_camera_info`, which writes it to `camera_info_url`. With a
  `package://` URL this is the copy in `install/…/share/tuw_camera/config/…`,
  which the next build overwrites: copy the file into
  [`config/camera_calibration/`](config/camera_calibration) to keep it.
- **SAVE** writes `/tmp/calibrationdata.tar.gz` (sample images plus
  `ost.yaml` / `ost.txt`). Copy `ost.yaml` into `config/camera_calibration/`,
  or convert `ost.txt` (INI format) with
  `ros2 run camera_calibration_parsers convert ost.ini <name>.yaml`.

Afterwards set `camera_name` in the calibration file to the
`camera_info_name` of the config (the calibrator writes `narrow_stereo`)
and reference it via `camera_info_url`. On startup the node logs
`Camera calibration data loaded.`

`cameracalibrator` processes slowly at high resolutions only if frames
arrive slowly. Check with `ros2 topic hz /camera/image` first (see below).

## Large images and Fast DDS

With the default Fast DDS settings, large best-effort messages (e.g. a
1920x1080 `mono8` image, 2 MB) are mostly dropped: on the Dell webcam the
camera delivered 5 fps, but only one frame every 5–10 s reached a
subscriber. 640x480 was unaffected. `FASTDDS_BUILTIN_TRANSPORTS=LARGE_DATA`
did not help.

The fix is the Fast DDS profile
[`fastdds_large_shm.xml`](../../../../fastdds_large_shm.xml) in the project
root (32 MB shared-memory segment, 4 MB UDP buffers). `env.sh` writes it
into a newly generated `.env.local`:

```bash
export FASTDDS_DEFAULT_PROFILES_FILE=${PROJECT_ROOT}/fastdds_large_shm.xml
```

It must be set for **every** process involved (camera node, `image_view`,
`cameracalibrator`, …); restart running nodes after setting it.

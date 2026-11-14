from launch_ros.actions import Node
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    # Camera Module 3 Wide parameters
    # (formerly config/pi_module3_wide_bgr.yaml).
    # For YUYV use: format:=YUYV target_format:=yuyv
    # For mono8 use: format:=YUV420 target_format:=mono8 (luma plane only)
    format_arg = DeclareLaunchArgument(
        'format', default_value='RGB888',
        description='libcamera pixel format of the video stream (e.g. RGB888, YUYV, YUV420)')
    target_format_arg = DeclareLaunchArgument(
        'target_format', default_value='bgr8',
        description='ROS image encoding of the published image (e.g. bgr8, yuyv, mono8)')
    frame_id_arg = DeclareLaunchArgument(
        'frame_id', default_value='camera_link',
        description='frame_id set in the header of published image/camera_info messages')
    camera_info_name_arg = DeclareLaunchArgument(
        'camera_info_name', default_value='pi_module3_wide',
        description='camera name for camera_info (must match camera_name in the calibration file; empty disables camera_info)')
    camera_info_url_arg = DeclareLaunchArgument(
        'camera_info_url', default_value='',
        description='URL of the camera calibration file (package://, file://); '
                    'empty uses ~/.ros/camera_info/<camera_info_name>.yaml')

    camera_params = {
        'camera_info_name': LaunchConfiguration('camera_info_name'),
        'camera_info_url': LaunchConfiguration('camera_info_url'),
        'stream_roles': ['video'],
        'frame_id': LaunchConfiguration('frame_id'),
        'streams.video.format': LaunchConfiguration('format'),
        'streams.video.target_format': LaunchConfiguration('target_format'),
        'streams.video.width': 1536,
        'streams.video.height': 864,
    }

    # standalone executable (no component container)
    capture_node = Node(
        package='tuw_camera',
        executable='tuw_camera_capture',
        name='libcamera_node',
        namespace='camera',
        output='screen',
        #ros_arguments=["--log-level", "debug"],
        #prefix='gdbserver :2222',
        parameters=[camera_params]
    )

    return LaunchDescription([
        format_arg,
        target_format_arg,
        frame_id_arg,
        camera_info_name_arg,
        camera_info_url_arg,
        capture_node
    ])

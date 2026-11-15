from launch_ros.actions import Node
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    # Camera Module 3 Wide parameters
    # (formerly config/pi_module3_wide_bgr.yaml).
    # For RGB use: format:=RGB888 target_format:=bgr8
    # For YUYV use: format:=YUYV  target_format:=yuyv
    # For mono8 use: format:=YUV420 target_format:=mono8
    format_arg = DeclareLaunchArgument(
        'format', default_value='YUYV',
        description='libcamera pixel format of the video stream (e.g. RGB888, YUYV, YUV420)')
    target_format_arg = DeclareLaunchArgument(
        'target_format', default_value='mono8',
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
    namespace_arg = DeclareLaunchArgument(
        'namespace',
        default_value='camera',
        description='Namespace for the camera nodes and topics')

    transport_launch_arg = DeclareLaunchArgument(
        'transport_launch',
        default_value='true',
        choices=['true', 'false'],
        description='Start camera_transport (compressed image topics)')

    resolution_arg = DeclareLaunchArgument(
        'resolution', default_value='',
        description='image resolution as <width>x<height> (e.g. 1920x1080); '
                    'empty uses the camera default')

    return LaunchDescription([
        format_arg,
        target_format_arg,
        frame_id_arg,
        camera_info_name_arg,
        camera_info_url_arg,
        resolution_arg,
        namespace_arg,
        transport_launch_arg,
        OpaqueFunction(function=launch_setup)
    ])


def launch_setup(context):
    camera_params = {
        'camera_info_name': LaunchConfiguration('camera_info_name'),
        'camera_info_url': LaunchConfiguration('camera_info_url'),
        'stream_roles': ['video'],
        'frame_id': LaunchConfiguration('frame_id'),
        'streams.video.format': LaunchConfiguration('format'),
        'streams.video.target_format': LaunchConfiguration('target_format')
    }

    # width/height are int parameters: only set them if a resolution is given
    resolution = LaunchConfiguration('resolution').perform(context).strip()
    if resolution:
        try:
            width, height = (int(v) for v in resolution.lower().split('x'))
        except ValueError:
            raise RuntimeError(
                f"invalid resolution '{resolution}', expected <width>x<height>, "
                "e.g. 1920x1080")
        camera_params['streams.video.width'] = width
        camera_params['streams.video.height'] = height

    # standalone executable (no component container)
    capture_node = Node(
        package='tuw_camera',
        executable='camera_capture',
        name='libcamera_node',
        namespace=LaunchConfiguration('namespace'),
        output='screen',
        #ros_arguments=["--log-level", "debug"],
        #prefix='gdbserver :2222',
        parameters=[camera_params]
    )

    # republishes camera/image through all image_transport plugins
    # (camera/image/compressed, ...), standalone executable as well
    transport_node = Node(
        package='tuw_camera',
        executable='camera_transport',
        name='transport',
        namespace=LaunchConfiguration('namespace'),
        output='screen',
        condition=IfCondition(LaunchConfiguration('transport_launch'))
    )
    return [capture_node, transport_node]

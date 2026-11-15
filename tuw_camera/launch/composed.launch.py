from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # name of a file in tuw_camera/config
    config_arg = DeclareLaunchArgument(
        'config',
        default_value='pi_module3_wide_yuyv_to_mono8.yaml',
        description='Camera configuration file in tuw_camera/config')

    namespace_arg = DeclareLaunchArgument(
        'namespace',
        default_value='camera',
        description='Namespace for the camera nodes and topics')

    transport_launch_arg = DeclareLaunchArgument(
        'transport_launch',
        default_value='true',
        choices=['true', 'false'],
        description='Start tuw_camera::TransportNode (compressed image topics)')

    rectify_launch_arg = DeclareLaunchArgument(
        'rectify_launch',
        default_value='true',
        choices=['true', 'false'],
        description='Start image_proc::RectifyNode (needs a camera calibration)')

    camera_params_file = PathJoinSubstitution([
        FindPackageShare('tuw_camera'),
        'config',
        LaunchConfiguration('config')
    ])

    capture_comp = ComposableNode(
        package='tuw_camera',
        plugin='tuw_camera::CaptureNode',
        name='libcamera_node',
        extra_arguments=[{'use_intra_process_comms': True}],
        namespace=LaunchConfiguration('namespace'),
        parameters=[camera_params_file]
    )

    transport_comp = ComposableNode(
        package='tuw_camera',
        plugin='tuw_camera::TransportNode',
        extra_arguments=[{'use_intra_process_comms': True}],
        namespace=LaunchConfiguration('namespace'),
        condition=IfCondition(LaunchConfiguration('transport_launch'))
    )

    # subscribes camera/image + camera/camera_info, publishes camera/image_rect
    rectify_comp = ComposableNode(
        package='image_proc',
        plugin='image_proc::RectifyNode',
        name='rectify_node',
        extra_arguments=[{'use_intra_process_comms': True}],
        namespace=LaunchConfiguration('namespace'),
        parameters=[{'image_transport': 'raw'}],
        condition=IfCondition(LaunchConfiguration('rectify_launch'))
    )

    container = ComposableNodeContainer(
        name='camera_processing_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        #ros_arguments=["--log-level", "debug"],
        #prefix='gdbserver :2222',
        composable_node_descriptions=[capture_comp, transport_comp, rectify_comp]
    )


    return LaunchDescription([
        config_arg,
        namespace_arg,
        transport_launch_arg,
        rectify_launch_arg,
        container
    ])

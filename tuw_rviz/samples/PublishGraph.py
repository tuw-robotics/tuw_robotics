#!/usr/bin/env python3 


from math import pi, sin, cos
from numpy import array
from numpy.linalg import norm

import rclpy
from rclpy.node import Node
from std_msgs.msg import Header
from tuw_graph_msgs.msg import Graph, Vertex
from geometry_msgs.msg import Point


def quaternion_about_axis(angle, axis):
    axis = array(axis)
    axis = axis / norm(axis)
    half_angle = angle / 2
    sine = sin(half_angle)
    w = cos(half_angle)
    x, y, z = axis * sine
    return x, y, z, w


def create_point(x, y):
    point = Point()
    point.x = x
    point.y = y
    point.z = 0.
    return point

def create_vertex(id, points, successors, predecessors):
    vertex = Vertex()
    vertex.valid = True
    for p in points:
        vertex.path.append(create_point(p[0],p[1]))
    for v in successors:
        vertex.successors.append(v)
    for v in predecessors:
        vertex.predecessors.append(v)
    return vertex
    
class publish_graph(Node):
    def __init__(self):
        super().__init__('publish_graph_sample')
        self.__pub = self.create_publisher(
            Graph, 'graph', 10)
        self.__timer = self.create_timer(0.1, self.pub_sample)
        self.__counter = 0
        self.__header = Header()


    def pub_sample(self):
        while self.__pub.get_subscription_count() == 0:
            return
        self.__header.stamp = self.get_clock().now().to_msg()
        self.__header.frame_id = 'map'

        # Reset counter and indices if counter is a multiple of 30
        if self.__counter % 10 == 0:
            self.__counter = 0

        # Create a single Graph message
        graph = Graph()
        graph.origin.position.x = 0.
        graph.origin.position.y = 0.
        graph.origin.position.z = 0.
        graph.origin.orientation.x = 0.
        graph.origin.orientation.y = 0.
        graph.origin.orientation.z = 0.
        graph.origin.orientation.w = 1.

        graph.vertices.append(create_vertex(0, [[-6., -6.], [-5., -7.], [-4., -7.], [-3., -6.]], [1,6],[3]))
        graph.vertices.append(create_vertex(1, [[-3., -6.], [-2., -2.], [-3., +1.]], [2,4],[0,6]))
        graph.vertices.append(create_vertex(2, [[-3., +1.], [-5.,  0.], [-6.,  0.]], [3],[1,4]))
        graph.vertices.append(create_vertex(3, [[-6.,  0.], [-6., -6.]], [0],[2]))
        graph.vertices.append(create_vertex(4, [[-3.,  1.], [-1., -1.], [ 3., -2.]], [5],[1,2]))
        graph.vertices.append(create_vertex(5, [[ 3., -2.], [ 4., -3.], [ 3., -5.]], [6],[4]))
        graph.vertices.append(create_vertex(6, [[ 3., -5.], [ 0., -6.], [-3., -6.]], [0,1],[5]))

        # Publish the BoundingBox3D message
        graph.header = self.__header
        self.__pub.publish(graph)
        self.__counter += 1


def main(args=None):
    rclpy.init(args=args)
    node = publish_graph()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()

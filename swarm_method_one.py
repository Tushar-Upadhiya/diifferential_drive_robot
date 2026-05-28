import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan, Image
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
import numpy as np
import math
import cv2
from cv_bridge import CvBridge

class SwarmAgentNode(Node):
    def __init__(self):
        super().__init__('swarm_agent_node')
        self.declare_parameter('robot_name', 'robot_1')
        self.robot_name = self.get_parameter('robot_name').get_value()
        
        self.bridge = CvBridge()
        self.goal_x = 3.0
        self.goal_y = 3.0
        
        self.robot_list = ['robot_1', 'robot_2', 'robot_3', 'robot_4']
        self.peer_poses = {}
        self.current_pose = [0.0, 0.0, 0.0]
        self.lidar_grid = np.ones(8, dtype=np.float32) * 12.0
        
        self.k_att = 0.6
        self.k_rep_obs = 0.35
        self.k_rep_peer = 0.75
        self.min_dist_peer = 0.85
        self.min_dist_obs = 0.55
        
        self.cmd_pub = self.create_publisher(Twist, f'/{self.robot_name}/cmd_vel', 10)
        
        self.create_subscription(LaserScan, f'/{self.robot_name}/scan', self.scan_callback, 10)
        self.create_subscription(Odometry, f'/{self.robot_name}/odom', self.odom_callback, 10)
        self.create_subscription(Image, f'/{self.robot_name}/camera/image_raw', self.camera_callback, 10)
        
        for peer in self.robot_list:
            if peer != self.robot_name:
                self.create_subscription(Odometry, f'/{peer}/odom', lambda msg, p=peer: self.peer_odom_callback(msg, p), 10)
                
        self.create_timer(0.05, self.control_loop)

    def get_yaw_from_quat(self, quat):
        siny_cosp = 2.0 * (quat.w * quat.z + quat.x * quat.y)
        cosy_cosp = 1.0 - 2.0 * (quat.y * quat.y + quat.z * quat.z)
        return math.atan2(siny_cosp, cosy_cosp)

    def odom_callback(self, msg):
        pos = msg.pose.pose.position
        yaw = self.get_yaw_from_quat(msg.pose.pose.orientation)
        self.current_pose = [pos.x, pos.y, yaw]

    def peer_odom_callback(self, msg, peer_name):
        pos = msg.pose.pose.position
        self.peer_poses[peer_name] = [pos.x, pos.y]

    def scan_callback(self, msg):
        src_ranges = np.array(msg.ranges, dtype=np.float32)
        src_ranges[~np.isfinite(src_ranges)] = 12.0
        split_segments = np.array_split(src_ranges, 8)
        self.lidar_grid = np.array([seg.min() for seg in split_segments], dtype=np.float32)

    def camera_callback(self, msg):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
            display_frame = cv2.resize(cv_image, (320, 240))
            
            min_lidar = self.lidar_grid.min()
            status_text = f"LiDAR Min: {min_lidar:.2f}m"
            color = (0, 0, 255) if min_lidar < self.min_dist_obs else (0, 255, 0)
            cv2.putText(display_frame, status_text, (10, 220), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)
            
            cv2.imshow(f"Live Feed - {self.robot_name.upper()}", display_frame)
            cv2.waitKey(1)
        except Exception:
            pass

    def control_loop(self):
        dx = self.goal_x - self.current_pose[0]
        dy = self.goal_y - self.current_pose[1]
        dist_to_goal = math.hypot(dx, dy)
        
        if dist_to_goal < 0.25:
            stop_msg = Twist()
            self.cmd_pub.publish(stop_msg)
            return
            
        f_x = self.k_att * (dx / (dist_to_goal + 0.001))
        f_y = self.k_att * (dy / (dist_to_goal + 0.001))
        
        for peer_name, pos in self.peer_poses.items():
            p_dx = self.current_pose[0] - pos[0]
            p_dy = self.current_pose[1] - pos[1]
            p_dist = math.hypot(p_dx, p_dy)
            if p_dist < self.min_dist_peer and p_dist > 0.01:
                rep_magnitude = self.k_rep_peer * (1.0 / p_dist - 1.0 / self.min_dist_peer) / (p_dist ** 2)
                f_x += rep_magnitude * (p_dx / p_dist)
                f_y += rep_magnitude * (p_dy / p_dist)
                
        angle_steps = np.linspace(-math.pi, math.pi, 8)
        for idx, dist in enumerate(self.lidar_grid):
            if dist < self.min_dist_obs and dist > 0.01:
                absolute_obstacle_angle = angle_steps[idx] + self.current_pose[2]
                rep_magnitude = self.k_rep_obs * (1.0 / dist - 1.0 / self.min_dist_obs) / (dist ** 2)
                f_x -= rep_magnitude * math.cos(absolute_obstacle_angle)
                f_y -= rep_magnitude * math.sin(absolute_obstacle_angle)
                
        target_heading = math.atan2(f_y, f_x)
        heading_error = target_heading - self.current_pose[2]
        heading_error = math.atan2(math.sin(heading_error), math.cos(heading_error))
        
        move_msg = Twist()
        if abs(heading_error) > 0.5:
            move_msg.linear.x = 0.0
            move_msg.angular.z = 0.5 if heading_error > 0 else -0.5
        else:
            move_msg.linear.x = min(0.22, math.hypot(f_x, f_y))
            move_msg.angular.z = 1.0 * heading_error
            
        self.cmd_pub.publish(move_msg)

def main(args=None):
    rclpy.init(args=args)
    node = SwarmAgentNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        cv2.destroyAllWindows()
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
import rclcpp
from rclcpp.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from gazebo_msgs.srv import SetModelState
from gazebo_msgs.msg import ModelState

import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
import random
import time
import collections

class DQN(nn.Module):
    def __init__(self, state_dim, action_dim):
        super(DQN, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(state_dim, 64),
            nn.ReLU(),
            nn.Linear(64, 64),
            nn.ReLU(),
            nn.Linear(64, action_dim)
        )
        
    def forward(self, x):
        return self.network(x)

class GazeboQLearningNode(Node):
    def __init__(self):
        super().__init__('gazebo_dqn_node')
        
        self.cmd_pub = self.create_publisher(Twist, '/cmd_vel', 10)
        self.odom_sub = self.create_subscription(Odometry, '/odom', self.odom_callback, 10)
        self.reset_client = self.create_client(SetModelState, '/gazebo/set_model_state')
        
        self.current_x = 1.0
        self.current_y = 1.0
        self.goal_x = 6.5
        self.goal_y = 6.5
        
        self.actions = [
            (0.4, 0.0),   # 0: Forward
            (0.1, 0.8),   # 1: Forward-Left Chicane
            (0.1, -0.8),  # 2: Forward-Right Chicane
            (0.0, 0.0)    # 3: Brake
        ]
        
        self.memory = collections.deque(maxlen=10000)
        self.model = DQN(2, 4)
        self.optimizer = optim.Adam(self.model.parameters(), lr=0.001)
        self.criterion = nn.MSELoss()
        
        self.epsilon = 1.0
        self.epsilon_decay = 0.99
        self.min_epsilon = 0.05
        self.gamma = 0.95
        self.batch_size = 32
        
    def odom_callback(self, msg):
        self.current_x = msg.pose.pose.position.x
        self.current_y = msg.pose.pose.position.y

    def teleport_reset(self):
        while not self.reset_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Waiting for Gazebo reset service...')
            
        request = SetModelState.Request()
        state = ModelState()
        state.model_name = 'tushar_car_v11'
        state.pose.position.x = 1.0
        state.pose.position.y = 1.0
        state.pose.position.z = 0.1
        request.model_state = state
        
        self.reset_client.call_async(request)
        time.sleep(0.5)

    def select_action(self, state):
        if random.random() < self.epsilon:
            return random.randint(0, 3)
        state_t = torch.FloatTensor(state)
        with torch.no_grad():
            return torch.argmax(self.model(state_t)).item()

    def train_step(self):
        if len(self.memory) < self.batch_size:
            return
            
        batch = random.sample(self.memory, self.batch_size)
        states, actions, rewards, next_states, dones = zip(*batch)
        
        states_t = torch.FloatTensor(np.array(states))
        actions_t = torch.LongTensor(actions).unsqueeze(1)
        rewards_t = torch.FloatTensor(rewards)
        next_states_t = torch.FloatTensor(np.array(next_states))
        dones_t = torch.FloatTensor(dones)
        
        current_q = self.model(states_t).gather(1, actions_t).squeeze(1)
        next_q = self.model(next_states_t).max(1)[0]
        target_q = rewards_t + (self.gamma * next_q * (1 - dones_t))
        
        loss = self.criterion(current_q, target_q.detach())
        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()

    def run_training(self):
        print("Starting Gazebo Deep Q-Learning Loop...")
        for episode in range(100):
            self.teleport_reset()
            state = np.array([self.current_x, self.current_y], dtype=np.float32)
            total_reward = 0
            done = False
            steps = 0
            
            while not done and steps < 150:
                rclcpp.spin_some(self)
                
                action_idx = self.select_action(state)
                linear, angular = self.actions[action_idx]
                
                # Execute action via topic publication
                twist = Twist()
                twist.linear.x = linear; twist.angular.z = angular
                self.cmd_pub.publish(twist)
                
                time.sleep(0.1)
                rclcpp.spin_some(self)
                
                next_state = np.array([self.current_x, self.current_y], dtype=np.float32)
                
                # Dynamic Distance Reward calculation
                dist_to_goal = np.sqrt((self.current_x - self.goal_x)**2 + (self.current_y - self.goal_y)**2)
                reward = -dist_to_goal * 0.1
                
                # Check bounding layouts or target reaching
                if dist_to_goal < 0.4:
                    reward = 200.0
                    done = True
                elif self.current_x < 0.1 or self.current_y < 0.1 or self.current_x > 7.9 or self.current_y > 7.9:
                    reward = -50.0
                    done = True
                    
                self.memory.append((state, action_idx, reward, next_state, done))
                state = next_state
                total_reward += reward
                steps += 1
                
                self.train_step()
                
            self.epsilon = max(self.min_epsilon, self.epsilon * self.epsilon_decay)
            print(f"Episode: {episode+1} | Score: {total_reward:.2f} | Epsilon: {self.epsilon:.2f}")

def main():
    rclcpp.init()
    node = GazeboQLearningNode()
    try:
        node.run_training()
    except KeyboardInterrupt:
        pass
    rclcpp.shutdown()

if __name__ == '__main__':
    main()
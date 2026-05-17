#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

class TeleopNode : public rclcpp::Node
{
public:
    TeleopNode() : Node("teleop_node")
    {
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        RCLCPP_INFO(this->get_logger(), "Teleop node started");
        RCLCPP_INFO(this->get_logger(), "W=forward S=backward A=left D=right X=stop Q=quit");
    }

    void run()
    {
        set_raw_mode();
        struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
        
        while (rclcpp::ok())
        {
            int num_events = poll(&pfd, 1, 50); // 50ms timeout
            
            if (num_events > 0 && (pfd.revents & POLLIN))
            {
                char key = getchar();
                auto msg = geometry_msgs::msg::Twist();
                bool valid_key = true;

                switch (key)
                {
                    case 'w': case 'W':
                        msg.linear.x  =  0.5;
                        msg.angular.z =  0.0;
                        RCLCPP_INFO(this->get_logger(), "FORWARD");
                        break;
                    case 's': case 'S':
                        msg.linear.x  = -0.5;
                        msg.angular.z =  0.0;
                        RCLCPP_INFO(this->get_logger(), "BACKWARD");
                        break;
                    case 'a': case 'A':
                        msg.linear.x  =  0.0;
                        msg.angular.z =  1.0;
                        RCLCPP_INFO(this->get_logger(), "LEFT");
                        break;
                    case 'd': case 'D':
                        msg.linear.x  =  0.0;
                        msg.angular.z = -1.0;
                        RCLCPP_INFO(this->get_logger(), "RIGHT");
                        break;
                    case 'x': case 'X':
                        msg.linear.x  =  0.0;
                        msg.angular.z =  0.0;
                        RCLCPP_INFO(this->get_logger(), "STOP");
                        break;
                    case 'q': case 'Q':
                        RCLCPP_INFO(this->get_logger(), "Quitting...");
                        restore_mode();
                        return;
                    default:
                        valid_key = false;
                        break;
                }
                
                if (valid_key)
                {
                    publisher_->publish(msg);
                }
            }
            
            rclcpp::spin_some(this->get_node_base_interface());
        }
        restore_mode();
    }

private:
    void set_raw_mode()
    {
        tcgetattr(STDIN_FILENO, &original_termios_);
        struct termios raw = original_termios_;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }

    void restore_mode()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios_);
    }

    struct termios original_termios_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TeleopNode>();
    node->run();
    rclcpp::shutdown();
    return 0;
}
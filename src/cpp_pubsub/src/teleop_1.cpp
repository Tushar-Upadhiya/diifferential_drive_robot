#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
class TeleopNode:public rclcpp::Node{
    public:
    TeleopNode():Node("Teleop_node"){
        publisher_=this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel",10);
        RCLCPP_INFO(this->get_logger(),"Teleop node started");
        RCLCPP_INFO(this->get_logger(),"W=forward S=backward A=left D=right X=stop Q=quit");

    }
    void run(){
        set_raw_mode();
        char key;
        while(rclcpp::ok()){
            key=getchar();
            auto msg=geometry_msgs::msg::Twist();
            switch(key){
                case 'w':case 'W':
                    msg.linear.x=0.5;
                    msg.angular.z=0.0;
                    RCLCPP_INFO(this->get_logger(),"FORWARD");
                    break;
                case 's':case 'S':
                    msg.linear.x=-0.5;
                    msg.angular.z=0.0;
                    RCLCPP_INFO(this->get_logger(),"BACKWARD");
                    break;
                case 'a':case 'A':
                    msg.linear.x=0.0;
                    msg.angular.z=0.5;
                    RCLCPP_INFO(this->get_logger(),"LEFT");
                    break;
                case 'd':case 'D':
                    msg.linear.x=0.0;
                    msg.angular.z=-0.5;
                    RCLCPP_INFO(this->get_logger(),"RIGHT");
                    break;
                case 'x':case 'X':
                    msg.linear.x=0.0;
                    msg.angular.z=0.0;
                    RCLCPP_INFO(this->get_logger(),"STOP");
                    break;
                case 'q':case 'Q':
                    RCLCPP_INFO(this->get_logger(),"Quitting...");
                    restore_mode();
                    return;
                default:                    continue;
            }   
            publisher_->publish(msg);
        }
        restore_mode();
    }
    private:
    void set_raw_mode(){
        struct termios raw;
        tcgetattr(STDIN_FILENO, &raw);
        raw.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }
    void restore_mode(){
       
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios_);
    }
    struct termios original_termios_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
};
int main(int argc,char* argv[]){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<TeleopNode>();
    node->run();
    rclcpp::shutdown();
    return 0;
}
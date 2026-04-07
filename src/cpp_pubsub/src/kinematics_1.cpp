#include<rclcpp/rclcpp.hpp>
#include<geometry_msgs/msg/twist.hpp>
#include<diff_drive_msgs/msg/coordinates.hpp>
#include<memory>
using std::placeholders::_1;
class KinematicsNode:public rclcpp::Node{
    public:
    KinematicsNode():Node("Kinematics_node"){
        subscription_=this->create_subscription<geometry_msgs::msg::Twist>("/cmd_vel",10,std::bind(&KinematicsNode::cmd_vel_callback,this,_1));
        publisher_=this->create_publisher<diff_drive_msgs::msg::Coordinates>("/wheel_speeds",10);
        RCLCPP_INFO(this->get_logger(),"Kinematics node started");
    }
    private:
    void cmd_vel_callback(const geometry_msgs::msg::Twist &msg){
        double linear=msg.linear.x;
        double angular=msg.angular.z;
        double left_speed=(linear-angular*wheel_base_/2.0)/wheel_radius_;
        double right_speed=(linear+angular*wheel_base_/2.0)/wheel_radius_;
        auto wheel_msg=diff_drive_msgs::msg::Coordinates();
        wheel_msg.x=left_speed;
        wheel_msg.y=right_speed;
        wheel_msg.z=0.0;
        RCLCPP_INFO(this->get_logger(),"Left: %.2f rad/s  Right: %.2f rad/s",left_speed,right_speed);
        publisher_->publish(wheel_msg); 

}
    const double wheel_radius_=0.033;
    const double wheel_base_=0.16;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
    rclcpp::Publisher<diff_drive_msgs::msg::Coordinates>::SharedPtr publisher_;
};
int main(int argc,char *argv[]){
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<KinematicsNode>());
    rclcpp::shutdown();
    return 0;
}
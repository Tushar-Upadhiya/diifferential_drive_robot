#include<chrono>
#include<memory>
#include<rclcpp/rclcpp.hpp>
#include<diff_drive_msgs/msg/coordinates.hpp>
using namespace std::chrono_literals;
class CoordPublisher:public rclcpp::Node
{
public:
        CoordPublisher():Node("coord_publisher")
        {
            publisher_=this->create_publisher<diff_drive_msgs::msg::Coordinates>("coordinates",10);
            timer_=this->create_wall_timer(500ms,[this](){
                auto msg = diff_drive_msgs::msg::Coordinates();
                msg.x=1.5;
                msg.y=2.3;
                msg.z=0.0;
                RCLCPP_INFO(this->get_logger(),"Publishing: x = %.2f , y = %.2f, z = %.2f", msg.x,msg.y,msg.z);
                publisher_->publish(msg);
            });

        }
private:
    rclcpp::Publisher<diff_drive_msgs::msg::Coordinates>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CoordPublisher>());
    rclcpp::shutdown();
    return 0;
}
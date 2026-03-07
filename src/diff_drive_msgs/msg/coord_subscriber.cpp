#include <rclcpp/rclcpp.hpp>
#include <diff_drive_msgs/msg/coordinates.hpp>
#include <memory>

using std::placeholders::_1;

class CoordSubscriber : public rclcpp::Node
{
public:
    CoordSubscriber() : Node("coord_subscriber")
    {
        subscription_ = this->create_subscription<diff_drive_msgs::msg::Coordinates>(
            "coordinates", 10, std::bind(&CoordSubscriber::topic_callback, this, _1));
    }

private:
    void topic_callback(const diff_drive_msgs::msg::Coordinates & msg) const
    {
        RCLCPP_INFO(this->get_logger(), "Received: x=%.2f y=%.2f z=%.2f", msg.x, msg.y, msg.z);
    }

    rclcpp::Subscription<diff_drive_msgs::msg::Coordinates>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CoordSubscriber>());
    rclcpp::shutdown();
    return 0;
}
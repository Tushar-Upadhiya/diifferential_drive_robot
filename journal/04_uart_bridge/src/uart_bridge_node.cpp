#include <rclcpp/rclcpp.hpp>
#include <diff_drive_msgs/msg/coordinates.hpp>
#include <memory>

using std::placeholders::_1;

class UartBridgeNode : public rclcpp::Node
{
public:
    UartBridgeNode() : Node("uart_bridge_node")
    {
        subscription_ = this->create_subscription<diff_drive_msgs::msg::Coordinates>(
            "/wheel_speeds", 10, std::bind(&UartBridgeNode::wheel_speeds_callback, this, _1));

        RCLCPP_INFO(this->get_logger(), "UART Bridge node started");
        RCLCPP_INFO(this->get_logger(), "Waiting for wheel speeds on /wheel_speeds...");
    }

private:
    void wheel_speeds_callback(const diff_drive_msgs::msg::Coordinates & msg)
    {
        double left  = msg.x;
        double right = msg.y;

        int left_pwm  = (int)(left  * 10.0);
        int right_pwm = (int)(right * 10.0);

        left_pwm  = std::clamp(left_pwm,  -255, 255);
        right_pwm = std::clamp(right_pwm, -255, 255);

        RCLCPP_INFO(this->get_logger(),
            "rad/s → L: %.2f R: %.2f | PWM → L: %d R: %d",
            left, right, left_pwm, right_pwm);
    }

    rclcpp::Subscription<diff_drive_msgs::msg::Coordinates>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<UartBridgeNode>());
    rclcpp::shutdown();
    return 0;
}
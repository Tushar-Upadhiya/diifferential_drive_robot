#include <rclcpp/rclcpp.hpp>
#include <diff_drive_msgs/msg/coordinates.hpp>
#include <memory>
#include <cmath>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

using std::placeholders::_1;

class UartBridgeNode : public rclcpp::Node
{
public:
    UartBridgeNode() : Node("uart_bridge_node")
    {
        this->declare_parameter("serial_port", "/dev/ttyACM0");
        this->declare_parameter("baud_rate", 115200);
        this->declare_parameter("simulation_mode", true);

        serial_port_name_ = this->get_parameter("serial_port").as_string();
        simulation_mode_  = this->get_parameter("simulation_mode").as_bool();

        if (!simulation_mode_)
        {
            serial_fd_ = open(serial_port_name_.c_str(), O_RDWR | O_NOCTTY);
            if (serial_fd_ < 0)
            {
                RCLCPP_ERROR(this->get_logger(), "Failed to open serial port: %s", serial_port_name_.c_str());
            }
            else
            {
                configure_serial(serial_fd_);
                RCLCPP_INFO(this->get_logger(), "Serial port opened: %s", serial_port_name_.c_str());
            }
        }
        else
        {
            RCLCPP_INFO(this->get_logger(), "Running in SIMULATION MODE - no serial port opened");
        }

        subscription_ = this->create_subscription<diff_drive_msgs::msg::Coordinates>(
            "/wheel_speeds", 10, std::bind(&UartBridgeNode::wheel_speeds_callback, this, _1));

        RCLCPP_INFO(this->get_logger(), "UART Bridge node started");
    }

    ~UartBridgeNode()
    {
        if (serial_fd_ >= 0)
            close(serial_fd_);
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

        uint8_t left_dir  = (left_pwm  >= 0) ? 1 : 0;
        uint8_t right_dir = (right_pwm >= 0) ? 1 : 0;

        uint8_t left_speed  = (uint8_t)std::abs(left_pwm);
        uint8_t right_speed = (uint8_t)std::abs(right_pwm);

        uint8_t buffer[5];
        buffer[0] = 0xFF;
        buffer[1] = left_dir;
        buffer[2] = left_speed;
        buffer[3] = right_dir;
        buffer[4] = right_speed;

        RCLCPP_INFO(this->get_logger(),
            "L: %.2f rad/s → dir=%d pwm=%d | R: %.2f rad/s → dir=%d pwm=%d",
            left, left_dir, left_speed, right, right_dir, right_speed);

        if (!simulation_mode_ && serial_fd_ >= 0)
        {
            write(serial_fd_, buffer, sizeof(buffer));
        }
    }

    void configure_serial(int fd)
    {
        struct termios tty;
        tcgetattr(fd, &tty);
        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tcsetattr(fd, TCSANOW, &tty);
    }

    int serial_fd_ = -1;
    bool simulation_mode_ = true;
    std::string serial_port_name_;

    rclcpp::Subscription<diff_drive_msgs::msg::Coordinates>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<UartBridgeNode>());
    rclcpp::shutdown();
    return 0;
}
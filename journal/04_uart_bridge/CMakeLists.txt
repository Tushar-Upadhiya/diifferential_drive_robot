cmake_minimum_required(VERSION 3.8)
project(04_uart_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(diff_drive_msgs REQUIRED)

add_executable(uart_bridge_node src/uart_bridge_node.cpp)
ament_target_dependencies(uart_bridge_node rclcpp diff_drive_msgs)

install(TARGETS
  uart_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
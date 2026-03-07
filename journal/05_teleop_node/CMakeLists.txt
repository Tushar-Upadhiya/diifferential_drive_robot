cmake_minimum_required(VERSION 3.8)
project(05_teleop_node)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(teleop_node src/teleop_node.cpp)
ament_target_dependencies(teleop_node rclcpp geometry_msgs)

install(TARGETS
  teleop_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
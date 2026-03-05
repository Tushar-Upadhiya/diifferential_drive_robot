cmake_minimum_required(VERSION 3.8)
project(03_kinematics)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(diff_drive_msgs REQUIRED)

add_executable(kinematics_node src/kinematics_node.cpp)
ament_target_dependencies(kinematics_node rclcpp geometry_msgs diff_drive_msgs)

install(TARGETS
  kinematics_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
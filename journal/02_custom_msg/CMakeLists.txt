cmake_minimum_required(VERSION 3.8)
project(02_custom_msg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(diff_drive_msgs REQUIRED)


add_executable(coord_publisher src/coord_publisher.cpp)
ament_target_dependencies(coord_publisher rclcpp diff_drive_msgs)

add_executable(coord_subscriber src/coord_subscriber.cpp)
ament_target_dependencies(coord_subscriber rclcpp diff_drive_msgs)

install(TARGETS
  coord_publisher
  coord_subscriber
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
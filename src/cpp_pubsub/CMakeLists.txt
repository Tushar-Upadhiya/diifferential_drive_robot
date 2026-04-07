cmake_minimum_required(VERSION 3.8)
project(cpp_pubsub)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(diff_drive_msgs REQUIRED)

# 2. Create the Teleop Executable
add_executable(teleop_node src/teleop_1.cpp)
ament_target_dependencies(teleop_node rclcpp geometry_msgs)

# 3. Create the Kinematics Executable
add_executable(kinematics_node src/kinematics_1.cpp)
ament_target_dependencies(kinematics_node rclcpp geometry_msgs diff_drive_msgs)

# 4. Install targets (so 'ros2 run' can find them)
install(TARGETS
  teleop_node
  kinematics_node
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
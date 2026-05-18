FROM osrf/ros:humble-desktop

RUN apt-get update && apt-get install -y \
    sudo \
    python3-colcon-common-extensions \
    tmux \
    python3-pip \
    ros-humble-turtlesim \
    ros-humble-gazebo-ros-pkgs \
    ros-humble-gazebo-ros2-control \
    ros-humble-xacro \
    ros-humble-slam-toolbox \
    ros-humble-rviz2 \
    ros-humble-navigation2 \
    ros-humble-nav2-bringup \
    ros-humble-joint-state-publisher-gui \

    libgl1-mesa-dri \
    libgl1-mesa-glx \

    ros-humble-ros2-control \
    ros-humble-ros2-controllers \

    && rm -rf /var/lib/apt/lists/*

ARG USERNAME=Tushar
RUN useradd -m $USERNAME && \
    echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME

USER $USERNAME
WORKDIR /workspaces/${PWD##*/}
RUN echo "alias sim='export GAZEBO_AUDIO=0 && ros2 launch gazebo_ros gazebo.launch.py world:=/workspaces/differential_drive_robot/arena/maze_arena.world'" >> ~/.bashrc
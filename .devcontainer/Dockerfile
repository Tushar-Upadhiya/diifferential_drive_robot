FROM osrf/ros:humble-desktop

RUN apt-get update && apt-get install -y \
    sudo \
    python3-colcon-common-extensions \
    tmux \
    python3-pip \
    ros-humble-turtlesim \
    ros-humble-gazebo-ros-pkgs \
    ros-humble-gazebo-ros2-control \
    # URDF and Xacro tools
    ros-humble-xacro \
    ros-humble-joint-state-publisher-gui \
    # These are often needed for custom messages and builds
    ros-humble-ros2-control \
    ros-humble-ros2-controllers \

    && rm -rf /var/lib/apt/lists/*

ARG USERNAME=Tushar
RUN useradd -m $USERNAME && \
    echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME

USER $USERNAME
WORKDIR /workspaces/${PWD##*/}

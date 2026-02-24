FROM osrf/ros:humble-desktop

RUN apt-get update && apt-get install -y \
    sudo \
    python3-colcon-common-extensions \
    ros-humble-turtlesim \
    && rm -rf /var/lib/apt/lists/*

ARG USERNAME=Tushar
RUN useradd -m $USERNAME && \
    echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME

USER $USERNAME
WORKDIR /workspaces/${PWD##*/}

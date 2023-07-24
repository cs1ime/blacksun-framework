#!/bin/bash

sudo apt-get update
sudo apt-get install -y cpu-checker
sudo apt-get install -y qemu-kvm virt-manager libvirt-daemon-system virtinst libvirt-clients bridge-utils
sudo apt-get install -y va-driver-all vdpau-driver-all
sudo apt-get install -y intel-media-va-driver-non-free
sudo apt-get install -y libegl1-mesa-dev libgl1-mesa-dev libopus-dev libqt5svg5-dev libsdl2-dev libsdl2-ttf-dev libssl-dev libavcodec-dev libva-dev libvdpau-dev libxkbcommon-dev qtwayland5 qt5-qmake qtbase5-dev qtdeclarative5-dev qtquickcontrols2-5-dev wayland-protocols qml-module-qtquick-controls2 qml-module-qtquick-layouts qml-module-qtquick-window2 qml-module-qtquick2
sudo apt-get install -y fuse libfuse2
sudo apt-get install -y net-tools 
sudo apt-get install -y linux-oem-22.04c
sudo apt-get install -y clang-14 make cmake libboost-all-dev libfmt-dev zlib1g-dev libssl-dev build-essential
sudo apt-get install -y gnome-control-center

filename='/etc/gdm3/custom.conf'

sudo sed -i 's/^#WaylandEnable=false/WaylandEnable=false/g' "$filename"

# blacksun framework for QEMU/KVM game cheat development

Already done

- Memory access and rendering both run outside the virtual machine, it is possible to bypass all anti-cheating detections, because cloud gaming service providers often use KVM.
- Modify Moonlight source code to render with imgui, compatible with vaapi(opengl), vdpau(sdlrender).
- Achieve ultra-low latency Moonlight game streaming with NAT network mode.
- Simulate keyboard and mouse input by sending SDL_Event to Moonlight.
- Simulate a mouse with a 100Hz polling rate to achieve more realistic aiming.
- Block side mouse button (the button that triggers aiming) inputs to the virtual machine to avoid aim detection.

## Instructions:

Environment: Ubuntu 22.04.2 LTS

### Prepare

Ubuntu needs a cpu integrated graphics or a discrete graphics card to decode video for moonlight, and pass the discrete graphics card through to the virtual machine to run games

A laptop with a discrete graphics card can meet this requirement. Change the graphics device of the laptop to integrated graphics, and then connect the EDID Spoofer or a monitor to the HDMI port of the discrete graphics card to meet the requirements.

### Installation

Install Ubuntu 22.04.2 LTS

Install the environment required for compiling and running

```bash
./setup-build-env.sh
```

Reboot

```bash
reboot
```

Disable Wayland 

```
sudo apt install -y vim
sudo vim /etc/gdm3/custom.conf

Uncomment #WaylandEnable=false in /etc/gdm3/custom.conf
```

Run the script to setup vfio-pci

```bash
cd gpu-passthrough
chmod +x *
sudo bash gpu_passthrough.sh
```

Reboot

```bash
reboot
```

Open virt-manager

Create a virtual machine named **win10**

Change the virtual network card to virtio mode, change the virtual disk to virtio mode, and add the discrete graphics card to the virtual machine

Install Windows10 21h2 in the virtual machine

Run Windows Update, manually download and install the graphics card driver

Install sunshine in the virtual machine


### Compile

```bash
./build.sh
```

### Run

```bash
cd build/apex
./runcheat.sh
```

The cheat will launch a Moonlight client, use this to control the virtual machine

Download Apex and enjoy it.

## ToDoList

- [ ]  Study computer graphics and try to implement the ray tracing algorithm of the Unreal Engine externally.
- [ ]  Add the new feature of Ept breakpoint, implement Hook and CEServer.

## ScreenShots

![1](./screenshots/1.PNG)


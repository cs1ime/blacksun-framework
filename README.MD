# blacksun framework for QEMU/KVM game cheat development

blacksun QEMU/KVM 辅助开发框架

已经做到

- 绘制和读写都在虚拟机外部实现,绕过一切反作弊外挂检测(云游戏厂商都用的KVM虚拟机)
- 修改moonlight源码实现imgui绘制,兼容vaapi(opengl),vdpau(sdlrender)
- 利用NAT网络通信以做到超低延迟的moonlight串流画面
- 通过对moonlight发送SDL_Event实现键鼠模拟
- 模拟100hz回报率的鼠标进行自瞄以做到丝滑的自瞄效果
- 屏蔽鼠标侧键(触发自瞄的按键)输入虚拟机以做到更隐蔽的自瞄

## 使用方法:

环境: Ubuntu 22.04.2 LTS

### 准备硬件

Ubuntu需要核显或者一张显卡来给moonlight做解码,还需要一张显卡直通进虚拟机运行游戏

一台带独显的笔记本完全满足这个要求,关闭独显直连就是内建显示器连接核显,独显接口插上显卡欺骗器或者显示器就可以了

如果你用的是台式电脑,CPU核显能和独立显卡一起工作,你需要一个显卡欺骗器,显示器连接CPU核显,显卡欺骗器连接独立显卡

如果你用的是台式电脑,CPU没有核显,你需要确保你的电脑主板有两个PCIE X16的插槽,你需要一个显卡欺骗器,一张GT1030显卡,并确保电源功率足够,拆下你电脑上原有的显卡,把GT1030装到主板的第一个PCIE X16插槽,把你原有的显卡装到主板的第二个PCIE X16插槽,这样做是为了让Ubuntu把GT1030作为Boot VGA设备.显示器连接GT1030,显卡欺骗器连接原有的显卡就行了
另外,你需要安装Ubuntu的NVIDIA显卡驱动

### 虚拟机搭建

安装Ubuntu 22.04.2 LTS

初始化编译和运行所需的环境

```bash
./setup-build-env.sh
```

重启Ubuntu

```bash
reboot
```

禁用Wayland 

```
sudo apt install -y vim
sudo vim /etc/gdm3/custom.conf

取消注释 /etc/gdm3/custom.conf 中的 #WaylandEnable=false
```

运行显卡直通脚本

```bash
cd gpu-passthrough
chmod +x *
sudo bash gpu_passthrough.sh
```

重启Ubuntu

```bash
reboot
```

打开virt-manager

创建一个名为**win10**的虚拟机,名字必须是win10

网卡改为virtio模式,磁盘改为virtio模式,把独显加入虚拟机

安装win10 21h2

执行Windows更新,下载安装显卡驱动

虚拟机安装[Sunshine](https://github.com/LizardByte/Sunshine)串流软件


### 编译辅助

```bash
./setup-build-env.sh
./build.sh
```

### 运行辅助

```bash
cd build/apex
./runcheat.sh
```

辅助会启动一个moonlight,连接虚拟机,显示设置仅在第二个显示器上显示

下载安装Apex,运行游戏就能看到辅助效果,鼠标侧键自瞄

## 未来将要做

- [ ]  学习计算机图形学,并尝试做出UE引擎的外部光线追踪写法
- [ ]  实现 EPT 读写执行分离,做一个Hook引擎和CEServer调试器

## 效果截图:

![1](./screenshots/1.PNG)

自瞄效果:

[video](https://github.com/cs1ime/blacksun-framework/blob/master/videos/aimingeffect.mp4)


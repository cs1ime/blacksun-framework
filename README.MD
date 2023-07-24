# blacksun framework for QEMU/KVM game cheat development

blacksun QEMU/KVM 辅助开发框架

已经做到

- 绘制和读写都在虚拟机外部实现,绕过一切反作弊外挂检测(云游戏厂商都用的KVM虚拟机)
- 修改moonlight源码实现imgui绘制,兼容vaapi(opengl),vdpau(sdlrender)
- 利用NAT网络通信以做到超低延迟的moonlight串流画面
- 通过对moonlight发送SDL_Event实现键鼠模拟
- 模拟100hz回报率的鼠标进行自瞄以做到真实丝滑的自瞄效果
- 屏蔽鼠标侧键(触发自瞄的按键)输入虚拟机以做到更隐蔽的自瞄

## 使用方法:

环境: Ubuntu 22.04.2 LTS

### 编译辅助:

```bash
./setup-build-env.sh
./build.sh
```

### 虚拟机搭建

Ubuntu需要核显或者一张显卡来给moonlight做解码,还需要一张显卡直通进虚拟机运行游戏

一台带独显的笔记本完全满足这个要求,关闭独显直连就是内建显示器连接核显,独显接口插上显卡欺骗器或者显示器就可以了

台式电脑如果没核显的话就得自行购买亮机卡了

先运行显卡直通脚本

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

执行Windows更新,下载显卡驱动

虚拟机安装sunshine串流软件

### 运行辅助

```bash
cd build/apex
./runcheat.sh
```

辅助会启动一个moonlight,连接虚拟机,显示设置仅在第二个显示器上显示

下载安装Apex,运行游戏就能看到辅助效果,鼠标侧键自瞄

效果截图:

![1](./screenshots/1.PNG)


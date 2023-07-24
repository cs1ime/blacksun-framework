#!/bin/bash

# Code from https://github.com/pavolelsig/passthrough_helper_ubuntu_20

#Making sure this script runs with elevated privileges
if [ $EUID -ne 0 ]
	then
		echo "Please run this as root!" 
		exit 1
fi

if [ -a /etc/initramfs-tools/scripts/init-top/vfio-pci-override-vga.sh ]
	then 
	echo "Please uninstall Passthrough Helper first! Then run gpu_passthrough.sh again."
	exit
fi

VIRT_USER=`logname`

apt -y install qemu-kvm libvirt-clients libvirt-daemon-system bridge-utils virt-manager ovmf


#Creating a GRUB variable equal to current content of grub cmdline. 
GRUB=`cat /etc/default/grub | grep "GRUB_CMDLINE_LINUX_DEFAULT" | rev | cut -c 2- | rev`


#Creating a grub backup for the uninstallation script and making uninstall.sh executable
cat /etc/default/grub > grub_backup.txt
chmod +x uninstall.sh

#Detecting CPU
CPU=$(lscpu | grep GenuineIntel | rev | cut -d ' ' -f 1 | rev )

INTEL="0"

if [ "$CPU" = "GenuineIntel" ]
	then
	INTEL="1"
fi

#Building string Intel or AMD iommu=on
if [ $INTEL = 1 ]
	then
	IOMMU="intel_iommu=on kvm.ignore_msrs=1"
	echo "Set Intel IOMMU On"
	else
	IOMMU="amd_iommu=on kvm.ignore_msrs=1"
	echo "Set AMD IOMMU On"
fi

#Putting together new grub string
OLD_OPTIONS=`cat /etc/default/grub | grep GRUB_CMDLINE_LINUX_DEFAULT | cut -d '"' -f 1,2`

NEW_OPTIONS="$OLD_OPTIONS $IOMMU\""
echo $NEW_OPTIONS

#Rebuilding grub 
sed -i -e "s|^GRUB_CMDLINE_LINUX_DEFAULT.*|${NEW_OPTIONS}|" /etc/default/grub

#User verification of new grub and prompt to manually edit it
echo 
echo "Grub was modified to look like this: "
echo `cat /etc/default/grub | grep "GRUB_CMDLINE_LINUX_DEFAULT"`
echo 
echo "Do you want to edit it? y/n"


#Updating grub
update-grub

usermod -a -G libvirt $VIRT_USER

cp vfio-pci-override-vga.sh /etc/initramfs-tools/scripts/init-top/vfio-pci-override-vga.sh

chmod 744 /etc/initramfs-tools/scripts/init-top/vfio-pci-override-vga.sh

if [ ${1-0} = "-e" ]
then
update-initramfs -u
else
update-initramfs -u 2> /dev/zero
fi


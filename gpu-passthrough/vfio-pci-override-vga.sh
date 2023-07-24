#!/bin/sh

for i in /sys/bus/pci/devices/*/boot_vga
do
    if [ $(cat "$i") -eq 0 ]; then
        GPU="${i%/boot_vga}"
        for part in `ls -d $(echo $GPU | cut -f1 -d'.').*`
        do
            echo "vfio-pci" > "$part/driver_override"
            BIND_DEVICE=`echo "$part" | cut -d '/' -f 6`
            echo "$BIND_DEVICE" >> /sys/bus/pci/drivers/vfio-pci/bind
        done
    fi
done

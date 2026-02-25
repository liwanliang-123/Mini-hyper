#!/bin/bash

cd u-boot
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- qemu_arm64_defconfig

# Set preboot
sed -i 's/CONFIG_PREBOOT="usb start"/CONFIG_PREBOOT="bootm 0x40200000 - ${fdtcontroladdr}"/g' "./.config"
make CROSS_COMPILE=aarch64-linux-gnu- -j4


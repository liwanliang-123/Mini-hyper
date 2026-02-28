#!/bin/bash

# 如果 u-boot 目录不存在，则克隆
if [ ! -d "u-boot" ]; then
    echo "u-boot 不存在"
    git clone git://git.denx.de/u-boot.git
fi

cd u-boot || { echo "进入 u-boot 目录失败"; exit 1; }

make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- qemu_arm64_defconfig

# Set preboot
sed -i 's/CONFIG_PREBOOT="usb start"/CONFIG_PREBOOT="bootm 0x40200000 - ${fdtcontroladdr}"/g' "./.config"
make CROSS_COMPILE=aarch64-linux-gnu- -j4


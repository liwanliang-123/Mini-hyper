#!/bin/bash

#如果 QEMU 没有开启 virtualization=on，那么 CPU 通常会进入 EL1 并运行内核。
#但如果明确指定了 virtualization=on，QEMU 直接以 EL2 启动。

CPU="cortex-a72"
QCPU="cortex-a72"
QEMU="/usr/bin/qemu-system-aarch64"
GIC_VERSION=3
MACHINE="virt,gic-version=$GIC_VERSION,virtualization=on"
NCPU=4
QEMUOPTS="-cpu $QCPU -machine $MACHINE -smp $NCPU -m 256M -nographic \
          -bios ./u-boot/u-boot.bin \
          -device loader,file=./build/Mini-Hyper_Uimage,addr=0x40200000,force-raw=on"

if [ $# -ne 0 ];then
    echo "debug mode ..."
    $QEMU $QEMUOPTS -S -gdb tcp::7788
else
    echo "run mode ..."
    $QEMU $QEMUOPTS
fi

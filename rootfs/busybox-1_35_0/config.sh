#!/bin/bash

set -e

rm rootfs.cpio rootfs.cpio.o

find ./* | cpio -H newc -o > rootfs.cpio
aarch64-linux-gnu-ld -r -b binary rootfs.cpio -o rootfs.cpio.o
cp -v rootfs.cpio rootfs.cpio.o /home/lwl/Hypervisor/test/Mini-Hyper/linux/

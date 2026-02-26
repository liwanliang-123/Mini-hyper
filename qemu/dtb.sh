#!/bin/bash

rm virt*

/usr/bin/qemu-system-aarch64 -machine virt,gic-version=3,virtualization=on -cpu cortex-a72 -smp 4 -m 256M -machine dumpdtb=./virt.dtb

dtc -I dtb -O dts -o virt.dts virt.dtb


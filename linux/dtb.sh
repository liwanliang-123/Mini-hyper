#!/bin/bash

# dtc to dtb
dtc -I dts -O dtb -o virt.dtb virt.dts

#dtb to dtb.o
aarch64-linux-gnu-objcopy -I binary -O elf64-littleaarch64 -B aarch64 virt.dtb virt.dtb.o


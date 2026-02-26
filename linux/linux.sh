#/bin/bash

cd /home/lwl/Hypervisor/test/Mini-Hyper/linux/src/linux6.19.0/ && sh build.sh && cd -

cp -v /home/lwl/Hypervisor/test/Mini-Hyper/linux/src/linux6.19.0/arch/arm64/boot/Image ./

aarch64-linux-gnu-objcopy -I binary -O elf64-littleaarch64 -B aarch64 ./Image ./image.o

aarch64-linux-gnu-objcopy \
    --redefine-sym _binary___Image_start=_binary_Image_start \
    --redefine-sym _binary___Image_end=_binary_Image_end \
    --redefine-sym _binary___Image_size=_binary_Image_size \
    ./image.o

aarch64-linux-gnu-nm ./image.o | grep _binary


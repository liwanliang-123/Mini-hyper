#!/bin/bash

export CC="aarch64-linux-gnu-gcc"

load_addr=0x40200000
entry_point=0x40200000
output_name=Mini-Hyper_Uimage
image_name=Mini-Hyper

make clean
make

if [ ! -f "build/${image_name}" ]; then
    echo "Error: build/${image_name} not found. Build可能失败。"
    exit 1
fi

# 进入 build 目录，生成 u-boot 镜像
cd build
mkimage -A arm64 -O linux -C none -a $load_addr -e $entry_point -d ${image_name} ${output_name}
cd ..

echo "Done. Output: build/${output_name}"
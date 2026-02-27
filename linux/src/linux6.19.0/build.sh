#!/bin/bash

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

#cp arch/arm64/config/defconfig .config
#imake menuconfig

export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
perl -minteger -e 'print "ok\n"'
make -j16

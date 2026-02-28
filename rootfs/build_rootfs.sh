#!/bin/bash

set -e

BUSYBOX_VERSION="1_35_0"
BUSYBOX_TARBALL="busybox-${BUSYBOX_VERSION}.tar.gz"
BUSYBOX_URL="https://github.com/mirror/busybox/archive/refs/tags/${BUSYBOX_VERSION}.tar.gz"
ROOTFS_DIR="./rootfs"
OUTPUT_CPIO="rootfs.cpio"
OUTPUT_GZIP="rootfs.cpio.gz"
OUTPUT_O="rootfs.cpio.o"
CROSS_COMPILE="aarch64-linux-gnu-"

# 下载并解压 busybox
if [ ! -d "busybox-${BUSYBOX_VERSION}" ]; then
    echo "下载 busybox..."
    wget ${BUSYBOX_URL} -O ${BUSYBOX_TARBALL}
    tar -xzf ${BUSYBOX_TARBALL}
fi

# 进入源码目录并执行配置
cd busybox-${BUSYBOX_VERSION}
echo "配置 busybox (使用默认配置)..."
make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} defconfig

echo "启用静态编译选项..."
sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config

# 编译
echo "编译 busybox..."
make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} -j$(nproc)

echo "安装 busybox 到临时目录 _install..."
make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} install
cd ..

# 准备根文件系统目录结构
echo "创建根文件系统目录结构 ${ROOTFS_DIR}"
rm -rf ${ROOTFS_DIR}
mkdir -p ${ROOTFS_DIR}
cp -a busybox-${BUSYBOX_VERSION}/_install/* ${ROOTFS_DIR}/

cd ${ROOTFS_DIR}
mkdir -p dev proc mnt sys tmp root
mkdir -p etc etc/init.d

# 创建 init 文件
echo "创建 init 文件"
cat > init << 'EOF'
#!/bin/sh
# devtmpfs does not get automounted for initramfs
echo "------>  I am a VM on X-Hyper <------"
/bin/mount -t devtmpfs devtmpfs /dev
exec 0</dev/console
exec 1>/dev/console
exec 2>/dev/console
exec /sbin/init "$@"
EOF
chmod +x init

# 创建 /etc/init.d/rcS
echo "创建 /etc/init.d/rcS"
cat > etc/init.d/rcS << 'EOF'
#!/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:/usr/lib
export PATH LD_LIBRARY_PATH runlevel
/bin/hostname hyperOS
mount -a
mkdir /dev/pts
mount -t devpts devpts /dev/pts
mdev -s
EOF
chmod +x etc/init.d/rcS

# 创建 /etc/fstab
echo "创建 /etc/fstab"
cat > etc/fstab << 'EOF'
proc    /proc   proc    defaults 0 0
tmpfs   /tmp    tmpfs   defaults 0 0
sysfs   /sys    sysfs   defaults 0 0
tmpfs   /dev    tmpfs   defaults 0 0
EOF

# 创建 /etc/inittab
echo "创建 /etc/inittab"
cat > etc/inittab << 'EOF'
#etc/inittab
::sysinit:/etc/init.d/rcS
console::askfirst:-/bin/sh
::restart:/sbin/init
::ctrlaltdel:/sbin/reboot
::shutdown:/bin/umount -a -r
::shutdown:/sbin/swapoff -a
EOF

# 创建 /etc/profile
echo "创建 /etc/profile"
cat > etc/profile << 'EOF'
USER="root"
LOGNAME=$USER
HOSTNAME=`/bin/hostname`
HOME=/root
PS1="[$USER@$HOSTNAME \W]\# "
PATH=$PATH
export USER LOGNAME HOSTNAME HOME PS1 PATH PATH LD_LIBRARY_PATH
EOF

cd ..
echo "打包根文件系统为 ${OUTPUT_CPIO} 并压缩"
( cd ${ROOTFS_DIR} && find ./* | cpio -H newc -o ) > ${OUTPUT_CPIO}
gzip -f ${OUTPUT_CPIO}
echo "生成 ${OUTPUT_GZIP}"

# 转换为 .o 文件 (用于链接到 hypervisor ELF)
echo "将 ${OUTPUT_GZIP} 转换为 ${OUTPUT_O}"

gunzip -c ${OUTPUT_GZIP} > ${OUTPUT_CPIO}
${CROSS_COMPILE}ld -r -b binary ${OUTPUT_CPIO} -o ${OUTPUT_O}
echo "生成 ${OUTPUT_O}"

echo "根文件系统构建完成！"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cp -v "$SCRIPT_DIR"/rootfs.cpio "$SCRIPT_DIR"/rootfs.cpio.o "$PROJECT_ROOT/linux/"

# clean
rm -rf $SCRIPT_DIR/rootfs*
cd busybox-${BUSYBOX_VERSION} && make clean

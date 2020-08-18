#!/bin/sh

. ./toolchain_path
. ./color

P=$(cat ./toolchain_path)
PP=$(echo $P | sed 's/.*://')
CROSS_COMPILE=$PP/arm-linux-gnueabihf-


ROOTDIR=$PWD
KERNEL=$ROOTDIR/orangepi-loboris-kernel-3.4.39


cecho y "*** Building linux kernel 3.4.39 ***"

if [ ! -d $KERNEL ]; then
  git clone https://github.com/uboborov/orangepi-loboris-kernel-3.4.39.git
  if [ $? -ne 0 ]; then
    cecho "!!! Failed to clone 3.4.39 kernel"
    exit 1
  fi
fi

mkdir -p $ROOTDIR/lib
cp $ROOTDIR/build_linux_kernel.sh $KERNEL
cp ./toolchain_path $KERNEL

cd $KERNEL
./build_linux_kernel.sh
if [ $? -ne 0 ]; then
  cecho r "!!! Failed to build linux kernel 3.4.39"
  exit 1
fi

cp -r $KERNEL/build/lib.opi2/* $ROOTDIR/lib
cp $ROOTDIR/modules.dep $ROOTDIR/lib/modules/3.4.39-02-lobo
cp $ROOTDIR/modules.dep.bin $ROOTDIR/lib/modules/3.4.39-02-lobo
cp $KERNEL/build/uImage_OPI-2 $ROOTDIR/uImage

echo "$KERNEL/linux-3.4" > $ROOTDIR/kernel_path


cecho g "!!! Done linux kernel 3.4.39 !!!\n"


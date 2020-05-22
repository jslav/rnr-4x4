#!/bin/sh

. ./toolchain_path


KERNEL=$(<"./kernel_path") 
CEDAR=$PWD/sunxi-cedar-mainline

CROSS_COMPILE=arm-linux-gnueabihf-

if [ -d $CEDAR ]; then
  make -C $CEDAR -f Makefile.linux clean
else
  git clone https://github.com/uboborov/sunxi-cedar-mainline.git
  if [ $? -ne 0 ]; then
    echo "!!! Failed to clone cedrus driver"
    exit
  fi
fi

cd $CEDAR
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=$KERNEL -f Makefile.linux

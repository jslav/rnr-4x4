#!/bin/sh

. ./toolchain_path
. ./color

P=$(cat ./toolchain_path)
PP=$(echo $P | sed 's/.*://')
CROSS_COMPILE=$PP/arm-linux-gnueabihf-

ROOTDIR=$PWD

PR=$1

cecho y "*** Building capture ***"

CAPTURE=$PWD/capture

if [ -d $CAPTURE ]; then
  make -C $CAPTURE -f Makefile.capture clean
else
  git clone https://github.com/uboborov/uvc_h264_lb_allwinner.git capture
  if [ $? -ne 0 ]; then
    cecho r "!!! Failed to clone capture"
    exit 1
  fi
fi

mkdir -p usr/local/bin

if [ $PR != "" ]; then
  cp pr.c $CAPTURE
fi

cp Makefile.capture $CAPTURE
cd $CAPTURE

make CROSS_COMPILE=$CROSS_COMPILE -f Makefile.capture
if [ $? -ne 0 ]; then
  cecho r "!!! Failed to build capture"
  exit 1
fi

cp capture $ROOTDIR/usr/local/bin

cecho g "!!! capture done !!!\n"

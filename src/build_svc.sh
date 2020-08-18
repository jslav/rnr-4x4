#!/bin/sh

. ./toolchain_path
. ./color

P=$(cat ./toolchain_path)
PP=$(echo $P | sed 's/.*://')
CROSS_COMPILE=$PP/arm-linux-gnueabihf-

ROOTDIR=$PWD

cecho y "*** Building supervisor ***"

SVC=$PWD/supervisor

if [ -d $SVC ]; then
  make -C $SVC -f Makefile.svc clean
else
    cecho r "!!! Failed to detect SVC sources"
    exit 1
fi

mkdir -p usr/local/bin

cp Makefile.svc $SVC
cd $SVC

make CROSS_COMPILE=$CROSS_COMPILE UCI_INC=$ROOTDIR/usr/local/include \
     UBOX_INC=$ROOTDIR/usr/local/include/libubox UCI_LIB=$ROOTDIR/usr/local/lib -f Makefile.svc
if [ $? -ne 0 ]; then
  cecho r "!!! Failed to build capture"
  exit 1
fi

cp supervisor $ROOTDIR/usr/local/bin

cecho g "!!! suupervisor done !!!\n"

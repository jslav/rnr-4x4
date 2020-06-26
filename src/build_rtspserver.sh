#!/bin/sh

. ./toolchain_path
. ./color

ROOTDIR=$PWD
RTSPDIR=$ROOTDIR/v4l2rtspserver
PATCH=$ROOTDIR/001-rtp-send.patch

cecho y "*** Building v4l2rtspserver ***"

if [ -d $RTSPDIR ]; then
  make -C $RTSPDIR clean
  rm -rf $RTSPDIR/CMakeFiles $RTSPDIR/CMakeCache.txt
else
  git clone https://github.com/mpromonet/v4l2rtspserver.git
  if [ $? -ne 0 ]; then
    cecho r "!!! Failed to clone RTSP server"
    exit 1
  fi
  
  cd $RTSPDIR
  git init modules
  git submodule update
  cd $ROOTDIR
fi

mkdir -p usr/local/bin
cd $RTSPDIR
patch -p1 -N --dry-run --silent < $PATCH 2>/dev/null
if [ $? -eq 0 ]; then
    #apply the patch
    patch -p1 -N < $PATCH
fi

cmake -DCMAKE_TOOLCHAIN_FILE=raspberry.toolchain .
if [ $? -ne 0 ]; then
  cecho r "!!! Failed to build with cmake"
  exit 1
fi

make
if [ $? -ne 0 ]; then
  cecho r "!!! Failed to build v4l2rtspserver"
  exit 1
fi

cp v4l2rtspserver $ROOTDIR/usr/local/bin

cecho g "!!! v4l2rtspserver done !!!\n"

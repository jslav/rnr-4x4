#!/bin/sh

. ./toolchain_path
. ./color

ROOTDIR=$PWD
RTSPDIR=$ROOTDIR/v4l2rtspserver

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
fi

mkdir -p usr/local/bin

cd $RTSPDIR
git init modules
git submodule update

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

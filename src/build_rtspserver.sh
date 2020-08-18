#!/bin/sh

. ./toolchain_path
. ./color

P=$(cat ./toolchain_path)
PP=$(echo $P | sed 's/.*://')
CROSS_COMPILE=$PP/arm-linux-gnueabihf-

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
  git checkout "1e37de89f8672cc14eea592eb90465b86925b8e8"
  if [ $? -ne 0 ]; then
    cecho r "!!! Failed to checkout RTSP server tag"
    exit 1
  fi
  
  cp $ROOTDIR/rtsp_git_config  $RTSPDIR/.git/config
  git init modules
  git submodule update
  
  cd $ROOTDIR
fi

mkdir -p usr/local/bin
cd $RTSPDIR

if [ ! -d live ]; then
#  wget http://www.live555.com/liveMedia/public/live555-latest.tar.gz
  wget https://download.videolan.org/pub/contrib/live555/live.2020.06.25.tar.gz
  if [ $? -ne 0 ]; then
    cecho r "!!! Failed to download live555 library"
    exit 1
  fi
#  tar xvf $RTSPDIR/live555-latest.tar.gz
  tar xvf live.2020.06.25.tar.gz
  if [ $? -ne 0 ]; then
    cecho r "!!! Failed to download live555 library"
    exit 1
  fi
fi

patch -p1 -N --dry-run --silent < $PATCH 2>/dev/null
if [ $? -eq 0 ]; then
    patch -p1 -N < $PATCH
fi

cmake -DCMAKE_C_COMPILER=$CROSS_COMPILEgcc -DCMAKE_CXX_COMPILER=CROSS_COMPILEg++ -DCMAKE_TOOLCHAIN_FILE=raspberry.toolchain .
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

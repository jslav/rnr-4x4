#!/bin/sh

ROOTDIR=$PWD

BUILDDIR=$ROOTDIR/build
SRCDIR=$ROOTDIR/src
OUTPUT=$ROOTDIR/output

# check toolchain
. $SRCDIR/toolchain_path
. $SRCDIR/color

arm-linux-gnueabihf-gcc --version
if [ $? -ne 0 ]; then
  cecho r "!!! Can't find arm-linux-gnueabihf toolchain. You have to install arm-linux-gnueabihf toolchain first"
  cecho r "The best choice would be: Linaro GCC 2013.04 4.7.3 20130328"
  exit 1
fi

# check cmake
cmake --version
if [ $? -ne 0 ]; then
  cecho r "!!! Can't find CMake. You have to install CMake first"
  exit 1
fi

# check cmake
git --version
if [ $? -ne 0 ]; then
  cecho r "!!! Can't find git. You have to install git first"
  exit 1
fi


if [ -d $BUILDDIR ]; then
  cp -n -r $SRCDIR/* $BUILDDIR
else
  mkdir -p $BUILDDIR
  cp -r $SRCDIR/* $BUILDDIR
fi

if [ ! -d $OUTPUT ]; then
  mkdir -p $OUTPUT
fi

cd $BUILDDIR

FILES="build_uci.sh build_capture.sh build_svc.sh build_rtspserver.sh build_kernel.sh build_loopback.sh"

for F in $FILES
do
	cecho y "Processing $F"
	"$PWD/$F"
	if [ $? -ne 0 ]; then
	  cecho r "!!! Failed on processing $F"
	  exit
	fi
done

cp $BUILDDIR/modules $BUILDDIR/etc/modules

tar -cf $OUTPUT/rootfs.tar -C $BUILDDIR etc usr lib
cp $BUILDDIR/uImage $OUTPUT

cecho g "Build done for $OUTPUT/rootfs.tar"



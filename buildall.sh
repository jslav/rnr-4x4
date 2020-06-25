#!/bin/sh

ROOTDIR=$PWD

BUILDDIR=$ROOTDIR/build
SRCDIR=$ROOTDIR/src
OUTPUT=$ROOTDIR/output
TOOLCHAIN=gcc-linaro-arm-linux-gnueabihf-4.8-2013.07-1_linux
UDEVDIR=$BUILDDIR/etc/udev/rules.d 
LDSODIR=$BUILDDIR/etc/ld.so.conf.d
MODPROBEDIR=$BUILDDIR/etc/modprobe.d
DEFAULTSDIR=$BUILDDIR/etc/defaults/

. $SRCDIR/color

# check cmake
cecho y "Checking CMake..."
cmake --version
if [ $? -ne 0 ]; then
  cecho r "!!! Can't find CMake. You have to install CMake first"
  exit 1
fi
cecho g "CMake OK"

# check cmake
cecho y "Checking git..."
git --version
if [ $? -ne 0 ]; then
  cecho r "!!! Can't find git. You have to install git first"
  exit 1
fi
cecho g "git OK"

if [ -d $BUILDDIR ]; then
  cp -n -r $SRCDIR/* $BUILDDIR
else
  mkdir -p $BUILDDIR
  cp -r $SRCDIR/* $BUILDDIR
fi

if [ ! -d $OUTPUT ]; then
  mkdir -p $OUTPUT
fi

if [ ! -d $BUILDDIR/toolchain ]; then
    cecho y "Installing toolchain: $TOOLCHAIN"
    mkdir -p $BUILDDIR/toolchain
    tar -C $BUILDDIR/toolchain -xf $ROOTDIR/toolchain/$TOOLCHAIN.tar.xz
fi

echo "PATH=\$PATH:$BUILDDIR/toolchain/$TOOLCHAIN/bin" > $BUILDDIR/toolchain_path

# check toolchain
. $BUILDDIR/toolchain_path

cecho y "Checking ARM toolchain..."
arm-linux-gnueabihf-gcc --version
if [ $? -ne 0 ]; then
  cecho r "!!! Can't find arm-linux-gnueabihf toolchain. You have to install arm-linux-gnueabihf toolchain first"
  cecho r "The best choice would be: Linaro GCC 2013.04 4.7.3 20130328"
  exit 1
fi
cecho g "ARM toolchain OK"

if [ ! -d $UDEVDIR ]; then
  mkdir -p $UDEVDIR
fi

if [ ! -d $LDSODIR ]; then
  mkdir -p $LDSODIR
fi

if [ ! -d $MODPROBEDIR ]; then
  mkdir -p $MODPROBEDIR
fi

if [ ! -d $DEFAULTSDIR ]; then
  mkdir -p $DEFAULTSDIR
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
cp $BUILDDIR/50-mali.rules $UDEVDIR
cp $BUILDDIR/usr-local-lib.conf $LDSODIR
cp $BUILDDIR/rc.local $BUILDDIR/etc
cp $BUILDDIR/gc2035.conf $MODPROBEDIR
cp $BUILDDIR/irqbalance $DEFAULTSDIR

cecho y "Making rootfs..."
tar -cf $OUTPUT/rootfs.tar -C $BUILDDIR etc usr lib
cp $BUILDDIR/uImage $OUTPUT
cecho g "rootfs OK"


cecho g "Build done for $OUTPUT/rootfs.tar"



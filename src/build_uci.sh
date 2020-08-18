#! /bin/sh

. ./toolchain_path
. ./color

P=$(cat ./toolchain_path)
PP=$(echo $P | sed 's/.*://')
CROSS_COMPILE=$PP/arm-linux-gnueabihf-

ROOTDIR=$PWD
LIBUBOXDIR=$ROOTDIR/libubox
UCIDIR=$ROOTDIR/uci

cecho y "*** Building UCI ***"

# 1. clone libubox
if [ -d $LIBUBOXDIR ]; then
  make -C $LIBUBOXDIR -f Makefile.ubox CROSS_COMPILE=$CROSS_COMPILE clean
else
  git clone https://github.com/libkit/libubox.git
  if [ $? -ne 0 ]; then
    cecho r "!!! Failed to clone libubox"
    exit 1
  fi
fi


# 3. create library path
mkdir -p usr/local/include/libubox
mkdir -p usr/local/lib
mkdir -p usr/local/bin
mkdir -p etc/config
if [ $? -ne 0 ]; then
  cecho r "!!! Failed to create dir"
  exit
fi

# 4. build and install libubox
cp Makefile.ubox $LIBUBOXDIR
cd $LIBUBOXDIR
make -f Makefile.ubox CROSS_COMPILE=$CROSS_COMPILE
if [ $? -ne 0 ]; then
  cecho r "!!! Failed to build libubox"
  exit 1
fi
cp *.so *.a $ROOTDIR/usr/local/lib
cp *.h $ROOTDIR/usr/local/include/libubox

cd $ROOTDIR

cecho g "libubox DONE"

# 2. clone libuci
if [ -d $UCIDIR ]; then
  make -C $UCIDIR -f Makefile.uci CROSS_COMPILE=$CROSS_COMPILE UBOXINC=$ROOTDIR/usr/local/include UBOXLIB=$ROOTDIR/usr/local/lib clean
else
  git clone git://nbd.name/uci.git
  if [ $? -ne 0 ]; then
    cecho r "!!! Failed to clone uci"
    exit 1
  fi
fi

# 5. build uci
cp Makefile.uci $UCIDIR
cd $UCIDIR
touch uci_config.h
make -f Makefile.uci CROSS_COMPILE=$CROSS_COMPILE UBOXINC=$ROOTDIR/usr/local/include UBOXLIB=$ROOTDIR/usr/local/lib
if [ $? -ne 0 ]; then
  echo "!!! Failed to build uci"
  exit 1
fi
cp *.so *.a $ROOTDIR/usr/local/lib
cp *.h $ROOTDIR/usr/local/include
cp uci $ROOTDIR/usr/local/bin

cp $ROOTDIR/system $ROOTDIR/etc/config

cecho g "!!! UCI done !!!\n"

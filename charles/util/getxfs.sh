#!/bin/sh
git clone git://oss.sgi.com/xfs/cmds/xfsprogs
cd xfsprogs
make
cd ..
echo "Copying header files..."
cp xfsprogs/include/*.h ../include
echo "Copying dynamically linked library..."
cp xfsprogs/libxfs/.libs/libxfs.so.0.0.0 ../lib/libxfs.so
echo "Copying static linked library..."
cp xfsprogs/libxfs/.libs/libxfs.a ../lib/
echo "Cleaning up..."
rm -Rf xfsprogs

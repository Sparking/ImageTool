
cd 35_cairo
gzip -d -f -c cairo-1.10.2.tar.gz > cairo-1.10.2.tar
tar -xf cairo-1.10.2.tar.gz
cd cairo-1.10.2


echo Patching source...

cd ..
patch -p0 < cairo-1.10.2-win32_surfaces.patch
cd cairo-1.10.2

echo Correct configure so that make does not fail on some warnings...

mv configure configure.old
cp ../configure configure
./configure --host=x86_64-w64-mingw32 --enable-win32=yes --enable-win32-font=yes --enable-png=yes --enable-ft=yes --enable-fc=yes --prefix=$PREFIX

echo Correct libtool for the undefined symbols problem,
echo which prevents shared DLL creation

mv libtool libtool.old
cp ../libtool libtool

echo Compile...

make clean
make 2>&1 | tee ../../logs/35_cairo-make.log
make install 2>&1 | tee ../../logs/35_cairo-makeinstall.log

echo Copy cairo.def to its final location...

cp src/cairo.def $PREFIX/lib


cd ..
rm -rf cairo-1.10.2
rm -f cairo-1.10.2.tar
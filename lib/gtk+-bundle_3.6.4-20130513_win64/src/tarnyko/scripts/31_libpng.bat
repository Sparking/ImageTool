
cd 31_libpng
xz -d -k -f libpng-1.5.14.tar.xz
tar -xf libpng-1.5.14.tar
cd libpng-1.5.14


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/31_libpng-make.log
make install 2>&1 | tee ../../logs/31_libpng-makeinstall.log


cd ..
rm -rf libpng-1.5.14
rm -f libpng-1.5.14.tar
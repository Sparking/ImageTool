
cd 61_libcroco
xz -d -k -f libcroco-0.6.8.tar.xz
tar -xf libcroco-0.6.8.tar
cd libcroco-0.6.8


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/61_libcroco-make.log
make install 2>&1 | tee ../../logs/61_libcroco-makeinstall.log


cd ..
rm -rf libcroco-0.6.8
rm -f libcroco-0.6.8.tar
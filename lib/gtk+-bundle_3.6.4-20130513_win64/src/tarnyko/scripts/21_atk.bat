
cd 21_atk
xz -d -k -f atk-2.6.0.tar.xz
tar -xf atk-2.6.0.tar
cd atk-2.6.0


echo Deleting tainted libtool files...

rm -f $PREFIX/lib/*.la

echo Compile...

./configure --host=x86_64-w64-mingw32 --enable-static --enable-shared --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/21_atk-make.log
make install 2>&1 | tee ../../logs/21_atk-makeinstall.log


cd ..
rm -rf atk-2.6.0
rm -f atk-2.6.0.tar
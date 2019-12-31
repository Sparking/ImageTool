
cd 1_zlib
unzip -o zlib127.zip
cd zlib-1.2.7


echo Compile...

cp win32/Makefile.gcc Makefile
make clean
make 2>&1 | tee ../../logs/1_zlib-make.log

echo Copying compilation result to the right place...

cp zlib1.dll $PREFIX/bin
cp zconf.h $PREFIX/include
cp zlib.h $PREFIX/include
cp libz.a $PREFIX/lib
cp libz.dll.a $PREFIX/lib

echo Manually copying pkg-config "zlib.pc" file to the right place...

cd ..
cp zlib.pc $PREFIX/lib/pkgconfig/zlib.pc



rm -rf zlib-1.2.7
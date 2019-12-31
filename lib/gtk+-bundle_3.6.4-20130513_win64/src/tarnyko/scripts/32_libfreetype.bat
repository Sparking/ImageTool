
cd 32_libfreetype
bzip2 -d -k -f freetype-2.4.11.tar.bz2
tar -xf freetype-2.4.11.tar
cd freetype-2.4.11


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/32_libfreetype-make.log
make install 2>&1 | tee ../../logs/32_libfreetype-makeinstall.log


cd ..
rm -rf freetype-2.4.11
rm -f freetype-2.4.11.tar
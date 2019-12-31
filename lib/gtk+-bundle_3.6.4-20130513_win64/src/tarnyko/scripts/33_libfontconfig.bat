
cd 33_libfontconfig
gzip -d -f -c fontconfig-2.10.2.tar.gz > fontconfig-2.10.2.tar
tar -xf fontconfig-2.10.2.tar
cd fontconfig-2.10.2


echo To fix ftheader.h location detection by make...

ln -s $PREFIX/include/freetype2/freetype $PREFIX/include/freetype

echo Compile...

./configure --host=x86_64-w64-mingw32 --enable-libxml2 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/33_libfontconfig-make.log
make install 2>&1 | tee ../../logs/33_libfontconfig-makeinstall.log

echo Doc installation partially fails at the end
echo so we install the .pc file manually

cp fontconfig.pc $PREFIX/lib/pkgconfig


cd ..
rm -rf fontconfig-2.10.2
rm -f fontconfig-2.10.2.tar
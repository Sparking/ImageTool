
cd 62_librsvg
xz -d -k -f librsvg-2.36.4.tar.xz
tar -xf librsvg-2.36.4.tar
cd librsvg-2.36.4


echo Compile...

./configure --host=x86_64-w64-mingw32 --disable-gtk-theme --enable-pixbuf-loader --enable-introspection=no --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/62_librsvg-make.log
make install 2>&1 | tee ../../logs/62_librsvg-makeinstall.log


cd ..
rm -rf librsvg-2.36.4
rm -f librsvg-2.36.4.tar

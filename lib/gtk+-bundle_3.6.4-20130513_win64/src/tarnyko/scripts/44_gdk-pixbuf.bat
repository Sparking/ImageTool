
cd 44_gdk-pixbuf
xz -d -k -f gdk-pixbuf-2.26.5.tar.xz
tar -xf gdk-pixbuf-2.26.5.tar
cd gdk-pixbuf-2.26.5


echo Compile...

./configure --host=x86_64-w64-mingw32 --with-included-loaders --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/44_gdk-pixbuf-make.log
make install 2>&1 | tee ../../logs/44_gdk-pixbuf-makeinstall.log


cd ..
rm -rf gdk-pixbuf-2.26.5
rm -f gdk-pixbuf-2.26.5.tar
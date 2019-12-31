
cd 13_pkg-config
gzip -d -f -c pkg-config-0.28.tar.gz > pkg-config-0.28.tar
tar -xf pkg-config-0.28.tar
cd pkg-config-0.28


echo To detect the formerly installed GLib...

export GLIB_CFLAGS="-I$PREFIX/include/glib-2.0 -I$PREFIX/lib/glib-2.0/include"
export GLIB_LIBS=-lglib-2.0

echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/13_pkg-config-make.log
make install 2>&1 | tee ../../logs/13_pkg-config-makeinstall.log


unset GLIB_CFLAGS
unset GLIB_LIBS
cd ..
rm -rf pkg-config-0.28
rm -f pkg-config-0.28.tar
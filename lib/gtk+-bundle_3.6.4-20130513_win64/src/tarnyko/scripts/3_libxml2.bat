
cd 3_libxml2
gzip -d -f -c libxml2-2.8.0.tar.gz > libxml2-2.8.0.tar
tar -xf libxml2-2.8.0.tar.gz
cd libxml2-2.8.0


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/3_libxml2-make.log
make install 2>&1 | tee ../../logs/3_libxml2-makeinstall.log


cd ..
rm -rf libxml2-2.8.0
rm -f libxml2-2.8.0.tar
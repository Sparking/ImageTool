
cd 43_libtiff
gzip -d -f -c tiff-4.0.3.tar.gz > tiff-4.0.3.tar
tar -xf tiff-4.0.3.tar
cd tiff-4.0.3


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/43_libtiff-make.log
make install 2>&1 | tee ../../logs/43_libtiff-makeinstall.log


cd ..
rm -rf tiff-4.0.3
rm -f tiff-4.0.3.tar
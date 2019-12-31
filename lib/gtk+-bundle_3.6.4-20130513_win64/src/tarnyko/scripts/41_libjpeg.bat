
cd 41_libjpeg
gzip -d -f -c jpegsrc.v9.tar.gz > jpegsrc.v9.tar
tar -xf jpegsrc.v9.tar
cd jpeg-9


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/41_libjpeg-make.log
make install 2>&1 | tee ../../logs/41_libjpeg-makeinstall.log


cd ..
rm -rf jpeg-9
rm -f jpegsrc.v9.tar
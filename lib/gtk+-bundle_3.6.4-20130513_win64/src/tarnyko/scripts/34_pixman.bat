
cd 34_pixman
gzip -d -f -c pixman-0.26.0.tar.gz > pixman-0.26.0.tar
tar -xf pixman-0.26.0.tar
cd pixman-0.26.0


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/34_pixman-make.log
make install 2>&1 | tee ../../logs/34_pixman-makeinstall.log


cd ..
rm -rf pixman-0.26.0
rm -f pixman-0.26.0.tar
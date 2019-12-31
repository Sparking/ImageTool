
cd 71_icon-naming-utils
bzip2 -d -f -c icon-naming-utils-0.8.90.tar.bz2 > icon-naming-utils-0.8.90.tar
tar -xf icon-naming-utils-0.8.90.tar
cd icon-naming-utils-0.8.90


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make 2>&1 | tee ../../logs/71_icon-naming-utils-make.log
make install 2>&1 | tee ../../logs/71_icon-naming-utils-makeinstall.log


cd ..
rm -rf icon-naming-utils-0.8.90
rm -f icon-naming-utils-0.8.90.tar
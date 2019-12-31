
cd 42_libjasper
unzip -o jasper-1.900.1.zip
cd jasper-1.900.1


echo Patch source for the undefined reference to sleep problem

cd ..
patch -p0 < jasper-1.900.1-win32_sleep.patch
cd jasper-1.900.1

./configure --host=x86_64-w64-mingw32 --enable-shared --enable-static --prefix=$PREFIX

echo Correct libtool for the undefined symbols problem,
echo which prevents shared DLL creation

mv libtool libtool.old
cp ../libtool libtool

echo Compile...

make clean
make 2>&1 | tee ../../logs/42_libjasper-make.log
make install 2>&1 | tee ../../logs/42_libjasper-makeinstall.log


cd ..
rm -rf jasper-1.900.1
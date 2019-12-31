
cd 73_gnome-icon-theme
xz -d -k -f gnome-icon-theme-3.6.2.tar.xz
tar -xf gnome-icon-theme-3.6.2.tar
cd gnome-icon-theme-3.6.2


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make 2>&1 | tee ../../logs/73_gnome-icon-theme-make.log
make install 2>&1 | tee ../../logs/73_gnome-icon-theme-makeinstall.log


cd ..
rm -rf gnome-icon-theme-3.6.2
rm -f gnome-icon-theme-3.6.2.tar
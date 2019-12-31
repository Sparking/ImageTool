
cd 74_gnome-icon-theme-symbolic
xz -d -k -f gnome-icon-theme-symbolic-3.6.2.tar.xz
tar -xf gnome-icon-theme-symbolic-3.6.2.tar
cd gnome-icon-theme-symbolic-3.6.2


echo Compile...

./configure --host=x86_64-w64-mingw32 --prefix=$PREFIX
make 2>&1 | tee ../../logs/74_gnome-icon-theme-symbolic-make.log
make install 2>&1 | tee ../../logs/74_gnome-icon-theme-symbolic-makeinstall.log


cd ..
rm -rf gnome-icon-theme-symbolic-3.6.2
rm -f gnome-icon-theme-symbolic-3.6.2.tar
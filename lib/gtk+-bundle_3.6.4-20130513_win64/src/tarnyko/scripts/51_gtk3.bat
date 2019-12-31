
cd 51_gtk3
xz -d -k -f gtk+-3.6.4.tar.xz
tar -xf gtk+-3.6.4.tar
cd gtk+-3.6.4


echo Save current CFLAGS var, we will restore it later...

export CFLAGS_SAVE="$CFLAGS"

echo Default Windows version on MinGW is NT4...
echo Here we need to redefine it to XP, as new GTK code
echo uses Monitor stuff depending on XP ifndefs 

export CFLAGS="$CFLAGS -DWINVER=0x0501"

cd ..

echo Patch makefile to NOT build Unix-only gtk-launch

patch -p0 < gtk+-3.6.4-win32_gtk-launch.patch

echo Patch GtkAssistant text to be readable with gray background
echo Bugzilla 696171

patch -p0 < gtk+-3.6.4-win32_gtkassistant.patch

echo Patch GtkNotebook tabs to render correctly
echo thanks to Andy Spencer, Bugzilla 691678

patch -p0 < gtk+-3.6.4-win32_gtknotebook.patch

echo Patch GtkSpinner to animate again with win32 theme
echo thanks to Martin Schlemmer, Bugzilla 696202

patch -p0 < gtk+-3.6.4-win32_gtkspinner.patch

cd gtk+-3.6.4


echo Compile...

./configure --host=x86_64-w64-mingw32 --enable-win32-backend --with-included-immodules --prefix=$PREFIX
make clean
make 2>&1 | tee ../../logs/51_gtk3-make.log
make install 2>&1 | tee ../../logs/51_gtk3-makeinstall.log


export CFLAGS="$CFLAGS_SAVE"
unset CFLAGS_SAVE
cd ..
rm -rf gtk+-3.6.4
rm -f gtk+-3.6.4.tar
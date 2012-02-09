aclocal
libtoolize --force --copy
autoheader
automake --add-missing --foreign
autoconf
moc synth_qt_gui.h -o synth_qt_gui.moc.cpp
./configure
make
#sudo checkinstall

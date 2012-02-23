sudo rm -R  /usr/local/lib/dssi/*
make clean
sleep 15
aclocal
sleep 15
libtoolize --force --copy
sleep 15
autoheader
sleep 15
automake --add-missing --foreign
sleep 15
autoconf
sleep 15
moc synth_qt_gui.h -o synth_qt_gui.moc.cpp
sleep 15
./configure
sleep 15
make
sleep 15
sudo make install
#sudo checkinstall

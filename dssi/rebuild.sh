#!/usr/bin/bash

#Run this script to rebuild everything.  There seems to be a problem with either Qt or GCC
#that results in only a popping noise coming out when you play a note due to an unexpected null value somewhere.
#This should fix it, unless there really is an error in your code.
./configure --enable-libtool-lock
echo -e "\nconfigure finished\n"
sleep 15
make clean
echo -e "\nmake clean finished\n"
sleep 15
cd examples
rm synth_qt_gui.moc.cpp
moc synth_qt_gui.h -o synth_qt_gui.moc.cpp
echo -e "\nmoc finished\n"
sleep 15
cd ..
make
echo -e "\nmake finished\n"
sleep 15
gksudo make install
#~/Desktop/libmodsynth-git/libmodsynth/jack-dssi-host/jack-dssi-host /usr/local/lib/dssi/synth.so

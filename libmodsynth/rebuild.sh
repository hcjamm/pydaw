#!/usr/bin/bash

#Run this script to rebuild everything.  There seems to be a problem with either Qt or GCC
#that results in only a popping noise coming out when you play a note due to an unexpected null value somewhere.
#This should fix it, unless there really is an error in your code.
cd examples
moc synth_qt_gui.h -o synth_qt_gui.moc.cpp
cd ..
make clean
./configure
make
gksudo make install
#~/Desktop/libmodsynth-git/libmodsynth/jack-dssi-host/jack-dssi-host /usr/local/lib/dssi/synth.so

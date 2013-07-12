#!/bin/bash

ROOT=/Developer/SDKs/MacOSX10.4u.sdk

export SNDFILE_LIBS=$ROOT/usr/lib/libsndfile.a
export SNDFILE_CFLAGS=-I$ROOT/usr/include

export MAD_LIBS=$ROOT/usr/local/lib/libmad.a

VSTROOT=../../../vst2.4
export VSTSDK_DIR="$VSTROOT/sdk"
export VSTPLUG_DIR="$VSTROOT/plugin"
export VSTGUI_DIR="$VSTROOT/vstgui"
export VST_CFLAGS="-I$VSTSDK_DIR -I$VSTPLUG_DIR -I$VSTGUI_DIR"
export VST_LIBS="-framework Carbon -framework QuickTime -framework System -framework ApplicationServices"

export WX_PATH=/usr/local/bin

#./configure --enable-mp3 --enable-sndfile --enable-portaudio --enable-wx --enable-static --disable-dependency-tracking --enable-multithreaded --disable-debug --enable-universal_binary --enable-vst && make clean && make

./configure --enable-mp3 --enable-sndfile --disable-portaudio --disable-wx --enable-static --enable-dependency-tracking --enable-multithreaded --disable-debug --disable-universal_binary --enable-vst && make clean && make

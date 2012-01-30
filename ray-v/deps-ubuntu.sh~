#!/usr/bin/bash

#This is a script to install dependencies for Ubuntu, since I intend to package LibModSynth as an all-in-one
#tar.gz file bundled with the no-installer version of Netbeans.

#I am open to dependency scripts for other distros if anybody wants to contribute one, please test it thoroughly before sending it to me

#dependencies, current as of Ubuntu 12.04 alpha
gksudo apt-get install liblo-dev dssi-dev ladspa-sdk libasound2-dev g++ qtractor qjackctl qt4-designer libjack-jackd2-dev libsndfile1-dev libsamplerate0-dev libtool autoconf openjdk-7-jre libsm-dev
#add your user to the audio group to allow proper access to Qjackctl
gksudo usermod -g audio $USER

#See rebuild.sh for how to build and run the example plugin

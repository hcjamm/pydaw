#!/usr/bin/bash

#This is a script to install dependencies for Ubuntu, since I intend to package LibModSynth as an all-in-one
#tar.gz file bundled with the no-installer version of Netbeans.

#dependencies
sudo apt-get install liblo-dev dssi-dev ladspa-sdk libasound2-dev g++ qtractor qjackctl qt4-designer

#add your user to the audio group to allow proper access to Qjackctl
sudo usermod -g audio $USER
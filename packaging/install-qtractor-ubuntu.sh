#!/usr/bin/bash

#usage:  sudo bash ./install-qtractor.sh

#This script installs Qtractor and creates a desktop shortcut.
#You can use the version of Qtractor that comes with your distro, the LibModSynth project
#merely provides pre-compiled binaries of bleeding-edge versions of various 3rd party components,
#as a convenience, as PPAs can be difficult for the average user to use.

#Licensing information for Qtractor can be found at qtractor.sourceforge.net, the LibModSynth project 
#is not affiliated with, and does not provide any kind of support for Qtractor.

cp ./qtractor /usr/bin/qtractor
ln -s /usr/bin/qtractor $HOME/Desktop/qtractor

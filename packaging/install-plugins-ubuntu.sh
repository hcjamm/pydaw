#!/usr/bin/bash


#usage:  sudo bash ./install-ubuntu.sh

#Feel free to contribute scripts or packages for other distros, I'll include them in the main release

#This script installs the plugins in Ubuntu, along with the required dependencies

apt-get install qjackctl

usermod -g audio $USER

PLUGIN_DIR="/usr/local/lib/dssi"

mkdir -p $PLUGIN_DIR

cp ./dssi/* $PLUGIN_DIR/

echo -e "Installation complete.\n\nYou must add this path to your host's DSSI plugin directories before it will recognize the plugins:\n"
echo $PLUGIN_DIR




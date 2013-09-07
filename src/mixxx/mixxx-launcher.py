#!/usr/bin/python

import os

pydaw_data = "/media/pydaw_data"
mixxx_home = pydaw_data + "/mixxx"
mixxx_link = "/home/ubuntu/.mixxx"

if not os.path.isdir("/media/pydaw_data"):
    os.system("gksudo mkdir " + pydaw_data)
    os.system("gksudo mount /dev/disk/by-label/pydaw_data " + pydaw_data)

if not os.path.isdir(mixxx_home):
    os.mkdir(mixxx_home)

if not os.path.islink(mixxx_link):
    os.symlink(mixxx_home, mixxx_link)

os.system("pasuspender mixxx")
#!/usr/bin/python3

import os

f_base_dir = os.path.dirname(__file__)
f_src_dir = "{}/src".format(f_base_dir)
f_bin_dir = "{}/bin".format(f_base_dir)

if os.path.isdir(f_bin_dir):
    os.system("rm -rf '{}'".format(f_bin_dir))

os.system("mkdir '{}'".format(f_bin_dir))

for f_file is os.listdir(f_src_dir):
    if os.path.isdir(f_file):
        f_dest_dir = "{}/{}/LC_MESSAGES".format(f_bin_dir, f_file)
        os.system("mkdir -p '{}'".format(f_dest_dir)
        if os.system("msgfmt -o '{}/pydaw4.mo' pydaw4.po".format(
        f_dest_dir)) != 0:
            print("\n\nERROR: {}/pydaw4.po returned non-zero \n\n".format(
                f_file))


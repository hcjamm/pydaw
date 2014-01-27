#!/usr/bin/python3

import os
import sys

f_base_dir = os.path.dirname(__file__)
f_src_dir = "{}/src".format(f_base_dir)
f_bin_dir = "{}/bin/locale".format(f_base_dir)

if os.path.isdir(f_bin_dir):
    os.system("rm -rf '{}'".format(f_bin_dir))

os.system("mkdir '{}'".format(f_bin_dir))
os.system("chmod 777 '{}'".format(f_bin_dir))

for f_file in os.listdir(f_src_dir):
    f_dest_dir = "{}/{}/LC_MESSAGES".format(f_bin_dir, f_file)
    os.system("mkdir -p '{}'".format(f_dest_dir))
    f_cmd = "msgfmt -o '{}/pydaw4.mo' '{}/{}/pydaw4.po'".format(f_dest_dir, f_src_dir, f_file)
    f_result = os.system(f_cmd)
    if f_result != 0:
        print("\n\nERROR: {}/pydaw4.po returned non-zero \n\n".format(f_file))
        exit(9999)

if len(sys.argv) > 1:
    f_install_dir = sys.argv[1]
    f_share_dir = "{}/share".format(f_install_dir)
    if not os.path.isdir(f_share_dir):
        os.system("mkdir -p '{}'".format(f_share_dir))
    os.system("cp -r '{}' '{}'".format(f_bin_dir, f_share_dir))

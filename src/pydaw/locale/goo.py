#!/usr/bin/python3
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import os

f_dir_name = os.path.abspath(os.path.dirname(__file__))
f_po_file = os.path.abspath("{}/../pydaw4.po".format(f_dir_name))
f_src_dir = "{}/src".format(f_dir_name)
f_goo_dir = "{}/goo".format(f_dir_name)

with open(f_po_file) as f_file:
    f_orig = f_file.read()

#Remove those stupid Gedit backup files that it places there
#even if you turn off creating backup files...
os.system("rm -f '{}'/*~".format(f_goo_dir))

f_dash_line = "\n" + "-" * 6 + "\n"

for f_file in os.listdir(f_goo_dir):
    with open("{}/{}".format(f_goo_dir, f_file)) as f_trans_file:
        f_new = f_trans_file.read()
        f_new = f_new.replace("{ }", "{}")
        f_new = f_new.replace(" .", ".")
        f_new = f_new.replace(" ( ", "(")
        f_new = f_new.replace(" ) ", ")")
        f_new = f_new.replace("\\ n", "\\n")
        f_new = f_new.replace("\\ N", "\\n")
        f_new_list = []
        f_dashes = "-" * 6
        for f_line in f_new.split("\n"):
            if f_dashes in f_line:
                f_new_list.append(f_line)
            else:
                f_line2 = f_line.strip('"')
                f_line2 = f_line.strip()
                f_line2 = f_line2.replace('"', "'")
                f_new_list.append('"{}"'.format(f_line))
        f_new = "\n".join(f_new_list)
    f_new_dir = "{}/{}".format(f_src_dir, f_file)
    if not os.path.isdir(f_new_dir):
        os.system('mkdir "{}"'.format(f_new_dir))
    f_result = ""
    for k, v in zip(f_orig.split('msgstr '), f_new.split(f_dash_line)):
        f_result += ("{}msgstr {}".format(k, v))

    f_new_po = "{}/pydaw4.po".format(f_new_dir)
    with open(f_new_po, "w") as f_new_po_file:
        f_new_po_file.write(f_result)


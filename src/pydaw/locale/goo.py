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

f_dash_line = "\n" + "-" * 6 + "\n"

for f_file in os.listdir(f_goo_dir):
    with open("{}/{}".format(f_goo_dir, f_file)) as f_trans_file:
        f_new = f_trans_file.read()
    f_new_dir = "{}/{}".format(f_src_dir, f_file)
    if not os.path.isdir(f_new_dir):
        os.system('mkdir "{}"'.format(f_new_dir))
    f_result = ""
    for k, v in zip(f_orig.split('msgstr ""'), f_new.split(f_dash_line)):
        f_result += ("{}msgstr {}".format(k, v))
    #Fix common issues with Google translate results
    f_result = f_result.replace('msgstr " ', 'msgstr "')
    f_result = f_result.replace(" \"\n", "\"\n")
    f_result = f_result.replace("\n\" ", "\n\"")
    f_result = f_result.replace(" .", ".")
    f_result = f_result.replace(" ( ", "(")
    f_result = f_result.replace(" ) ", ")")

    f_new_po = "{}/pydaw4.po".format(f_new_dir)
    with open(f_new_po, "w") as f_new_po_file:
        f_new_po_file.write(f_result)


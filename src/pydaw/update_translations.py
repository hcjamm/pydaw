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
f_po_file = "{}/pydaw4.po".format(f_dir_name)
f_path = "{}/locale/src".format(f_dir_name)

def check_escape_sequences(a_file):
    f_dict = {}
    with open(a_file) as f_file:
        f_arr = f_file.read().split("msgid")
        for f_kvp in f_arr[1:]:
            f_key, f_val = f_kvp.split("msgstr")
            f_val = f_val.split("#")[0]
            f_dict[f_key.strip()] = f_val.strip()
    for k, v in f_dict.items():
        if v != '""':
            k_count = k.count("{}")
            v_count = v.count("{}")
            if k_count != v_count:
                print("\nwarning {}, count {} != {}\n\nmsgid {}\n\n msgstr {}\n\n".format(
                    a_file, k_count, v_count, k, v))

os.system('find python -iname "*.py" | xargs xgettext '
    '--from-code=UTF-8 --default-domain=pydaw4')

os.system("sed --in-place '{}' --expression=s/CHARSET/UTF-8/".format(f_po_file))

print(f_path)

for f_file in os.listdir(f_path):
    f_locale_file = "{}/{}/pydaw4.po".format(f_path, f_file)
    print(f_locale_file)
    if os.path.isfile(f_locale_file):
        os.system('msgmerge --update "{}" "{}"'.format(f_locale_file, f_po_file))
        check_escape_sequences(f_locale_file)


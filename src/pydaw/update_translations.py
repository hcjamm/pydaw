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
f_path = "{}/locale".format(f_dir_name)

os.system('find python -iname "*.py" | xargs xgettext '
    '--from-code=UTF-8 --default-domain=pydaw4')

for f_file in os.listdir(f_path):
    if os.path.isdir(f_file):
        f_locale_file = "{}/{}/LC_MESSAGES/pydaw4.po".format(f_path, f_file)
        if os.path.isfile(f_locale_file):
            os.system('msgmerge --update "{}" "{}"'.format(f_locale_file, f_po_file))


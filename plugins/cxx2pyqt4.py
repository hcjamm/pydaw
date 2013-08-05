#!/usr/bin/python
# -*- coding: utf-8 -*-
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

import sys, os

def print_help():
    print("Usage:  cxx2pyqt4.py file_to_convert.cpp")
    sys.exit(69)

if len(sys.argv) < 2:
    print_help()

f_arg_list = sys.argv[1:]


if not os.path.isfile(sys.argv[1]):
    print(sys.argv[1] + " is not a valid file")
    sys.exit(6969)

f_handle = open(sys.argv[1])
f_text = f_handle.read()
f_handle.close()

f_text = f_text.replace("!", " not ")

for f_i in range(30):
    f_text = f_text.replace("  ", " ")

f_text = f_text.replace("Qt::", "QtCore.Qt.")
f_text = f_text.replace("QString", "")
f_text = f_text.replace("this", "self")
f_text = f_text.replace("TRUE", "True")
f_text = f_text.replace("true", "True")
f_text = f_text.replace("FALSE", "False")
f_text = f_text.replace("false", "False")
f_text = f_text.replace("NULL", "None")
f_text = f_text.replace("switch", "f_switch = ")
f_text = f_text.replace("case", "elif f_switch == ")  #Not ideal, but basically works
f_text = f_text.replace("&&", "and")
f_text = f_text.replace("||", "or")
f_text = f_text.replace("static", "")
f_text = f_text.replace("protected", "")
f_text = f_text.replace("void", "def")
f_text = f_text.replace("::", ".")
f_text = f_text.replace("->", ".")
f_text = f_text.replace(";", "")
f_text = f_text.replace("/*", '"""')
f_text = f_text.replace("*/", '"""')
f_text = f_text.replace("//", "#")
f_text = f_text.replace("<<", ",")
f_text = f_text.replace("const", "")
f_text = f_text.replace("char", "")
f_text = f_text.replace("new", "")
f_text = f_text.replace("typedef", "")
f_text = f_text.replace("struct", "class")
f_text = f_text.replace(" argv", " sys.argv")
f_text = f_text.replace("argc", "len(sys.argv)")
f_text = f_text.replace(".0f", ".0")
f_text = f_text.replace("++", "+= 1")


#PyDAW Specific
f_text = f_text.replace("LMS_multieffect", "pydaw_modulex_single")
f_text = f_text.replace("LMS_knob_regular", "pydaw_knob_control")


f_scope_depth = 0

f_cxx_types = ["float", "double", "int", "long", "char"]

f_result = \
"""#!/usr/bin/python
#Converted from C++ to PyQt using the PyDAW project's cxx2pyqt4.py utility

from PyQt4 import QtGui, QtCore
import sys, os

"""

f_line_arr = f_text.split("\n")

for f_line in f_line_arr:
    f_new_line = f_line.strip()
    if "{" in f_new_line:
        f_new_line = f_line.replace("{", ":")
        f_scope_depth += 1
    if "}" in f_new_line:
        f_new_line = f_line.replace("}", ":")
        f_scope_depth -= 1
    if f_new_line == ":":
        f_result += f_new_line
        continue
    elif f_scope_depth == 0 and (f_new_line.startswith("int main") or f_new_line.startswith("void main")):
        f_new_line = 'if __name__ == "__main__":'
    elif f_new_line == "":
        continue
    elif f_new_line.startswith("connect"):
        f_new_line = "#" + f_new_line  #Just comment it out, these are all getting re-arranged anyways
    elif f_new_line.startswith("#define"):
        f_new_line = f_new_line.replace("#define", "")
        f_new_line_arr = f_new_line.split(" ")
        try:
            while True:
                f_new_line_arr.remove("")
        except:
            pass
        f_new_line_arr.insert(1, " = ")
        f_new_line = " ".join(f_new_line_arr)
    elif f_new_line == "" or f_new_line.startswith("#include"):
        continue
    else:
        f_new_line_arr = f_new_line.split(" ")
        f_result_arr = []
        for f_item in f_new_line_arr:
            f_item_stripped = f_item.strip()
            if f_item_stripped.startswith("Q"):
                f_result_arr.append("QtGui." + f_item_stripped)
            else:
                f_result_arr.append(f_item_stripped)
        f_new_line = " ".join(f_result_arr)
    for f_i in range(f_scope_depth):
        f_new_line = "    " + f_new_line
    f_result += "\n" + f_new_line

f_result = f_result.replace("::", ":")

f_output = open(sys.argv[1] + ".py", "w")
f_output.write(f_result)
f_output.close()


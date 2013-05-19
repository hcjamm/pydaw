# -*- coding: utf-8 -*-
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

from PyQt4 import QtGui, QtCore
from pydaw_project import pydaw_midi_track_count

def pydaw_linear_interpolate_gradient(a_pos):
    f_frac = a_pos % 1
    f_int = int(a_pos - f_frac)
    if f_int >= len(pydaw_rainbow_gradient) - 1:
        f_int -= len(pydaw_rainbow_gradient)
    f_red = ((pydaw_rainbow_gradient[f_int][0] - pydaw_rainbow_gradient[f_int + 1][0]) * f_frac) + pydaw_rainbow_gradient[f_int + 1][0]
    f_green = ((pydaw_rainbow_gradient[f_int][0] - pydaw_rainbow_gradient[f_int + 1][1]) * f_frac) + pydaw_rainbow_gradient[f_int + 1][1]
    f_blue = ((pydaw_rainbow_gradient[f_int][0] - pydaw_rainbow_gradient[f_int + 1][2]) * f_frac) + pydaw_rainbow_gradient[f_int + 1][2]
    if __name__ == "__main__":
        print("rgba(" + str(f_red) + ", " + str(f_green) + ", " + str(f_blue) + ", 255.0)")
    return (f_red, f_green, f_blue)

pydaw_g_hi = 210.0
pydaw_g_lo = 60.0
pydaw_rainbow_gradient = [(pydaw_g_lo, pydaw_g_hi, pydaw_g_lo),
                          (pydaw_g_lo, pydaw_g_lo, pydaw_g_hi),
                          (pydaw_g_hi, pydaw_g_lo, pydaw_g_hi),
                          (pydaw_g_hi, pydaw_g_lo, pydaw_g_lo),
                          (pydaw_g_hi, pydaw_g_hi, pydaw_g_lo)]

pydaw_rainbow_inc = 1.0 # (float(len(pydaw_rainbow_gradient))/float(pydaw_midi_track_count))
f_rainbox_pos = 0.5

pydaw_track_gradients = []

for f_i_gradient in range(pydaw_midi_track_count):
    f_colors = pydaw_linear_interpolate_gradient(f_rainbox_pos)
    f_gradient = QtGui.QLinearGradient(QtCore.QPointF(0, 0), QtCore.QPointF(0, 100))
    f_gradient.setColorAt(0, QtGui.QColor(f_colors[0], f_colors[1], f_colors[2]))
    f_colors = pydaw_linear_interpolate_gradient(f_rainbox_pos + 0.5)
    f_gradient.setColorAt(1, QtGui.QColor(f_colors[0], f_colors[1], f_colors[2]))
    pydaw_track_gradients.append(f_gradient)
    f_rainbox_pos += pydaw_rainbow_inc
    if f_rainbox_pos >= len(pydaw_rainbow_gradient):
        f_rainbox_pos -= len(pydaw_rainbow_gradient)

pydaw_region_gradient = QtGui.QLinearGradient(QtCore.QPointF(0, 0), QtCore.QPointF(100, 100))
pydaw_region_gradient.setColorAt(0, QtGui.QColor(190, 170, 40))
pydaw_region_gradient.setColorAt(1, QtGui.QColor(230, 221, 45))

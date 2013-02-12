# -*- coding: utf-8 -*-
"""
Provides the various standard gradients that aren't part of the CSS theme
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

for i in range(pydaw_midi_track_count):
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



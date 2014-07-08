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
from libpydaw.pydaw_project import pydaw_midi_track_count

pydaw_track_gradients = []

pydaw_g_hi = 100.0
pydaw_g_hi2 = 255.0
pydaw_g_lo = 36.0

pydaw_rainbow_gradient = \
    [((pydaw_g_lo, pydaw_g_hi, pydaw_g_lo),
      (pydaw_g_lo, pydaw_g_hi2, pydaw_g_lo)),
      ((pydaw_g_lo, pydaw_g_lo, pydaw_g_hi),
       (pydaw_g_lo, pydaw_g_lo, pydaw_g_hi2)),
      ((pydaw_g_hi, pydaw_g_lo, pydaw_g_hi),
       (pydaw_g_hi2, pydaw_g_lo, pydaw_g_hi2)),
      ((pydaw_g_hi, pydaw_g_lo, pydaw_g_lo),
       (pydaw_g_hi2, pydaw_g_lo, pydaw_g_lo)),
      ((pydaw_g_hi, pydaw_g_hi, pydaw_g_lo),
       (pydaw_g_hi2, pydaw_g_hi2, pydaw_g_lo))]

pydaw_region_gradient = QtGui.QLinearGradient(
    QtCore.QPointF(0, 0), QtCore.QPointF(0, 50))

pydaw_note_gradient = QtGui.QLinearGradient(
    QtCore.QPointF(0, 0), QtCore.QPointF(0, 12))

def pydaw_linear_interpolate_gradient(a_pos):
    f_frac = a_pos % 1
    f_int = int(a_pos - f_frac)
    if f_int >= len(pydaw_rainbow_gradient) - 1:
        f_int -= len(pydaw_rainbow_gradient)
    f_red = ((pydaw_rainbow_gradient[f_int][0][0] -
        pydaw_rainbow_gradient[f_int][1][0]) *
        f_frac) + pydaw_rainbow_gradient[f_int][1][0]
    f_green = ((pydaw_rainbow_gradient[f_int][0][1] -
        pydaw_rainbow_gradient[f_int][1][1]) *
        f_frac) + pydaw_rainbow_gradient[f_int][1][1]
    f_blue = ((pydaw_rainbow_gradient[f_int][0][2] -
        pydaw_rainbow_gradient[f_int ][1][2]) *
        f_frac) + pydaw_rainbow_gradient[f_int][1][2]

    return (f_red, f_green, f_blue)

def pydaw_set_track_gradients():
    pydaw_rainbow_inc = 0.25
    f_rainbox_pos = 0.0
    f_rainbox_intervals = [0.0, 0.15, 0.5, 1.0]

    for f_i_gradient in range(pydaw_midi_track_count):
        f_gradient = QtGui.QLinearGradient(
            QtCore.QPointF(0, 0), QtCore.QPointF(0, 100))
        for f_i2 in range(4):
            f_colors = pydaw_linear_interpolate_gradient(f_rainbox_pos)
            f_gradient.setColorAt(
                f_rainbox_intervals[f_i2], QtGui.QColor(*f_colors))
            f_rainbox_pos += pydaw_rainbow_inc
            if f_rainbox_pos >= len(pydaw_rainbow_gradient):
                f_rainbox_pos -= len(pydaw_rainbow_gradient)
        pydaw_track_gradients.append(f_gradient)

    f_rainbox_pos = 4.0

    for f_i2 in range(4):
        f_colors = pydaw_linear_interpolate_gradient(f_rainbox_pos)
        pydaw_region_gradient.setColorAt(
            f_rainbox_intervals[f_i2], QtGui.QColor(*f_colors))
        pydaw_note_gradient.setColorAt(
            f_rainbox_intervals[f_i2], QtGui.QColor(*f_colors))
        f_rainbox_pos += pydaw_rainbow_inc
        if f_rainbox_pos >= len(pydaw_rainbow_gradient):
            f_rainbox_pos -= len(pydaw_rainbow_gradient)

pydaw_set_track_gradients()

pydaw_selected_gradient = QtGui.QLinearGradient(
    QtCore.QPointF(0, 0), QtCore.QPointF(0, 100))
pydaw_selected_gradient.setColorAt(0, QtGui.QColor(255, 255, 255))
pydaw_selected_gradient.setColorAt(1, QtGui.QColor(237, 237, 243))
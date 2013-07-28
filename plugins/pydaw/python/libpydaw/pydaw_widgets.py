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

import pydaw_util
from PyQt4 import QtGui, QtCore

global_knob_arc_pen = QtGui.QPen(QtGui.QColor.fromRgb(255, 30, 30, 255), 5.0, QtCore.Qt.SolidLine, QtCore.Qt.RoundCap, QtCore.Qt.RoundJoin)

class pydaw_pixmap_knob(QtGui.QDial):
    def __init__(self, a_size):
        QtGui.QDial.__init__(self)
        self.setRange(0, 127)
        self.setGeometry(0, 0, a_size, a_size)
        self.set_pixmap_knob(a_size)

    def set_pixmap_knob(self, a_size):
        f_pixmap = QtGui.QPixmap("/usr/lib/pydaw3/themes/default/pydaw-knob.png")
        self.pixmap_size = a_size - 10
        self.pixmap = f_pixmap.scaled(self.pixmap_size, self.pixmap_size, QtCore.Qt.KeepAspectRatio, QtCore.Qt.SmoothTransformation)

    def paintEvent(self, a_event):
        p = QtGui.QPainter(self)
        f_frac_val = (((float)(self.value() - self.minimum())) / ((float)(self.maximum() - self.minimum())))
        f_rotate_value =  f_frac_val * 270.0
        f_rect = self.rect()
        f_rect.setWidth(f_rect.width() - 3)
        f_rect.setHeight(f_rect.height() - 3)
        f_rect.setX(f_rect.x() + 3)
        f_rect.setY(f_rect.y() + 3)
        p.setPen(global_knob_arc_pen)
        p.drawArc(f_rect, -136 * 16, (f_rotate_value + 1.0) * -16)
        p.setRenderHints(QtGui.QPainter.HighQualityAntialiasing | QtGui.QPainter.SmoothPixmapTransform)
        #xc and yc are the center of the widget's rect.
        xc = self.width() * 0.5
        yc = self.height() * 0.5
        #translates the coordinate system by xc and yc
        p.translate(xc, yc)
        p.rotate(f_rotate_value)
        #we need to move the rectangle that we draw by rx and ry so it's in the center.
        rx = -(self.pixmap_size * 0.5)
        ry = -(self.pixmap_size * 0.5)
        p.drawPixmap(rx, ry, self.pixmap)

class pydaw_knob_control:
    kc_integer = 0
    kc_decimal = 1
    kc_pitch = 2
    kc_none = 3
    kc_127_pitch = 4
    kc_127_zero_to_x = 5
    kc_log_time = 6
    kc_127_zero_to_x_int = 7

    def __init__(self, a_size, a_label="", a_knob_conversion=kc_none):
        self.name_label = QtGui.QLabel(str(a_label))
        self.knob = pydaw_pixmap_knob(a_size)
        self.knob.valueChanged.connect(self.knob_value_changed)
        self.value_label = QtGui.QLabel("")
        self.knob_conversion = a_knob_conversion

    def knob_value_changed(self, a_val):
        print(str(a_val))
        if self.knob_conversion == self.kc_integer:
            pass


class pydaw_modulex_effect:
    def __init__(self, a_title=None):
        self.group_box = QtGui.QGroupBox()
        if a_title is not None:
            self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout()
        self.group_box.setLayout(self.layout)
        self.knobs = []
        for f_i in range(3):
            f_knob = pydaw_knob_control(51)
            self.layout.addWidget(f_knob.name_label, 0, f_i)
            self.layout.addWidget(f_knob.knob, 1, f_i)
            self.layout.addWidget(f_knob.value_label, 2, f_i)
            self.knobs.append(f_knob)
        self.layout.addWidget(QtGui.QLabel("Type"), 0, 3)
        self.combobox = QtGui.QComboBox()
        self.combobox.setMinimumWidth(132)
        self.combobox.currentIndexChanged.connect(self.type_combobox_changed)
        self.combobox.addItems(["Off", "LP2" , "LP4", "HP2", "HP4", "BP2", "BP4" , "Notch2", "Notch4", "EQ" , "Distortion",
                                "Comb Filter", "Amp/Pan", "Limiter" , "Saturator", "Formant", "Chorus", "Glitch" , "RingMod",
                                "LoFi", "S/H", "LP-Dry/Wet" , "HP-Dry/Wet"])
        self.layout.addWidget(self.combobox, 1, 3)

    def type_combobox_changed(self, a_val):
        print(str(a_val))


if __name__ == "__main__":
    def pydaw_knob_test():
        import sys
        app = QtGui.QApplication(sys.argv)
        #f_pydaw_test_knob = pydaw_pixmap_knob(90)
        #f_pydaw_test_knob.show()
        f_modulex = pydaw_modulex_effect()
        f_modulex.group_box.setStyleSheet(open("/usr/lib/pydaw3/themes/default/style.txt").read())
        f_modulex.group_box.show()
        sys.exit(app.exec_())
    pydaw_knob_test()

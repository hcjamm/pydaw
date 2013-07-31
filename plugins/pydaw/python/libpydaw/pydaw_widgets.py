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

from pydaw_project import pydaw_audio_item_fx
from PyQt4 import QtGui, QtCore

global_knob_arc_gradient = QtGui.QLinearGradient(0.0, 0.0, 90.0, 0.0)
global_knob_arc_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(120, 120, 255, 255))
global_knob_arc_gradient.setColorAt(0.66, QtGui.QColor.fromRgb(255, 0, 0, 255))
global_knob_arc_pen = QtGui.QPen(global_knob_arc_gradient, 5.0, QtCore.Qt.SolidLine, QtCore.Qt.RoundCap, QtCore.Qt.RoundJoin)

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

    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_knob_conversion=kc_none):
        self.name_label = QtGui.QLabel(str(a_label))
        self.name_label.setAlignment(QtCore.Qt.AlignCenter)
        self.knob = pydaw_pixmap_knob(a_size)
        self.knob.setValue(64)
        self.knob.valueChanged.connect(self.knob_value_changed)
        self.knob.sliderReleased.connect(self.knob_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.knob_conversion = a_knob_conversion
        self.port_num = int(a_port_num)
        self.val_callback = a_val_callback
        self.rel_callback = a_rel_callback
        self.suppress_changes = False

    def set_value(self, a_val):
        self.suppress_changes = True
        self.knob.setValue(a_val)
        self.knob_value_changed(a_val)
        self.suppress_changes = False

    def lms_set_127_min_max(self, a_min, a_max):
        self.min_label_value_127 = a_min;
        self.max_label_value_127 = a_max;
        self.label_value_127_add_to = 0.0 - a_min;
        self.label_value_127_multiply_by = ((a_max - a_min) / 127.0);

    def knob_released(self):
        self.rel_callback(self.port_num, self.knob.value())

    def knob_value_changed(self, a_value):
        if not self.suppress_changes:
            self.val_callback(self.port_num, self.knob.value())
        f_dec_value = 0.0
        if self.knob_conversion == self.kc_decimal:
            self.value_label.setText(str(a_value * .01))
        elif self.knob_conversion == self.kc_integer:
            self.value_label.setText(str(a_value))
        elif self.knob_conversion == self.kc_none:
            pass
        elif self.knob_conversion == self.kc_pitch:
            self.value_label.setText(str(int(440.0 * pow(2.0,((float)(a_value - 57.0)) * 0.0833333))))
        elif self.knob_conversion == self.kc_127_pitch:
            self.value_label.setText(str(int(440.0 * pow(2.0, ((float)(((a_value * 0.818897638) + 20.0) -57.0)) * 0.0833333))))
        elif self.knob_conversion == self.kc_127_zero_to_x:
            f_dec_value = (float(a_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
            f_dec_value = ((int)(f_dec_value * 10.0)) * 0.1
            self.value_label.setText(str(f_dec_value))
        elif self.knob_conversion == self.kc_127_zero_to_x_int:
            f_dec_value = (float(a_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
            self.value_label.setText(str(int(f_dec_value)))
        elif self.knob_conversion == self.kc_log_time:
            f_dec_value = float(a_value) * 0.01
            f_dec_value = f_dec_value * f_dec_value
            f_dec_value = (int(f_dec_value * 100.0)) * 0.01
            self.value_label.setText(str(f_dec_value))

class pydaw_combobox_control:
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_items_list=[]):
        self.suppress_changes = True
        self.name_label = QtGui.QLabel(str(a_label))
        self.name_label.setAlignment(QtCore.Qt.AlignCenter)
        self.control = QtGui.QComboBox()
        self.control.setMinimumWidth(a_size)
        self.control.currentIndexChanged.connect(self.combobox_index_changed)
        self.control.addItems(a_items_list)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.port_num = int(a_port_num)
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.suppress_changes = False

    def combobox_index_changed(self, a_val):
        if not self.suppress_changes:
            self.val_callback(self.port_num, a_val)
            self.rel_callback(self.port_num, a_val)

    def set_value(self, a_val):
        self.suppress_changes = True
        self.control.setCurrentIndex(a_val)
        self.suppress_changes = False

class pydaw_modulex_single:
    def __init__(self, a_title, a_port_k1, a_rel_callback, a_val_callback):
        self.group_box = QtGui.QGroupBox()
        if a_title is not None:
            self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout()
        self.group_box.setLayout(self.layout)
        self.knobs = []
        for f_i in range(3):
            f_knob = pydaw_knob_control(51, "", a_port_k1 + f_i, a_rel_callback, a_val_callback)
            self.layout.addWidget(f_knob.name_label, 0, f_i)
            self.layout.addWidget(f_knob.knob, 1, f_i)
            self.layout.addWidget(f_knob.value_label, 2, f_i)
            self.knobs.append(f_knob)
        self.combobox = pydaw_combobox_control(132, "Type", a_port_k1 + 3, a_rel_callback, a_val_callback,
               ["Off", "LP2" , "LP4", "HP2", "HP4", "BP2", "BP4" , "Notch2", "Notch4", "EQ" , "Distortion",
                "Comb Filter", "Amp/Pan", "Limiter" , "Saturator", "Formant", "Chorus", "Glitch" , "RingMod",
                "LoFi", "S/H", "LP-Dry/Wet" , "HP-Dry/Wet"])
        self.layout.addWidget(self.combobox.name_label, 0, 3)
        self.combobox.control.currentIndexChanged.connect(self.type_combobox_changed)
        self.layout.addWidget(self.combobox.control, 1, 3)

    def set_from_class(self, a_class):
        """ a_class is a pydaw_audio_item_fx instance """
        self.knobs[0].set_value(a_class.knobs[0])
        self.knobs[1].set_value(a_class.knobs[1])
        self.knobs[2].set_value(a_class.knobs[2])
        self.combobox.set_value(a_class.fx_type)

    def get_class(self):
        """ return a pydaw_audio_item_fx instance """
        return pydaw_audio_item_fx(self.knobs[0].knob.value(), self.knobs[1].knob.value(), self.knobs[2].knob.value(), self.combobox.control.currentIndex())

    def type_combobox_changed(self, a_val):
        if a_val == 0: #Off
            self.knobs[0].name_label.setText((""))
            self.knobs[1].name_label.setText((""))
            self.knobs[2].name_label.setText((""))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].value_label.setText((""))
            self.knobs[2].value_label.setText((""))
        elif a_val == 1: #LP2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 2: #LP4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 3: #HP2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 4: #HP4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 5: #BP2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 6: #BP4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 7: #Notch2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 8: #Notch4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 9: #EQ
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Q"))
            self.knobs[2].name_label.setText(("Gain"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[2].lms_set_127_min_max(-24.0, 24.0)
            self.knobs[1].value_label.setText((""))
        elif a_val == 10: #Distortion
            self.knobs[0].name_label.setText(("Gain"))
            self.knobs[1].name_label.setText(("Dry/Wet"))
            self.knobs[2].name_label.setText(("Out Gain"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[0].lms_set_127_min_max(0.0, 36.0)
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[2].lms_set_127_min_max(-12.0, 0.0)
        elif a_val == 11: #Comb Filter
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Amt"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].value_label.setText((""))
            self.knobs[2].value_label.setText((""))
        elif a_val == 12: #Amp/Panner
            self.knobs[0].name_label.setText(("Pan"))
            self.knobs[1].name_label.setText(("Amp"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 6.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].value_label.setText((""))
            self.knobs[2].value_label.setText((""))
        elif a_val == 13: #Limiter
            self.knobs[0].name_label.setText(("Thresh"))
            self.knobs[1].name_label.setText(("Ceiling"))
            self.knobs[2].name_label.setText(("Release"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[0].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-12.0, -0.1)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_127_zero_to_x_int
            self.knobs[2].lms_set_127_min_max(150.0, 400.0)
        elif a_val == 14: #Saturator
            self.knobs[0].name_label.setText(("InGain"))
            self.knobs[1].name_label.setText(("Amt"))
            self.knobs[2].name_label.setText(("OutGain"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[0].lms_set_127_min_max(-12.0, 12.0)
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[2].lms_set_127_min_max(-12.0, 12.0)
        elif a_val == 15: #Formant Filter
            self.knobs[0].name_label.setText(("Vowel"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 16: #Chorus
            self.knobs[0].name_label.setText(("Rate"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[0].lms_set_127_min_max(0.3, 6.0)
            self.knobs[0].value_label.setText((""))
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 17: #Glitch
            self.knobs[0].name_label.setText(("Pitch"))
            self.knobs[1].name_label.setText(("Glitch"))
            self.knobs[2].name_label.setText(("Wet"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 18: #RingMod
            self.knobs[0].name_label.setText(("Pitch"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 19: #LoFi
            self.knobs[0].name_label.setText(("Bits"))
            self.knobs[1].name_label.setText(("unused"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[0].lms_set_127_min_max(4.0, 16.0)
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 20: #Sample and Hold
            self.knobs[0].name_label.setText(("Pitch"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 21: #LP2-Dry/Wet
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("Wet"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 22: #HP2-Dry/Wet
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("Wet"))
            self.knobs[0].knob_conversion = pydaw_knob_control.kc_127_pitch
            self.knobs[1].knob_conversion = pydaw_knob_control.kc_127_zero_to_x
            self.knobs[1].lms_set_127_min_max(-30.0, 0.0)
            self.knobs[2].knob_conversion = pydaw_knob_control.kc_none
            self.knobs[2].value_label.setText((""))

        self.knobs[0].set_value(self.knobs[0].knob.value())
        self.knobs[1].set_value(self.knobs[1].knob.value())
        self.knobs[2].set_value(self.knobs[2].knob.value())

        #self.knobs[0].knob_value_changed(self.knobs[0].knob.value())
        #self.knobs[1].knob_value_changed(self.knobs[1].knob.value())
        #self.knobs[2].knob_value_changed(self.knobs[2].knob.value())

class pydaw_per_audio_item_fx_widget:
    def __init__(self, a_rel_callback, a_val_callback):
        self.effects = []
        self.widget = QtGui.QWidget()
        self.layout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.layout)
        f_port = 0
        for f_i in range(8):
            f_effect = pydaw_modulex_single(("FX" + str(f_i)), f_port, a_rel_callback, a_val_callback)
            self.effects.append(f_effect)
            self.layout.addWidget(f_effect.group_box)
            f_port += 4
        self.widget.setGeometry(0, 0, 348, 1100)  #ensure minimum size
        self.scroll_area = QtGui.QScrollArea()
        self.scroll_area.setGeometry(0, 0, 360, 1120)
        self.scroll_area.setWidget(self.widget)

    def set_from_list(self, a_list):
        """ a_class is a pydaw_audio_item_fx instance """
        for f_i in range(len(a_list)):
            self.effects[f_i].set_from_class(a_list[f_i])

    def get_list(self):
        """ return a list of pydaw_audio_item_fx instances """
        f_result = []
        for f_effect in self.effects:
            f_result.append(f_effect.get_class())
        return f_result


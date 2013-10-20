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

import os, subprocess
from . import pydaw_util, pydaw_ports
from libpydaw.pydaw_project import pydaw_audio_item_fx
from PyQt4 import QtGui, QtCore

global_knob_arc_gradient = QtGui.QLinearGradient(0.0, 0.0, 90.0, 0.0)
global_knob_arc_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(60, 60, 255, 255))
global_knob_arc_gradient.setColorAt(0.25, QtGui.QColor.fromRgb(255, 120, 0, 255))
global_knob_arc_gradient.setColorAt(0.75, QtGui.QColor.fromRgb(255, 0, 0, 255))
global_knob_arc_pen = QtGui.QPen(global_knob_arc_gradient, 5.0, QtCore.Qt.SolidLine, QtCore.Qt.RoundCap, QtCore.Qt.RoundJoin)

class pydaw_plugin_file:
    """ Abstracts a .pyinst or .pyfx file """
    def __init__(self, a_path=None):
        self.port_dict = {}
        self.configure_dict = {}
        if a_path is not None:
            f_text = pydaw_util.pydaw_read_file_text(a_path)
            f_line_arr = f_text.split("\n")
            for f_line in f_line_arr:
                if f_line == "\\":
                    break
                f_items = f_line.split("|", 1)
                f_config = False
                try:
                    int(f_items[0])
                except ValueError:
                    f_config = True
                if f_config:
                    self.configure_dict[(f_items[0])] = f_items[1]
                else:
                    self.port_dict[int(f_items[0])] = float(f_items[1])

    @staticmethod
    def from_dict(a_port_dict, a_control_dict, a_configure_dict):
        f_result = pydaw_plugin_file()
        for k, v in list(a_port_dict.items()):
            f_result.port_dict[a_control_dict[int(k)]] = v
        for k, v in list(a_configure_dict.items()):
            f_result.configure_dict[k] = v
        return f_result

    def __str__(self):
        f_result = ""
        for k, v in list(self.configure_dict.items()):
            f_result += str(k) + "|" + str(v) + "\n"
        for k, v in list(self.port_dict.items()):
            f_result += str(k) + "|" + str(v.get_value()) + "\n"
        return f_result + "\\"

global_pydaw_knob_pixmap = None
global_pydaw_knob_pixmap_cache = {}

def get_scaled_pixmap_knob(a_size):
    global global_pydaw_knob_pixmap, global_pydaw_knob_pixmap_cache
    if global_pydaw_knob_pixmap is None:
        global_pydaw_knob_pixmap = QtGui.QPixmap("%s/lib/%s/themes/default/pydaw-knob.png" %
        (pydaw_util.global_pydaw_install_prefix, pydaw_util.global_pydaw_version_string))

    if not a_size in global_pydaw_knob_pixmap_cache:
        global_pydaw_knob_pixmap_cache[a_size] = \
        global_pydaw_knob_pixmap.scaled(a_size, a_size, \
        QtCore.Qt.KeepAspectRatio, QtCore.Qt.SmoothTransformation)

    return global_pydaw_knob_pixmap_cache[a_size]


class pydaw_pixmap_knob(QtGui.QDial):
    def __init__(self, a_size, a_min_val, a_max_val):
        QtGui.QDial.__init__(self)
        self.setRange(a_min_val, a_max_val)
        self.setGeometry(0, 0, a_size, a_size)
        self.pixmap_size = a_size - 10
        self.pixmap = get_scaled_pixmap_knob(self.pixmap_size)
        self.setFixedSize(a_size, a_size)

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

kc_integer = 0
kc_decimal = 1
kc_pitch = 2
kc_none = 3
kc_127_pitch = 4
kc_127_zero_to_x = 5
kc_log_time = 6
kc_127_zero_to_x_int = 7

class pydaw_abstract_ui_control:
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion=kc_none, a_port_dict=None, \
    a_preset_mgr=None, a_default_value=None):
        if a_label is None:
            self.name_label = None
        else:
            self.name_label = QtGui.QLabel(str(a_label))
            self.name_label.setAlignment(QtCore.Qt.AlignCenter)
        self.port_num = int(a_port_num)
        self.val_callback = a_val_callback
        self.rel_callback = a_rel_callback
        self.suppress_changes = False
        self.val_conversion = a_val_conversion
        if a_port_dict is not None:
            a_port_dict[self.port_num] = self
        if a_preset_mgr is not None:
            a_preset_mgr.add_control(self)
        self.default_value = a_default_value

    def reset_default_value(self):
        if self.default_value is not None:
            self.set_value(self.default_value)
            self.control_value_changed(self.default_value)

    def set_value(self, a_val):
        self.suppress_changes = True
        f_val = int(a_val)
        self.control.setValue(f_val)
        self.control_value_changed(f_val)
        self.suppress_changes = False

    def get_value(self):
        return self.control.value()

    def set_127_min_max(self, a_min, a_max):
        self.min_label_value_127 = a_min;
        self.max_label_value_127 = a_max;
        self.label_value_127_add_to = 0.0 - a_min;
        self.label_value_127_multiply_by = ((a_max - a_min) / 127.0);

    def control_released(self):
        if self.rel_callback is not None:
            self.rel_callback(self.port_num, self.control.value())

    def control_value_changed(self, a_value):
        if not self.suppress_changes:
            self.val_callback(self.port_num, self.control.value())

        if self.value_label is not None:
            f_value = float(a_value)
            f_dec_value = 0.0
            if self.val_conversion == kc_none:
                pass
            elif self.val_conversion == kc_decimal:
                self.value_label.setText(str(round(f_value * .01, 2)))
            elif self.val_conversion == kc_integer:
                self.value_label.setText(str(int(f_value)))
            elif self.val_conversion == kc_pitch:
                self.value_label.setText(str(int(440.0 * pow(2.0,((float)(f_value - 57.0)) * 0.0833333))))
            elif self.val_conversion == kc_127_pitch:
                self.value_label.setText(str(int(440.0 * pow(2.0, ((float)(((f_value * 0.818897638) + 20.0) -57.0)) * 0.0833333))))
            elif self.val_conversion == kc_127_zero_to_x:
                f_dec_value = (float(f_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
                f_dec_value = ((int)(f_dec_value * 10.0)) * 0.1
                self.value_label.setText(str(round(f_dec_value, 2)))
            elif self.val_conversion == kc_127_zero_to_x_int:
                f_dec_value = (float(f_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
                self.value_label.setText(str(int(f_dec_value)))
            elif self.val_conversion == kc_log_time:
                f_dec_value = float(f_value) * 0.01
                f_dec_value = f_dec_value * f_dec_value
                f_dec_value = (int(f_dec_value * 100.0)) * 0.01
                self.value_label.setText(str(round(f_dec_value, 2)))

    def add_to_grid_layout(self, a_layout, a_x):
        if self.name_label is not None:
            a_layout.addWidget(self.name_label, 0, a_x, alignment=QtCore.Qt.AlignHCenter)
        a_layout.addWidget(self.control, 1, a_x, alignment=QtCore.Qt.AlignHCenter)
        if self.value_label is not None:
            a_layout.addWidget(self.value_label, 2, a_x, alignment=QtCore.Qt.AlignHCenter)

class pydaw_null_control:
    """ For controls with no visual representation, ie: controls that share a UI widget
    depending on selected index, so that they can participate normally in the data
    representation mechanisms"""
    def __init__(self, a_port_num, a_rel_callback, a_val_callback, a_default_val, a_port_dict, a_preset_mgr=None):
        self.name_label = None
        self.value_label = None
        self.port_num = int(a_port_num)
        self.val_callback = a_val_callback
        self.rel_callback = a_rel_callback
        self.suppress_changes = False
        self.value = a_default_val
        a_port_dict[self.port_num] = self
        self.default_value = a_default_val
        if a_preset_mgr is not None:
            a_preset_mgr.add_control(self)

    def reset_default_value(self):
        if self.default_value is not None:
            self.set_value(self.default_value)
            self.control_value_changed(self.default_value)

    def get_value(self):
        return self.value

    def set_value(self, a_val):
        self.value = a_val

    def control_released(self):
        if self.rel_callback is not None:
            self.rel_callback(self.port_num, self.value)

    def control_value_changed(self, a_value):
        self.val_callback(self.port_num, self.value)


class pydaw_knob_control(pydaw_abstract_ui_control):
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, \
        a_port_dict, a_preset_mgr, a_default_val)
        self.control = pydaw_pixmap_knob(a_size, a_min_val, a_max_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.set_value(a_default_val)


class pydaw_slider_control(pydaw_abstract_ui_control):
    def __init__(self, a_orientation, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, \
        a_port_dict, a_preset_mgr, a_default_val)
        self.control = QtGui.QSlider()
        self.control.setRange(a_min_val, a_max_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.set_value(a_default_val)


class pydaw_spinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, \
        a_port_dict, a_preset_mgr, a_default_val)
        self.control = QtGui.QSpinBox()
        self.control.setRange(a_min_val, a_max_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)
        self.value_label = None
        self.set_value(a_default_val)


class pydaw_doublespinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, \
        a_port_dict, a_preset_mgr, a_default_val)
        self.control = QtGui.QDoubleSpinBox()
        self.control.setRange(a_min_val, a_max_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)
        self.value_label = None
        self.set_value(a_default_val)


class pydaw_checkbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_port_dict=a_port_dict, \
        a_preset_mgr=a_preset_mgr, a_default_value=0)
        self.control = QtGui.QCheckBox(a_label)
        self.control.stateChanged.connect(self.control_value_changed)
        self.control.stateChanged.connect(self.control_released)
        self.value_label = None
        self.suppress_changes = False

    def control_value_changed(self, a_val=None):
        if not self.suppress_changes:
            self.val_callback(self.port_num, self.get_value())

    def control_released(self, a_val=None):
        pass

    def set_value(self, a_val):
        f_val = int(a_val)
        if f_val == 0:
            self.control.setChecked(False)
        else:
            self.control.setChecked(True)

    def get_value(self):
        if self.control.isChecked():
            return 1
        else:
            return 0


class pydaw_combobox_control(pydaw_abstract_ui_control):
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_items_list=[], a_port_dict=None, \
    a_default_index=None, a_preset_mgr=None):
        self.suppress_changes = True
        self.name_label = QtGui.QLabel(str(a_label))
        self.name_label.setAlignment(QtCore.Qt.AlignCenter)
        self.control = QtGui.QComboBox()
        self.control.setMinimumWidth(a_size)
        self.control.addItems(a_items_list)
        self.control.setCurrentIndex(0)
        self.control.currentIndexChanged.connect(self.control_value_changed)
        self.port_num = int(a_port_num)
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.suppress_changes = False
        if a_port_dict is not None:
            a_port_dict[self.port_num] = self
        self.value_label = None
        self.default_value = a_default_index
        if a_default_index is not None:
            self.set_value(a_default_index)
        if a_preset_mgr is not None:
            a_preset_mgr.add_control(self)

    def control_value_changed(self, a_val):
        if not self.suppress_changes:
            self.val_callback(self.port_num, a_val)
            if self.rel_callback is not None:
                self.rel_callback(self.port_num, a_val)

    def set_value(self, a_val):
        self.suppress_changes = True
        self.control.setCurrentIndex(int(a_val))
        self.suppress_changes = False

    def get_value(self):
        return self.control.currentIndex()

class pydaw_adsr_widget:
    def __init__(self, a_size, a_sustain_in_db, a_attack_port, a_decay_port, a_sustain_port, a_release_port, \
            a_label, a_rel_callback, a_val_callback, a_port_dict=None, a_preset_mgr=None, a_attack_default=10):
        self.attack_knob = pydaw_knob_control(a_size, "Attack", a_attack_port, a_rel_callback, a_val_callback, 0, 100, a_attack_default, kc_decimal, a_port_dict, a_preset_mgr)
        self.decay_knob = pydaw_knob_control(a_size, "Decay", a_decay_port, a_rel_callback, a_val_callback, 10, 100, 50, kc_decimal, a_port_dict, a_preset_mgr)
        if a_sustain_in_db:
            self.sustain_knob = pydaw_knob_control(a_size, "Sustain", a_sustain_port, a_rel_callback, a_val_callback, -30, 0, 0, kc_integer, a_port_dict, a_preset_mgr)
        else:
            self.sustain_knob = pydaw_knob_control(a_size, "Sustain", a_sustain_port, a_rel_callback, a_val_callback, 0, 100, 100, kc_decimal, a_port_dict, a_preset_mgr)
        self.release_knob = pydaw_knob_control(a_size, "Release", a_release_port, a_rel_callback, a_val_callback, 10, 400, 50, kc_decimal, a_port_dict, a_preset_mgr)
        self.groupbox = QtGui.QGroupBox(a_label)
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.attack_knob.add_to_grid_layout(self.layout, 0)
        self.decay_knob.add_to_grid_layout(self.layout, 1)
        self.sustain_knob.add_to_grid_layout(self.layout, 2)
        self.release_knob.add_to_grid_layout(self.layout, 3)

class pydaw_filter_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict, a_cutoff_port, a_res_port, a_type_port=None, a_label="Filter", a_preset_mgr=None):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.cutoff_knob = pydaw_knob_control(a_size, "Cutoff", a_cutoff_port, a_rel_callback, a_val_callback, \
        20, 124, 124, kc_pitch, a_port_dict, a_preset_mgr)
        self.cutoff_knob.add_to_grid_layout(self.layout, 0)
        self.res_knob = pydaw_knob_control(a_size, "Res", a_res_port, a_rel_callback, a_val_callback, \
        -30, 0, -12, kc_integer, a_port_dict, a_preset_mgr)
        self.res_knob.add_to_grid_layout(self.layout, 1)
        if a_type_port is not None:
            self.type_combobox = pydaw_combobox_control(150, "Type", a_type_port, a_rel_callback, a_val_callback, \
            ["LP 2", "HP 2", "BP2", "LP 4", "HP 4", "BP4", "Off"], a_port_dict, a_preset_mgr=a_preset_mgr)
            self.layout.addWidget(self.type_combobox.name_label, 2, 0)
            self.layout.addWidget(self.type_combobox.control, 2, 1)

class pydaw_ramp_env_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict, a_time_port, a_amt_port, a_label="Ramp Env", a_preset_mgr=None):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.layout = QtGui.QGridLayout(self.groupbox)
        if a_amt_port is not None:
            self.amt_knob = pydaw_knob_control(a_size, "Amt", a_amt_port, a_rel_callback, a_val_callback, \
            -36, 36, 0, kc_integer, a_port_dict, a_preset_mgr)
            self.amt_knob.add_to_grid_layout(self.layout, 0)
        self.time_knob = pydaw_knob_control(a_size, "Time", a_time_port, a_rel_callback, a_val_callback, \
        1, 200, 100, kc_decimal, a_port_dict, a_preset_mgr)
        self.time_knob.add_to_grid_layout(self.layout, 1)

class pydaw_lfo_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict, a_freq_port, a_type_port, a_type_list, a_label="LFO", a_preset_mgr=None):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.freq_knob = pydaw_knob_control(a_size, "Freq", a_freq_port, a_rel_callback, a_val_callback, \
        10, 1600, 200, kc_decimal, a_port_dict, a_preset_mgr)
        self.freq_knob.add_to_grid_layout(self.layout, 0)
        self.type_combobox = pydaw_combobox_control(120, "Type", a_type_port, a_rel_callback, a_val_callback, a_type_list, \
        a_port_dict, 0, a_preset_mgr=a_preset_mgr)
        self.layout.addWidget(self.type_combobox.name_label, 0, 1)
        self.layout.addWidget(self.type_combobox.control, 1, 1)

class pydaw_osc_widget:
    def __init__(self, a_size, a_pitch_port, a_fine_port, a_vol_port, a_type_port, a_osc_types_list, \
    a_rel_callback, a_val_callback, a_label, a_port_dict=None, a_preset_mgr=None, a_default_type=0):
        self.pitch_knob = pydaw_knob_control(a_size, "Pitch", a_pitch_port, a_rel_callback, a_val_callback, -36, 36, 0, \
        a_val_conversion=kc_integer, a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr)
        self.fine_knob = pydaw_knob_control(a_size, "Fine", a_fine_port, a_rel_callback, a_val_callback, -100, 100, 0, \
        a_val_conversion=kc_decimal, a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr)
        self.vol_knob = pydaw_knob_control(a_size, "Vol", a_vol_port, a_rel_callback, a_val_callback, -30, 0, -6, \
        a_val_conversion=kc_integer, a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr)
        self.osc_type_combobox = pydaw_combobox_control(114, "Type", a_type_port, a_rel_callback, a_val_callback, a_osc_types_list, a_port_dict, \
        a_preset_mgr=a_preset_mgr, a_default_index=a_default_type)
        self.grid_layout = QtGui.QGridLayout()
        self.group_box = QtGui.QGroupBox(str(a_label))
        self.group_box.setLayout(self.grid_layout)
        self.pitch_knob.add_to_grid_layout(self.grid_layout, 0)
        self.fine_knob.add_to_grid_layout(self.grid_layout, 1)
        self.vol_knob.add_to_grid_layout(self.grid_layout, 2)
        self.grid_layout.addWidget(self.osc_type_combobox.name_label, 0, 3)
        self.grid_layout.addWidget(self.osc_type_combobox.control, 1, 3)

class pydaw_note_selector_widget:
    def __init__(self, a_port_num, a_rel_callback, a_val_callback, a_port_dict=None, a_default_value=None, a_preset_mgr=None):
        self.control = self
        self.port_num = a_port_num
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.selected_note = 48
        self.note_combobox = QtGui.QComboBox()
        self.note_combobox.setMinimumWidth(60)
        self.note_combobox.addItems(["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"])
        self.octave_spinbox = QtGui.QSpinBox()
        self.octave_spinbox.setRange(-2, 8)
        self.octave_spinbox.setValue(3)
        self.widget = QtGui.QWidget()
        self.layout = QtGui.QHBoxLayout()
        self.layout.setMargin(0)
        self.widget.setLayout(self.layout)
        self.layout.addWidget(self.note_combobox)
        self.layout.addWidget(self.octave_spinbox)
        self.note_combobox.currentIndexChanged.connect(self.control_value_changed)
        self.octave_spinbox.valueChanged.connect(self.control_value_changed)
        self.suppress_changes = False
        if a_port_dict is not None:
            a_port_dict[self.port_num] = self
        self.name_label = None
        self.value_label = None
        if a_default_value is not None:
            self.set_value(a_default_value)
        if a_preset_mgr is not None:
            a_preset_mgr.add_control(self)

    def control_value_changed(self, a_val=None):
        self.selected_note = (self.note_combobox.currentIndex()) + (((self.octave_spinbox.value()) + 2) * 12)
        if not self.suppress_changes:
            self.val_callback(self.port_num, self.selected_note)
            if self.rel_callback is not None:
                self.rel_callback(self.port_num, self.selected_note)

    def set_value(self, a_val):
        self.suppress_changes = True
        self.note_combobox.setCurrentIndex(a_val % 12)
        self.octave_spinbox.setValue((int(float(a_val) / 12.0)) - 2)
        self.suppress_changes = False

    def get_value(self):
        return self.selected_note

class pydaw_file_select_widget:
    def __init__(self):
        self.layout =  QtGui.QHBoxLayout()
        self.open_button =  QtGui.QPushButton("Open")
        self.open_button.setMaximumWidth(60)
        self.clear_button =  QtGui.QPushButton("Clear")
        self.clear_button.setMaximumWidth(60)
        self.open_in_editor_button =  QtGui.QPushButton("Edit")
        self.open_in_editor_button.pressed.connect(self.open_in_editor_button_pressed)
        self.open_in_editor_button.setMaximumWidth(60)
        self.reload_button =  QtGui.QPushButton("Reload")
        self.reload_button.setMaximumWidth(60)
        self.file_path =  QtGui.QLineEdit()
        self.file_path.setReadOnly(True)
        self.file_path.setMinimumWidth(360)
        self.last_directory = ("")
        self.editor_path = ("audacity")
        f_global_config_path = pydaw_util.global_pydaw_home + "/self.global_wave_editor.txt"
        if os.path.exists(f_global_config_path):
            self.editor_path = pydaw_util.pydaw_read_file_text(f_global_config_path)
        else:
            pydaw_util.pydaw_write_file_text(f_global_config_path, "audacity")
        self.layout.addWidget(self.file_path)
        self.layout.addWidget(self.clear_button)
        self.layout.addWidget(self.open_button)
        self.layout.addWidget(self.open_in_editor_button)
        self.layout.addWidget(self.reload_button)

    def open_button_pressed(self):
        f_result = QtGui.QFileDialog.getOpenFileName(self.file_path, "Select an audio sample file", self.last_directory, "Audio files (*.wav *.aiff)")
        if f_result is not None:
            self.file_path.setText(f_result)
            self.last_directory = os.path.pardir(str(f_result))
        return self.file_path.text()

    def open_button_pressed_multiple(self):
        f_result = QtGui.QFileDialog.getOpenFileNames(self.file_path, "Select one or more audio sample files", self.last_directory, "Audio files (*.wav)All files (*)")
        if len(f_result) > 0:
            self.file_path.setText(str(f_result[-1]))
            self.last_directory = os.path.pardir(str(f_result))
        return f_result

    def clear_button_pressed(self):
        self.file_path.setText((""))

    def get_file(self):
        return self.file_path.text()

    def set_file(self, a_file):
        self.file_path.setText(str(a_file))

    def open_in_editor_button_pressed(self):
        if pydaw_util.pydaw_which(self.editor_path) is None and not os.path.exists(self.editor_path):
            QtGui.QMessageBox.warning(self.file_path, ("No Wave Editor Found"), ("""Could not locate %s
or another suitable wave editor. Please edit
~/%s/self.global_wave_editor.txt
with your wave editor of choice, or install Audacity.""" % (self.editor_path, pydaw_util.global_pydaw_version_string)))
            return
        else:
            f_cmd = [self.editor_path, str(self.file_path.text())]
            subprocess.Popen(f_cmd)


class pydaw_file_browser_widget:
    def __init__(self):
        if os.path.isdir("/media/pydaw_data") and os.path.isdir("/home/ubuntu"):
            self.home_path = ("/media/pydaw_data")
        else:
            self.home_path = os.path.expanduser("~")
        self.file_browser_vsplitter =  QtGui.QSplitter(QtCore.Qt.Vertical)
        self.file_browser_vsplitter.setMaximumWidth(510)
        self.file_browser_vsplitter.setContentsMargins(0, 0, 0, 0)
        self.bookmark_widget = QtGui.QWidget()
        self.file_browser_vsplitter.addWidget(self.bookmark_widget)
        self.bookmark_layout = QtGui.QVBoxLayout(self.bookmark_widget)
        self.bookmarks_label =  QtGui.QLabel("Bookmarks")
        self.bookmarks_label.setAlignment(QtCore.Qt.AlignCenter)
        self.bookmark_layout.addWidget(self.bookmarks_label)
        self.folder_path_lineedit =  QtGui.QLineEdit()
        self.folder_path_lineedit.setReadOnly(True)
        self.bookmarks_listWidget =  QtGui.QListWidget()
        self.bookmarks_listWidget.itemClicked.connect(self.bookmark_clicked)
        self.bookmarks_listWidget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.bookmarks_listWidget.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.bookmarks_listWidget.setAcceptDrops(True)
        self.bookmarks_listWidget.setDropIndicatorShown(True)
        self.bookmarks_listWidget.setToolTip(("Press the 'bookmark' button to add folders here."))
        self.bookmarks_listWidget.setDragDropMode(QtGui.QAbstractItemView.DropOnly)
        self.bookmark_layout.addWidget(self.bookmarks_listWidget)
        self.bookmarks_button_widget = QtGui.QWidget()
        self.bookmarks_hlayout0 =  QtGui.QHBoxLayout(self.bookmarks_button_widget)
        self.bookmarks_delete_button =  QtGui.QPushButton()
        self.bookmarks_delete_button.setText(("Delete"))
        self.bookmarks_hlayout0.addWidget(self.bookmarks_delete_button)
        self.bookmarks_delete_button.pressed.connect(self.bookmark_delete_button_pressed)
        self.bookmark_layout.addWidget(self.bookmarks_button_widget)
        self.folders_widget = QtGui.QWidget()
        self.file_browser_vsplitter.addWidget(self.folders_widget)
        self.folders_layout = QtGui.QVBoxLayout(self.folders_widget)
        self.folders_label =  QtGui.QLabel("Folders")
        self.folders_label.setAlignment(QtCore.Qt.AlignCenter)
        self.folders_layout.addWidget(self.folders_label)
        self.folders_layout.addWidget(self.folder_path_lineedit)
        self.folders_listWidget =  QtGui.QListWidget()
        self.folders_listWidget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.folders_listWidget.itemClicked.connect(self.folder_item_clicked)
        self.folders_listWidget.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.folders_listWidget.setDragDropMode(QtGui.QAbstractItemView.DragOnly)
        self.folders_layout.addWidget(self.folders_listWidget)
        self.folder_buttons_widget = QtGui.QWidget()
        self.folders_hlayout0 =  QtGui.QHBoxLayout(self.folder_buttons_widget)
        self.up_pushButton =  QtGui.QPushButton("Up")
        self.up_pushButton.pressed.connect(self.up_button_pressed)
        self.folders_hlayout0.addWidget(self.up_pushButton)
        self.bookmark_button =  QtGui.QPushButton("Bookmark")
        self.folders_hlayout0.addWidget(self.bookmark_button)
        self.bookmark_button.pressed.connect(self.bookmark_button_pressed)
        self.folders_layout.addWidget(self.folder_buttons_widget)
        self.files_widget = QtGui.QWidget()
        self.file_browser_vsplitter.addWidget(self.files_widget)
        self.files_layout = QtGui.QVBoxLayout(self.files_widget)
        self.files_label =  QtGui.QLabel("Files")
        self.files_label.setAlignment(QtCore.Qt.AlignCenter)
        self.files_layout.addWidget(self.files_label)
        self.files_listWidget =  QtGui.QListWidget()
        self.files_listWidget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.files_listWidget.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        self.files_listWidget.setToolTip(("Select the file(s) you wish to load and click the 'load' button.\nThe samples will be loaded sequentially starting from the currently selected row."))
        self.files_layout.addWidget(self.files_listWidget)
        self.file_buttons_widget = QtGui.QWidget()
        self.files_hlayout0 =  QtGui.QHBoxLayout(self.file_buttons_widget)
        self.load_pushButton =  QtGui.QPushButton("Load")
        self.files_hlayout0.addWidget(self.load_pushButton)
        self.preview_pushButton =  QtGui.QPushButton("Preview")
        self.files_hlayout0.addWidget(self.preview_pushButton)
        self.files_layout.addWidget(self.file_buttons_widget)
        self.folder_path_lineedit.setText(self.home_path)

        self.files_listWidget.setSortingEnabled(True)
        self.set_folder(pydaw_util.global_home, True)
        self.open_bookmarks()


    def folder_opened(self, a_folder, a_relative_path):
        if a_folder is None or a_folder == "":
            return
        if(a_relative_path):
            if str(self.folder_path_lineedit.text()) == "/":
                self.enumerate_folders_and_files(("/") + a_folder)
            else:
                self.enumerate_folders_and_files(self.folder_path_lineedit.text() + ("/") + a_folder)
        else:
            self.enumerate_folders_and_files(a_folder)

    def up_one_folder(self):
        self.set_folder("..")

    def bookmark_delete_button_pressed(self):
        f_items = self.bookmarks_listWidget.selectedItems()
        if len(f_items) > 0:
            pydaw_util.global_delete_file_bookmark(str(f_items[0].text()))

    def files_selected(self):
        f_dir_path = str(self.folder_path_lineedit.text()) + "/"
        f_result = []
        for f_item in self.files_listWidget.selectedItems():
            f_result.append(f_dir_path + str(f_item.text()))
        return f_result

    #def on_preview(self):
    #    f_list = self.files_listWidget.selectedItems()
    #    if len(f_list) > 0:
    #        this_pydaw_project.this_pydaw_osc.pydaw_preview_audio(self.last_open_dir + "/" + str(f_list[0].text()))

    def open_bookmarks(self):
        self.bookmarks_listWidget.clear()
        f_dict = pydaw_util.global_get_file_bookmarks()
        for k, v in list(f_dict.items()):
            self.bookmarks_listWidget.addItem(str(k))

    def bookmark_button_pressed(self):
        pydaw_util.global_add_file_bookmark(self.last_open_dir)
        self.open_bookmarks()

    def bookmark_clicked(self, a_item):
        f_dict = pydaw_util.global_get_file_bookmarks()
        f_folder_name = str(a_item.text())
        f_full_path = f_dict[f_folder_name] + "/" + f_folder_name
        self.set_folder(f_full_path, True)

    def file_mouse_press_event(self, a_event):
        QtGui.QListWidget.mousePressEvent(self.files_listWidget, a_event)
        global global_audio_items_to_drop
        global_audio_items_to_drop = []
        for f_item in self.files_listWidget.selectedItems():
            global_audio_items_to_drop.append(self.last_open_dir + "/" + str(f_item.text()))

    def folder_item_clicked(self, a_item):
        self.set_folder(a_item.text())

    def up_button_pressed(self):
        self.set_folder("..")

    def set_folder(self, a_folder, a_full_path=False):
        self.files_listWidget.clear()
        self.folders_listWidget.clear()
        if a_full_path:
            self.last_open_dir = str(a_folder)
        else:
            self.last_open_dir = os.path.abspath(self.last_open_dir + "/" + str(a_folder))
        self.last_open_dir = self.last_open_dir.replace("//", "/")
        self.folder_path_lineedit.setText(self.last_open_dir)
        f_list = os.listdir(self.last_open_dir)
        f_list.sort(key=str.lower)
        for f_file in f_list:
            f_full_path = self.last_open_dir + "/" + f_file
            if  not f_file.startswith("."):
                if os.path.isdir(f_full_path):
                    self.folders_listWidget.addItem(f_file)
                elif f_file.upper().endswith(".WAV") and os.path.isfile(f_full_path):
                    if not pydaw_util.pydaw_str_has_bad_chars(f_full_path):
                        self.files_listWidget.addItem(f_file)
                    else:
                        print(("Not adding '" + f_full_path + "' because it contains bad chars, you must rename this file path without:"))
                        print(("\n".join(pydaw_util.pydaw_bad_chars)))

class pydaw_preset_manager_widget:
    def __init__(self, a_plugin_name):
        self.factory_preset_path = pydaw_util.global_pydaw_install_prefix + "/lib/" + pydaw_util.global_pydaw_version_string + \
        "/presets/" + str(a_plugin_name) + ".pypreset2"
        self.preset_path = pydaw_util.global_pydaw_home + "/" + str(a_plugin_name) + ".pypreset2"
        self.group_box = QtGui.QGroupBox()
        self.layout = QtGui.QHBoxLayout(self.group_box)
        self.program_combobox = QtGui.QComboBox()
        self.program_combobox.setEditable(True)
        self.program_combobox.setMinimumWidth(190)
        self.layout.addWidget(self.program_combobox)
        self.save_button = QtGui.QPushButton("Save")
        self.save_button.setToolTip("Save the current settings to a preset.  Plugin settings are saved to the project automatically\nwhen you close the plugin window, this button is only for presets.")
        self.save_button.pressed.connect(self.save_presets)
        self.layout.addWidget(self.save_button)
        self.reset_button = QtGui.QPushButton("Reset")
        self.reset_button.setToolTip("Resets all controls to their default value")
        self.reset_button.pressed.connect(self.reset_controls)
        self.layout.addWidget(self.reset_button)
        self.presets_delimited = []
        self.controls = {}
        for f_i in range(128):
            self.program_combobox.addItem("empty")
        self.load_presets()
        self.program_combobox.currentIndexChanged.connect(self.program_changed)

    def reset_controls(self):
        for k, v in list(self.controls.items()):
            v.reset_default_value()

    def load_presets(self):
        if os.path.isfile(self.preset_path):
            print("loading presets from file")
            f_text = pydaw_util.pydaw_read_file_text(self.preset_path)
            f_line_arr = f_text.split("\n")
        elif os.path.isfile(self.factory_preset_path):
            os.system('cp "%s" "%s"' % (self.factory_preset_path, self.preset_path))
            print("loading factory presets")
            f_text = pydaw_util.pydaw_read_file_text(self.preset_path)
            f_line_arr = f_text.split("\n")
        else:
            f_line_arr = []

        self.presets_delimited = []
        for f_i in range(128):
            if f_i >= len(f_line_arr):
                self.presets_delimited.append(["empty"])
                self.program_combobox.setItemText(f_i, "empty")
            else:
                self.presets_delimited.append(f_line_arr[f_i].split("|"))
                self.program_combobox.setItemText(f_i, self.presets_delimited[f_i][0])

    def save_presets(self):
        print("saving preset")
        if str(self.program_combobox.currentText()) == "empty":
            QtGui.QMessageBox.warning(self.group_box, "Error", "Preset name cannot be 'empty'")
            return
        if self.program_combobox.currentIndex() == 0:
            QtGui.QMessageBox.warning(self.group_box, "Error", "The first preset must be empty")
            return
        f_result_values = [str(self.program_combobox.currentText())]
        for k, f_control in list(self.controls.items()):
            f_result_values.append("%s:%s" % (f_control.port_num, f_control.get_value(),))
        self.presets_delimited[(self.program_combobox.currentIndex())] = f_result_values
        f_result = ""
        for f_list in self.presets_delimited:
            f_result += "|".join(f_list) + "\n"
        pydaw_util.pydaw_write_file_text(self.preset_path, f_result)
        self.load_presets()

    def program_changed(self, a_val=None):
        if str(self.program_combobox.currentText()) == "empty":
            print("empty")
        else:
            f_preset = self.presets_delimited[self.program_combobox.currentIndex()]
            print(("setting preset " + str(f_preset)))
            for f_i in range(1, len(f_preset)):
                f_port, f_val = f_preset[f_i].split(":")
                f_port = int(f_port)
                self.controls[f_port].set_value(f_val)
                self.controls[f_port].control_value_changed(f_val)

    def add_control(self, a_control):
        self.controls[a_control.port_num] = a_control

class pydaw_master_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_master_vol_port, a_master_glide_port, a_master_pitchbend_port, \
    a_port_dict, a_title="Master", a_master_uni_voices_port=None, a_master_uni_spread_port=None, a_preset_mgr=None):
        self.group_box = QtGui.QGroupBox()
        self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout(self.group_box)
        self.vol_knob = pydaw_knob_control(a_size, "Vol", a_master_vol_port, a_rel_callback, a_val_callback, -30, 12, -6, kc_integer, a_port_dict, a_preset_mgr)
        self.vol_knob.add_to_grid_layout(self.layout, 0)
        if a_master_uni_voices_port is not None and a_master_uni_spread_port is not None:
            self.uni_voices_knob = pydaw_knob_control(a_size, "Unison", a_master_uni_voices_port, a_rel_callback, a_val_callback,\
            1, 7, 4, kc_integer, a_port_dict, a_preset_mgr)
            self.uni_voices_knob.add_to_grid_layout(self.layout, 1)
            self.uni_spread_knob = pydaw_knob_control(a_size, "Spread", a_master_uni_spread_port, a_rel_callback, a_val_callback,\
            10, 100, 50, kc_decimal, a_port_dict, a_preset_mgr)
            self.uni_spread_knob.add_to_grid_layout(self.layout, 2)
        self.glide_knob = pydaw_knob_control(a_size, "Glide", a_master_glide_port, a_rel_callback, a_val_callback, 0, 200, 0, kc_decimal, a_port_dict, a_preset_mgr)
        self.glide_knob.add_to_grid_layout(self.layout, 3)
        self.pb_knob = pydaw_knob_control(a_size, "Pitchbend", a_master_pitchbend_port, a_rel_callback, a_val_callback, 1, 36, 18, kc_integer, \
        a_port_dict, a_preset_mgr)
        self.pb_knob.add_to_grid_layout(self.layout, 4)


pydaw_audio_item_scene_height = 1200.0  #TODO:  merge this with the one in pydaw_projects.py
pydaw_audio_item_scene_width = 6000.0
pydaw_audio_item_scene_rect = QtCore.QRectF(0.0, 0.0, pydaw_audio_item_scene_width, pydaw_audio_item_scene_height)

pydaw_start_end_gradient = QtGui.QLinearGradient(0.0, 0.0, 66.0, 66.0)
pydaw_start_end_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(246, 30, 30))
pydaw_start_end_gradient.setColorAt(1.0, QtGui.QColor.fromRgb(226, 42, 42))
pydaw_start_end_pen = QtGui.QPen(QtGui.QColor.fromRgb(246, 30, 30), 12.0)

pydaw_audio_item_gradient = QtGui.QLinearGradient(0.0, 0.0, 66.0, 66.0)
pydaw_audio_item_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(246, 246, 30))
pydaw_audio_item_gradient.setColorAt(1.0, QtGui.QColor.fromRgb(226, 226, 42))

pydaw_loop_gradient = QtGui.QLinearGradient(0.0, 0.0, 66.0, 66.0)
pydaw_loop_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(246, 180, 30))
pydaw_loop_gradient.setColorAt(1.0, QtGui.QColor.fromRgb(226, 180, 42))
pydaw_loop_pen = QtGui.QPen(QtGui.QColor.fromRgb(246, 180, 30), 12.0)

class pydaw_audio_marker_widget(QtGui.QGraphicsRectItem):
    def __init__(self, a_type, a_val, a_pen, a_brush, a_label, a_offset=0, a_callback=None):
        """ a_type:  0 == start, 1 == end, more types eventually... """
        self.audio_item_marker_height = 66.0
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, self.audio_item_marker_height, self.audio_item_marker_height)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.callback = a_callback
        self.line = QtGui.QGraphicsLineItem(0.0, 0.0, 0.0, pydaw_audio_item_scene_height)
        self.line.setParentItem(self)
        self.line.setPen(a_pen)
        self.marker_type = a_type
        self.pos_x = 0.0
        self.max_x = pydaw_audio_item_scene_width - self.audio_item_marker_height
        self.value = a_val
        self.other = None
        if a_type == 0:
            self.min_x = 0.0
            self.y_pos = 0.0 + (a_offset * self.audio_item_marker_height)
            self.line.setPos(0.0, self.y_pos * -1.0)
        elif a_type == 1:
            self.min_x = 66.0
            self.y_pos = pydaw_audio_item_scene_height - self.audio_item_marker_height - (a_offset * self.audio_item_marker_height)
            self.line.setPos(self.audio_item_marker_height, self.y_pos * -1.0)
        self.setPen(a_pen)
        self.setBrush(a_brush)
        self.text_item = QtGui.QGraphicsTextItem(a_label)
        self.text_item.setParentItem(self)
        self.text_item.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)

    def set_pos(self):
        if self.marker_type == 0:
            f_new_val = self.value * 0.6
        elif self.marker_type == 1:
            f_new_val = ((10000 - self.value) * 0.6) - self.audio_item_marker_height
        f_new_val = pydaw_util.pydaw_clip_value(f_new_val, self.min_x, self.max_x)
        self.setPos(f_new_val, self.y_pos)

    def set_other(self, a_other):
        self.other = a_other

    def get_inverted_value(self, a_val):  #TODO:  Get rid of this inversion goofyness at PyDAWv4
        return (a_val - 10000.0) * -1.0

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        self.pos_x = a_event.scenePos().x()
        self.pos_x = pydaw_util.pydaw_clip_value(self.pos_x, self.min_x, self.max_x)
        self.setPos(self.pos_x, self.y_pos)
        if self.marker_type == 0:
            f_new_val = self.pos_x * 1.666666
        elif self.marker_type == 1:
            f_new_val = (a_event.scenePos().x() + self.audio_item_marker_height) * 1.666666 # / 6.0
            f_new_val = (f_new_val - 10000) * -1.0
        f_new_val = pydaw_util.pydaw_clip_value(f_new_val, 0.0, 9940.0)
        self.value = f_new_val
        if self.other is not None:
            if self.marker_type == 0:
                f_inverted = self.get_inverted_value(self.other.value)
                if self.value > f_inverted - 60:
                    self.other.value = self.get_inverted_value(self.value) - 60
                    self.other.value = pydaw_util.pydaw_clip_value(self.other.value, 0.0, 9940.0)
                    self.other.set_pos()
            elif self.marker_type == 1:
                f_inverted = self.get_inverted_value(self.value)
                if self.other.value > f_inverted - 60:
                    self.other.value = f_inverted - 60
                    self.other.value = pydaw_util.pydaw_clip_value(self.other.value, 0.0, 9940.0)
                    self.other.set_pos()

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        if self.callback is not None:
            self.callback(self.value)
        if self.other.callback is not None:
            self.other.callback(self.other.value)


class pydaw_audio_item_viewer_widget(QtGui.QGraphicsView):
    def __init__(self, a_start_callback, a_end_callback, a_loop_start_callback, a_loop_end_callback):
        QtGui.QGraphicsView.__init__(self)
        self.start_callback = a_start_callback
        self.end_callback = a_end_callback
        self.loop_start_callback = a_loop_start_callback
        self.loop_end_callback = a_loop_end_callback
        self.scene = QtGui.QGraphicsScene()
        self.setScene(self.scene)
        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.scene.setBackgroundBrush(QtCore.Qt.darkGray)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.scroll_bar_height = self.horizontalScrollBar().height()
        self.last_x_scale = 1.0
        self.last_y_scale = 1.0
        self.waveform_brush = QtGui.QLinearGradient(0.0, 0.0, 1200.0, 6000.0)
        self.waveform_brush.setColorAt(0.0, QtGui.QColor(140, 140, 240))
        self.waveform_brush.setColorAt(0.5, QtGui.QColor(240, 190, 140))
        self.waveform_brush.setColorAt(1.0, QtGui.QColor(140, 140, 240))
        self.waveform_pen = QtGui.QPen(QtCore.Qt.white, 6.0)

    def clear_drawn_items(self):
        self.scene.clear()

    def draw_item(self, a_path_list, a_start, a_end, a_loop_start, a_loop_end):
        self.clear_drawn_items()
        f_path_inc = pydaw_audio_item_scene_height / len(a_path_list)
        f_path_y_pos = 0.0
        for f_path in a_path_list:
            f_path_item = QtGui.QGraphicsPathItem(f_path)
            f_path_item.setPen(self.waveform_pen)
            f_path_item.setBrush(self.waveform_brush)
            self.scene.addItem(f_path_item)
            f_path_item.setPos(0.0, f_path_y_pos)
            f_path_y_pos += f_path_inc
        self.start_marker = pydaw_audio_marker_widget(0, a_start, pydaw_start_end_pen, pydaw_start_end_gradient, "S", 0, self.start_callback)
        self.scene.addItem(self.start_marker)
        self.end_marker = pydaw_audio_marker_widget(1, a_end, pydaw_start_end_pen, pydaw_start_end_gradient, "E", 0, self.end_callback)
        self.scene.addItem(self.end_marker)
        self.loop_start_marker = pydaw_audio_marker_widget(0, a_loop_start, pydaw_loop_pen, pydaw_loop_gradient, "L", 1, self.loop_start_callback)
        self.scene.addItem(self.loop_start_marker)
        self.loop_end_marker = pydaw_audio_marker_widget(1, a_loop_end, pydaw_loop_pen, pydaw_loop_gradient, "L", 1, self.loop_end_callback)
        self.scene.addItem(self.loop_end_marker)
        self.start_marker.set_other(self.end_marker)
        self.end_marker.set_other(self.start_marker)
        self.loop_start_marker.set_other(self.loop_end_marker)
        self.loop_end_marker.set_other(self.loop_start_marker)
        self.start_marker.set_pos()
        self.end_marker.set_pos()
        self.loop_start_marker.set_pos()
        self.loop_end_marker.set_pos()

    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / pydaw_audio_item_scene_width
        self.last_y_scale = (f_rect.height() - self.scroll_bar_height) / pydaw_audio_item_scene_height
        self.scale(self.last_x_scale, self.last_y_scale)


class pydaw_modulex_single:
    def __init__(self, a_title, a_port_k1, a_rel_callback, a_val_callback, a_port_dict=None, a_preset_mgr=None):
        self.group_box = QtGui.QGroupBox()
        if a_title is not None:
            self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout()
        self.group_box.setLayout(self.layout)
        self.knobs = []
        for f_i in range(3):
            f_knob = pydaw_knob_control(51, "", a_port_k1 + f_i, a_rel_callback, a_val_callback, 0, 127, 64, a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr)
            f_knob.add_to_grid_layout(self.layout, f_i)
            self.knobs.append(f_knob)
        self.combobox = pydaw_combobox_control(132, "Type", a_port_k1 + 3, a_rel_callback, a_val_callback,
               ["Off", "LP2" , "LP4", "HP2", "HP4", "BP2", "BP4" , "Notch2", "Notch4", "EQ" , "Distortion",
                "Comb Filter", "Amp/Pan", "Limiter" , "Saturator", "Formant", "Chorus", "Glitch" , "RingMod",
                "LoFi", "S/H", "LP-Dry/Wet" , "HP-Dry/Wet", "Monofier", "LP<-->HP", "Growl Filter"], a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr, a_default_index=0)
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
        return pydaw_audio_item_fx(self.knobs[0].control.value(), self.knobs[1].control.value(), self.knobs[2].control.value(), self.combobox.control.currentIndex())

    def type_combobox_changed(self, a_val):
        if a_val == 0: #Off
            self.knobs[0].name_label.setText((""))
            self.knobs[1].name_label.setText((""))
            self.knobs[2].name_label.setText((""))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_none
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].value_label.setText((""))
            self.knobs[2].value_label.setText((""))
        elif a_val == 1: #LP2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 2: #LP4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 3: #HP2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 4: #HP4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 5: #BP2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 6: #BP4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 7: #Notch2
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 8: #Notch4
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 9: #EQ
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Q"))
            self.knobs[2].name_label.setText(("Gain"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_none
            self.knobs[2].val_conversion = kc_127_zero_to_x
            self.knobs[2].set_127_min_max(-24.0, 24.0)
            self.knobs[1].value_label.setText((""))
        elif a_val == 10: #Distortion
            self.knobs[0].name_label.setText(("Gain"))
            self.knobs[1].name_label.setText(("Dry/Wet"))
            self.knobs[2].name_label.setText(("Out Gain"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(0.0, 36.0)
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_127_zero_to_x
            self.knobs[2].set_127_min_max(-12.0, 0.0)
        elif a_val == 11: #Comb Filter
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Amt"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_none
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].value_label.setText((""))
            self.knobs[2].value_label.setText((""))
        elif a_val == 12: #Amp/Panner
            self.knobs[0].name_label.setText(("Pan"))
            self.knobs[1].name_label.setText(("Amp"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 6.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].value_label.setText((""))
            self.knobs[2].value_label.setText((""))
        elif a_val == 13: #Limiter
            self.knobs[0].name_label.setText(("Thresh"))
            self.knobs[1].name_label.setText(("Ceiling"))
            self.knobs[2].name_label.setText(("Release"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(-30.0, 0.0)
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-12.0, -0.1)
            self.knobs[2].val_conversion = kc_127_zero_to_x_int
            self.knobs[2].set_127_min_max(150.0, 400.0)
        elif a_val == 14: #Saturator
            self.knobs[0].name_label.setText(("InGain"))
            self.knobs[1].name_label.setText(("Amt"))
            self.knobs[2].name_label.setText(("OutGain"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(-12.0, 12.0)
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_127_zero_to_x
            self.knobs[2].set_127_min_max(-12.0, 12.0)
        elif a_val == 15: #Formant Filter
            self.knobs[0].name_label.setText(("Vowel"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 16: #Chorus
            self.knobs[0].name_label.setText(("Rate"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(0.3, 6.0)
            self.knobs[0].value_label.setText((""))
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 17: #Glitch
            self.knobs[0].name_label.setText(("Pitch"))
            self.knobs[1].name_label.setText(("Glitch"))
            self.knobs[2].name_label.setText(("Wet"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 18: #RingMod
            self.knobs[0].name_label.setText(("Pitch"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 19: #LoFi
            self.knobs[0].name_label.setText(("Bits"))
            self.knobs[1].name_label.setText(("unused"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(4.0, 16.0)
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 20: #Sample and Hold
            self.knobs[0].name_label.setText(("Pitch"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 21: #LP2-Dry/Wet
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("Wet"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 22: #HP2-Dry/Wet
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("Wet"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 23: #Monofier
            self.knobs[0].name_label.setText(("Pan"))
            self.knobs[1].name_label.setText(("Amp"))
            self.knobs[2].name_label.setText(("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 6.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].value_label.setText((""))
            self.knobs[2].value_label.setText((""))
        elif a_val == 24: #LP<-->HP
            self.knobs[0].name_label.setText(("Cutoff"))
            self.knobs[1].name_label.setText(("Res"))
            self.knobs[2].name_label.setText(("LP<->HP"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))
        elif a_val == 25: #Growl Filter
            self.knobs[0].name_label.setText(("Vowel"))
            self.knobs[1].name_label.setText(("Wet"))
            self.knobs[2].name_label.setText(("Type"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText((""))
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText((""))
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText((""))

        self.knobs[0].set_value(self.knobs[0].control.value())
        self.knobs[1].set_value(self.knobs[1].control.value())
        self.knobs[2].set_value(self.knobs[2].control.value())


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

    def clear_effects(self):
        for f_effect in self.effects:
            f_effect.combobox.set_value(0)
            for f_knob in f_effect.knobs:
                f_knob.set_value(64)

class pydaw_abstract_plugin_ui:
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, a_close_callback, a_configure_callback):
        self.track_num = int(a_track_num)
        self.pydaw_project = a_project
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.configure_callback = a_configure_callback
        self.track_type = int(a_track_type)
        self.widget = QtGui.QWidget()
        self.widget.setGeometry(0, 0, 10, 10)
        self.widget.setStyleSheet(str(a_stylesheet))
        self.widget.closeEvent = self.widget_close_event
        self.layout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.layout)
        self.port_dict = {}
        self.port_to_control_dict = {}
        self.control_to_port_dict = {}
        self.effects = []
        self.close_callback = a_close_callback
        self.configure_dict = {}
        self.save_file_on_exit = True
        self.is_quitting = False


    def delete_plugin_file(self):
        self.save_file_on_exit = False

    def open_plugin_file(self):
        f_file_path = self.pydaw_project.project_folder + "/" + self.folder + "/" + self.file
        if os.path.isfile(f_file_path):
            f_file = pydaw_plugin_file(f_file_path)
            for k, v in list(f_file.port_dict.items()):
                self.set_control_val(self.control_to_port_dict[int(k)], v)
            for k, v in list(f_file.configure_dict.items()):
                self.set_configure(k, v)
        else:
            print(("pydaw_abstract_plugin_ui.open_plugin_file(): + " + f_file_path + " did not exist, not loading."))

    def save_plugin_file(self):
        f_file = pydaw_plugin_file.from_dict(self.port_dict, self.port_to_control_dict, self.configure_dict)
        self.pydaw_project.save_file(self.folder, self.file, str(f_file))
        self.pydaw_project.commit("Update controls for " + self.track_name)
        self.pydaw_project.flush_history()

    def widget_close_event(self, a_event):
        if self.save_file_on_exit:
            self.save_plugin_file()
        if self.is_quitting:
            a_event.accept()
        else:
            self.widget.hide()
            self.close_callback(self.track_num, self.track_type)
            a_event.ignore()
        #QtGui.QWidget.closeEvent(self.widget, a_event)

    def plugin_rel_callback(self, a_port, a_val):
        self.rel_callback(self.is_instrument, self.track_type, self.track_num, a_port, a_val)

    def plugin_val_callback(self, a_port, a_val):
        self.val_callback(self.is_instrument, self.track_type, self.track_num, a_port, a_val)

    def set_control_val(self, a_port, a_val):
        f_port = int(a_port)
        if f_port in self.port_dict:
            self.port_dict[int(a_port)].set_value(a_val)
        else:
            print(("pydaw_abstract_plugin_ui.set_control_val():  Did not have port " + str(f_port)))

    def configure_plugin(self, a_key, a_message):
        """ Override this function to allow str|str key/value pair messages to be sent to the back-end"""
        pass

    def set_configure(self, a_key, a_message):
        """ Override this function to configure the plugin from the state file """
        pass

    def generate_control_dict(self):
        """ Aligning the file save format to the same as the parts forked from DSSI.
        TODO:  Change the file format and get rid of this at PyDAWv4"""
        f_keys = list(self.port_dict.keys())
        f_keys.sort()
        f_index = 0
        for f_key in f_keys:
            self.port_to_control_dict[f_key] = f_index
            self.control_to_port_dict[f_index] = f_key
            f_index += 1

    def set_window_title(self, a_track_name):
        pass  #Override this function



class pydaw_modulex_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, \
    a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, \
        a_close_callback, a_configure_callback)
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyfx"
        self.set_window_title(a_track_name)
        self.is_instrument = False

        self.preset_manager =  pydaw_preset_manager_widget("MODULEX")
        self.presets_hlayout = QtGui.QHBoxLayout()
        self.presets_hlayout.addWidget(self.preset_manager.group_box)
        self.presets_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.layout.addLayout(self.presets_hlayout)
        self.tab_widget = QtGui.QTabWidget()
        self.layout.addWidget(self.tab_widget)

        self.fx_tab = QtGui.QWidget()
        self.tab_widget.addTab(self.fx_tab, "Effects")
        self.fx_layout = QtGui.QGridLayout()
        self.fx_hlayout = QtGui.QHBoxLayout(self.fx_tab)
        self.fx_hlayout.addLayout(self.fx_layout)

        self.delay_tab = QtGui.QWidget()
        self.tab_widget.addTab(self.delay_tab, "Delay")
        self.delay_vlayout = QtGui.QVBoxLayout()
        self.delay_tab.setLayout(self.delay_vlayout)
        self.delay_hlayout = QtGui.QHBoxLayout()
        self.delay_vlayout.addLayout(self.delay_hlayout)

        self.delay_groupbox = QtGui.QGroupBox("Delay")
        self.delay_groupbox_layout = QtGui.QGridLayout(self.delay_groupbox)

        f_port = 4
        f_column = 0
        f_row = 0
        for f_i in range(8):
            f_effect = pydaw_modulex_single(("FX" + str(f_i)), f_port, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
            self.effects.append(f_effect)
            self.fx_layout.addWidget(f_effect.group_box, f_row, f_column)
            f_column += 1
            if f_column > 1:
                f_column = 0
                f_row += 1
            f_port += 4

        self.volume_gridlayout = QtGui.QGridLayout()
        self.fx_hlayout.addLayout(self.volume_gridlayout)
        self.volume_slider = pydaw_slider_control(QtCore.Qt.Vertical, "Vol", pydaw_ports.MODULEX_VOL_SLIDER, self.plugin_rel_callback, \
        self.plugin_val_callback, -60, 24, 0, kc_integer, self.port_dict)
        self.volume_slider.add_to_grid_layout(self.volume_gridlayout, 0)

        delay_groupbox =  QtGui.QGroupBox("Delay")
        delay_groupbox.setMaximumSize(387, 126)
        self.delay_hlayout.addWidget(delay_groupbox)
        delay_groupbox.setGeometry(0, 0, 10, 10)
        delay_gridlayout = QtGui.QGridLayout(delay_groupbox)
        self.delay_hlayout.addWidget(delay_groupbox)
        self.delay_time_knob =  pydaw_knob_control(51, "Time", pydaw_ports.MODULEX_DELAY_TIME, self.plugin_rel_callback, \
        self.plugin_val_callback, 10, 100, 50, kc_decimal, self.port_dict, self.preset_manager)
        self.delay_time_knob.add_to_grid_layout(delay_gridlayout, 0)
        m_feedback =  pydaw_knob_control(51, "Feedbk", pydaw_ports.MODULEX_FEEDBACK, self.plugin_rel_callback, \
        self.plugin_val_callback, -20, 0, -12, kc_integer, self.port_dict, self.preset_manager)
        m_feedback.add_to_grid_layout(delay_gridlayout, 1)
        m_dry =  pydaw_knob_control(51, "Dry", pydaw_ports.MODULEX_DRY, self.plugin_rel_callback, \
        self.plugin_val_callback, -30, 0, 0, kc_integer, self.port_dict, self.preset_manager)
        m_dry.add_to_grid_layout(delay_gridlayout, 2)
        m_wet =  pydaw_knob_control(51, "Wet", pydaw_ports.MODULEX_WET, self.plugin_rel_callback, \
        self.plugin_val_callback, -30, 0, -30, kc_integer, self.port_dict, self.preset_manager)
        m_wet.add_to_grid_layout(delay_gridlayout, 3)
        m_duck =  pydaw_knob_control(51, "Duck", pydaw_ports.MODULEX_DUCK, self.plugin_rel_callback, \
        self.plugin_val_callback, -40, 0, 0, kc_integer, self.port_dict, self.preset_manager)
        m_duck.add_to_grid_layout(delay_gridlayout, 4)
        m_cutoff =  pydaw_knob_control(51, "Cutoff", pydaw_ports.MODULEX_CUTOFF, self.plugin_rel_callback, \
        self.plugin_val_callback, 20, 124, 66, kc_pitch, self.port_dict, self.preset_manager)
        m_cutoff.add_to_grid_layout(delay_gridlayout, 5)
        m_stereo =  pydaw_knob_control(51, "Stereo", pydaw_ports.MODULEX_STEREO, self.plugin_rel_callback, \
        self.plugin_val_callback, 0, 100, 100, kc_decimal, self.port_dict, self.preset_manager)
        m_stereo.add_to_grid_layout(delay_gridlayout, 6)
        self.bpm_groupbox =  QtGui.QGroupBox()
        self.delay_hlayout.addWidget(self.bpm_groupbox)
        self.delay_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.bpm_groupbox.setGeometry(0, 0, 10, 10)
        self.bpm_groupbox.setTitle(("Tempo Sync"))
        self.bpm_groupbox_layout =  QtGui.QGridLayout(self.bpm_groupbox)
        self.bpm_spinbox =  QtGui.QDoubleSpinBox()
        #self.bpm_spinbox.setGeometry((100, 130, 71, 27))
        self.bpm_spinbox.setDecimals(1)
        self.bpm_spinbox.setRange(60, 200)
        self.bpm_spinbox.setSingleStep(0.1)
        self.bpm_spinbox.setValue(140.0)
        f_beat_fracs = ["1/4", "1/3", "1/2", "2/3", "3/4", "1"]
        self.beat_frac_combobox =  QtGui.QComboBox()
        self.beat_frac_combobox.setMinimumWidth(75)
        self.beat_frac_combobox.addItems(f_beat_fracs)
        self.beat_frac_combobox.setCurrentIndex(2)
        self.bpm_sync_button =  QtGui.QPushButton()
        self.bpm_sync_button.setText("Sync")
        self.bpm_sync_button.setMaximumWidth(60)
        self.bpm_sync_button.pressed.connect(self.bpmSyncPressed)
        f_bpm_label =  QtGui.QLabel("BPM")
        f_bpm_label.setMinimumWidth(60)
        f_beat_label =  QtGui.QLabel("Beats")
        f_beat_label.setMinimumWidth(60)
        self.bpm_groupbox_layout.addWidget(f_bpm_label, 0, 0, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(self.bpm_spinbox, 1, 0, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(f_beat_label, 0, 1, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(self.beat_frac_combobox, 1, 1, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(self.bpm_sync_button, 2, 1, QtCore.Qt.AlignCenter)
        reverb_groupbox =  QtGui.QGroupBox("Reverb")
        reverb_groupbox.setMaximumSize(186, 126)
        self.reverb_groupbox_gridlayout = QtGui.QGridLayout(reverb_groupbox)
        self.reverb_hlayout = QtGui.QHBoxLayout()
        self.delay_vlayout.addLayout(self.reverb_hlayout)
        self.reverb_hlayout.addWidget(reverb_groupbox)
        self.reverb_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        m_reverb_time =  pydaw_knob_control(51, "Time", pydaw_ports.MODULEX_REVERB_TIME, self.plugin_rel_callback, \
        self.plugin_val_callback, 0, 100, 50, kc_decimal, self.port_dict, self.preset_manager)
        m_reverb_time.add_to_grid_layout(self.reverb_groupbox_gridlayout, 0)
        m_reverb_wet =  pydaw_knob_control(51, "Wet", pydaw_ports.MODULEX_REVERB_WET, self.plugin_rel_callback, \
        self.plugin_val_callback, 0, 100, 0, kc_decimal, self.port_dict, self.preset_manager)
        m_reverb_wet.add_to_grid_layout(self.reverb_groupbox_gridlayout, 1)
        m_reverb_color =  pydaw_knob_control(51, "Color", pydaw_ports.MODULEX_REVERB_COLOR, self.plugin_rel_callback, \
        self.plugin_val_callback, 0, 100, 50, kc_decimal, self.port_dict, self.preset_manager)
        m_reverb_color.add_to_grid_layout(self.reverb_groupbox_gridlayout, 2)
        self.delay_spacer_layout = QtGui.QVBoxLayout()
        self.delay_vlayout.addLayout(self.delay_spacer_layout)
        self.delay_spacer_layout.addItem(QtGui.QSpacerItem(1, 1, vPolicy=QtGui.QSizePolicy.Expanding))

        self.generate_control_dict()
        self.open_plugin_file()

    def bpmSyncPressed(self):
        f_frac = 1.0
        f_switch = (self.beat_frac_combobox.currentIndex())
        if f_switch ==  0: # 1/4
            f_frac = 0.25
        elif f_switch ==  1: # 1/3
            f_frac = 0.3333
        elif f_switch ==  2: # 1/2
            f_frac = 0.5
        elif f_switch ==  3: # 2/3
            f_frac = 0.6666
        elif f_switch ==  4: # 3/4
            f_frac = 0.75
        elif f_switch ==  5: # 1
            f_frac = 1.0
        f_seconds_per_beat = 60/(self.bpm_spinbox.value())
        f_result = (int)(f_seconds_per_beat * f_frac * 100)
        self.delay_time_knob.set_value(f_result)

    def set_window_title(self, a_track_name):
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Modulex - " + self.track_name)


class pydaw_rayv_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, \
    a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, \
        a_close_callback, a_configure_callback)
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyinst"
        self.set_window_title(a_track_name)
        self.is_instrument = True
        f_osc_types = ["Saw" , "Square" , "Triangle" , "Sine" , "Off"]
        f_lfo_types = ["Off" , "Sine" , "Triangle"]
        self.preset_manager =  pydaw_preset_manager_widget("RAYV")
        self.main_layout = QtGui.QVBoxLayout()
        self.layout.addLayout(self.main_layout)
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout0)
        self.hlayout0.addWidget(self.preset_manager.group_box)
        self.hlayout0.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        f_logo_label =  QtGui.QLabel()
        f_pixmap = QtGui.QPixmap(pydaw_util.global_pydaw_install_prefix + "/lib/" + pydaw_util.global_pydaw_version_string +
        "/themes/default/rayv.png").scaled(120, 60, transformMode=QtCore.Qt.SmoothTransformation)
        f_logo_label.setMinimumSize(90, 30)
        f_logo_label.setPixmap(f_pixmap)
        self.hlayout0.addWidget(f_logo_label)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout1)
        self.osc1 =  pydaw_osc_widget(64, pydaw_ports.RAYV_OSC1_PITCH, pydaw_ports.RAYV_OSC1_TUNE, pydaw_ports.RAYV_OSC1_VOLUME, pydaw_ports.RAYV_OSC1_TYPE, \
        f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 1", self.port_dict, a_preset_mgr=self.preset_manager)
        self.hlayout1.addWidget(self.osc1.group_box)
        self.adsr_amp = pydaw_adsr_widget(64, True, pydaw_ports.RAYV_ATTACK, pydaw_ports.RAYV_DECAY, pydaw_ports.RAYV_SUSTAIN, pydaw_ports.RAYV_RELEASE, \
        "ADSR Amp", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout1.addWidget(self.adsr_amp.groupbox)
        self.groupbox_distortion =  QtGui.QGroupBox("Distortion")
        self.groupbox_distortion_layout = QtGui.QGridLayout(self.groupbox_distortion)
        self.hlayout1.addWidget(self.groupbox_distortion)
        self.dist =  pydaw_knob_control(64, "Gain", pydaw_ports.RAYV_DIST, self.plugin_rel_callback, self.plugin_val_callback, \
        -6, 36, 12, kc_integer, self.port_dict, self.preset_manager)
        self.dist.add_to_grid_layout(self.groupbox_distortion_layout, 0)
        self.dist_wet =  pydaw_knob_control(64, "Wet", pydaw_ports.RAYV_DIST_WET, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_none, self.port_dict, self.preset_manager)
        self.dist_wet.add_to_grid_layout(self.groupbox_distortion_layout, 1)
        self.groupbox_noise =  QtGui.QGroupBox("Noise")
        self.noise_layout = QtGui.QGridLayout(self.groupbox_noise)
        self.hlayout1.addWidget(self.groupbox_noise)
        self.noise_amp =  pydaw_knob_control(64, "Vol", pydaw_ports.RAYV_NOISE_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -60, 0, -30, kc_integer, self.port_dict, self.preset_manager)
        self.noise_amp.add_to_grid_layout(self.noise_layout, 0)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout2)
        self.osc2 =  pydaw_osc_widget(64, pydaw_ports.RAYV_OSC2_PITCH, pydaw_ports.RAYV_OSC2_TUNE, pydaw_ports.RAYV_OSC2_VOLUME, pydaw_ports.RAYV_OSC2_TYPE, \
        f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 2", self.port_dict, self.preset_manager, 4)
        self.hlayout2.addWidget(self.osc2.group_box)
        self.sync_groupbox =  QtGui.QGroupBox("Sync")
        self.hlayout2.addWidget(self.sync_groupbox)
        self.sync_gridlayout = QtGui.QGridLayout(self.sync_groupbox)
        self.hard_sync =  pydaw_checkbox_control("On", pydaw_ports.RAYV_OSC_HARD_SYNC, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict, self.preset_manager)
        self.hard_sync.control.setToolTip("Setting self hard sync's Osc1 to Osc2. Usually you would want to distort and pitchbend if this is enabled.")
        self.sync_gridlayout.addWidget(self.hard_sync.control, 1, 0, QtCore.Qt.AlignCenter)
        self.adsr_filter =  pydaw_adsr_widget(64, False, pydaw_ports.RAYV_FILTER_ATTACK, pydaw_ports.RAYV_FILTER_DECAY, pydaw_ports.RAYV_FILTER_SUSTAIN, \
        pydaw_ports.RAYV_FILTER_RELEASE, "ADSR Filter", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout2.addWidget(self.adsr_filter.groupbox)
        self.filter =  pydaw_filter_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_TIMBRE, pydaw_ports.RAYV_RES, a_preset_mgr=self.preset_manager)
        self.hlayout2.addWidget(self.filter.groupbox)
        self.filter_env_amt =  pydaw_knob_control(64, "Env Amt", pydaw_ports.RAYV_FILTER_ENV_AMT, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0, kc_integer, self.port_dict, self.preset_manager)
        self.filter_env_amt.add_to_grid_layout(self.filter.layout, 2)
        self.hlayout3 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout3)
        self.master =  pydaw_master_widget(64, self.plugin_rel_callback, self.plugin_val_callback, pydaw_ports.RAYV_MASTER_VOLUME, \
        pydaw_ports.RAYV_MASTER_GLIDE, pydaw_ports.RAYV_MASTER_PITCHBEND_AMT, self.port_dict, "Master", \
        pydaw_ports.RAYV_MASTER_UNISON_VOICES,pydaw_ports.RAYV_MASTER_UNISON_SPREAD, self.preset_manager)
        self.hlayout3.addWidget(self.master.group_box)
        self.pitch_env =  pydaw_ramp_env_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_PITCH_ENV_TIME, pydaw_ports.RAYV_PITCH_ENV_AMT, "Pitch Env", self.preset_manager)
        self.hlayout3.addWidget(self.pitch_env.groupbox)
        self.lfo =  pydaw_lfo_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_LFO_FREQ, pydaw_ports.RAYV_LFO_TYPE, f_lfo_types, "LFO", self.preset_manager)
        self.hlayout3.addWidget(self.lfo.groupbox)

        self.lfo_amp =  pydaw_knob_control(64, "Amp", pydaw_ports.RAYV_LFO_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -24, 24, 0, kc_integer, self.port_dict, self.preset_manager)
        self.lfo_amp.add_to_grid_layout(self.lfo.layout, 2)
        self.lfo_pitch =  pydaw_knob_control(64, "Pitch", pydaw_ports.RAYV_LFO_PITCH, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0, kc_integer, self.port_dict, self.preset_manager)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 3)
        self.lfo_cutoff =  pydaw_knob_control(64, "Filter", pydaw_ports.RAYV_LFO_FILTER, self.plugin_rel_callback, self.plugin_val_callback, \
        -48, 48, 0, kc_integer, self.port_dict, self.preset_manager)
        self.lfo_cutoff.add_to_grid_layout(self.lfo.layout, 4)

        self.generate_control_dict()
        self.open_plugin_file()

    def set_window_title(self, a_track_name):
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Ray-V - " + self.track_name)



class pydaw_wayv_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, \
    a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, \
        a_close_callback, a_configure_callback)
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyinst"
        self.set_window_title(a_track_name)
        self.is_instrument = True

        f_osc_types = ["Off",
            #Saw-like waves
            "Plain Saw" , "SuperbSaw" , "Viral Saw" , "Soft Saw" , "Mid Saw" , "Lush Saw",
            #Square-like waves
            "Evil Square" , "Punchy Square" , "Soft Square",
            #Glitchy and distorted waves
            "Pink Glitch" , "White Glitch" , "Acid" , "Screetch",
            #Sine and triangle-like waves
            "Thick Bass" , "Rattler" , "Deep Saw" , "Sine"
        ]
        f_lfo_types = [ "Off" , "Sine" , "Triangle"]
        self.tab_widget =  QtGui.QTabWidget()
        self.layout.addWidget(self.tab_widget)
        self.osc_tab =  QtGui.QWidget()
        self.tab_widget.addTab(self.osc_tab, ("Oscillators"))
        self.poly_fx_tab =  QtGui.QWidget()
        self.tab_widget.addTab(self.poly_fx_tab, ("PolyFX"))
        self.oscillator_layout =  QtGui.QVBoxLayout(self.osc_tab)
        self.preset_manager =  pydaw_preset_manager_widget("WAYV")
        self.hlayout0 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout0)
        self.hlayout0.addWidget(self.preset_manager.group_box)
        self.hlayout0.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))

        self.hlayout1 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout1)
        self.osc1 =  pydaw_osc_widget(51, pydaw_ports.WAYV_OSC1_PITCH, pydaw_ports.WAYV_OSC1_TUNE, pydaw_ports.WAYV_OSC1_VOLUME, \
        pydaw_ports.WAYV_OSC1_TYPE, f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 1", self.port_dict, self.preset_manager, 1)
        self.osc1_uni_voices =  pydaw_knob_control(51, "Unison", pydaw_ports.WAYV_OSC1_UNISON_VOICES, self.plugin_rel_callback, self.plugin_val_callback, \
        1, 7, 4, kc_integer, self.port_dict, self.preset_manager)
        self.osc1_uni_voices.add_to_grid_layout(self.osc1.grid_layout, 4)
        self.osc1_uni_spread =  pydaw_knob_control(51, "Spread", pydaw_ports.WAYV_OSC1_UNISON_SPREAD, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 50, kc_decimal, self.port_dict, self.preset_manager)
        self.osc1_uni_spread.add_to_grid_layout(self.osc1.grid_layout, 5)

        self.hlayout1.addWidget(self.osc1.group_box)

        self.adsr_amp1 =  pydaw_adsr_widget(51,  True, pydaw_ports.WAYV_ATTACK1, pydaw_ports.WAYV_DECAY1, pydaw_ports.WAYV_SUSTAIN1, \
        pydaw_ports.WAYV_RELEASE1, "ADSR Osc1", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout1.addWidget(self.adsr_amp1.groupbox)

        self.adsr_amp1_checkbox =  pydaw_checkbox_control("On", pydaw_ports.WAYV_ADSR1_CHECKBOX, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict, self.preset_manager)
        self.adsr_amp1_checkbox.add_to_grid_layout(self.adsr_amp1.layout, 4)

        self.groupbox_osc1_fm =  QtGui.QGroupBox("Osc1 FM")
        self.groupbox_osc1_fm_layout = QtGui.QGridLayout(self.groupbox_osc1_fm)

        self.osc1_fm1 =  pydaw_knob_control(51, "Osc1", pydaw_ports.WAYV_OSC1_FM1, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc1_fm1.add_to_grid_layout(self.groupbox_osc1_fm_layout, 0)

        self.osc1_fm2 =  pydaw_knob_control(51, "Osc2", pydaw_ports.WAYV_OSC1_FM2, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc1_fm2.add_to_grid_layout(self.groupbox_osc1_fm_layout, 1)

        self.osc1_fm3 =  pydaw_knob_control(51, "Osc3", pydaw_ports.WAYV_OSC1_FM3, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc1_fm3.add_to_grid_layout(self.groupbox_osc1_fm_layout, 2)

        self.hlayout1.addWidget(self.groupbox_osc1_fm)
        #self.hlayout1.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))

        #Osc2
        self.hlayout2 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout2)
        self.osc2 =  pydaw_osc_widget(51, pydaw_ports.WAYV_OSC2_PITCH, pydaw_ports.WAYV_OSC2_TUNE, pydaw_ports.WAYV_OSC2_VOLUME, \
        pydaw_ports.WAYV_OSC2_TYPE, f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 2", self.port_dict, self.preset_manager)
        self.osc2_uni_voices =  pydaw_knob_control(51, "Unison", pydaw_ports.WAYV_OSC2_UNISON_VOICES, self.plugin_rel_callback, self.plugin_val_callback, \
        1, 7, 4, kc_integer, self.port_dict, self.preset_manager)
        self.osc2_uni_voices.add_to_grid_layout(self.osc2.grid_layout, 4)
        self.osc2_uni_spread =  pydaw_knob_control(51, "Spread", pydaw_ports.WAYV_OSC2_UNISON_SPREAD, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 50, kc_decimal, self.port_dict, self.preset_manager)
        self.osc2_uni_spread.add_to_grid_layout(self.osc2.grid_layout, 5)

        self.hlayout2.addWidget(self.osc2.group_box)

        self.adsr_amp2 =  pydaw_adsr_widget(51,  True, pydaw_ports.WAYV_ATTACK2, pydaw_ports.WAYV_DECAY2, pydaw_ports.WAYV_SUSTAIN2, \
        pydaw_ports.WAYV_RELEASE2, "ADSR Osc2", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout2.addWidget(self.adsr_amp2.groupbox)

        self.adsr_amp2_checkbox =  pydaw_checkbox_control("On", pydaw_ports.WAYV_ADSR2_CHECKBOX, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict, self.preset_manager)
        self.adsr_amp2_checkbox.add_to_grid_layout(self.adsr_amp2.layout, 4)

        self.groupbox_osc2_fm =  QtGui.QGroupBox("Osc2 FM")
        self.groupbox_osc2_fm_layout = QtGui.QGridLayout(self.groupbox_osc2_fm)

        self.osc2_fm1 =  pydaw_knob_control(51, "Osc1", pydaw_ports.WAYV_OSC2_FM1, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc2_fm1.add_to_grid_layout(self.groupbox_osc2_fm_layout, 0)

        self.osc2_fm2 =  pydaw_knob_control(51, "Osc2", pydaw_ports.WAYV_OSC2_FM2, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc2_fm2.add_to_grid_layout(self.groupbox_osc2_fm_layout, 1)

        self.osc2_fm3 =  pydaw_knob_control(51, "Osc3", pydaw_ports.WAYV_OSC2_FM3, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc2_fm3.add_to_grid_layout(self.groupbox_osc2_fm_layout, 2)

        self.hlayout2.addWidget(self.groupbox_osc2_fm)
        #self.hlayout2.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))

        #osc3
        self.hlayout3 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout3)
        self.osc3 =  pydaw_osc_widget(51, pydaw_ports.WAYV_OSC3_PITCH, pydaw_ports.WAYV_OSC3_TUNE, pydaw_ports.WAYV_OSC3_VOLUME, \
        pydaw_ports.WAYV_OSC3_TYPE, f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 3", self.port_dict, self.preset_manager)
        self.osc3_uni_voices =  pydaw_knob_control(51, "Unison", pydaw_ports.WAYV_OSC3_UNISON_VOICES, self.plugin_rel_callback, self.plugin_val_callback, \
        1, 7, 4, kc_integer, self.port_dict, self.preset_manager)
        self.osc3_uni_voices.add_to_grid_layout(self.osc3.grid_layout, 4)
        self.osc3_uni_spread =  pydaw_knob_control(51, "Spread", pydaw_ports.WAYV_OSC3_UNISON_SPREAD, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 50, kc_decimal, self.port_dict, self.preset_manager)
        self.osc3_uni_spread.add_to_grid_layout(self.osc3.grid_layout, 5)

        self.hlayout3.addWidget(self.osc3.group_box)

        self.adsr_amp3 =  pydaw_adsr_widget(51,  True, pydaw_ports.WAYV_ATTACK3, pydaw_ports.WAYV_DECAY3, pydaw_ports.WAYV_SUSTAIN3, \
        pydaw_ports.WAYV_RELEASE3, "ADSR Osc3", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout3.addWidget(self.adsr_amp3.groupbox)

        self.adsr_amp3_checkbox =  pydaw_checkbox_control("On", pydaw_ports.WAYV_ADSR3_CHECKBOX, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict, self.preset_manager)
        self.adsr_amp3_checkbox.add_to_grid_layout(self.adsr_amp3.layout, 4)

        self.groupbox_osc3_fm =  QtGui.QGroupBox("Osc3 FM")
        self.groupbox_osc3_fm_layout = QtGui.QGridLayout(self.groupbox_osc3_fm)

        self.osc3_fm1 =  pydaw_knob_control(51, "Osc1", pydaw_ports.WAYV_OSC3_FM1, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc3_fm1.add_to_grid_layout(self.groupbox_osc3_fm_layout, 0)

        self.osc3_fm2 =  pydaw_knob_control(51, "Osc2", pydaw_ports.WAYV_OSC3_FM2, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc3_fm2.add_to_grid_layout(self.groupbox_osc3_fm_layout, 1)

        self.osc3_fm3 =  pydaw_knob_control(51, "Osc3", pydaw_ports.WAYV_OSC3_FM3, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict, self.preset_manager)
        self.osc3_fm3.add_to_grid_layout(self.groupbox_osc3_fm_layout, 2)

        self.hlayout3.addWidget(self.groupbox_osc3_fm)
        #self.hlayout3.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))

        self.hlayout4 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout4)
        self.master =  pydaw_master_widget(51,  self.plugin_rel_callback, self.plugin_val_callback, pydaw_ports.WAYV_MASTER_VOLUME, \
        pydaw_ports.WAYV_MASTER_GLIDE, pydaw_ports.WAYV_MASTER_PITCHBEND_AMT, self.port_dict, a_preset_mgr=self.preset_manager)

        self.hlayout4.addWidget(self.master.group_box)

        self.adsr_amp_main =  pydaw_adsr_widget(51, True, pydaw_ports.WAYV_ATTACK_MAIN, pydaw_ports.WAYV_DECAY_MAIN, \
        pydaw_ports.WAYV_SUSTAIN_MAIN, pydaw_ports.WAYV_RELEASE_MAIN, "ADSR Master", self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict, self.preset_manager)
        self.hlayout4.addWidget(self.adsr_amp_main.groupbox)

        self.groupbox_noise =  QtGui.QGroupBox("Noise")
        self.groupbox_noise_layout = QtGui.QGridLayout(self.groupbox_noise)
        self.hlayout4.addWidget(self.groupbox_noise)
        self.noise_amp =  pydaw_knob_control(51, "Vol", pydaw_ports.WAYV_NOISE_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -60, 0, -30, kc_integer, self.port_dict, self.preset_manager)
        self.noise_amp.add_to_grid_layout(self.groupbox_noise_layout, 0)

        self.noise_type =  pydaw_combobox_control(87, "Type", pydaw_ports.LMS_NOISE_TYPE, self.plugin_rel_callback, self.plugin_val_callback, \
        ["Off", "White", "Pink"], self.port_dict, a_preset_mgr=self.preset_manager)
        self.noise_type.control.setMaximumWidth(87)
        self.noise_type.add_to_grid_layout(self.groupbox_noise_layout, 1)

        self.main_layout =  QtGui.QVBoxLayout(self.poly_fx_tab)
        self.hlayout5 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout5)
        self.hlayout6 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout6)
        #From Modulex
        self.fx0 =  pydaw_modulex_single("FX0", pydaw_ports.WAYV_FX0_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout5.addWidget(self.fx0.group_box)
        self.fx1 =  pydaw_modulex_single("FX1", pydaw_ports.WAYV_FX1_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout5.addWidget(self.fx1.group_box)
        self.fx2 =  pydaw_modulex_single("FX2", pydaw_ports.WAYV_FX2_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout6.addWidget(self.fx2.group_box)
        self.fx3 =  pydaw_modulex_single("FX3", pydaw_ports.WAYV_FX3_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout6.addWidget(self.fx3.group_box)

        self.mod_matrix = QtGui.QTableWidget()
        self.mod_matrix.setRowCount(4)
        self.mod_matrix.setColumnCount(12)
        self.mod_matrix.setFixedHeight(172)
        self.mod_matrix.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setHorizontalHeaderLabels(["FX0\nCtrl1", "FX0\nCtrl2", "FX0\nCtrl3", "FX1\nCtrl1", "FX1\nCtrl2", "FX1\nCtrl3", "FX2\nCtrl1",
            "FX2\nCtrl2", "FX2\nCtrl3", "FX3\nCtrl1", "FX3\nCtrl2", "FX3\nCtrl3" ])
        self.mod_matrix.setVerticalHeaderLabels(["ADSR 1", "ADSR 2", "Ramp Env", "LFO"])
        f_port_num = pydaw_ports.WAVV_PFXMATRIX_FIRST_PORT

        for f_i_dst in range(4):
            for f_i_src in range(4):
                for f_i_ctrl in range(3):
                    f_ctrl = pydaw_spinbox_control(None, f_port_num, self.plugin_rel_callback, self.plugin_val_callback, -100, 100, 0, kc_none, \
                    self.port_dict, self.preset_manager)
                    f_x = (f_i_dst * 3) + f_i_ctrl
                    self.mod_matrix.setCellWidget(f_i_src, f_x, f_ctrl.control)
                    f_port_num += 1

        self.main_layout.addWidget(self.mod_matrix)
        self.mod_matrix.resizeColumnsToContents()

        self.hlayout7 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout7)

        self.adsr_amp =  pydaw_adsr_widget(51, True, pydaw_ports.WAYV_ATTACK_PFX1, pydaw_ports.WAYV_DECAY_PFX1, pydaw_ports.WAYV_SUSTAIN_PFX1, \
        pydaw_ports.WAYV_RELEASE_PFX1, "ADSR 1", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        #self.adsr_amp.lms_release.lms_knob.setMinimum(5) #overriding the default for self, because we want a low minimum default that won't click
        self.hlayout7.addWidget(self.adsr_amp.groupbox)

        self.adsr_filter =  pydaw_adsr_widget(51,  False, pydaw_ports.WAYV_ATTACK_PFX2, pydaw_ports.WAYV_DECAY_PFX2, pydaw_ports.WAYV_SUSTAIN_PFX2, \
        pydaw_ports.WAYV_RELEASE_PFX2, "ADSR 2", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, self.preset_manager)
        self.hlayout7.addWidget(self.adsr_filter.groupbox)

        self.pitch_env =  pydaw_ramp_env_widget(51, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.WAYV_RAMP_ENV_TIME, pydaw_ports.WAYV_PITCH_ENV_AMT, "Ramp Env", self.preset_manager)
        self.pitch_env.amt_knob.name_label.setText("Pitch")
        self.hlayout7.addWidget(self.pitch_env.groupbox)

        self.lfo =  pydaw_lfo_widget(51,  self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, pydaw_ports.WAYV_LFO_FREQ, \
        pydaw_ports.WAYV_LFO_TYPE, f_lfo_types, "LFO", self.preset_manager)
        self.hlayout7.addWidget(self.lfo.groupbox)

        self.lfo_amount =  pydaw_knob_control(51, "Amount", pydaw_ports.WAYV_LFO_AMOUNT, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 100, kc_decimal, self.port_dict, self.preset_manager)
        self.lfo_amount.add_to_grid_layout(self.lfo.layout, 2)

        self.lfo_amp =  pydaw_knob_control(51, "Amp", pydaw_ports.WAYV_LFO_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -24, 24, 0, kc_integer, self.port_dict, self.preset_manager)
        self.lfo_amp.add_to_grid_layout(self.lfo.layout, 3)

        self.lfo_pitch =  pydaw_knob_control(51, "Pitch", pydaw_ports.WAYV_LFO_PITCH, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0,  kc_integer, self.port_dict, self.preset_manager)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 4)

        self.generate_control_dict()
        self.open_plugin_file()

    def set_window_title(self, a_track_name):
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Way-V - " + self.track_name)

"""Used for outputting sampler parameters to text files"""
LMS_DELIMITER  =  "|"

SMP_TB_RADIOBUTTON_INDEX  =  0
SMP_TB_FILE_PATH_INDEX  =  1
SMP_TB_NOTE_INDEX  =  2
SMP_TB_LOW_NOTE_INDEX  =  3
SMP_TB_HIGH_NOTE_INDEX  =  4
SMP_TB_VOLUME_INDEX  =  5
SMP_TB_VEL_SENS_INDEX  =  6
SMP_TB_VEL_LOW_INDEX  =  7
SMP_TB_VEL_HIGH_INDEX  =  8
SMP_TB_PITCH_INDEX  =  9
SMP_TB_TUNE_INDEX  =  10
SMP_TB_INTERPOLATION_MODE_INDEX  =  11

class pydaw_euphoria_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, \
    a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, \
        a_close_callback, a_configure_callback)
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyinst"
        self.set_window_title(a_track_name)
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Euphoria - " + self.track_name)
        self.is_instrument = True
        #Begin Euphoria C++
        self.selected_row_index = 0
        self.handle_control_updates = True
        self.creating_instrument_file = False
        self.suppress_selected_sample_changed = False
        f_interpolation_modes = [("Pitched") , ("Percussion") , ("No Pitch")]
        f_sample_table_columns = [
            "", #Selected row
            "Path", #File path
            "Sample Pitch", #Sample base pitch
            "Low Note", #Low Note
            "High Note", #High Note
            "Volume", #Volume
            "Vel. Sens.", #Velocity Sensitivity
            "Low Vel.", #Low Velocity
            "High Vel.", #High Velocity
            "Pitch", #Pitch
            "Tune", #Tune
            "Mode", #Interpolation Mode
            "Noise Type",
            "Noise Amp",
        ]

        self.selected_sample_port = pydaw_null_control(pydaw_ports.EUPHORIA_SELECTED_SAMPLE, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)

        self.sample_table = QtGui.QTableWidget(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT, len(f_sample_table_columns))
        self.sample_table.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.sample_table.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)

        self.selected_radiobuttons = []
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_radiobutton = QtGui.QRadioButton(self.sample_table)
            self.selected_radiobuttons.append(f_radiobutton)
            self.sample_table.setCellWidget(f_i, 0, f_radiobutton)
            f_radiobutton.clicked.connect(self.selectionChanged)

        self.sample_base_pitches = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_pitch = pydaw_note_selector_widget(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, 60)
            self.sample_table.setCellWidget(f_i, 2, f_sample_pitch.widget)
            self.sample_base_pitches.append(f_sample_pitch)

        self.sample_low_notes = []
        f_port_start = pydaw_ports.EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_low_pitch = pydaw_note_selector_widget(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, 0)
            self.sample_table.setCellWidget(f_i, 3, f_low_pitch.widget)
            self.sample_low_notes.append(f_low_pitch)

        self.sample_high_notes = []
        f_port_start = pydaw_ports.EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_high_pitch = pydaw_note_selector_widget(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, 120)
            self.sample_table.setCellWidget(f_i, 4, f_high_pitch.widget)
            self.sample_high_notes.append(f_high_pitch)

        self.sample_vols = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_vol = pydaw_spinbox_control(None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            -50.0, 36.0, 0.0, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 5, f_sample_vol.control)
            self.sample_vols.append(f_sample_vol)

        self.sample_vel_sens = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_vel_sens = pydaw_spinbox_control(None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            0, 20, 10, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 6, f_vel_sens.control)
            self.sample_vols.append(f_vel_sens)

        self.sample_low_vels = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_vel_low = pydaw_spinbox_control(None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            0, 127, 0, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 7, f_vel_low.control)
            self.sample_vols.append(f_vel_low)

        self.sample_high_vels = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_vel_high = pydaw_spinbox_control(None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            0, 127, 127, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 8, f_vel_high.control)
            self.sample_vols.append(f_vel_high)

        self.sample_pitches = []
        f_port_start = pydaw_ports.EUPHORIA_PITCH_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_pitch = pydaw_spinbox_control(None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            -36, 36, 0, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 9, f_sample_pitch.control)
            self.sample_vols.append(f_sample_pitch)

        self.sample_tunes = []
        f_port_start = pydaw_ports.EUPHORIA_TUNE_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_tune = pydaw_spinbox_control(None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            -100, 100, 0, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 10, f_sample_tune.control)
            self.sample_vols.append(f_sample_tune)

        self.sample_modes = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_mode = pydaw_combobox_control(120, None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            f_interpolation_modes, self.port_dict, 1)
            self.sample_table.setCellWidget(f_i, 11, f_sample_mode.control)
            self.sample_vols.append(f_sample_mode)

        self.noise_types = []
        f_port_start = pydaw_ports.EUPHORIA_NOISE_TYPE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_noise_type = pydaw_combobox_control(75, None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            [("Off") , ("White") , ("Pink")], self.port_dict, 0)
            self.sample_table.setCellWidget(f_i, 12, f_noise_type.control)
            self.noise_types.append(f_noise_type)

        self.noise_amps = []
        f_port_start = pydaw_ports.EUPHORIA_NOISE_AMP_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_noise_amp = pydaw_spinbox_control(None, f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, \
            -60, 0, -30, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 13, f_noise_amp.control)
            self.noise_amps.append(f_noise_amp)



        self.sample_starts = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_START_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_start = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.sample_starts.append(f_sample_start)

        self.sample_ends = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_END_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_end = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)  #TODO: at PyDAWv4 make it 10000
            self.sample_ends.append(f_sample_end)

        self.loop_starts = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_loop_start = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.loop_starts.append(f_loop_start)

        self.loop_modes = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_loop_mode = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.loop_modes.append(f_loop_mode)

        self.loop_ends = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_loop_end = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict) #TODO: at PyDAWv4 make it 10000
            self.loop_ends.append(f_loop_end)
        #MonoFX0
        self.monofx0knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx0knob0_ctrls.append(f_ctrl)

        self.monofx0knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx0knob1_ctrls.append(f_ctrl)

        self.monofx0knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx0knob2_ctrls.append(f_ctrl)

        self.monofx0comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.monofx0comboboxes.append(f_ctrl)
        #MonoFX1
        self.monofx1knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx1knob0_ctrls.append(f_ctrl)

        self.monofx1knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx1knob1_ctrls.append(f_ctrl)

        self.monofx1knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx1knob2_ctrls.append(f_ctrl)

        self.monofx1comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.monofx1comboboxes.append(f_ctrl)
        #MonoFX2
        self.monofx2knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx2knob0_ctrls.append(f_ctrl)

        self.monofx2knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx2knob1_ctrls.append(f_ctrl)

        self.monofx2knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx2knob2_ctrls.append(f_ctrl)

        self.monofx2comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.monofx2comboboxes.append(f_ctrl)
        #MonoFX3
        self.monofx3knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx3knob0_ctrls.append(f_ctrl)

        self.monofx3knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx3knob1_ctrls.append(f_ctrl)

        self.monofx3knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 64, self.port_dict)
            self.monofx3knob2_ctrls.append(f_ctrl)

        self.monofx3comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.monofx3comboboxes.append(f_ctrl)

        self.monofx_groups = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_monofx_group = pydaw_null_control(f_port_start + f_i, self.plugin_rel_callback, self.plugin_val_callback, 0, self.port_dict)
            self.monofx_groups.append(f_monofx_group)


        self.sample_table.setHorizontalHeaderLabels(f_sample_table_columns)
        self.sample_table.verticalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.sample_table.horizontalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.sample_table.resizeRowsToContents()

        self.file_selector =  pydaw_file_select_widget()
        self.file_selector.open_button.pressed.connect(self.fileSelect)
        self.file_selector.clear_button.pressed.connect(self.clearFile)
        self.file_selector.reload_button.pressed.connect(self.reloadSample)
        self.file_selector.file_path.setMinimumWidth(480)
        """Set all of the array variables that are per-sample"""

        actionMove_files_to_single_directory =  QtGui.QAction("Move files to single directory", self.widget)
        #actionSave_instrument_to_file =  QtGui.QAction("Save instrument to file", self.widget)
        #actionOpen_instrument_from_file =  QtGui.QAction("Open instrument from file", self.widget)
        actionMapToWhiteKeys =  QtGui.QAction("Map All Samples to 1 White Key", self.widget)
        actionMapToMonoFX =  QtGui.QAction("Map All Samples to Own MonoFX Group", self.widget)
        actionClearAllSamples =  QtGui.QAction("Clear All Samples", self.widget)
        menubar =  QtGui.QMenuBar(self.widget)
        menuFile =  QtGui.QMenu("Menu", menubar)
        menubar.addAction(menuFile.menuAction())
        menuFile.addAction(actionMove_files_to_single_directory)
        #menuFile.addAction(actionSave_instrument_to_file)
        #menuFile.addAction(actionOpen_instrument_from_file)
        menuFile.addAction(actionMapToWhiteKeys)
        menuFile.addAction(actionMapToMonoFX)
        menuFile.addAction(actionClearAllSamples)

        actionMove_files_to_single_directory.triggered.connect(self.moveSamplesToSingleDirectory)
        #actionSave_instrument_to_file.triggered.connect(self.saveToFile)
        #actionOpen_instrument_from_file.triggered.connect(self.openFromFile)
        actionMapToWhiteKeys.triggered.connect(self.mapAllSamplesToOneWhiteKey)
        actionMapToMonoFX.triggered.connect(self.mapAllSamplesToOneMonoFXgroup)
        actionClearAllSamples.triggered.connect(self.clearAllSamples)

        self.widget.resize(1200, 680)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widget.sizePolicy().hasHeightForWidth())
        self.widget.setSizePolicy(sizePolicy)
        self.layout.addWidget(menubar)
        self.main_tab =  QtGui.QTabWidget()

        self.sample_tab =  QtGui.QWidget()
        self.sample_tab_layout = QtGui.QVBoxLayout(self.sample_tab)
        self.sample_tab_horizontal_splitter =  QtGui.QSplitter(QtCore.Qt.Horizontal)
        self.sample_tab_layout.addWidget(self.sample_tab_horizontal_splitter)

        self.file_browser =  pydaw_file_browser_widget()
        self.file_browser.load_pushButton.pressed.connect(self.file_browser_load_button_pressed)
        self.file_browser.preview_pushButton.pressed.connect(self.file_browser_preview_button_pressed)
        self.preview_file = ""  #TODO:  Get rid of this at PyDAWv4, it's no longer needed
        self.sample_tab_horizontal_splitter.addWidget(self.file_browser.file_browser_vsplitter)

        self.smp_tab_main_widget = QtGui.QWidget()
        self.smp_tab_main_verticalLayout = QtGui.QVBoxLayout(self.smp_tab_main_widget)
        self.sample_tab_horizontal_splitter.addWidget(self.smp_tab_main_widget)

        self.smp_tab_main_verticalLayout.addWidget(self.sample_table, QtCore.Qt.AlignCenter)
        self.smp_tab_main_verticalLayout.addLayout(self.file_selector.layout)

        f_logo_label =  QtGui.QLabel()
        f_pixmap = QtGui.QPixmap("%s/lib/%s/themes/default/euphoria.png" %
        (pydaw_util.global_pydaw_install_prefix, pydaw_util.global_pydaw_version_string \
        )).scaled(80, 80, transformMode=QtCore.Qt.SmoothTransformation)
        f_logo_label.setPixmap(f_pixmap)
        f_logo_label.setAlignment(QtCore.Qt.AlignCenter)
        self.file_selector.layout.addWidget(f_logo_label, -1, QtCore.Qt.AlignRight)
        self.main_tab.addTab(self.sample_tab, "Samples")
        self.poly_fx_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.poly_fx_tab, "Poly FX")
        self.mono_fx_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.mono_fx_tab, "Mono FX")
        self.layout.addWidget(self.main_tab)
        self.main_tab.setCurrentIndex(0)
        self.sample_table.resizeColumnsToContents()
        #m_view_sample_tab
        self.view_sample_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.view_sample_tab, "View")
        self.view_sample_tab_main_vlayout =  QtGui.QVBoxLayout(self.view_sample_tab)
        self.view_sample_tab_main_vlayout.setContentsMargins(0, 0, 0, 0)

        #Sample Graph
        self.sample_graph = pydaw_audio_item_viewer_widget(self.sample_start_callback, self.sample_end_callback, self.loop_start_callback, self.loop_end_callback)
        self.view_sample_tab_main_vlayout.addWidget(self.sample_graph)
        #The combobox for selecting the sample on the 'view' tab
        self.sample_view_select_sample_widget = QtGui.QWidget()
        self.sample_view_select_sample_widget.setMaximumHeight(200)
        self.sample_view_select_sample_hlayout =  QtGui.QHBoxLayout(self.sample_view_select_sample_widget)

        self.sample_view_extra_controls_left_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_select_sample_hlayout.addItem(self.sample_view_extra_controls_left_hspacer)
        self.sample_view_extra_controls_gridview =  QtGui.QGridLayout()
        self.selected_sample_index_combobox =  QtGui.QComboBox()
        sizePolicy1 = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.selected_sample_index_combobox.sizePolicy().hasHeightForWidth())
        self.selected_sample_index_combobox.setSizePolicy(sizePolicy1)
        self.selected_sample_index_combobox.setMinimumWidth(320)
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.selected_sample_index_combobox.addItem("")
        self.selected_sample_index_combobox.currentIndexChanged.connect(self.viewSampleSelectedIndexChanged)
        self.sample_view_extra_controls_gridview.addWidget(self.selected_sample_index_combobox, 1, 0, 1, 1)
        self.selected_sample_index_label =  QtGui.QLabel("Selected Sample")
        self.sample_view_extra_controls_gridview.addWidget(self.selected_sample_index_label, 0, 0, 1, 1)
        self.sample_view_select_sample_hlayout.addLayout(self.sample_view_extra_controls_gridview)
        self.sample_view_extra_controls_right_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_select_sample_hlayout.addItem(self.sample_view_extra_controls_right_hspacer)
        self.view_sample_tab_main_vlayout.addWidget(self.sample_view_select_sample_widget)
        #The loop mode combobox
        self.loop_mode_combobox =  QtGui.QComboBox(self.view_sample_tab)
        self.loop_mode_combobox.addItems(["Off", "On"])
        self.loop_mode_label =  QtGui.QLabel("Loop Mode")
        self.loop_mode_combobox.currentIndexChanged.connect(self.loopModeChanged)
        self.sample_view_extra_controls_gridview.addWidget(self.loop_mode_label, 0, 1, 1, 1)
        self.sample_view_extra_controls_gridview.addWidget(self.loop_mode_combobox, 1, 1, 1, 1)

        #The file select on the 'view' tab
        self.sample_view_file_select_hlayout =  QtGui.QHBoxLayout()
        self.sample_view_file_select_left_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_file_select_hlayout.addItem(self.sample_view_file_select_left_hspacer)
        self.view_file_selector =  pydaw_file_select_widget()
        self.view_file_selector.open_button.pressed.connect(self.fileSelect)
        self.view_file_selector.clear_button.pressed.connect(self.clearFile)
        self.view_file_selector.reload_button.pressed.connect(self.reloadSample)
        self.view_file_selector.file_path.setMinimumWidth(400)
        self.sample_view_file_select_hlayout.addLayout(self.view_file_selector.layout)
        self.sample_view_file_select_right_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_file_select_hlayout.addItem(self.sample_view_file_select_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_view_file_select_hlayout)

        f_lfo_types = ["Off" , "Sine" , "Triangle"]

        self.main_layout =  QtGui.QVBoxLayout(self.poly_fx_tab)
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout0)
        self.fx0 =  pydaw_modulex_single("FX0", pydaw_ports.EUPHORIA_FX0_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout0.addWidget(self.fx0.group_box)
        self.fx1 =  pydaw_modulex_single("FX1", pydaw_ports.EUPHORIA_FX1_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout0.addWidget(self.fx1.group_box)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout1)
        self.fx2 =  pydaw_modulex_single("FX2", pydaw_ports.EUPHORIA_FX2_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout1.addWidget(self.fx2.group_box)
        self.fx3 =  pydaw_modulex_single("FX3", pydaw_ports.EUPHORIA_FX3_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout1.addWidget(self.fx3.group_box)

        self.mod_matrix = QtGui.QTableWidget()
        self.mod_matrix.setRowCount(4)
        self.mod_matrix.setColumnCount(12)
        self.mod_matrix.setFixedHeight(172)
        self.mod_matrix.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setHorizontalHeaderLabels(["FX0\nCtrl1", "FX0\nCtrl2", "FX0\nCtrl3", "FX1\nCtrl1", "FX1\nCtrl2", "FX1\nCtrl3", "FX2\nCtrl1",
            "FX2\nCtrl2", "FX2\nCtrl3", "FX3\nCtrl1", "FX3\nCtrl2", "FX3\nCtrl3" ])
        self.mod_matrix.setVerticalHeaderLabels(["ADSR 1", "ADSR 2", "Ramp Env", "LFO"])
        f_port_num = pydaw_ports.EUPHORIA_PFXMATRIX_FIRST_PORT

        for f_i_dst in range(4):
            for f_i_src in range(4):
                for f_i_ctrl in range(3):
                    f_ctrl = pydaw_spinbox_control(None, f_port_num, self.plugin_rel_callback, self.plugin_val_callback, -100, 100, 0, kc_none, self.port_dict)
                    f_x = (f_i_dst * 3) + f_i_ctrl
                    self.mod_matrix.setCellWidget(f_i_src, f_x, f_ctrl.control)
                    f_port_num += 1

        self.main_layout.addWidget(self.mod_matrix)
        self.mod_matrix.resizeColumnsToContents()

        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout2)
        #End from Modulex
        self.adsr_amp =  pydaw_adsr_widget(55, True, pydaw_ports.EUPHORIA_ATTACK, pydaw_ports.EUPHORIA_DECAY, pydaw_ports.EUPHORIA_SUSTAIN, \
        pydaw_ports.EUPHORIA_RELEASE, "ADSR Amp", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, a_attack_default=0)
        self.adsr_amp.release_knob.control.setMinimum(5) #overriding the default for self, because we want a low minimum default that won't click
        self.hlayout2.addWidget(self.adsr_amp.groupbox)
        self.adsr_filter =  pydaw_adsr_widget(55, False, pydaw_ports.EUPHORIA_FILTER_ATTACK, pydaw_ports.EUPHORIA_FILTER_DECAY, \
        pydaw_ports.EUPHORIA_FILTER_SUSTAIN, pydaw_ports.EUPHORIA_FILTER_RELEASE, "ADSR 2", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout2.addWidget(self.adsr_filter.groupbox)
        self.pitch_env =  pydaw_ramp_env_widget(55, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.EUPHORIA_PITCH_ENV_TIME, None, "Ramp Env")

        self.hlayout2.addWidget(self.pitch_env.groupbox)

        self.lfo =  pydaw_lfo_widget(55, self.plugin_rel_callback, self.plugin_val_callback,  self.port_dict, \
        pydaw_ports.EUPHORIA_LFO_FREQ, pydaw_ports.EUPHORIA_LFO_TYPE, f_lfo_types, "LFO")
        self.hlayout2.addWidget(self.lfo.groupbox)

        self.lfo_pitch =  pydaw_knob_control(55, "Pitch", pydaw_ports.EUPHORIA_LFO_PITCH, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0, kc_integer, self.port_dict)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 2)

        #MonoFX Tab
        self.mono_fx_tab_main_layout =  QtGui.QVBoxLayout(self.mono_fx_tab)
        self.selected_container =  QtGui.QWidget()
        self.mono_fx_tab_selected_hlayout =  QtGui.QHBoxLayout(self.selected_container)
        self.mono_fx_tab_selected_sample =  QtGui.QComboBox(self.mono_fx_tab)
        self.mono_fx_tab_selected_group =  QtGui.QComboBox(self.mono_fx_tab)
        self.mono_fx_tab_selected_sample_label = QtGui.QLabel("Selected Sample:")
        self.mono_fx_tab_selected_group_label =  QtGui.QLabel("FX Group:")
        for f_i in range(1, pydaw_ports.EUPHORIA_MONO_FX_GROUPS_COUNT):
            self.mono_fx_tab_selected_group.addItem(str(f_i))
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.mono_fx_tab_selected_sample.addItem("")
        self.mono_fx_tab_selected_group.currentIndexChanged.connect(self.sample_selected_monofx_groupChanged)
        self.mono_fx_tab_selected_sample.currentIndexChanged.connect(self.monoFXSampleSelectedIndexChanged)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_sample_label)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_sample)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_group_label)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_group)
        self.hlayout10 = QtGui.QHBoxLayout()
        self.hlayout10.addWidget(self.selected_container)
        self.mono_fx_tab_main_layout.addLayout(self.hlayout10)

        self.hlayout11 = QtGui.QHBoxLayout()
        self.mono_fx_tab_main_layout.addLayout(self.hlayout11)
        self.mono_fx0 =  pydaw_modulex_single("FX0", 0, None, self.monofx0_callback)
        self.hlayout11.addWidget(self.mono_fx0.group_box)
        self.mono_fx1 =  pydaw_modulex_single("FX1", 0, None, self.monofx1_callback)
        self.hlayout11.addWidget(self.mono_fx1.group_box)
        self.hlayout12 = QtGui.QHBoxLayout()
        self.mono_fx_tab_main_layout.addLayout(self.hlayout12)
        self.mono_fx2 =  pydaw_modulex_single("FX2", 0, None, self.monofx2_callback)
        self.hlayout12.addWidget(self.mono_fx2.group_box)
        self.mono_fx3 =  pydaw_modulex_single("FX3", 0, None, self.monofx3_callback)
        self.hlayout12.addWidget(self.mono_fx3.group_box)

        self.master =  pydaw_master_widget(55, self.plugin_rel_callback, self.plugin_val_callback, pydaw_ports.EUPHORIA_MASTER_VOLUME, \
        pydaw_ports.EUPHORIA_MASTER_GLIDE, pydaw_ports.EUPHORIA_MASTER_PITCHBEND_AMT, self.port_dict, "Master")
        self.mono_fx_tab_main_layout.addWidget(self.master.group_box)
        self.master.vol_knob.control.setRange(-24, 24)

        self.generate_control_dict()
        self.open_plugin_file()

    def monofx0_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx0knob0_ctrls, self.monofx0knob1_ctrls, \
        self.monofx0knob2_ctrls, self.monofx0comboboxes])

    def monofx1_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx1knob0_ctrls, self.monofx1knob1_ctrls, \
        self.monofx1knob2_ctrls, self.monofx1comboboxes])

    def monofx2_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx2knob0_ctrls, self.monofx2knob1_ctrls, \
        self.monofx2knob2_ctrls, self.monofx2comboboxes])

    def monofx3_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx3knob0_ctrls, self.monofx3knob1_ctrls, \
        self.monofx3knob2_ctrls, self.monofx3comboboxes])

    def monofx_all_callback(self, a_port, a_val, a_list):
        f_index = self.mono_fx_tab_selected_group.currentIndex()
        f_ctrl = a_list[a_port][f_index]
        f_ctrl.set_value(a_val)
        f_ctrl.control_value_changed(a_val)


    def sample_start_callback(self, a_val):
        f_index = self.selected_sample_index_combobox.currentIndex()
        self.sample_starts[f_index].set_value(a_val)
        self.sample_starts[f_index].control_value_changed(a_val)

    def sample_end_callback(self, a_val):
        f_index = self.selected_sample_index_combobox.currentIndex()
        self.sample_ends[f_index].set_value(a_val)
        self.sample_ends[f_index].control_value_changed(a_val)

    def loop_start_callback(self, a_val):
        f_index = self.selected_sample_index_combobox.currentIndex()
        self.loop_starts[f_index].set_value(a_val)
        self.loop_starts[f_index].control_value_changed(a_val)

    def loop_end_callback(self, a_val):
        f_index = self.selected_sample_index_combobox.currentIndex()
        self.loop_ends[f_index].set_value(a_val)
        self.loop_ends[f_index].control_value_changed(a_val)

    def set_sample_graph(self):
        self.find_selected_radio_button()
        if self.sample_table.item(self.selected_row_index, SMP_TB_FILE_PATH_INDEX) is not None:
            f_file_name = str(self.sample_table.item(self.selected_row_index, SMP_TB_FILE_PATH_INDEX).text())
            if f_file_name != "":
                f_graph = self.pydaw_project.get_sample_graph_by_name(f_file_name)
                self.sample_graph.draw_item(f_graph.create_sample_graph(True), self.sample_starts[self.selected_row_index].get_value(), \
                self.sample_ends[self.selected_row_index].get_value(), self.loop_starts[self.selected_row_index].get_value(), \
                self.loop_ends[self.selected_row_index].get_value())
            else:
                self.sample_graph.clear_drawn_items()
        else:
            self.sample_graph.clear_drawn_items()

    def open_plugin_file(self):
        pydaw_abstract_plugin_ui.open_plugin_file(self)
        self.sample_table.resizeColumnsToContents()
        f_combobox_items = []
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_item = self.sample_table.item(f_i, SMP_TB_FILE_PATH_INDEX)
            if f_item is None or str(f_item.text()) == "":
                f_combobox_items.append("")
            else:
                f_arr = str(f_item.text()).split("/")
                f_combobox_items.append(f_arr[-1])
        self.selected_sample_index_combobox.clear()
        self.selected_sample_index_combobox.addItems(f_combobox_items)
        self.mono_fx_tab_selected_sample.clear()
        self.mono_fx_tab_selected_sample.addItems(f_combobox_items)
        self.selected_radiobuttons[0].click()

    def set_window_title(self, a_track_name):
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Euphoria - " + self.track_name)

    def configure_plugin(self, a_key, a_message):
        self.configure_dict[a_key] = a_message
        self.configure_callback(True, 0, self.track_num, a_key, a_message)  #TODO:  Get rid of this at PyDAWv4 and use a single delimiter

    def set_configure(self, a_key, a_message):
        self.configure_dict[a_key] = a_message
        if a_key == "load":
            print("set_configure: load")
            self.configure_dict[a_key] = a_message
            f_arr = a_message.split("|")
            for f_i in range(len(f_arr)):
                if f_arr[f_i] == "":
                    f_path = ""
                else:
                    f_path = self.pydaw_project.get_wav_path_by_uid(f_arr[f_i])
                f_table_item = QtGui.QTableWidgetItem(f_path)
                self.sample_table.setItem(f_i, SMP_TB_FILE_PATH_INDEX, f_table_item)
        else:
            print(("Unknown configure message '%s'" % (a_key,)))

    def clearAllSamples(self):
        for i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.set_selected_sample_combobox_item(i, "")
            self.sample_graph.clear_drawn_items()
            f_item =  QtGui.QTableWidgetItem()
            f_item.setText((""))
            f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
            self.sample_table.setItem(i, SMP_TB_FILE_PATH_INDEX, f_item)
        self.generate_files_string()
        self.view_file_selector.set_file((""))
        self.file_selector.set_file((""))

    def mapAllSamplesToOneMonoFXgroup(self):
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.monofx_groups[f_i].set_value(f_i)
            self.monofx_groups[f_i].control_value_changed(f_i)
        self.mono_fx_tab_selected_sample.setCurrentIndex(1)
        self.mono_fx_tab_selected_sample.setCurrentIndex(0)

    def mapAllSamplesToOneWhiteKey(self):
        f_current_note = 36
        i_white_notes = 0
        f_white_notes = [2,2,1,2,2,2,1]
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.sample_base_pitches[f_i].set_value(f_current_note)
            self.sample_high_notes[f_i].set_value(f_current_note)
            self.sample_low_notes[f_i].set_value(f_current_note)
            self.sample_base_pitches[f_i].control_value_changed(f_current_note)
            self.sample_high_notes[f_i].control_value_changed(f_current_note)
            self.sample_low_notes[f_i].control_value_changed(f_current_note)
            f_current_note += f_white_notes[i_white_notes]
            i_white_notes+= 1
            if i_white_notes >= 7:
                i_white_notes = 0

    def viewSampleSelectedIndexChanged(self, a_index):
        if self.suppress_selected_sample_changed:
            return
        self.suppress_selected_sample_changed = True
        self.selected_radiobuttons[a_index].setChecked(True)
        self.mono_fx_tab_selected_sample.setCurrentIndex(a_index)
        self.mono_fx_tab_selected_group.setCurrentIndex(self.monofx_groups[a_index].get_value())
        self.suppress_selected_sample_changed = False
        self.set_sample_graph()

    def monoFXSampleSelectedIndexChanged(self, a_index):
        if self.suppress_selected_sample_changed:
            return
        self.suppress_selected_sample_changed = True
        self.selected_radiobuttons[a_index].setChecked(True)
        self.selected_sample_index_combobox.setCurrentIndex(a_index)
        self.mono_fx_tab_selected_group.setCurrentIndex(self.monofx_groups[a_index].get_value())
        self.suppress_selected_sample_changed = False
        self.set_sample_graph()

    def loopModeChanged(self, a_value):
        self.find_selected_radio_button()
        self.loop_modes[self.selected_row_index].set_value(a_value)
        self.loop_modes[self.selected_row_index].control_value_changed(a_value)

    def set_selected_sample_combobox_item(self, a_index,  a_text):
        self.suppress_selected_sample_changed = True
        self.selected_sample_index_combobox.removeItem(a_index)
        self.selected_sample_index_combobox.insertItem(a_index, a_text)
        self.selected_sample_index_combobox.setCurrentIndex(a_index)
        self.mono_fx_tab_selected_sample.removeItem(a_index)
        self.mono_fx_tab_selected_sample.insertItem(a_index, a_text)
        self.mono_fx_tab_selected_sample.setCurrentIndex(a_index)
        self.suppress_selected_sample_changed = False

    def fileSelect(self):
        paths = self.file_selector.open_button_pressed_multiple()
        self.view_file_selector.set_file(self.file_selector.get_file())
        self.load_files(paths)

    def find_selected_radio_button(self):
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            if self.selected_radiobuttons[f_i].isChecked():
                self.selected_row_index = f_i
                return

    def load_files(self, paths):
        if len(paths) > 0:
            self.find_selected_radio_button()
            f_sample_index_to_load = (self.selected_row_index)
            for i in range(len(paths)):
                path = str(paths[i])
                if path != "":
                    if( not os.path.isfile(path)):
                        QtGui.QMessageBox.warning(self, "Error", "File cannot be read.")
                        continue
                    self.pydaw_project.get_wav_uid_by_name(path)
                    f_path_sections = path.split(("/"))
                    self.set_selected_sample_combobox_item(f_sample_index_to_load, f_path_sections[-1])
                    f_item =  QtGui.QTableWidgetItem()
                    f_item.setText(path)
                    f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
                    self.sample_table.setItem(f_sample_index_to_load, SMP_TB_FILE_PATH_INDEX, f_item)
                    f_sample_index_to_load += 1
                    if(f_sample_index_to_load >= pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
                        break

            self.generate_files_string()
            self.configure_plugin("load", self.files_string)
            self.sample_table.resizeColumnsToContents()
            self.selectionChanged()

    def generate_files_string(self, a_index=-1):
        self.files_string = ""
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_item = self.sample_table.item(f_i, SMP_TB_FILE_PATH_INDEX)
            if f_item is not None and str(f_item.text()).strip() != "":
                f_uid = self.pydaw_project.get_wav_uid_by_name(str(f_item.text()))
                self.files_string += str(f_uid)
            if f_i < pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT - 1:
                self.files_string += pydaw_ports.EUPHORIA_FILES_STRING_DELIMITER

    def clearFile(self):
        self.find_selected_radio_button()
        self.sample_graph.clear_drawn_items()
        self.set_selected_sample_combobox_item((self.selected_row_index), (""))
        f_item =  QtGui.QTableWidgetItem()
        f_item.setText((""))
        f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
        self.sample_table.setItem((self.selected_row_index), SMP_TB_FILE_PATH_INDEX, f_item)
        self.file_selector.clear_button_pressed()
        self.view_file_selector.clear_button_pressed()
        self.generate_files_string()
        self.configure_plugin("load", self.files_string)
        self.sample_table.resizeColumnsToContents()

    def reloadSample(self):
        QtGui.QMessageBox.warning(self.widget, "Error", "TODO:  Fix this again...")
        #path = str(self.file_selector.file_path.text())
        #if path.strip() != "":
        #    self.find_selected_radio_button()
        #    self.generate_files_string((self.selected_row_index))
        #    self.configure_plugin("load", self.files_string)
        #    self.sample_graph.clear_drawn_items()
        #    self.set_sample_graph()


    def selectionChanged(self):
        if(self.suppress_selected_sample_changed):
            return
        self.suppress_selected_sample_changed = True
        self.find_selected_radio_button()
        self.selected_sample_index_combobox.setCurrentIndex((self.selected_row_index))
        self.mono_fx_tab_selected_sample.setCurrentIndex((self.selected_row_index))
        self.setSelectedMonoFX()
        self.suppress_selected_sample_changed = False
        self.selected_sample_index_combobox.setCurrentIndex((self.selected_row_index))
        if self.sample_table.item(self.selected_row_index, SMP_TB_FILE_PATH_INDEX) is None:
            f_file_path = ""
        else:
            f_file_path = str(self.sample_table.item(self.selected_row_index, SMP_TB_FILE_PATH_INDEX).text())
        self.file_selector.set_file(f_file_path)
        self.view_file_selector.set_file(f_file_path)
        self.set_sample_graph()
        self.loop_mode_combobox.setCurrentIndex(self.loop_modes[(self.selected_row_index)].get_value())

    def file_browser_load_button_pressed(self):
        f_result = self.file_browser.files_selected()
        self.load_files(f_result)

    def file_browser_preview_button_pressed(self):
        f_list = self.file_browser.files_listWidget.selectedItems()
        if len(f_list) > 0:
            f_preview_file = str(self.file_browser.folder_path_lineedit.text()) + "/" + str(f_list[0].text())
            self.pydaw_project.this_pydaw_osc.pydaw_preview_audio(f_preview_file)

    def moveSamplesToSingleDirectory(self):
        f_selected_path = ("")
        if(self.creating_instrument_file):
            f_selected_path = self.inst_file_tmp_path
        else:
            f_selected_path = QtGui.QFileDialog.getExistingDirectory(self, "Select a directory to move the samples to...", ".")
        if not f_selected_path.isEmpty():
            #TODO: check that the directory is empty...
            self.find_selected_radio_button()
            for i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
                f_current_file_path = self.sample_table.item(i, SMP_TB_FILE_PATH_INDEX).text()
                if (f_current_file_path is None) or (f_current_file_path == "") or \
                f_current_file_path.startswith(f_selected_path):
                    continue
                f_file_arr = str(f_current_file_path).split("/")
                f_new_file_path = f_selected_path + ("/") + f_file_arr[-1]
                os.system('cp "' + '" "' + f_new_file_path + '"')
                f_item =  QtGui.QTableWidgetItem()
                f_item.setText(f_new_file_path)
                f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
                self.sample_table.setItem(i, SMP_TB_FILE_PATH_INDEX, f_item)
                self.generate_files_string()
                self.configure_plugin("load", self.files_string)

    def sample_selected_monofx_groupChanged(self, a_value):
        self.mono_fx0.knobs[0].set_value(self.monofx0knob0_ctrls[a_value].get_value())
        self.mono_fx0.knobs[1].set_value(self.monofx0knob1_ctrls[a_value].get_value())
        self.mono_fx0.knobs[2].set_value(self.monofx0knob2_ctrls[a_value].get_value())
        self.mono_fx0.combobox.set_value(self.monofx0comboboxes[a_value].get_value())
        self.mono_fx1.knobs[0].set_value(self.monofx1knob0_ctrls[a_value].get_value())
        self.mono_fx1.knobs[1].set_value(self.monofx1knob1_ctrls[a_value].get_value())
        self.mono_fx1.knobs[2].set_value(self.monofx1knob2_ctrls[a_value].get_value())
        self.mono_fx1.combobox.set_value(self.monofx1comboboxes[a_value].get_value())
        self.mono_fx2.knobs[0].set_value(self.monofx2knob0_ctrls[a_value].get_value())
        self.mono_fx2.knobs[1].set_value(self.monofx2knob1_ctrls[a_value].get_value())
        self.mono_fx2.knobs[2].set_value(self.monofx2knob2_ctrls[a_value].get_value())
        self.mono_fx2.combobox.set_value(self.monofx2comboboxes[a_value].get_value())
        self.mono_fx3.knobs[0].set_value(self.monofx3knob0_ctrls[a_value].get_value())
        self.mono_fx3.knobs[1].set_value(self.monofx3knob1_ctrls[a_value].get_value())
        self.mono_fx3.knobs[2].set_value(self.monofx3knob2_ctrls[a_value].get_value())
        self.mono_fx3.combobox.set_value(self.monofx3comboboxes[a_value].get_value())
        if not self.suppress_selected_sample_changed:
            self.monofx_groups[self.selected_row_index].set_value(a_value)
            self.monofx_groups[self.selected_row_index].control_value_changed(a_value)


    def setSelectedMonoFX(self):
        self.mono_fx_tab_selected_group.setCurrentIndex(self.monofx_groups[self.selected_row_index].get_value())

    def saveToFile(self):
        pass

    def openFromFile(self):
        pass
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

import os
import pydaw_util, pydaw_ports
from pydaw_project import pydaw_audio_item_fx
from PyQt4 import QtGui, QtCore

global_knob_arc_gradient = QtGui.QLinearGradient(0.0, 0.0, 90.0, 0.0)
global_knob_arc_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(120, 120, 255, 255))
global_knob_arc_gradient.setColorAt(0.66, QtGui.QColor.fromRgb(255, 0, 0, 255))
global_knob_arc_pen = QtGui.QPen(global_knob_arc_gradient, 5.0, QtCore.Qt.SolidLine, QtCore.Qt.RoundCap, QtCore.Qt.RoundJoin)

class pydaw_plugin_file:
    """ Abstracts a .pyinst or .pyfx file """
    def __init__(self, a_path=None):
        self.port_dict = {}
        self.euphoria_samples = []
        if a_path is not None:
            f_text = pydaw_util.pydaw_read_file_text(a_path)
            f_line_arr = f_text.split("\n")
            for f_line in f_line_arr:
                if f_line == "\\":
                    break
                f_items = f_line.split("|", 1)
                if f_items[0] == "load":
                    for f_sample_path in f_items[1].split("~"):
                        self.euphoria_samples.append(f_sample_path)
                else:
                    self.port_dict[int(f_items[0])] = float(f_items[1])

    @staticmethod
    def from_dict(a_port_dict, a_control_dict):
        f_result = pydaw_plugin_file()
        for k, v in a_port_dict.items():
            f_result.port_dict[a_control_dict[int(k)]] = v
        return f_result

    def __str__(self):
        f_result = ""
        for k, v in self.port_dict.items():
            f_result += str(k) + "|" + str(v.get_value()) + "\n"
        return f_result + "\\"

class pydaw_pixmap_knob(QtGui.QDial):
    def __init__(self, a_size, a_min_val, a_max_val, a_default_val):
        QtGui.QDial.__init__(self)
        self.setRange(a_min_val, a_max_val)
        self.setValue(a_default_val)
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

kc_integer = 0
kc_decimal = 1
kc_pitch = 2
kc_none = 3
kc_127_pitch = 4
kc_127_zero_to_x = 5
kc_log_time = 6
kc_127_zero_to_x_int = 7

class pydaw_abstract_ui_control:
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion=kc_none, a_port_dict=None):
        self.name_label = QtGui.QLabel(str(a_label))
        self.name_label.setAlignment(QtCore.Qt.AlignCenter)
        self.port_num = int(a_port_num)
        self.val_callback = a_val_callback
        self.rel_callback = a_rel_callback
        self.suppress_changes = False
        self.val_conversion = a_val_conversion
        if a_port_dict is not None:
            a_port_dict[self.port_num] = self

    def set_value(self, a_val):
        self.suppress_changes = True
        self.control.setValue(a_val)
        self.control_value_changed(a_val)
        self.suppress_changes = False

    def get_value(self):
        return self.control.value()

    def set_127_min_max(self, a_min, a_max):
        self.min_label_value_127 = a_min;
        self.max_label_value_127 = a_max;
        self.label_value_127_add_to = 0.0 - a_min;
        self.label_value_127_multiply_by = ((a_max - a_min) / 127.0);

    def control_released(self):
        self.rel_callback(self.port_num, self.control.value())

    def control_value_changed(self, a_value):
        if not self.suppress_changes:
            self.val_callback(self.port_num, self.control.value())
        if self.value_label is not None:
            f_dec_value = 0.0
            if self.val_conversion == kc_none:
                pass
            elif self.val_conversion == kc_decimal:
                self.value_label.setText(str(a_value * .01))
            elif self.val_conversion == kc_integer:
                self.value_label.setText(str(a_value))
            elif self.val_conversion == kc_pitch:
                self.value_label.setText(str(int(440.0 * pow(2.0,((float)(a_value - 57.0)) * 0.0833333))))
            elif self.val_conversion == kc_127_pitch:
                self.value_label.setText(str(int(440.0 * pow(2.0, ((float)(((a_value * 0.818897638) + 20.0) -57.0)) * 0.0833333))))
            elif self.val_conversion == kc_127_zero_to_x:
                f_dec_value = (float(a_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
                f_dec_value = ((int)(f_dec_value * 10.0)) * 0.1
                self.value_label.setText(str(f_dec_value))
            elif self.val_conversion == kc_127_zero_to_x_int:
                f_dec_value = (float(a_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
                self.value_label.setText(str(int(f_dec_value)))
            elif self.val_conversion == kc_log_time:
                f_dec_value = float(a_value) * 0.01
                f_dec_value = f_dec_value * f_dec_value
                f_dec_value = (int(f_dec_value * 100.0)) * 0.01
                self.value_label.setText(str(f_dec_value))

    def add_to_grid_layout(self, a_layout, a_x):
        a_layout.addWidget(self.name_label, 0, a_x)
        a_layout.addWidget(self.control, 1, a_x)
        if self.value_label is not None:
            a_layout.addWidget(self.value_label, 2, a_x)


class pydaw_knob_control(pydaw_abstract_ui_control):
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, a_port_dict)
        self.control = pydaw_pixmap_knob(a_size, a_min_val, a_max_val, a_default_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)


class pydaw_slider_control(pydaw_abstract_ui_control):
    def __init__(self, a_orientation, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, a_port_dict)
        self.control = QtGui.QSlider()
        self.control.setRange(a_min_val, a_max_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.set_value(a_default_val)


class pydaw_spinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, a_port_dict)
        self.control = QtGui.QSpinBox()
        self.control.setRange(a_min_val, a_max_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)
        self.value_label = None
        self.set_value(a_default_val)


class pydaw_doublespinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, a_port_dict)
        self.control = QtGui.QDoubleSpinBox()
        self.control.setRange(a_min_val, a_max_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)
        self.value_label = None
        self.set_value(a_default_val)


class pydaw_checkbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_port_dict=a_port_dict)
        self.control = QtGui.QCheckBox()
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


class pydaw_combobox_control:
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_items_list=[], a_port_dict=None):
        self.suppress_changes = True
        self.name_label = QtGui.QLabel(str(a_label))
        self.name_label.setAlignment(QtCore.Qt.AlignCenter)
        self.control = QtGui.QComboBox()
        self.control.setMinimumWidth(a_size)
        self.control.addItems(a_items_list)
        self.control.setCurrentIndex(0)
        self.control.currentIndexChanged.connect(self.combobox_index_changed)
        self.port_num = int(a_port_num)
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.suppress_changes = False
        if a_port_dict is not None:
            a_port_dict[self.port_num] = self
        self.value_label = None

    def combobox_index_changed(self, a_val):
        if not self.suppress_changes:
            self.val_callback(self.port_num, a_val)
            self.rel_callback(self.port_num, a_val)

    def set_value(self, a_val):
        self.suppress_changes = True
        self.control.setCurrentIndex(a_val)
        self.suppress_changes = False

    def get_value(self):
        return self.control.currentIndex()

class pydaw_adsr_widget:
    def __init__(self, a_size, a_sustain_in_db, a_attack_port, a_decay_port, a_sustain_port, a_release_port, \
            a_label, a_rel_callback, a_val_callback, a_port_dict=None):
        self.attack_knob = pydaw_knob_control(a_size, "Attack", a_attack_port, a_rel_callback, a_val_callback, 0, 100, 0, kc_log_time, a_port_dict)
        self.decay_knob = pydaw_knob_control(a_size, "Decay", a_decay_port, a_rel_callback, a_val_callback, 0, 100, 0, kc_log_time, a_port_dict)
        if a_sustain_in_db:
            self.sustain_knob = pydaw_knob_control(a_size, "Sustain", a_sustain_port, a_rel_callback, a_val_callback, -60, 0, 0, kc_integer, a_port_dict)
        else:
            self.sustain_knob = pydaw_knob_control(a_size, "Sustain", a_sustain_port, a_rel_callback, a_val_callback, 0, 100, 100, kc_decimal, a_port_dict)
        self.release_knob = pydaw_knob_control(a_size, "Release", a_release_port, a_rel_callback, a_val_callback, 10, 200, 0, kc_log_time, a_port_dict)
        self.groupbox = QtGui.QGroupBox(a_label)
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.layout.addWidget(self.attack_knob.name_label, 0, 0)
        self.layout.addWidget(self.attack_knob.control, 0, 1)
        self.layout.addWidget(self.attack_knob.value_label, 0, 2)
        self.layout.addWidget(self.decay_knob.name_label, 1, 0)
        self.layout.addWidget(self.decay_knob.control, 1, 1)
        self.layout.addWidget(self.decay_knob.value_label, 1, 2)
        self.layout.addWidget(self.sustain_knob.name_label, 2, 0)
        self.layout.addWidget(self.sustain_knob.control, 2, 1)
        self.layout.addWidget(self.sustain_knob.value_label, 2, 2)
        self.layout.addWidget(self.release_knob.name_label, 3, 0)
        self.layout.addWidget(self.release_knob.control, 3, 1)
        self.layout.addWidget(self.release_knob.value_label, 3, 2)

class pydaw_filter_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict, a_cutoff_port, a_res_port, a_type_port=None, a_label="Filter"):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.cutoff_knob = pydaw_knob_control(a_size, "Cutoff", a_cutoff_port, a_rel_callback, a_val_callback, \
        20, 124, 96, kc_pitch, a_port_dict)
        self.cutoff_knob.add_to_grid_layout(self.layout, 0)
        self.res_knob = pydaw_knob_control(a_size, "Res", a_res_port, a_rel_callback, a_val_callback, \
        -30, 0, -12, kc_integer, a_port_dict)
        self.res_knob.add_to_grid_layout(self.layout, 1)
        if a_type_port is not None:
            self.type_combobox = pydaw_combobox_control(150, "Type", a_type_port, a_rel_callback, a_val_callback, \
            ["LP 2", "HP 2", "BP2", "LP 4", "HP 4", "BP4", "Off"], a_port_dict)
            self.layout.addWidget(self.type_combobox.name_label, 2, 0)
            self.layout.addWidget(self.type_combobox.control, 2, 1)

class pydaw_ramp_env_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict, a_time_port, a_amt_port, a_label="Ramp Env"):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.amt_knob = pydaw_knob_control(a_size, "Amt", a_amt_port, a_rel_callback, a_val_callback, \
        -36, 36, 0, kc_integer, a_port_dict)
        self.amt_knob.add_to_grid_layout(self.layout, 0)
        self.time_knob = pydaw_knob_control(a_size, "Time", a_time_port, a_rel_callback, a_val_callback, \
        1, 200, 100, kc_decimal, a_port_dict)
        self.time_knob.add_to_grid_layout(self.layout, 1)

class pydaw_lfo_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict, a_freq_port, a_type_port, a_type_list, a_label="LFO"):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.freq_knob = pydaw_knob_control(a_size, "Freq", a_freq_port, a_rel_callback, a_val_callback, \
        10, 1600, 200, kc_decimal, a_port_dict)
        self.freq_knob.add_to_grid_layout(self.layout, 0)
        self.type_combobox = pydaw_combobox_control(150, "Type", a_type_port, a_rel_callback, a_val_callback, a_type_list, a_port_dict)
        self.layout.addWidget(self.type_combobox.name_label, 2, 0)
        self.layout.addWidget(self.type_combobox.control, 2, 1)

class pydaw_osc_widget:
    def __init__(self, a_size, a_pitch_port, a_fine_port, a_vol_port, a_type_port, a_osc_types_list, \
    a_rel_callback, a_val_callback, a_label, a_port_dict=None):
        self.pitch_knob = pydaw_knob_control(a_size, "Pitch", a_pitch_port, a_rel_callback, a_val_callback, -36, 36, 0, \
        a_val_conversion=kc_integer, a_port_dict=a_port_dict)
        self.fine_knob = pydaw_knob_control(a_size, "Fine", a_fine_port, a_rel_callback, a_val_callback, -100, 100, 0, \
        a_val_conversion=kc_decimal, a_port_dict=a_port_dict)
        self.vol_knob = pydaw_knob_control(a_size, "Vol", a_vol_port, a_rel_callback, a_val_callback, -60, 0, -6, \
        a_val_conversion=kc_integer, a_port_dict=a_port_dict)
        self.osc_type_combobox = pydaw_combobox_control(114, "Type", a_type_port, a_rel_callback, a_val_callback, a_osc_types_list, a_port_dict)
        self.grid_layout = QtGui.QGridLayout()
        self.group_box = QtGui.QGroupBox(str(a_label))
        self.group_box.setLayout(self.grid_layout)
        self.grid_layout.addWidget(self.pitch_knob.name_label, 0, 0)
        self.grid_layout.addWidget(self.pitch_knob.control, 0, 1)
        self.grid_layout.addWidget(self.pitch_knob.value_label, 0, 2)

        self.grid_layout.addWidget(self.fine_knob.name_label, 1, 0)
        self.grid_layout.addWidget(self.fine_knob.control, 1, 1)
        self.grid_layout.addWidget(self.fine_knob.value_label, 1, 2)

        self.grid_layout.addWidget(self.vol_knob.name_label, 2, 0)
        self.grid_layout.addWidget(self.vol_knob.control, 2, 1)
        self.grid_layout.addWidget(self.vol_knob.value_label, 2, 2)

        self.grid_layout.addWidget(self.osc_type_combobox.name_label, 3, 0)
        self.grid_layout.addWidget(self.osc_type_combobox.control, 3, 1)

class pydaw_note_selector_widget:
    def __init__(self, a_port, a_rel_callback, a_val_callback, a_port_dict=None):
        self.control = self
        self.port = a_port
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.selected_note = 48
        self.note_combobox = QtGui.QComboBox()
        self.note_combobox.addItems(["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"])
        self.octave_spinbox = QtGui.QSpinBox()
        self.octave_spinbox.setRange(-2, 8)
        self.octave_spinbox.setValue(3)
        self.widget = QtGui.QWidget()
        self.layout = QtGui.QHBoxLayout()
        self.widget.setLayout(self.layout)
        self.layout.addWidget(self.note_combobox)
        self.layout.addWidget(self.octave_spinbox)
        self.note_combobox.currentIndexChanged.connect(self.value_changed)
        self.octave_spinbox.currentIndexChanged.connect(self.value_changed)
        self.suppress_changes = False

    def value_changed(self, a_val=None):
        if not self.suppress_changes:
            self.selected_note = (self.note_combobox.currentIndex()) + (((self.octave_spinbox.currentIndex()) + 2) * 12)
        self.val_callback(self.port, self.selected_note)
        self.rel_callback(self.port, self.selected_note)

    def set_value(self, a_val):
        self.note_combobox.setCurrentIndex(a_val % 12)
        self.octave_spinbox.setValue((int(float(a_val) / 12.0)) - 2)

    def get_value(self):
        return self.selected_note


class pydaw_preset_manager_widget:
    def __init__(self, a_plugin_name,  a_default_presets, a_port_num, a_rel_callback, a_val_callback, a_port_dict=None):
        if os.path.isdir("/media/pydaw_data") and os.path.isdir("/home/ubuntu"):
            self.preset_path = "/media/pydaw_data/pydaw3/" + str(a_plugin_name) + ".pypresets"
        else:
            self.preset_path = os.path.expanduser("~") + "/pydaw3/" + str(a_plugin_name) + ".pypresets"
        self.layout = QtGui.QHBoxLayout()
        #TODO:  I believe it's not necessary to use the custom version, just a QComboBox would be better
        self.program_combobox = QtGui.QComboBox()
        self.program_combobox.setMinimumWidth(150)
        self.program_combobox.currentIndexChanged.connect(self.index_changed)
        self.group_box = QtGui.QGroupBox()
        self.group_box.setLayout(self.layout)
        self.layout.addWidget(self.program_combobox.control)
        self.save_button = QtGui.QPushButton("Save")
        self.save_button.pressed.connect(self.save_presets)
        self.layout.addWidget(self.save_button)
        self.presets_tab_delimited = []
        self.controls = []
        self.load_presets()

    def load_presets(self):
        f_text = pydaw_util.pydaw_read_file_text(self.preset_path)
        f_line_arr = f_text.split("\n")
        self.presets_tab_delimited = []
        for f_i in range(128):
            self.presets_tab_delimited.append([f_line_arr[f_i].split("\t")])

    def save_presets(self):
        if str(self.program_combobox.control.currentText()) == "empty":
            QtGui.QMessageBox.warning(self.group_box, "Error", "Preset name cannot be 'empty'")
            return
        if self.program_combobox.control.currentIndex() == 0:
            QtGui.QMessageBox.warning(self.group_box, "Error", "The first preset must be empty")
            return
        f_result_values = [str(self.program_combobox.control.currentText())]
        for f_control in self.controls:
            f_result_values.append(str(f_control.get_value()))
        self.presets_tab_delimited[(self.program_combobox.control.currentIndex())] = f_result_values
        f_result = ""
        for f_list in self.presets_tab_delimited:
            f_result += "\t".join(f_list)
        pydaw_util.pydaw_write_file_text(self.preset_path, f_result)

    def index_changed(self, a_val=None):
        if str(self.program_combobox.currentText()) == "empty":
            pass
        else:
            f_preset = self.presets_tab_delimited[self.program_combobox.currentIndex()]
            for f_i in range(len(f_preset)):
                self.controls[f_i].set_value(f_preset[f_i])
                self.controls[f_i].value_changed()

    def add_control(self, a_control):
        self.controls.append(a_control)

class pydaw_master_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_master_vol_port, a_master_glide_port, a_master_pitchbend_port, \
    a_port_dict, a_title="Master", a_master_uni_voices_port=None, a_master_uni_spread_port=None):
        self.group_box = QtGui.QGroupBox()
        self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout(self.group_box)
        self.vol_knob = pydaw_knob_control(a_size, "Vol", a_master_vol_port, a_rel_callback, a_val_callback, -60, 12, -6, kc_integer, a_port_dict)
        self.vol_knob.add_to_grid_layout(self.layout, 0)
        if a_master_uni_voices_port is not None and a_master_uni_spread_port is not None:
            self.uni_voices_knob = pydaw_knob_control(a_size, "Unison", a_master_uni_voices_port, a_rel_callback, a_val_callback,\
            1, 7, 1, kc_integer, a_port_dict)
            self.uni_voices_knob.add_to_grid_layout(self.layout, 1)
            self.uni_spread_knob = pydaw_knob_control(a_size, "Spraed", a_master_uni_spread_port, a_rel_callback, a_val_callback,\
            10, 100, 50, kc_decimal, a_port_dict)
            self.uni_spread_knob.add_to_grid_layout(self.layout, 2)
        self.glide_knob = pydaw_knob_control(a_size, "Glide", a_master_glide_port, a_rel_callback, a_val_callback, 0, 200, 0, kc_decimal, a_port_dict)
        self.glide_knob.add_to_grid_layout(self.layout, 3)
        self.pb_knob = pydaw_knob_control(a_size, "Pitchbend", a_master_pitchbend_port, a_rel_callback, a_val_callback, -36, 36, 0, kc_integer, a_port_dict)
        self.pb_knob.add_to_grid_layout(self.layout, 4)


class pydaw_modulex_single:
    def __init__(self, a_title, a_port_k1, a_rel_callback, a_val_callback, a_port_dict=None):
        self.group_box = QtGui.QGroupBox()
        if a_title is not None:
            self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout()
        self.group_box.setLayout(self.layout)
        self.knobs = []
        for f_i in range(3):
            f_knob = pydaw_knob_control(51, "", a_port_k1 + f_i, a_rel_callback, a_val_callback, 0, 127, 64, a_port_dict=a_port_dict)
            f_knob.add_to_grid_layout(self.layout, f_i)
            self.knobs.append(f_knob)
        self.combobox = pydaw_combobox_control(132, "Type", a_port_k1 + 3, a_rel_callback, a_val_callback,
               ["Off", "LP2" , "LP4", "HP2", "HP4", "BP2", "BP4" , "Notch2", "Notch4", "EQ" , "Distortion",
                "Comb Filter", "Amp/Pan", "Limiter" , "Saturator", "Formant", "Chorus", "Glitch" , "RingMod",
                "LoFi", "S/H", "LP-Dry/Wet" , "HP-Dry/Wet"], a_port_dict=a_port_dict)
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

        self.knobs[0].set_value(self.knobs[0].control.value())
        self.knobs[1].set_value(self.knobs[1].control.value())
        self.knobs[2].set_value(self.knobs[2].control.value())

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

    def clear_effects(self):
        for f_effect in self.effects:
            f_effect.combobox.set_value(0)
            for f_knob in f_effect.knobs:
                f_knob.set_value(64)

class pydaw_abstract_plugin_ui:
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, a_close_callback):
        self.track_num = int(a_track_num)
        self.pydaw_project = a_project
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
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

    def open_plugin_file(self):
        f_file_path = self.pydaw_project.project_folder + "/" + self.folder + "/" + self.file
        if os.path.isfile(f_file_path):
            f_file = pydaw_plugin_file(f_file_path)
            for k, v in f_file.port_dict.items():
                self.set_control_val(self.control_to_port_dict[int(k)], v)
        else:
            print("pydaw_abstract_plugin_ui.open_plugin_file(): + " + f_file_path + " did not exist, not loading.")

    def save_plugin_file(self):
        f_file = pydaw_plugin_file.from_dict(self.port_dict, self.port_to_control_dict)
        self.pydaw_project.save_file(self.folder, self.file, str(f_file))
        self.pydaw_project.commit("Update controls for " + self.track_name)

    def widget_close_event(self, a_event):
        self.save_plugin_file()
        self.close_callback(self.track_num, self.track_type)
        QtGui.QWidget.closeEvent(self.widget, a_event)

    def plugin_rel_callback(self, a_port, a_val):
        self.rel_callback(self.is_instrument, self.track_type, self.track_num, a_port, a_val)

    def plugin_val_callback(self, a_port, a_val):
        self.val_callback(self.is_instrument, self.track_type, self.track_num, a_port, a_val)

    def set_control_val(self, a_port, a_val):
        f_port = int(a_port)
        if f_port in self.port_dict:
            self.port_dict[int(a_port)].set_value(a_val)
        else:
            print("pydaw_abstract_plugin_ui.set_control_val():  Did not have port " + str(f_port))

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


class pydaw_modulex_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, a_close_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, a_close_callback)
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyfx"
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Modulex - " + self.track_name)
        self.is_instrument = False

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
            f_effect = pydaw_modulex_single(("FX" + str(f_i)), f_port, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
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
        self.plugin_val_callback, 10, 100, 50, kc_decimal, self.port_dict)
        self.delay_time_knob.add_to_grid_layout(delay_gridlayout, 0)
        m_feedback =  pydaw_knob_control(51, "Feedbk", pydaw_ports.MODULEX_FEEDBACK, self.plugin_rel_callback, \
        self.plugin_val_callback, -20, 0, -12, kc_integer, self.port_dict)
        m_feedback.add_to_grid_layout(delay_gridlayout, 1)
        m_dry =  pydaw_knob_control(51, "Dry", pydaw_ports.MODULEX_DRY, self.plugin_rel_callback, \
        self.plugin_val_callback, -30, 0, 0, kc_integer, self.port_dict)
        m_dry.add_to_grid_layout(delay_gridlayout, 2)
        m_wet =  pydaw_knob_control(51, "Wet", pydaw_ports.MODULEX_WET, self.plugin_rel_callback, \
        self.plugin_val_callback, -30, 0, -30, kc_integer, self.port_dict)
        m_wet.add_to_grid_layout(delay_gridlayout, 3)
        m_duck =  pydaw_knob_control(51, "Duck", pydaw_ports.MODULEX_DUCK, self.plugin_rel_callback, \
        self.plugin_val_callback, -40, 0, 0, kc_integer, self.port_dict)
        m_duck.add_to_grid_layout(delay_gridlayout, 4)
        m_cutoff =  pydaw_knob_control(51, "Cutoff", pydaw_ports.MODULEX_CUTOFF, self.plugin_rel_callback, \
        self.plugin_val_callback, 20, 124, 66, kc_pitch, self.port_dict)
        m_cutoff.add_to_grid_layout(delay_gridlayout, 5)
        m_stereo =  pydaw_knob_control(51, "Stereo", pydaw_ports.MODULEX_STEREO, self.plugin_rel_callback, \
        self.plugin_val_callback, 0, 100, 100, kc_decimal, self.port_dict)
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
        self.plugin_val_callback, 0, 100, 50, kc_decimal, self.port_dict)
        m_reverb_time.add_to_grid_layout(self.reverb_groupbox_gridlayout, 0)
        m_reverb_wet =  pydaw_knob_control(51, "Wet", pydaw_ports.MODULEX_REVERB_WET, self.plugin_rel_callback, \
        self.plugin_val_callback, 0, 100, 0, kc_decimal, self.port_dict)
        m_reverb_wet.add_to_grid_layout(self.reverb_groupbox_gridlayout, 1)
        m_reverb_color =  pydaw_knob_control(51, "Color", pydaw_ports.MODULEX_REVERB_COLOR, self.plugin_rel_callback, \
        self.plugin_val_callback, 0, 100, 100, kc_decimal, self.port_dict)
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


class pydaw_rayv_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, a_close_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, a_close_callback)

        f_osc_types = ["Saw" , "Square" , "Triangle" , "Sine" , "Off"]
        f_lfo_types = ["Off" , "Sine" , "Triangle"]
        f_default_presets = ("empty\nclassic 5th pad\t68\t95\t-8\t153\t90\t-15\t15\t94\t100\t4\t166\t-19\t36\t1\t0\t0\t0\t0\t0\t7\t0\t0\t-14\t5\t42\t1\t18\t38\t0\t10\t0\t0\t0\t0\t0\n303 acid lead\t41\t58\t-9\t47\t70\t0\t36\t37\t95\t1\t99\t-30\t36\t100\t0\t0\t0\t0\t4\t0\t0\t0\t-8\t1\t10\t1\t18\t1\t0\t10\t0\t0\t0\t0\t0\nhoover\t39\t53\t-9\t45\t124\t-16\t15\t12\t29\t1\t99\t-12\t0\t1\t0\t0\t0\t0\t4\t0\t0\t0\t-8\t4\t42\t1\t18\t73\t-26\t10\t0\t0\t0\t0\t0\nbendy saw\t10\t49\t-3\t16\t124\t-16\t15\t100\t100\t1\t162\t-60\t0\t1\t0\t0\t0\t0\t4\t0\t0\t0\t-16\t1\t42\t54\t36\t1\t0\t10\t0\t0\t0\t0\t0\nsupersaw lead\t10\t49\t-3\t61\t124\t-15\t36\t10\t33\t1\t162\t-12\t0\t1\t0\t0\t0\t-6\t4\t0\t0\t0\t-16\t5\t41\t1\t17\t1\t0\t10\t0\t0\t0\t0\t0\n3rd Plucks\t10\t49\t-20\t124\t90\t-9\t36\t10\t29\t1\t73\t-12\t36\t1\t0\t0\t0\t-6\t0\t5\t0\t0\t-16\t5\t50\t1\t17\t1\t0\t10\t0\t0\t0\t0\t0\nsquare lead\t3\t49\t-12\t60\t124\t-9\t36\t1\t21\t1\t73\t-12\t36\t1\t1\t0\t0\t-6\t4\t0\t0\t0\t-16\t4\t50\t1\t17\t1\t0\t0\t0\t0\t0\t0\t0\ntriangle kick drum\t3\t49\t-12\t60\t124\t-9\t36\t1\t21\t1\t73\t-37\t36\t1\t2\t0\t0\t-6\t4\t0\t0\t0\t-5\t4\t50\t1\t17\t8\t-24\t0\t0\t0\t0\t0\t0\nnoise snare\t10\t51\t-30\t14\t99\t-6\t36\t21\t38\t1\t73\t-3\t36\t100\t4\t0\t0\t-6\t4\t0\t0\t0\t-18\t4\t50\t1\t17\t17\t-24\t10\t0\t0\t0\t0\t0\nelectro open hihat\t39\t49\t-30\t14\t95\t-3\t36\t36\t43\t1\t73\t-3\t36\t100\t4\t0\t0\t-6\t4\t0\t0\t0\t-18\t4\t50\t1\t17\t17\t-24\t10\t0\t0\t0\t0\t0\nSynchronize Me\t10\t32\t0\t57\t124\t-15\t15\t32\t32\t75\t57\t-60\t0\t99\t3\t0\t0\t0\t4\t0\t0\t0\t-6\t1\t50\t0\t18\t100\t-12\t205\t0\t0\t0\t0\t1\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty")
        m_program =  pydaw_preset_manager_widget("RAYV", f_default_presets)
        m_main_layout = QtGui.QVBoxLayout(self.widget)
        self.hlayout0 = QtGui.QHBoxLayout()
        m_main_layout.addLayout(self.hlayout0)
        self.hlayout0.addWidget(m_program.group_box)
        self.hlayout0.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        f_logo_label =  QtGui.QLabel()
        f_logo_label.setTextFormat(QtCore.Qt.RichText)
        """This string is a base64 encoded .png image I created using Gimp for the logo. To get the base64 encoded string,
        run it through png_to_base64.pl in the packaging folder"""
        f_logo_text = "<html><img src=\"data:image/pngbase64,iVBORw0KGgoAAAANSUhEUgAAAKAAAAAwCAYAAACWqXFuAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9wDHRcgB15yh5IAACAASURBVHjatd13nGRllTfw763Q3dW5e2a6J+fAMIkchgEBJYmCIAomzGkN765h3XVRX93grjmsuuqquyKoiKIgQSTngSENDJNz6OkwnXNX1X3/6Fsz1TUdBvTtz6c+1dV96977PM95zvmd3wk3MMpPFsHwr0EY/S0Y8Xb4HYR5fwgL/ln4ufAn9#RzhGOdrFRvjPacS/nfAr+F45xr+PdZzDGWAuPN8bnseZtvLGN926ca411DROMzRjHFxyT+3eYd64wNsb8BKMtQpj3v+iAWN7xQf4xEy3sRMI2npCGY93oMSzwRN99ucIznlCEYwjreIs80fXDlyH0Xsa9H6vCGG9eRpvOvOPCvL+Fuc9BJJDZPGE66r6zR/4YFAhcEH0vCCOhy#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#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#a+pTnvdMNUxfIu7zFSvwjt8B290khtt8C5f0u2LzrJV9S1bVS/5gKI5b/HC7tfwOQAADQVJREFUn8+12UOu9At1TtbiHA97WI0GJZ4zaIcpFuhziZ7jfm0gWuTBKDN6KJ7Rc+F9DnhBpTRazbZV2ku6lTvgnw1ZJ21QTLvLBWarVSvQsvoJv3jTTZrD4ZBWdyTUYTAsUP1oz6s/GbMXS15CajoqRwiiNLjepY8avKVIvdNsV6bfdL12a7Bdt15bNUhartvj6i22TI1N4uY53574gO3f+Zpn03FN8YyuSLDzEw4KNeC4RHQ4AY88bkr+eMmnBROTq4KrbHyd7rDTdJUO2G6aPaps0ut0j5trtXstTyxw7tQm7VEiwSAy6biOeMYclAnsMOBrit2AP2vwTuU2W+MJWTV3fNMOPzU5AsahA6ql1EoIvKTOCueZ4QmdpviErZ5xkrWKtuw2a8t9+n3Bax3CNbo1iUlqM0m/R8y3XLPl1lqi1l3mndxlb04AIwJWLNRXMqC5dptDrS2qbEKbemUCJ3hKmRL9TnSKJnvMN9seoSpV/PB8pdgfDHu+gwUhrXRwBNYclUEyyqKGeen6sVxNyiVb7fy3bzvDI3rda7Fd+lyg1SL7rHS7lEE7VNrmVHv0OcVrLdCMvhuvsR6HEhltUaZRNi/n8qjmVWO9B8dWvjF2JGSshK7gaA84V/NQPBhXPFTmtypsMc9mZzsgLiO0xAI1bhKK6z17rb6I1BwIhzHBYCKjp2RA/8V32IK1ir0Nv5E2X0attGptzhYzL6xk/8zDvGN6ecoGHVJi5pqhRZNZsuY5YJ5nJC3X5AytPu4B5eZ7s33eYb/nJHWp8u8u9WsLTUadpMBkj5rjLJ2rnx1emOAIBkpHgj/w5maPer9npCT1qEa1jZaj3UWe9agqMfOEqgVq3nCbF5fvtiGMTFteFZlIA2bDSKs5otnSeQVTI/4WRr1ygiPdpjIYWHO/ViXanKfSKoPeJ3S8YgfNs1edn7oAU73GFmeotsAu7Xqnrrfzzb+3JxM7TLqn80NuwSi5VqM1jgwmyHoZL4QSz/3yf0cSzEFBMmR+ieLhCv2fvd1xt52qXtqgSe7SZY8T7bXaQXsl1cs63Z/PfUb9G35ve5RZ0Rup93hIau4u0372XgsMeAxnStgv4RGB/ao0+gcXW6Ik1ia4cO0wvC7tlvzD+z0jNFO5m5RbJCMtba4Wg2J6zPWin1lmnlaNqlyrzyo91ogZMqDXfFvUOVXMR610jVb1tl#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\"/></html>"
        f_logo_label.setText(f_logo_text)
        f_logo_label.setMinimumSize(90, 30)
        f_logo_label.show()
        self.hlayout0.addWidget(f_logo_label)
        self.hlayout1 = QtGui.QHBoxLayout()
        m_main_layout.addLayout(self.hlayout1)
        m_osc1 =  pydaw_osc_widget(64, pydaw_ports.RAYV_OSC1_PITCH, pydaw_ports.RAYV_OSC1_TUNE, pydaw_ports.RAYV_OSC1_VOLUME, pydaw_ports.RAYV_OSC1_TYPE, \
        f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 1", self.port_dict)
        self.hlayout1.addWidget(m_osc1.group_box)
        m_adsr_amp = pydaw_adsr_widget(64, True, pydaw_ports.RAYV_ATTACK, pydaw_ports.RAYV_DECAY, pydaw_ports.RAYV_SUSTAIN, pydaw_ports.RAYV_RELEASE, \
        "ADSR Amp", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout1.addWidget(m_adsr_amp.groupbox)
        m_groupbox_distortion =  QtGui.QGroupBox("Distortion")
        self.groupbox_distortion_layout = QtGui.QGridLayout(m_groupbox_distortion)
        self.hlayout1.addWidget(m_groupbox_distortion)
        m_dist =  pydaw_knob_control(64, "Gain", pydaw_ports.RAYV_DIST, self.plugin_rel_callback, self.plugin_val_callback, \
        -6, 36, 12, kc_integer, self.port_dict)
        m_dist.add_to_grid_layout(self.groupbox_distortion_layout, 0)
        m_dist_wet =  pydaw_knob_control(64, "Wet", pydaw_ports.RAYV_DIST_WET, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_none, self.port_dict)
        m_dist_wet.add_to_grid_layout(self.groupbox_distortion_layout, 1)
        m_groupbox_noise =  QtGui.QGroupBox("Noise")
        self.noise_layout = QtGui.QGridLayout(m_groupbox_noise)
        self.hlayout1.addWidget(m_groupbox_noise)
        m_noise_amp =  pydaw_knob_control(64, "Vol", pydaw_ports.RAYV_NOISE_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -60, 0, 30, kc_integer, self.port_dict)
        m_noise_amp.add_to_grid_layout(self.noise_layout, 0)
        self.hlayout2 = QtGui.QHBoxLayout()
        m_main_layout.addLayout(self.hlayout2)
        m_osc2 =  pydaw_osc_widget(64, pydaw_ports.RAYV_OSC2_PITCH, pydaw_ports.RAYV_OSC2_TUNE, pydaw_ports.RAYV_OSC2_VOLUME, pydaw_ports.RAYV_OSC2_TYPE, \
        f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 2", self.port_dict)
        m_main_layout.lms_add_widget(m_osc2.group_box)
        m_sync_groupbox =  QtGui.QGroupBox("Sync")
        self.hlayout2.addWidget(m_sync_groupbox)
        self.sync_gridlayout = QtGui.QGridLayout(m_sync_groupbox)
        m_hard_sync =  pydaw_checkbox_control("On", pydaw_ports.RAYV_OSC_HARD_SYNC, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict)
        m_hard_sync.control.setToolTip(("Setting self hard sync's Osc1 to Osc2. Usually you would want to distort and pitchbend self."))
        self.sync_gridlayout.addWidget(m_hard_sync.control, 1, 0)
        m_adsr_filter =  pydaw_adsr_widget(64, False, pydaw_ports.RAYV_FILTER_ATTACK, pydaw_ports.RAYV_FILTER_DECAY, pydaw_ports.RAYV_FILTER_SUSTAIN, \
        pydaw_ports.RAYV_FILTER_RELEASE, "ADSR Filter", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout2.addWidget(m_adsr_filter.groupbox)
        m_filter =  pydaw_filter_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_TIMBRE, pydaw_ports.RAYV_RES)
        self.hlayout2.addWidget(m_filter.groupbox)
        m_filter_env_amt =  pydaw_knob_control(64, "Env Amt", pydaw_ports.RAYV_FILTER_ENV_AMT, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0, kc_integer, self.port_dict)
        m_filter_env_amt.add_to_grid_layout(m_filter.layout)
        self.hlayout3 = QtGui.QHBoxLayout()
        m_main_layout.addLayout(self.hlayout3)
        m_master =  pydaw_master_widget(64, self.plugin_rel_callback, self.plugin_val_callback, pydaw_ports.RAYV_MASTER_VOLUME, \
        pydaw_ports.RAYV_MASTER_GLIDE, pydaw_ports.RAYV_MASTER_PITCHBEND_AMT, self.port_dict, "Master", \
        pydaw_ports.RAYV_MASTER_UNISON_VOICES,pydaw_ports.RAYV_MASTER_UNISON_SPREAD)
        self.hlayout3.addWidget(m_master.group_box)
        m_pitch_env =  pydaw_ramp_env_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_PITCH_ENV_TIME, pydaw_ports.RAYV_PITCH_ENV_AMT, "Pitch Env")
        self.hlayout3.addWidget(m_pitch_env.groupbox)
        m_lfo =  pydaw_lfo_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_LFO_FREQ, pydaw_ports.RAYV_LFO_TYPE, f_lfo_types, "LFO")
        self.hlayout3.addWidget(m_lfo.groupbox)

        m_lfo_amp =  pydaw_knob_control(64, "Amp", pydaw_ports.RAYV_LFO_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -24, 24, 0, kc_integer, self.port_dict)
        m_lfo_amp.add_to_grid_layout(m_lfo.layout, 2)
        m_lfo_pitch =  pydaw_knob_control(64, "Pitch", pydaw_ports.RAYV_LFO_PITCH, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0, kc_integer, self.port_dict)
        m_lfo_pitch.add_to_grid_layout(m_lfo.layout, 3)
        m_lfo_cutoff =  pydaw_knob_control(64, "Filter", pydaw_ports.RAYV_LFO_FILTER, self.plugin_rel_callback, self.plugin_val_callback, \
        -48, 48, 0, kc_integer, self.port_dict)
        m_lfo.lms_groupbox.lms_add_h(m_lfo_cutoff)

        """Add the knobs to the preset module"""
        m_program.add_control(m_adsr_amp.attack_knob)
        m_program.add_control(m_adsr_amp.decay_knob)
        m_program.add_control(m_adsr_amp.sustain_knob)
        m_program.add_control(m_adsr_amp.release_knob)
        m_program.add_control(m_filter.cutoff_knob)
        m_program.add_control(m_filter.res_knob)
        m_program.add_control(m_dist)
        m_program.add_control(m_adsr_filter.attack_knob)
        m_program.add_control(m_adsr_filter.decay_knob)
        m_program.add_control(m_adsr_filter.sustain_knob)
        m_program.add_control(m_adsr_filter.release_knob)
        m_program.add_control(m_noise_amp)
        m_program.add_control(m_filter_env_amt)
        m_program.add_control(m_dist_wet)
        m_program.add_control(m_osc1.osc_type_combobox)
        m_program.add_control(m_osc1.pitch_knob)
        m_program.add_control(m_osc1.fine_knob)
        m_program.add_control(m_osc1.vol_knob)
        m_program.add_control(m_osc2.osc_type_combobox)
        m_program.add_control(m_osc2.pitch_knob)
        m_program.add_control(m_osc2.fine_knob)
        m_program.add_control(m_osc2.vol_knob)
        m_program.add_control(m_master.vol_knob)
        m_program.add_control(m_master.uni_voices_knob)
        m_program.add_control(m_master.uni_spread_knob)
        m_program.add_control(m_master.glide_knob)
        m_program.add_control(m_master.pb_knob)
        m_program.add_control(m_pitch_env.time_knob)
        m_program.add_control(m_pitch_env.amt_knob)
        m_program.add_control(m_lfo.freq_knob)
        m_program.add_control(m_lfo.type_combobox)
        m_program.add_control(m_lfo_amp)
        m_program.add_control(m_lfo_pitch)
        m_program.add_control(m_lfo_cutoff)
        m_program.add_control(m_hard_sync)

        self.generate_control_dict()
        self.open_plugin_file()

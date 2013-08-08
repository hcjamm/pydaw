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
    def __init__(self, a_size, a_min_val, a_max_val):
        QtGui.QDial.__init__(self)
        self.setRange(a_min_val, a_max_val)
        self.setGeometry(0, 0, a_size, a_size)
        self.set_pixmap_knob(a_size)
        self.setFixedSize(a_size, a_size)

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
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion=kc_none, a_port_dict=None, a_port_offset_callback=None):
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
        self.port_offset_callback = a_port_offset_callback

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
        self.rel_callback(self.port_num, self.control.value())

    def control_value_changed(self, a_value):
        if not self.suppress_changes:
            if self.port_offset_callback is None:
                self.val_callback(self.port_num, self.control.value())
            else:
                self.val_callback(self.port_num + self.port_offset_callback(), self.control.value())
        if self.value_label is not None:
            f_value = float(a_value)
            f_dec_value = 0.0
            if self.val_conversion == kc_none:
                pass
            elif self.val_conversion == kc_decimal:
                self.value_label.setText(str(f_value * .01))
            elif self.val_conversion == kc_integer:
                self.value_label.setText(str(f_value))
            elif self.val_conversion == kc_pitch:
                self.value_label.setText(str(int(440.0 * pow(2.0,((float)(f_value - 57.0)) * 0.0833333))))
            elif self.val_conversion == kc_127_pitch:
                self.value_label.setText(str(int(440.0 * pow(2.0, ((float)(((f_value * 0.818897638) + 20.0) -57.0)) * 0.0833333))))
            elif self.val_conversion == kc_127_zero_to_x:
                f_dec_value = (float(f_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
                f_dec_value = ((int)(f_dec_value * 10.0)) * 0.1
                self.value_label.setText(str(f_dec_value))
            elif self.val_conversion == kc_127_zero_to_x_int:
                f_dec_value = (float(f_value) * self.label_value_127_multiply_by) - self.label_value_127_add_to
                self.value_label.setText(str(int(f_dec_value)))
            elif self.val_conversion == kc_log_time:
                f_dec_value = float(f_value) * 0.01
                f_dec_value = f_dec_value * f_dec_value
                f_dec_value = (int(f_dec_value * 100.0)) * 0.01
                self.value_label.setText(str(f_dec_value))

    def add_to_grid_layout(self, a_layout, a_x):
        if self.name_label is not None:
            a_layout.addWidget(self.name_label, 0, a_x)
        a_layout.addWidget(self.control, 1, a_x)
        if self.value_label is not None:
            a_layout.addWidget(self.value_label, 2, a_x)


class pydaw_knob_control(pydaw_abstract_ui_control):
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion, a_port_dict)
        self.control = pydaw_pixmap_knob(a_size, a_min_val, a_max_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.set_value(a_default_val)


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
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_items_list=[], a_port_dict=None):
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

    def control_value_changed(self, a_val):
        if not self.suppress_changes:
            self.val_callback(self.port_num, a_val)
            self.rel_callback(self.port_num, a_val)

    def set_value(self, a_val):
        self.suppress_changes = True
        self.control.setCurrentIndex(int(a_val))
        self.suppress_changes = False

    def get_value(self):
        return self.control.currentIndex()

class pydaw_adsr_widget:
    def __init__(self, a_size, a_sustain_in_db, a_attack_port, a_decay_port, a_sustain_port, a_release_port, \
            a_label, a_rel_callback, a_val_callback, a_port_dict=None):
        self.attack_knob = pydaw_knob_control(a_size, "Attack", a_attack_port, a_rel_callback, a_val_callback, 0, 100, 0, kc_log_time, a_port_dict)
        self.decay_knob = pydaw_knob_control(a_size, "Decay", a_decay_port, a_rel_callback, a_val_callback, 10, 100, 0, kc_log_time, a_port_dict)
        if a_sustain_in_db:
            self.sustain_knob = pydaw_knob_control(a_size, "Sustain", a_sustain_port, a_rel_callback, a_val_callback, -30, 0, 0, kc_integer, a_port_dict)
        else:
            self.sustain_knob = pydaw_knob_control(a_size, "Sustain", a_sustain_port, a_rel_callback, a_val_callback, 0, 100, 100, kc_decimal, a_port_dict)
        self.release_knob = pydaw_knob_control(a_size, "Release", a_release_port, a_rel_callback, a_val_callback, 10, 400, 0, kc_log_time, a_port_dict)
        self.groupbox = QtGui.QGroupBox(a_label)
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.attack_knob.add_to_grid_layout(self.layout, 0)
        self.decay_knob.add_to_grid_layout(self.layout, 1)
        self.sustain_knob.add_to_grid_layout(self.layout, 2)
        self.release_knob.add_to_grid_layout(self.layout, 3)

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
        self.type_combobox = pydaw_combobox_control(120, "Type", a_type_port, a_rel_callback, a_val_callback, a_type_list, a_port_dict)
        self.layout.addWidget(self.type_combobox.name_label, 0, 1)
        self.layout.addWidget(self.type_combobox.control, 1, 1)

class pydaw_osc_widget:
    def __init__(self, a_size, a_pitch_port, a_fine_port, a_vol_port, a_type_port, a_osc_types_list, \
    a_rel_callback, a_val_callback, a_label, a_port_dict=None):
        self.pitch_knob = pydaw_knob_control(a_size, "Pitch", a_pitch_port, a_rel_callback, a_val_callback, -36, 36, 0, \
        a_val_conversion=kc_integer, a_port_dict=a_port_dict)
        self.fine_knob = pydaw_knob_control(a_size, "Fine", a_fine_port, a_rel_callback, a_val_callback, -100, 100, 0, \
        a_val_conversion=kc_decimal, a_port_dict=a_port_dict)
        self.vol_knob = pydaw_knob_control(a_size, "Vol", a_vol_port, a_rel_callback, a_val_callback, -30, 0, -6, \
        a_val_conversion=kc_integer, a_port_dict=a_port_dict)
        self.osc_type_combobox = pydaw_combobox_control(114, "Type", a_type_port, a_rel_callback, a_val_callback, a_osc_types_list, a_port_dict)
        self.grid_layout = QtGui.QGridLayout()
        self.group_box = QtGui.QGroupBox(str(a_label))
        self.group_box.setLayout(self.grid_layout)
        self.pitch_knob.add_to_grid_layout(self.grid_layout, 0)
        self.fine_knob.add_to_grid_layout(self.grid_layout, 1)
        self.vol_knob.add_to_grid_layout(self.grid_layout, 2)
        self.grid_layout.addWidget(self.osc_type_combobox.name_label, 0, 3)
        self.grid_layout.addWidget(self.osc_type_combobox.control, 1, 3)

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
    def __init__(self, a_plugin_name,  a_default_presets):
        if os.path.isdir("/media/pydaw_data") and os.path.isdir("/home/ubuntu"):
            self.preset_path = "/media/pydaw_data/pydaw3/" + str(a_plugin_name) + ".pypresets"
        else:
            self.preset_path = os.path.expanduser("~") + "/pydaw3/" + str(a_plugin_name) + ".pypresets"
        self.group_box = QtGui.QGroupBox()
        self.layout = QtGui.QHBoxLayout(self.group_box)
        self.program_combobox = QtGui.QComboBox()
        self.program_combobox.setEditable(True)
        self.program_combobox.setMinimumWidth(190)
        self.layout.addWidget(self.program_combobox)
        self.save_button = QtGui.QPushButton("Save")
        self.save_button.pressed.connect(self.save_presets)
        self.layout.addWidget(self.save_button)
        self.presets_tab_delimited = []
        self.controls = []
        self.load_presets(a_default_presets)
        self.program_combobox.currentIndexChanged.connect(self.program_changed)

    def load_presets(self, a_text=None):
        if os.path.isfile(self.preset_path):
            print("loading presets from file")
            f_text = pydaw_util.pydaw_read_file_text(self.preset_path)
        else:
            print("loading presets from defaults")
            f_text = str(a_text)
        f_line_arr = f_text.split("\n")
        print(str(len(f_line_arr)))
        self.presets_tab_delimited = []
        for f_i in range(128):
            self.presets_tab_delimited.append(f_line_arr[f_i].split("\t"))
            self.program_combobox.addItem(self.presets_tab_delimited[f_i][0])

    def save_presets(self):
        print("saving preset")
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

    def program_changed(self, a_val=None):
        if str(self.program_combobox.currentText()) == "empty":
            print("empty")
        else:
            f_preset = self.presets_tab_delimited[self.program_combobox.currentIndex()]
            print("setting preset " + str(f_preset))
            for f_i in range(1, len(f_preset)):
                self.controls[f_i - 1].set_value(f_preset[f_i])
                self.controls[f_i - 1].control_value_changed(f_preset[f_i])

    def add_control(self, a_control):
        self.controls.append(a_control)

class pydaw_master_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_master_vol_port, a_master_glide_port, a_master_pitchbend_port, \
    a_port_dict, a_title="Master", a_master_uni_voices_port=None, a_master_uni_spread_port=None):
        self.group_box = QtGui.QGroupBox()
        self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout(self.group_box)
        self.vol_knob = pydaw_knob_control(a_size, "Vol", a_master_vol_port, a_rel_callback, a_val_callback, -30, 12, -6, kc_integer, a_port_dict)
        self.vol_knob.add_to_grid_layout(self.layout, 0)
        if a_master_uni_voices_port is not None and a_master_uni_spread_port is not None:
            self.uni_voices_knob = pydaw_knob_control(a_size, "Unison", a_master_uni_voices_port, a_rel_callback, a_val_callback,\
            1, 7, 1, kc_integer, a_port_dict)
            self.uni_voices_knob.add_to_grid_layout(self.layout, 1)
            self.uni_spread_knob = pydaw_knob_control(a_size, "Spread", a_master_uni_spread_port, a_rel_callback, a_val_callback,\
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
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyinst"
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Ray-V - " + self.track_name)
        self.is_instrument = True
        f_osc_types = ["Saw" , "Square" , "Triangle" , "Sine" , "Off"]
        f_lfo_types = ["Off" , "Sine" , "Triangle"]
        f_default_presets = ("empty\nclassic 5th pad\t68\t95\t-8\t153\t90\t-15\t15\t94\t100\t4\t166\t-19\t36\t1\t0\t0\t0\t0\t0\t7\t0\t0\t-14\t5\t42\t1\t18\t38\t0\t10\t0\t0\t0\t0\t0\n303 acid lead\t41\t58\t-9\t47\t70\t0\t36\t37\t95\t1\t99\t-30\t36\t100\t0\t0\t0\t0\t4\t0\t0\t0\t-8\t1\t10\t1\t18\t1\t0\t10\t0\t0\t0\t0\t0\nhoover\t39\t53\t-9\t45\t124\t-16\t15\t12\t29\t1\t99\t-12\t0\t1\t0\t0\t0\t0\t4\t0\t0\t0\t-8\t4\t42\t1\t18\t73\t-26\t10\t0\t0\t0\t0\t0\nbendy saw\t10\t49\t-3\t16\t124\t-16\t15\t100\t100\t1\t162\t-60\t0\t1\t0\t0\t0\t0\t4\t0\t0\t0\t-16\t1\t42\t54\t36\t1\t0\t10\t0\t0\t0\t0\t0\nsupersaw lead\t10\t49\t-3\t61\t124\t-15\t36\t10\t33\t1\t162\t-12\t0\t1\t0\t0\t0\t-6\t4\t0\t0\t0\t-16\t5\t41\t1\t17\t1\t0\t10\t0\t0\t0\t0\t0\n3rd Plucks\t10\t49\t-20\t124\t90\t-9\t36\t10\t29\t1\t73\t-12\t36\t1\t0\t0\t0\t-6\t0\t5\t0\t0\t-16\t5\t50\t1\t17\t1\t0\t10\t0\t0\t0\t0\t0\nsquare lead\t3\t49\t-12\t60\t124\t-9\t36\t1\t21\t1\t73\t-12\t36\t1\t1\t0\t0\t-6\t4\t0\t0\t0\t-16\t4\t50\t1\t17\t1\t0\t0\t0\t0\t0\t0\t0\ntriangle kick drum\t3\t49\t-12\t60\t124\t-9\t36\t1\t21\t1\t73\t-37\t36\t1\t2\t0\t0\t-6\t4\t0\t0\t0\t-5\t4\t50\t1\t17\t8\t-24\t0\t0\t0\t0\t0\t0\nnoise snare\t10\t51\t-30\t14\t99\t-6\t36\t21\t38\t1\t73\t-3\t36\t100\t4\t0\t0\t-6\t4\t0\t0\t0\t-18\t4\t50\t1\t17\t17\t-24\t10\t0\t0\t0\t0\t0\nelectro open hihat\t39\t49\t-30\t14\t95\t-3\t36\t36\t43\t1\t73\t-3\t36\t100\t4\t0\t0\t-6\t4\t0\t0\t0\t-18\t4\t50\t1\t17\t17\t-24\t10\t0\t0\t0\t0\t0\nSynchronize Me\t10\t32\t0\t57\t124\t-15\t15\t32\t32\t75\t57\t-60\t0\t99\t3\t0\t0\t0\t4\t0\t0\t0\t-6\t1\t50\t0\t18\t100\t-12\t205\t0\t0\t0\t0\t1\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty")
        self.preset_manager =  pydaw_preset_manager_widget("LMS_RAYV", f_default_presets)
        self.main_layout = QtGui.QVBoxLayout()
        self.layout.addLayout(self.main_layout)
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout0)
        self.hlayout0.addWidget(self.preset_manager.group_box)
        self.hlayout0.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        f_logo_label =  QtGui.QLabel()
        f_pixmap = QtGui.QPixmap("/usr/lib/pydaw3/themes/default/rayv.png").scaledToHeight(60)
        f_logo_label.setMinimumSize(90, 30)
        f_logo_label.setPixmap(f_pixmap)
        self.hlayout0.addWidget(f_logo_label)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout1)
        self.osc1 =  pydaw_osc_widget(64, pydaw_ports.RAYV_OSC1_PITCH, pydaw_ports.RAYV_OSC1_TUNE, pydaw_ports.RAYV_OSC1_VOLUME, pydaw_ports.RAYV_OSC1_TYPE, \
        f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 1", self.port_dict)
        self.hlayout1.addWidget(self.osc1.group_box)
        self.adsr_amp = pydaw_adsr_widget(64, True, pydaw_ports.RAYV_ATTACK, pydaw_ports.RAYV_DECAY, pydaw_ports.RAYV_SUSTAIN, pydaw_ports.RAYV_RELEASE, \
        "ADSR Amp", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout1.addWidget(self.adsr_amp.groupbox)
        self.groupbox_distortion =  QtGui.QGroupBox("Distortion")
        self.groupbox_distortion_layout = QtGui.QGridLayout(self.groupbox_distortion)
        self.hlayout1.addWidget(self.groupbox_distortion)
        self.dist =  pydaw_knob_control(64, "Gain", pydaw_ports.RAYV_DIST, self.plugin_rel_callback, self.plugin_val_callback, \
        -6, 36, 12, kc_integer, self.port_dict)
        self.dist.add_to_grid_layout(self.groupbox_distortion_layout, 0)
        self.dist_wet =  pydaw_knob_control(64, "Wet", pydaw_ports.RAYV_DIST_WET, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_none, self.port_dict)
        self.dist_wet.add_to_grid_layout(self.groupbox_distortion_layout, 1)
        self.groupbox_noise =  QtGui.QGroupBox("Noise")
        self.noise_layout = QtGui.QGridLayout(self.groupbox_noise)
        self.hlayout1.addWidget(self.groupbox_noise)
        self.noise_amp =  pydaw_knob_control(64, "Vol", pydaw_ports.RAYV_NOISE_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -60, 0, -30, kc_integer, self.port_dict)
        self.noise_amp.add_to_grid_layout(self.noise_layout, 0)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout2)
        self.osc2 =  pydaw_osc_widget(64, pydaw_ports.RAYV_OSC2_PITCH, pydaw_ports.RAYV_OSC2_TUNE, pydaw_ports.RAYV_OSC2_VOLUME, pydaw_ports.RAYV_OSC2_TYPE, \
        f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 2", self.port_dict)
        self.hlayout2.addWidget(self.osc2.group_box)
        self.sync_groupbox =  QtGui.QGroupBox("Sync")
        self.hlayout2.addWidget(self.sync_groupbox)
        self.sync_gridlayout = QtGui.QGridLayout(self.sync_groupbox)
        self.hard_sync =  pydaw_checkbox_control("On", pydaw_ports.RAYV_OSC_HARD_SYNC, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict)
        self.hard_sync.control.setToolTip(("Setting self hard sync's Osc1 to Osc2. Usually you would want to distort and pitchbend self."))
        self.sync_gridlayout.addWidget(self.hard_sync.control, 1, 0, QtCore.Qt.AlignCenter)
        self.adsr_filter =  pydaw_adsr_widget(64, False, pydaw_ports.RAYV_FILTER_ATTACK, pydaw_ports.RAYV_FILTER_DECAY, pydaw_ports.RAYV_FILTER_SUSTAIN, \
        pydaw_ports.RAYV_FILTER_RELEASE, "ADSR Filter", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout2.addWidget(self.adsr_filter.groupbox)
        self.filter =  pydaw_filter_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_TIMBRE, pydaw_ports.RAYV_RES)
        self.hlayout2.addWidget(self.filter.groupbox)
        self.filter_env_amt =  pydaw_knob_control(64, "Env Amt", pydaw_ports.RAYV_FILTER_ENV_AMT, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0, kc_integer, self.port_dict)
        self.filter_env_amt.add_to_grid_layout(self.filter.layout, 2)
        self.hlayout3 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout3)
        self.master =  pydaw_master_widget(64, self.plugin_rel_callback, self.plugin_val_callback, pydaw_ports.RAYV_MASTER_VOLUME, \
        pydaw_ports.RAYV_MASTER_GLIDE, pydaw_ports.RAYV_MASTER_PITCHBEND_AMT, self.port_dict, "Master", \
        pydaw_ports.RAYV_MASTER_UNISON_VOICES,pydaw_ports.RAYV_MASTER_UNISON_SPREAD)
        self.hlayout3.addWidget(self.master.group_box)
        self.pitch_env =  pydaw_ramp_env_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_PITCH_ENV_TIME, pydaw_ports.RAYV_PITCH_ENV_AMT, "Pitch Env")
        self.hlayout3.addWidget(self.pitch_env.groupbox)
        self.lfo =  pydaw_lfo_widget(64, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.RAYV_LFO_FREQ, pydaw_ports.RAYV_LFO_TYPE, f_lfo_types, "LFO")
        self.hlayout3.addWidget(self.lfo.groupbox)

        self.lfo_amp =  pydaw_knob_control(64, "Amp", pydaw_ports.RAYV_LFO_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -24, 24, 0, kc_integer, self.port_dict)
        self.lfo_amp.add_to_grid_layout(self.lfo.layout, 2)
        self.lfo_pitch =  pydaw_knob_control(64, "Pitch", pydaw_ports.RAYV_LFO_PITCH, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0, kc_integer, self.port_dict)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 3)
        self.lfo_cutoff =  pydaw_knob_control(64, "Filter", pydaw_ports.RAYV_LFO_FILTER, self.plugin_rel_callback, self.plugin_val_callback, \
        -48, 48, 0, kc_integer, self.port_dict)
        self.lfo_cutoff.add_to_grid_layout(self.lfo.layout, 4)

        """Add the knobs to the preset module"""
        self.preset_manager.add_control(self.adsr_amp.attack_knob)
        self.preset_manager.add_control(self.adsr_amp.decay_knob)
        self.preset_manager.add_control(self.adsr_amp.sustain_knob)
        self.preset_manager.add_control(self.adsr_amp.release_knob)
        self.preset_manager.add_control(self.filter.cutoff_knob)
        self.preset_manager.add_control(self.filter.res_knob)
        self.preset_manager.add_control(self.dist)
        self.preset_manager.add_control(self.adsr_filter.attack_knob)
        self.preset_manager.add_control(self.adsr_filter.decay_knob)
        self.preset_manager.add_control(self.adsr_filter.sustain_knob)
        self.preset_manager.add_control(self.adsr_filter.release_knob)
        self.preset_manager.add_control(self.noise_amp)
        self.preset_manager.add_control(self.filter_env_amt)
        self.preset_manager.add_control(self.dist_wet)
        self.preset_manager.add_control(self.osc1.osc_type_combobox)
        self.preset_manager.add_control(self.osc1.pitch_knob)
        self.preset_manager.add_control(self.osc1.fine_knob)
        self.preset_manager.add_control(self.osc1.vol_knob)
        self.preset_manager.add_control(self.osc2.osc_type_combobox)
        self.preset_manager.add_control(self.osc2.pitch_knob)
        self.preset_manager.add_control(self.osc2.fine_knob)
        self.preset_manager.add_control(self.osc2.vol_knob)
        self.preset_manager.add_control(self.master.vol_knob)
        self.preset_manager.add_control(self.master.uni_voices_knob)
        self.preset_manager.add_control(self.master.uni_spread_knob)
        self.preset_manager.add_control(self.master.glide_knob)
        self.preset_manager.add_control(self.master.pb_knob)
        self.preset_manager.add_control(self.pitch_env.time_knob)
        self.preset_manager.add_control(self.pitch_env.amt_knob)
        self.preset_manager.add_control(self.lfo.freq_knob)
        self.preset_manager.add_control(self.lfo.type_combobox)
        self.preset_manager.add_control(self.lfo_amp)
        self.preset_manager.add_control(self.lfo_pitch)
        self.preset_manager.add_control(self.lfo_cutoff)
        self.preset_manager.add_control(self.hard_sync)

        self.generate_control_dict()
        self.open_plugin_file()



class pydaw_wayv_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, a_close_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, a_close_callback)
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyinst"
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Way-V - " + self.track_name)
        self.is_instrument = True

        f_osc_types = [
            "Off"
            #Saw-like waves
            , "Plain Saw" , "SuperbSaw" , "Viral Saw" , "Soft Saw" , "Mid Saw" , "Lush Saw"
            #Square-like waves
            , "Evil Square" , "Punchy Square" , "Soft Square"
            #Glitchy and distorted waves
            , "Pink Glitch" , "White Glitch" , "Acid" , "Screetch"
            #Sine and triangle-like waves
            , "Thick Bass" , "Rattler" , "Deep Saw" , "Sine"
        ]
        f_lfo_types = [ "Off" , "Sine" , "Triangle"]
        f_default_presets = ("empty\nWaterfall Strings\t10\t32\t0\t57\t10\t55\t0\t57\t32\t55\t75\t57\t10\t32\t0\t57\t10\t32\t0\t57\t407\t0\t-60\t0\t200\t1\t0\t0\t-9\t12\t12\t0\t-9\t-9\t0\t18\t0\t0\t63\t63\t63\t1\t63\t63\t63\t0\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t41\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t4\t50\t4\t50\t100\t0\t0\t0\t0\t10\t32\t0\t57\t0\t4\t50\t0\t49\t0\t0\t0\t0\t0\t0\t0\nJP Pad\t41\t32\t0\t57\t10\t55\t0\t57\t32\t55\t75\t57\t10\t32\t0\t57\t10\t32\t0\t57\t35\t1\t-16\t1\t100\t2\t0\t0\t0\t0\t0\t0\t0\t-9\t0\t18\t0\t0\t63\t63\t63\t8\t63\t63\t63\t0\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t27\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t4\t50\t4\t50\t100\t0\t0\t0\t0\t10\t32\t0\t57\t0\t4\t50\t0\t0\t0\t0\t0\t0\t0\t0\t0\nCyborg Honeybee\t10\t32\t0\t57\t10\t55\t0\t57\t32\t55\t75\t57\t10\t32\t0\t57\t10\t32\t0\t57\t407\t0\t-14\t1\t100\t17\t0\t0\t-10\t17\t0\t0\t-30\t-9\t45\t18\t0\t0\t63\t63\t127\t17\t63\t63\t63\t0\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t1\t50\t4\t50\t100\t6\t0\t0\t-7\t10\t32\t0\t57\t0\t4\t50\t0\t73\t0\t73\t0\t0\t43\t0\t0\nWeird FM Pluck\t41\t32\t0\t57\t10\t55\t0\t57\t32\t55\t75\t57\t10\t32\t0\t57\t10\t37\t-30\t57\t35\t1\t-60\t1\t100\t17\t0\t0\t-6\t17\t0\t0\t0\t-9\t0\t18\t0\t1\t63\t63\t63\t8\t63\t63\t63\t0\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t27\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t1\t50\t1\t50\t100\t0\t0\t0\t0\t10\t32\t0\t57\t0\t4\t50\t0\t52\t0\t27\t0\t0\t0\t0\t0\nSlow Thunder\t41\t32\t0\t57\t10\t55\t0\t57\t32\t55\t75\t57\t10\t32\t0\t57\t10\t100\t-30\t57\t35\t0\t-60\t1\t200\t15\t0\t0\t-6\t15\t7\t0\t0\t-9\t0\t18\t0\t1\t123\t127\t0\t10\t102\t104\t63\t5\t63\t63\t63\t16\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-52\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t3\t50\t1\t50\t100\t15\t12\t0\t-30\t100\t32\t0\t57\t1\t4\t50\t0\t19\t0\t27\t0\t20\t25\t16\t0\nAlien Talk\t41\t32\t0\t57\t10\t55\t0\t57\t12\t37\t0\t57\t10\t32\t0\t57\t10\t23\t-11\t57\t249\t1\t-60\t1\t200\t6\t0\t0\t-6\t15\t-12\t0\t-8\t-9\t0\t18\t0\t1\t58\t127\t0\t15\t127\t104\t63\t1\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t21\t0\t0\t0\t0\t0\t53\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t4\t50\t1\t50\t100\t0\t12\t0\t-30\t100\t32\t0\t57\t0\t4\t50\t0\t19\t0\t0\t0\t0\t25\t16\t0\nThick FM Bass\t41\t32\t0\t57\t10\t55\t0\t57\t12\t37\t0\t57\t10\t32\t0\t57\t10\t23\t-11\t57\t249\t0\t-60\t1\t200\t17\t0\t0\t-6\t17\t-12\t0\t-8\t-9\t0\t18\t0\t1\t58\t127\t0\t0\t127\t104\t63\t0\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t1\t50\t1\t50\t100\t17\t12\t0\t-15\t100\t32\t0\t57\t0\t4\t50\t0\t19\t0\t0\t0\t0\t25\t16\t0\nDubstep Apocalypse\t41\t32\t0\t57\t10\t55\t0\t57\t12\t37\t0\t57\t10\t32\t0\t57\t10\t23\t-11\t57\t249\t0\t-60\t1\t200\t17\t0\t0\t-6\t17\t-12\t0\t-8\t-19\t0\t18\t0\t1\t58\t64\t0\t16\t66\t90\t63\t11\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-33\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t1\t50\t1\t50\t100\t17\t12\t0\t-15\t100\t32\t0\t57\t0\t1\t50\t0\t39\t0\t0\t0\t0\t57\t44\t0\nSoft Pluck\t25\t32\t-9\t153\t10\t55\t0\t57\t12\t37\t0\t57\t10\t32\t0\t57\t10\t23\t-11\t57\t249\t0\t-9\t1\t11\t6\t0\t0\t0\t0\t-12\t0\t-8\t-8\t0\t18\t0\t0\t58\t64\t0\t0\t124\t90\t63\t2\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-59\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t4\t50\t1\t50\t100\t0\t12\t0\t-15\t100\t32\t0\t57\t0\t1\t50\t0\t0\t0\t0\t0\t0\t0\t0\t0\nThin Reese\t25\t32\t-9\t84\t10\t55\t0\t57\t12\t37\t0\t57\t10\t32\t0\t57\t10\t23\t-11\t57\t249\t0\t-9\t1\t11\t3\t0\t0\t0\t16\t-12\t0\t0\t-8\t0\t18\t0\t0\t85\t64\t116\t9\t124\t90\t0\t10\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t1\t50\t1\t50\t100\t0\t12\t0\t-15\t100\t32\t0\t57\t0\t1\t50\t0\t31\t0\t0\t0\t0\t0\t0\t0\nFM Percussion\t10\t32\t0\t57\t10\t55\t0\t57\t32\t55\t75\t57\t10\t32\t0\t57\t10\t47\t-30\t57\t407\t0\t-60\t0\t100\t17\t0\t0\t0\t17\t6\t0\t-30\t-9\t0\t18\t0\t1\t63\t63\t63\t0\t63\t63\t63\t0\t63\t63\t63\t0\t63\t63\t63\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t1\t50\t1\t50\t100\t17\t12\t0\t-30\t10\t32\t-30\t57\t1\t1\t50\t0\t17\t0\t0\t0\t42\t0\t0\t0\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty")
        self.tab_widget =  QtGui.QTabWidget()
        self.layout.addWidget(self.tab_widget)
        self.osc_tab =  QtGui.QWidget()
        self.tab_widget.addTab(self.osc_tab, ("Oscillators"))
        self.poly_fx_tab =  QtGui.QWidget()
        self.tab_widget.addTab(self.poly_fx_tab, ("PolyFX"))
        self.oscillator_layout =  QtGui.QVBoxLayout(self.osc_tab)
        self.program =  pydaw_preset_manager_widget("WAY_V", f_default_presets)
        self.hlayout0 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout0)
        self.hlayout0.addWidget(self.program.group_box)
        self.hlayout0.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))

        self.hlayout1 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout1)
        self.osc1 =  pydaw_osc_widget(51, pydaw_ports.WAYV_OSC1_PITCH, pydaw_ports.WAYV_OSC1_TUNE, pydaw_ports.WAYV_OSC1_VOLUME, \
        pydaw_ports.WAYV_OSC1_TYPE, f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 1", self.port_dict)
        self.osc1_uni_voices =  pydaw_knob_control(51, "Unison", pydaw_ports.WAYV_OSC1_UNISON_VOICES, self.plugin_rel_callback, self.plugin_val_callback, \
        1, 7, 4, kc_integer, self.port_dict)
        self.osc1.osc_type_combobox.control.setCurrentIndex(1)
        self.osc1_uni_voices.add_to_grid_layout(self.osc1.grid_layout, 4)
        self.osc1_uni_spread =  pydaw_knob_control(51, "Spread", pydaw_ports.WAYV_OSC1_UNISON_SPREAD, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 30, kc_decimal, self.port_dict)
        self.osc1_uni_spread.add_to_grid_layout(self.osc1.grid_layout, 5)

        self.hlayout1.addWidget(self.osc1.group_box)

        self.adsr_amp1 =  pydaw_adsr_widget(51,  True, pydaw_ports.WAYV_ATTACK1, pydaw_ports.WAYV_DECAY1, pydaw_ports.WAYV_SUSTAIN1, \
        pydaw_ports.WAYV_RELEASE1, "ADSR Osc1", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout1.addWidget(self.adsr_amp1.groupbox)

        self.adsr_amp1_checkbox =  pydaw_checkbox_control("On", pydaw_ports.WAYV_ADSR1_CHECKBOX, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict)
        self.adsr_amp1_checkbox.add_to_grid_layout(self.adsr_amp1.layout, 4)

        self.groupbox_osc1_fm =  QtGui.QGroupBox("Osc1 FM")
        self.groupbox_osc1_fm_layout = QtGui.QGridLayout(self.groupbox_osc1_fm)

        self.osc1_fm1 =  pydaw_knob_control(51, "Osc1", pydaw_ports.WAYV_OSC1_FM1, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc1_fm1.add_to_grid_layout(self.groupbox_osc1_fm_layout, 0)

        self.osc1_fm2 =  pydaw_knob_control(51, "Osc2", pydaw_ports.WAYV_OSC1_FM2, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc1_fm2.add_to_grid_layout(self.groupbox_osc1_fm_layout, 1)

        self.osc1_fm3 =  pydaw_knob_control(51, "Osc3", pydaw_ports.WAYV_OSC1_FM3, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc1_fm3.add_to_grid_layout(self.groupbox_osc1_fm_layout, 2)

        self.hlayout1.addWidget(self.groupbox_osc1_fm)
        #self.hlayout1.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))

        #Osc2
        self.hlayout2 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout2)
        self.osc2 =  pydaw_osc_widget(51, pydaw_ports.WAYV_OSC2_PITCH, pydaw_ports.WAYV_OSC2_TUNE, pydaw_ports.WAYV_OSC2_VOLUME, \
        pydaw_ports.WAYV_OSC2_TYPE, f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 2", self.port_dict)
        self.osc2_uni_voices =  pydaw_knob_control(51, "Unison", pydaw_ports.WAYV_OSC2_UNISON_VOICES, self.plugin_rel_callback, self.plugin_val_callback, \
        1, 7, 4, kc_integer, self.port_dict)
        self.osc2_uni_voices.add_to_grid_layout(self.osc2.grid_layout, 4)
        self.osc2_uni_spread =  pydaw_knob_control(51, "Spread", pydaw_ports.WAYV_OSC2_UNISON_SPREAD, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 30, kc_decimal, self.port_dict)
        self.osc2_uni_spread.add_to_grid_layout(self.osc2.grid_layout, 5)

        self.hlayout2.addWidget(self.osc2.group_box)

        self.adsr_amp2 =  pydaw_adsr_widget(51,  True, pydaw_ports.WAYV_ATTACK2, pydaw_ports.WAYV_DECAY2, pydaw_ports.WAYV_SUSTAIN2, \
        pydaw_ports.WAYV_RELEASE2, "ADSR Osc2", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout2.addWidget(self.adsr_amp2.groupbox)

        self.adsr_amp2_checkbox =  pydaw_checkbox_control("On", pydaw_ports.WAYV_ADSR2_CHECKBOX, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict)
        self.adsr_amp2_checkbox.add_to_grid_layout(self.adsr_amp2.layout, 4)

        self.groupbox_osc2_fm =  QtGui.QGroupBox("Osc2 FM")
        self.groupbox_osc2_fm_layout = QtGui.QGridLayout(self.groupbox_osc2_fm)

        self.osc2_fm1 =  pydaw_knob_control(51, "Osc2", pydaw_ports.WAYV_OSC2_FM1, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc2_fm1.add_to_grid_layout(self.groupbox_osc2_fm_layout, 0)

        self.osc2_fm2 =  pydaw_knob_control(51, "Osc2", pydaw_ports.WAYV_OSC2_FM2, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc2_fm2.add_to_grid_layout(self.groupbox_osc2_fm_layout, 1)

        self.osc2_fm3 =  pydaw_knob_control(51, "Osc3", pydaw_ports.WAYV_OSC2_FM3, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc2_fm3.add_to_grid_layout(self.groupbox_osc2_fm_layout, 2)

        self.hlayout2.addWidget(self.groupbox_osc2_fm)
        #self.hlayout2.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))

        #osc3
        self.hlayout3 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout3)
        self.osc3 =  pydaw_osc_widget(51, pydaw_ports.WAYV_OSC3_PITCH, pydaw_ports.WAYV_OSC3_TUNE, pydaw_ports.WAYV_OSC3_VOLUME, \
        pydaw_ports.WAYV_OSC3_TYPE, f_osc_types, self.plugin_rel_callback, self.plugin_val_callback, "Oscillator 3", self.port_dict)
        self.osc3_uni_voices =  pydaw_knob_control(51, "Unison", pydaw_ports.WAYV_OSC3_UNISON_VOICES, self.plugin_rel_callback, self.plugin_val_callback, \
        1, 7, 4, kc_integer, self.port_dict)
        self.osc3_uni_voices.add_to_grid_layout(self.osc3.grid_layout, 4)
        self.osc3_uni_spread =  pydaw_knob_control(51, "Spread", pydaw_ports.WAYV_OSC3_UNISON_SPREAD, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 30, kc_decimal, self.port_dict)
        self.osc3_uni_spread.add_to_grid_layout(self.osc3.grid_layout, 5)

        self.hlayout3.addWidget(self.osc3.group_box)

        self.adsr_amp3 =  pydaw_adsr_widget(51,  True, pydaw_ports.WAYV_ATTACK3, pydaw_ports.WAYV_DECAY3, pydaw_ports.WAYV_SUSTAIN3, \
        pydaw_ports.WAYV_RELEASE3, "ADSR Osc3", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout3.addWidget(self.adsr_amp3.groupbox)

        self.adsr_amp3_checkbox =  pydaw_checkbox_control("On", pydaw_ports.WAYV_ADSR3_CHECKBOX, self.plugin_rel_callback, self.plugin_val_callback, \
        self.port_dict)
        self.adsr_amp3_checkbox.add_to_grid_layout(self.adsr_amp3.layout, 4)

        self.groupbox_osc3_fm =  QtGui.QGroupBox("Osc3 FM")
        self.groupbox_osc3_fm_layout = QtGui.QGridLayout(self.groupbox_osc3_fm)

        self.osc3_fm1 =  pydaw_knob_control(51, "Osc1", pydaw_ports.WAYV_OSC3_FM1, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc3_fm1.add_to_grid_layout(self.groupbox_osc3_fm_layout, 0)

        self.osc3_fm2 =  pydaw_knob_control(51, "Osc2", pydaw_ports.WAYV_OSC3_FM2, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc3_fm2.add_to_grid_layout(self.groupbox_osc3_fm_layout, 1)

        self.osc3_fm3 =  pydaw_knob_control(51, "Osc3", pydaw_ports.WAYV_OSC3_FM3, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_integer, self.port_dict)
        self.osc3_fm3.add_to_grid_layout(self.groupbox_osc3_fm_layout, 2)

        self.hlayout3.addWidget(self.groupbox_osc3_fm)
        #self.hlayout3.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))


        self.hlayout4 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout4)
        self.master =  pydaw_master_widget(51,  self.plugin_rel_callback, self.plugin_val_callback, pydaw_ports.WAYV_MASTER_VOLUME, \
        pydaw_ports.WAYV_MASTER_GLIDE, pydaw_ports.WAYV_MASTER_PITCHBEND_AMT, self.port_dict)

        self.hlayout4.addWidget(self.master.group_box)

        self.adsr_amp_main =  pydaw_adsr_widget(51, True, pydaw_ports.WAYV_ATTACK_MAIN, pydaw_ports.WAYV_DECAY_MAIN, \
        pydaw_ports.WAYV_SUSTAIN_MAIN, pydaw_ports.WAYV_RELEASE_MAIN, "ADSR Master", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict )
        self.hlayout4.addWidget(self.adsr_amp_main.groupbox)

        self.groupbox_noise =  QtGui.QGroupBox("Noise")
        self.groupbox_noise_layout = QtGui.QGridLayout(self.groupbox_noise)
        self.hlayout4.addWidget(self.groupbox_noise)
        self.noise_amp =  pydaw_knob_control(51, "Vol", pydaw_ports.WAYV_NOISE_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -60, 0, -30, kc_integer, self.port_dict)
        self.noise_amp.add_to_grid_layout(self.groupbox_noise_layout, 0)

        self.noise_type =  pydaw_combobox_control(87, "Type", pydaw_ports.LMS_NOISE_TYPE, self.plugin_rel_callback, self.plugin_val_callback, \
        ["Off", "White", "Pink"], self.port_dict)
        self.noise_type.control.setMaximumWidth(87)
        self.noise_type.add_to_grid_layout(self.groupbox_noise_layout, 1)

        self.main_layout =  QtGui.QVBoxLayout(self.poly_fx_tab)
        self.hlayout5 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout5)
        self.hlayout6 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout6)
        #From Modulex
        self.fx0 =  pydaw_modulex_single("FX0", pydaw_ports.WAYV_FX0_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout5.addWidget(self.fx0.group_box)
        self.fx1 =  pydaw_modulex_single("FX1", pydaw_ports.WAYV_FX1_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout5.addWidget(self.fx1.group_box)
        self.fx2 =  pydaw_modulex_single("FX2", pydaw_ports.WAYV_FX2_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout6.addWidget(self.fx2.group_box)
        self.fx3 =  pydaw_modulex_single("FX3", pydaw_ports.WAYV_FX3_KNOB0, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout6.addWidget(self.fx3.group_box)

        self.mod_matrix = QtGui.QTableWidget()
        self.mod_matrix.setRowCount(4)
        self.mod_matrix.setColumnCount(12)
        #self.polyfx_mod_matrix[0].lms_mod_matrix.setMinimumHeight(165)
        #self.polyfx_mod_matrix[0].lms_mod_matrix.setMaximumHeight(165)
        self.mod_matrix.setHorizontalHeaderLabels(["FX0\nCtrl1", "FX0\nCtrl2", "FX0\nCtrl3", "FX1\nCtrl1", "FX1\nCtrl2", "FX1\nCtrl3", "FX2\nCtrl1",
            "FX2\nCtrl2", "FX2\nCtrl3", "FX3\nCtrl1", "FX3\nCtrl2", "FX3\nCtrl3" ])
        self.mod_matrix.setVerticalHeaderLabels(["ADSR 1", "ADSR 2", "Ramp Env", "LFO"])
        f_port_num = pydaw_ports.WAVV_PFXMATRIX_FIRST_PORT

        for f_i_dst in range(4):
            for f_i_src in range(4):
                for f_i_ctrl in range(3):
                    f_ctrl = pydaw_spinbox_control(None, f_port_num, self.plugin_rel_callback, self.plugin_val_callback, -100, 100, 0, kc_none, self.port_dict)
                    self.program.add_control(f_ctrl)
                    f_x = (f_i_dst * 3) + f_i_ctrl
                    self.mod_matrix.setCellWidget(f_i_src, f_x, f_ctrl.control)
                    f_port_num += 1

        self.main_layout.addWidget(self.mod_matrix)
        self.mod_matrix.resizeColumnsToContents()

        self.hlayout7 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout7)

        self.adsr_amp =  pydaw_adsr_widget(51, True, pydaw_ports.WAYV_ATTACK_PFX1, pydaw_ports.WAYV_DECAY_PFX1, pydaw_ports.WAYV_SUSTAIN_PFX1, \
        pydaw_ports.WAYV_RELEASE_PFX1, "ADSR 1", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        #self.adsr_amp.lms_release.lms_knob.setMinimum(5) #overriding the default for self, because we want a low minimum default that won't click
        self.hlayout7.addWidget(self.adsr_amp.groupbox)

        self.adsr_filter =  pydaw_adsr_widget(51,  False, pydaw_ports.WAYV_ATTACK_PFX2, pydaw_ports.WAYV_DECAY_PFX2, pydaw_ports.WAYV_SUSTAIN_PFX2, \
        pydaw_ports.WAYV_RELEASE_PFX2, "ADSR 2", self.plugin_rel_callback, self.plugin_val_callback, self.port_dict)
        self.hlayout7.addWidget(self.adsr_filter.groupbox)

        self.pitch_env =  pydaw_ramp_env_widget(51, self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, \
        pydaw_ports.WAYV_RAMP_ENV_TIME, pydaw_ports.WAYV_PITCH_ENV_AMT, "Ramp Env")
        self.pitch_env.amt_knob.name_label.setText("Pitch")
        self.hlayout7.addWidget(self.pitch_env.groupbox)

        self.lfo =  pydaw_lfo_widget(51,  self.plugin_rel_callback, self.plugin_val_callback, self.port_dict, pydaw_ports.WAYV_LFO_FREQ, \
        pydaw_ports.WAYV_LFO_TYPE, f_lfo_types, "LFO")
        self.hlayout7.addWidget(self.lfo.groupbox)

        self.lfo_amount =  pydaw_knob_control(51, "Amount", pydaw_ports.WAYV_LFO_AMOUNT, self.plugin_rel_callback, self.plugin_val_callback, \
        0, 100, 0, kc_decimal, self.port_dict)
        self.lfo_amount.add_to_grid_layout(self.lfo.layout, 2)

        self.lfo_amp =  pydaw_knob_control(51, "Amp", pydaw_ports.WAYV_LFO_AMP, self.plugin_rel_callback, self.plugin_val_callback, \
        -24, 24, 0, kc_integer, self.port_dict)
        self.lfo_amp.add_to_grid_layout(self.lfo.layout, 3)

        self.lfo_pitch =  pydaw_knob_control(51, "Pitch", pydaw_ports.WAYV_LFO_PITCH, self.plugin_rel_callback, self.plugin_val_callback, \
        -36, 36, 0,  kc_integer, self.port_dict)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 4)

        #Add the knobs to the preset module
        self.program.add_control(self.adsr_amp_main.attack_knob)
        self.program.add_control(self.adsr_amp_main.decay_knob)
        self.program.add_control(self.adsr_amp_main.sustain_knob)
        self.program.add_control(self.adsr_amp_main.release_knob)
        self.program.add_control(self.adsr_amp.attack_knob)
        self.program.add_control(self.adsr_amp.decay_knob)
        self.program.add_control(self.adsr_amp.sustain_knob)
        self.program.add_control(self.adsr_amp.release_knob)
        self.program.add_control(self.adsr_filter.attack_knob)
        self.program.add_control(self.adsr_filter.decay_knob)
        self.program.add_control(self.adsr_filter.sustain_knob)
        self.program.add_control(self.adsr_filter.release_knob)
        self.program.add_control(self.adsr_amp1.attack_knob)
        self.program.add_control(self.adsr_amp1.decay_knob)
        self.program.add_control(self.adsr_amp1.sustain_knob)
        self.program.add_control(self.adsr_amp1.release_knob)
        self.program.add_control(self.adsr_amp2.attack_knob)
        self.program.add_control(self.adsr_amp2.decay_knob)
        self.program.add_control(self.adsr_amp2.sustain_knob)
        self.program.add_control(self.adsr_amp2.release_knob)
        self.program.add_control(self.lfo.freq_knob)
        self.program.add_control(self.lfo.type_combobox)
        self.program.add_control(self.noise_amp)
        self.program.add_control(self.noise_type)
        self.program.add_control(self.pitch_env.time_knob)
        self.program.add_control(self.osc1.osc_type_combobox)
        self.program.add_control(self.osc1.pitch_knob)
        self.program.add_control(self.osc1.fine_knob)
        self.program.add_control(self.osc1.vol_knob)
        self.program.add_control(self.osc2.osc_type_combobox)
        self.program.add_control(self.osc2.pitch_knob)
        self.program.add_control(self.osc2.fine_knob)
        self.program.add_control(self.osc2.vol_knob)
        self.program.add_control(self.master.vol_knob)
        self.program.add_control(self.master.glide_knob)
        self.program.add_control(self.master.pb_knob)
        self.program.add_control(self.adsr_amp1_checkbox)
        self.program.add_control(self.adsr_amp2_checkbox)
        self.program.add_control(self.fx0.knobs[0])
        self.program.add_control(self.fx0.knobs[1])
        self.program.add_control(self.fx0.knobs[2])
        self.program.add_control(self.fx0.combobox)
        self.program.add_control(self.fx1.knobs[0])
        self.program.add_control(self.fx1.knobs[1])
        self.program.add_control(self.fx1.knobs[2])
        self.program.add_control(self.fx1.combobox)
        self.program.add_control(self.fx2.knobs[0])
        self.program.add_control(self.fx2.knobs[1])
        self.program.add_control(self.fx2.knobs[2])
        self.program.add_control(self.fx2.combobox)
        self.program.add_control(self.fx3.knobs[0])
        self.program.add_control(self.fx3.knobs[1])
        self.program.add_control(self.fx3.knobs[2])
        self.program.add_control(self.fx3.combobox)
        #mod matrix stuff was here, now at the beginning
        self.program.add_control(self.lfo_amp)
        self.program.add_control(self.lfo_pitch)
        self.program.add_control(self.pitch_env.amt_knob)
        self.program.add_control(self.osc1_uni_voices)
        self.program.add_control(self.osc1_uni_spread)
        self.program.add_control(self.osc2_uni_voices)
        self.program.add_control(self.osc2_uni_spread)
        self.program.add_control(self.lfo_amount)
        self.program.add_control(self.osc3.osc_type_combobox)
        self.program.add_control(self.osc3.pitch_knob)
        self.program.add_control(self.osc3.fine_knob)
        self.program.add_control(self.osc3.vol_knob)
        self.program.add_control(self.adsr_amp3.attack_knob)
        self.program.add_control(self.adsr_amp3.decay_knob)
        self.program.add_control(self.adsr_amp3.sustain_knob)
        self.program.add_control(self.adsr_amp3.release_knob)
        self.program.add_control(self.adsr_amp3_checkbox)
        self.program.add_control(self.osc3_uni_voices)
        self.program.add_control(self.osc3_uni_spread)
        self.program.add_control(self.osc1_fm1)
        self.program.add_control(self.osc1_fm2)
        self.program.add_control(self.osc1_fm3)
        self.program.add_control(self.osc2_fm1)
        self.program.add_control(self.osc2_fm2)
        self.program.add_control(self.osc2_fm3)
        self.program.add_control(self.osc3_fm1)
        self.program.add_control(self.osc3_fm2)
        self.program.add_control(self.osc3_fm3)

        self.generate_control_dict()
        self.open_plugin_file()

"""Used for outputting sampler parameters to text files"""
LMS_DELIMITER  =  "|"
"""These define the index of each column in m_sample_table. Re-order these if you add or remove columns"""
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
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_folder, a_track_type, a_track_name, a_stylesheet, a_close_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback, a_track_num, a_project, a_track_type, a_stylesheet, a_close_callback)
        self.folder = str(a_folder)
        self.file = str(self.track_num) + ".pyinst"
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Way-V - " + self.track_name)
        self.is_instrument = True
        #Begin Euphoria C++
        self.handle_control_updates = True
        self.creating_instrument_file = False
        self.suppress_selected_sample_changed = False
        #a_style.LMS_set_value_style(("color : white background-color: rgba(0,0,0,0)"), 64)
         #a_style.LMS_set_label_style(("QLabel{color:blackbackground-color:whiteborder:solid 2px whiteborder-radius:2px text-align : center:"), 64)
        #a_style.LMS_set_value_style("")
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
            "Mode" #Interpolation Mode
        ]

        self.sample_table = QtGui.QTableWidget(pydaw_ports.pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT, len(f_sample_table_columns))
        for f_i in range(pydaw_ports.pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            pass #TODO:  Add the controls

        self.sample_table.setHorizontalHeaderLabels(f_sample_table_columns)
        self.sample_table.verticalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.sample_table.horizontalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.file_selector =  LMS_file_select(self)
        """Set all of the array variables that are per-sample"""
        self.note_indexes = {}
        for i in range(pydaw_ports.pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            #f_rb = (QRadioButton*)m_sample_table.lms_mod_matrix.cellWidget(i , SMP_TB_RADIOBUTTON_INDEX)
            self.note_indexes[i] = 0

        actionMove_files_to_single_directory =  QtGui.QAction(self.widget)
        actionSave_instrument_to_file =  QtGui.QAction(self.widget)
        actionOpen_instrument_from_file =  QtGui.QAction(self.widget)
        actionMapToWhiteKeys =  QtGui.QAction(self.widget)
        actionMapToMonoFX =  QtGui.QAction(self.widget)
        actionClearAllSamples =  QtGui.QAction(self.widget)
        menubar =  QtGui.QMenuBar(self.widget)
        menuFile =  QtGui.QMenu(menubar)
        menubar.addAction(menuFile.menuAction())
        menuFile.addAction(actionMove_files_to_single_directory)
        menuFile.addAction(actionSave_instrument_to_file)
        menuFile.addAction(actionOpen_instrument_from_file)
        menuFile.addAction(actionMapToWhiteKeys)
        menuFile.addAction(actionMapToMonoFX)
        menuFile.addAction(actionClearAllSamples)
        actionMove_files_to_single_directory.setText("Move files to single directory")
        actionSave_instrument_to_file.setText( "Save instrument to file" )
        actionOpen_instrument_from_file.setText( "Open instrument from file" )
        actionMapToWhiteKeys.setText(("Map All Samples to 1 White Key"))
        actionMapToMonoFX.setText(("Map All Samples to Own MonoFX Group"))
        actionClearAllSamples.setText(("Clear All Samples"))
        menuFile.setTitle("Menu")

        self.resize(1200, 680)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.sizePolicy().hasHeightForWidth())
        self.setSizePolicy(sizePolicy)
        self.smp_tab_main_verticalLayout =  QtGui.QVBoxLayout()
        self.main_v_layout =  QtGui.QVBoxLayout(self)
        self.main_v_layout.addWidget(menubar)
        self.main_tab =  QtGui.QTabWidget(self)
        self.sample_tab =  QtGui.QWidget()
        self.sample_tab_horizontalLayout =  QtGui.QHBoxLayout(self.sample_tab)
        self.smp_tab_scrollAreaWidgetContents =  QtGui.QWidget(self.sample_tab)
        self.smp_tab_scrollAreaWidgetContents.setGeometry(QtCore.QRect(0, 0, 966, 728))
        horizontalLayout =  QtGui.QHBoxLayout(self.smp_tab_scrollAreaWidgetContents)
        self.smp_tab_main_verticalLayout.addWidget(self.sample_table.lms_mod_matrix, QtGui.QtCore.Qt.AlignCenter)
        self.smp_tab_main_verticalLayout.addLayout(self.file_selector.lms_layout)
        self.file_browser =  LMS_file_browser(self)
        self.preview_file = ""
        horizontalLayout.addLayout(self.file_browser.m_file_browser_verticalLayout, -1)
        horizontalLayout.addLayout(self.smp_tab_main_verticalLayout)
        f_settings_and_logo_hlayout =  QtGui.QHBoxLayout()
        f_logo_label =  QtGui.QLabel("", self)
        f_logo_label.setTextFormat(QtCore.Qt.RichText)
        f_logo_text = "<html><img src=\"data:image/pngbase64,iVBORw0KGgoAAAANSUhEUgAAAGAAAABgCAYAAADimHc4AAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9wHCBEcGthN72sAAAAZdEVYdENvbW1lbnQAQ3JlYXRlZCB3aXRoIEdJTVBXgQ4XAAAgAElEQVR42u2cd5weZdX3v9fMXbf3kt4rhJJQpSiKNKU3QQED+IgCIiLgI4gIjyAIKIb2gKA0QSBEwFAklCSQkJDesymb7dl67+7dZ67rvH/MfW82kEAC8sr7efd8PvOZ2XtnrrnmnOv8Tp2BARqgARqgARqgARqgARqgARqgARqgARqgARqgARqgARqgL5zU/w8Pucy0gyrlAKVYbVoJ4lOIwsUQVDZ+lAy1Cv9zE9xiulgqbXt1TatE9/jctdK+l/OJ/Fue60Pdzgrp8OagP/n56tPdrHDa/q/z3geQEM0BUrZXF66KtvG+tHO4+vTrQsa3d5N6cHXf8btNGzh60PjP9HDTbG9uc6WRSaocgHm6flC5yjlURKUcjBRbgXAVgRV+Fdr0H4egTaZzfxemuRAXAASVOUUQBGUHlbJLCLxTrnJr9+QG60w7E60yNpnOoRq+4YAroPtGFZWdhfIpFSrAXjJYFSxf3LqFgypGfe4HfMtt4mtWNcpSzNONU8eqgqdz8I0zYHzKUq3phNqoE3cen1N97X8cK9frzkc6dFx6dMJEdFxHTMJETMJEdNxEdFxHTVK2pCPyQu+2S/d0zBodsQBW6/Yrtpuo9JiERExce+Nnt4SOSkoa0j3yRrThjwAiwskXf+9zPc8KswNSPtAtp7WYaKNjUqJ10nUlqUVcebOtqWnsX+#IRD+T/B8J2yIKUkOAvJRSRcEUX16opQSGzu3I5Zq+eGzz2kgD/hUQxBFA9CrRFeAmy8KB9Kqn+4JiB872J50zO8+WNRNZUnFwUq1LQbZ2wd6URo4TXrZYEoZb5VnbEHbT4ar8O3F+ENGTLejUD4sC5TvzXnzIjWX/7wUGAz8X4chq/8faSTtIlojWoOrMY7GOFrEdRENvlRHUzNtd98/PgTD9uQGCWUyY5N2EEcrcTXiaMTVIq5GXI12wE6nYgl34cN/Hcr2znE15eX+vX2YJ6SR09QQFqsSNd5XkWH+9htHq/AfSvCHHHEjBrEAbPxEYnHn3ZdeUvTGchQk/xMasJMAXAQRREBEiYjKWgnlmQDQurc3QHtntQuhPbmBk1nDaREx2RWtlIASFAKIeHcR4zgq2NRSBeSnbWuvXeTvqcGICAepSkF+bS0xrfeMJuc3+djGQXeFrBzbsmyjxQgomru7nM0rV/UADUDXf14AghYlRpQIgskwJyMM8ZilLMG2NXsID66S7F6MiDcektkyx969jChlsG0DaIW1V/Bzuamj07SilAJBLZbLHh5N+KocrKhWdAVV2Ldg7aqeLY3NiZAV8GzElppU6/bWCNC8Sw1I/WHXx1+YABTGiBgjIlqMaDFGY4xGjEY8JmEMfcZhDzRgx14Mok3/MUWMRkR7wjUg0qd3e3iHW3QdANdLkBKrAtw1oQ/M9sfHEZ4eFBXXSqUChOy#/OVnjOvurJVlC0WPgXoRR8uTbN9e2uu7Ws2ZIxVhm7TjYjVz9cIXsVNul5tdps53d36RQlAjFEYAYPKrP7sJhkNEA+n9jSKdrzrcCQzdt+40m/19x/byJ7E6DN0EwA32sMQEYbYlSAP2vNV6b3jVfC7AayoZfkTYRVWD738YvycM09v3vegad0TBg0JGNImjTHrN2xM4EqrLze3s#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#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#faQbTfmr7DId0MYOKOwQlhVC2UGwg2AFwQorZeVgWSGlrJDCCivLCmHZQcEOidghhRVUyg6i7KAYy+= 16/kp/pTb4zavfuuwEd/VWDbwgYlbxkS7lPaLWm7z9SZOKioZUDC0tKkgHEBXAsiNL11WgaQCaMcb5cgsA4OenePun3nume9naV2bd/uDkjqam4qG+YU6BCloBZXwBxA5mNxE7KFhBhRUUrADKDkDmf3jHgu0XsQKWsQutsDXOv3+0tydhPXvsd7/V+= 1oHpcDf8KpaLZ/pKSpu9vbDqksqh1SUVIYK3GLyROuOcG99cx6wDejM1P6/VGTv9NcdF8LNz8IlX4OltbC2cXEqV7F5xfpvOpLMHT5+bG+FqnL8SpSljGUjlgWWBZaNWDbGssGyEGWD8imsgGWpoPJTpApMGUPTNnl67j+eHTb7u9cdm1i4TgHPALOA9f26WD4bXfj1r+wzcdyhQ8tK3VwqUrULV1SsenzmCNMVezWTAkh/2QTwcRP+1NVw/t3wrf3hleXeb/sMOpyCvIvLJo0ZPeVbR3RNOuqQtvLi8pSFiIurBLEE3RevebGDJRaW2PjF4JOWzqbAyhf/VbX2mTdGds1fUUZSr8z40PMzrR768z6M/8lrvl1qOM+qaQqZ+vbCaO32odGlG5vpSdyU8ev1l18A#Nd+OWT3vGJ+8Ps5X32jfKck6guPb1wSNXw4lGDTeW+Y2JlVVWJguLCdDg/z7GV36AQx03ZyZ6ov6ezx9e0eXNex4qa4o4VmwviG+uFtK4HFmYgZzXQCv8maHjj5jBL1g/mwdfHsK1zGpCP316Po+f0b4b6UgsgZTRBa2dk4pSDFf9YJAB+i3JHcTCFOQfisydaAX+pnRvMsQL+oFLKJyJKHBcTS2F6E2LiqThp3QnUKliFz64RbeoQae7XtfK5KDD/TtJL1sBP/gLARLA2KV+BEdfSYLBUDCOOZ/LMl1cARhss27PLruNYSZ9P8nbfC6+Asky+pBwownvZ2e6rYHoMjmS2LiwrkmmQ+I9CwYUi/FV9yb5T4mZyIo7emTciEhSRHBHx74VQVf88ypeBfuG6Rd/v6BiS/ftKkS8fHkki2Z/xh4rIb7pE/rZN5B9NIo9HRe4Uke+ISJ7XvKbIHVzJ+Jsu3jvR/AfoayKnDHnlldnsu8/XANQBB3z5PtXTnO1KE/nBWpGWW5cu3f6t227beuTPr9l03C23bP2v117r+u9ly9xpV1313ztBl+MgIkqLWAkRKymiutK79ii7jKEunc7eh6QxqlHEWua66lc1NZk8hkFEVJ0x1gcialby4wHse1rzjuMoGpsQEdYbo2YZox4RUX/aRYfEOJGTAw89tIGSktsBeOAB5vV6JdqV3d08aIy6WkRNN0adY0xfueGSJu9tzF/W1vIzEc6Kezm3f4rwGxF1Vne34lLvddahd9zxb9ACkanrRbac+Pjjq1HqCeAnwInAcfh857Hf/o9QUfGU8vD/U2lRKslNrsvizAtIn0bPObsPVs9obeMmEd7aQ/iYGo/3F8CJgYceXEJJya8BeOSRPRtn6VJO6u7+1NOqfvUr7z733vv5BBARufSe7u4tHHjgc8AJj4vs5BKNggAwnJycvH5CGywiJ4rIpWmRa0TkIhHp+9bMhGf+1nd9UsyE9t7eqZnrhorI9FaRHzeKHPaRhXBko8gPVoqc0yRS9dF5fmhM2ZJEYlrm3Jw6keNeFDn/SZGvikjwY0lSkRMDDz20NCuAM5Yt63+v0NMiB1wtcvxFIid8v9/ccxcvBuD6FSusX4mM+i9jKgBqRIbcKnLsOSJHZdt0yi+7bK/5/bEvKb0IkRafL11+8ME9bUuXzrlAKZ2FC6UUWzzvZhuZ1bVQpGxWc/OTa1atOrhm85bWaCpJ0YQJwUnHHLP9Fq1vudG2Z64/9zt9489GXbhg27YjH37rraeeXbXq3Jfffntwa34+1aeeGj+yu/v2ufXbnl9QNejWnz#wrcXbFhv/EccoXxHHfVh7quv3xI74bia0hNvlTQAAAoRSURBVHvuoeOnP+VVpfZbWV#i2/e8buXH9i+fczMF144rKahwQ4cepj/18d87V221t7GyBF9DbsGRPpBywsHHADAv0TGXKD19PeeeurQtsWLc1Aq15o61Q1+= 1OHzqXXrZsQOOqgb4O9TpvispqbLOp57rviMJUveuGjevNPXL148JrZ1ayHPPnsD8GzH/Pl7/P70rqAHgP2XLNnn0p6e96/u6Zk/q63tfBEZKyI5u7vuWpH84dOnX01h4aPANcB3gR+NfOwvLx/b2rao7Kabjui7x+rV6gaRn56ZTL579tNPLy0aPfoR4GfAxfz5kb9UJxLP37pmzUNn3nDjy8CdwA845JBfsrHmX/5nnvkz/WDvdJFDq5PJJ8fOnv1e+fHHvwbcDlzC8OG38Ne/ruKVl5+juqpPc4aJnOC7#6lFBfflP1toUjFScb83vejHy/IROU3ABcxZcp9/O53Hfz2tw+qTL6sQMTPkiW/sZ58cnn+ddctoKrqWeBO8vJm4PdfZmXO+1wQlBVCxS23XHHs3Lmbb04mN80UeW+xyGONIrekRU7uf+55m7f1T+rt/PWUQYMrjunoeHnIr371RKauy51vvKF+L/KTk1pb5zFx0kzg+Hs7OnwAk/71xn4HiDw9+L77FgD/HbCtMdmhBsfjlwdnzfqAsrIzs79NFzlknMgzXHrpO8AvSgcN2tGlcdtt5zFr1gZOOeXX2Z+GiJzgmzFjKcXFfb+dJnJa7h/+MAf4CzB5p/lff/2V/PrXrUyefDZAWW1t0Kqru4E771xHQcErCo4ZHgplk5kh8vOtz8LznS7KyVjx1htv/NO/zj#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#1fSKR7oydcb4uMnXZO+/c1fm9C5bQUP9HJk2+mGBgKs3NMVpatmTqzB/0jX/0UZUsWXoh0Wgn8GdAKC1VdHSIgjIpL/8vSksPwraDxGIxGhvbcJx6vFehPgRg9Ohp1NWdieO8lqno/fvpxn4JqikQyoHyIAwvhCEVOeG8j10wZUqfEENQmQvDK4uKivuqVWATDH58xdh2gIMO8o7vuqv/ivCRn78j/3TmWf0A0wpkb/ZVkQOK57w1h1GjHspCaT6UB2AYlZU73l2oKP9YAY2hQ/tl0ar65h+GXD8M9sOwIBQU7TpZp/D7/fw/SYHAx38755xdn2vbnzjUESJTi9+b/y5jxjwA5O5hxXsHle3Fx8p3Vz37lDl+JiP8hVJ6F10hzz6763P1J1ctu5JJK718eSGxWN5uWW19AoPa9+KT+rtLW+svS5OFbe9YDf+GVbEndIqIZY0dexa2/SPIfOghN/fzPUNeHgP0abR8CaMff2LnhX7uuQN8+VJo4QANMH+ABmiABmiABmiABmiABmiABmiABmiABmiABmh39H8A0Z5o4CSEcKoAAAAASUVORK5CYII=\"/></html>"
        f_logo_label.setText(f_logo_text)
        f_logo_label.setMinimumSize(90, 30)
        f_logo_label.setAlignment(QtCore.Qt.AlignCenter)
        f_logo_label.show()
        f_settings_and_logo_hlayout.addWidget(f_logo_label, -1, QtGui.QtCore.Qt.AlignRight)
        self.smp_tab_main_verticalLayout.addLayout(f_settings_and_logo_hlayout, -1)
        self.sample_tab_horizontalLayout.addWidget(self.smp_tab_scrollAreaWidgetContents)
        self.main_tab.addTab(self.sample_tab, ())
        self.poly_fx_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.poly_fx_tab, ())
        self.mono_fx_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.mono_fx_tab, ())
        self.main_v_layout.addWidget(self.main_tab)
        self.main_tab.setTabText(self.main_tab.indexOf(self.sample_tab), "Samples" )
        self.main_tab.setTabText(self.main_tab.indexOf(self.poly_fx_tab), "Poly FX" )
        self.main_tab.setTabText(self.main_tab.indexOf(self.mono_fx_tab), "Mono FX" )
        self.main_tab.setCurrentIndex(0)
        self.sample_table.lms_mod_matrix.resizeColumnsToContents()
        #m_sample_table.lms_mod_matrix.horizontalHeader().setStretchLastSection(True)

        #m_view_sample_tab
        self.view_sample_tab =  QtGui.QWidget()
        #m_view_sample_tab.setStyleSheet(("color : white background-color : black"))
        self.main_tab.addTab(self.view_sample_tab, ())
        self.main_tab.setTabText(self.main_tab.indexOf(self.view_sample_tab), "View" )
        self.view_sample_tab_main_vlayout =  QtGui.QVBoxLayout(self.view_sample_tab)
        self.view_sample_tab_main_vlayout.setContentsMargins(0, 0, 0, 0)
        self.start_end_label =  QtGui.QLabel(self.view_sample_tab)
        self.start_end_label.setText(("Start/End"))
        self.view_sample_tab_main_vlayout.addWidget(self.start_end_label, -1, QtGui.QtCore.Qt.AlignHCenter)
        #Sample Start
        self.sample_start_hlayout =  QtGui.QHBoxLayout()
        self.sample_start_left_hspacer =  QtGui.QSpacerItem(0, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_start_hlayout.addItem(self.sample_start_left_hspacer)
        self.sample_start_hslider =  QtGui.QSlider(self.view_sample_tab)
        self.sample_start_hslider.setMaximum(10000)
        self.sample_start_hslider.setOrientation(QtCore.Qt.Horizontal)
        self.sample_start_hlayout.addWidget(self.sample_start_hslider)
        self.sample_start_right_hspacer =  QtGui.QSpacerItem(20, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_start_hlayout.addItem(self.sample_start_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_start_hlayout)
        #Sample End
        self.sample_end_hlayout =  QtGui.QHBoxLayout()
        self.sample_end_left_hspacer =  QtGui.QSpacerItem(0, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_end_hlayout.addItem(self.sample_end_left_hspacer)
        self.sample_end_hslider =  QtGui.QSlider(self.view_sample_tab)
        self.sample_end_hslider.setObjectName(("wavright"))
        self.sample_end_hslider.setLayoutDirection(QtCore.Qt.RightToLeft)
        self.sample_end_hslider.setMaximum(10000)
        self.sample_end_hslider.setValue(0)
        self.sample_end_hslider.setOrientation(QtCore.Qt.Horizontal)
        self.sample_end_hslider.setInvertedAppearance(False)
        self.sample_end_hslider.setInvertedControls(False)
        self.sample_end_hlayout.addWidget(self.sample_end_hslider)
        self.sample_end_right_hspacer =  QtGui.QSpacerItem(20, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_end_hlayout.addItem(self.sample_end_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_end_hlayout)
        self.loop_start_end_label =  QtGui.QLabel(self.view_sample_tab)
        self.loop_start_end_label.setText(("Loop Start/End"))
        self.view_sample_tab_main_vlayout.addWidget(self.loop_start_end_label, -1, QtGui.QtCore.Qt.AlignHCenter)
        #Sample Loop Start
        self.sample_loop_start_hlayout =  QtGui.QHBoxLayout()
        self.sample_loop_start_left_hspacer =  QtGui.QSpacerItem(0, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_loop_start_hlayout.addItem(self.sample_loop_start_left_hspacer)
        self.sample_loop_start_hslider =  QtGui.QSlider(self.view_sample_tab)
        self.sample_loop_start_hslider.setObjectName(("wavleft"))
        self.sample_loop_start_hslider.setMaximum(10000)
        self.sample_loop_start_hslider.setOrientation(QtCore.Qt.Horizontal)
        self.sample_loop_start_hlayout.addWidget(self.sample_loop_start_hslider)
        self.sample_loop_start_right_hspacer =  QtGui.QSpacerItem(20, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_loop_start_hlayout.addItem(self.sample_loop_start_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_loop_start_hlayout)
        #Sample Loop End
        self.sample_loop_end_hlayout =  QtGui.QHBoxLayout()
        self.sample_loop_end_left_hspacer =  QtGui.QSpacerItem(0, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_loop_end_hlayout.addItem(self.sample_loop_end_left_hspacer)
        self.sample_loop_end_hslider =  QtGui.QSlider(self.view_sample_tab)
        self.sample_loop_end_hslider.setObjectName(("wavright"))
        self.sample_loop_end_hslider.setLayoutDirection(QtCore.Qt.RightToLeft)
        self.sample_loop_end_hslider.setMaximum(10000)
        self.sample_loop_end_hslider.setValue(0)
        self.sample_loop_end_hslider.setOrientation(QtCore.Qt.Horizontal)
        self.sample_loop_end_hslider.setInvertedAppearance(False)
        self.sample_loop_end_hslider.setInvertedControls(False)
        self.sample_loop_end_hlayout.addWidget(self.sample_loop_end_hslider)
        self.sample_loop_end_right_hspacer =  QtGui.QSpacerItem(20, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_loop_end_hlayout.addItem(self.sample_loop_end_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_loop_end_hlayout)
        #Sample Graph
        self.sample_graph_hlayout =  QtGui.QHBoxLayout()
        self.sample_graph_left_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_graph_hlayout.addItem(self.sample_graph_left_hspacer)
        self.sample_graph =  LMS_sample_graph(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT, 300, LMS_SAMPLE_GRAPH_WIDTH, self.view_sample_tab)
        self.sample_graph_hlayout.addWidget(self.sample_graph.m_sample_graph)
        self.sample_graph_right_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_graph_hlayout.addItem(self.sample_graph_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_graph_hlayout)
        #The combobox for selecting the sample on the 'view' tab
        self.sample_view_select_sample_hlayout =  QtGui.QHBoxLayout()
        self.sample_view_extra_controls_left_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_select_sample_hlayout.addItem(self.sample_view_extra_controls_left_hspacer)
        self.sample_view_extra_controls_gridview =  QtGui.QGridLayout()
        self.selected_sample_index_combobox =  QtGui.QComboBox(self.view_sample_tab)
        sizePolicy1 = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.selected_sample_index_combobox.sizePolicy().hasHeightForWidth())
        self.selected_sample_index_combobox.setSizePolicy(sizePolicy1)
        self.selected_sample_index_combobox.setMinimumWidth(320)
        for f_i in range(pydaw_ports.pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.selected_sample_index_combobox.addItem("")
        self.sample_view_extra_controls_gridview.addWidget(self.selected_sample_index_combobox, 1, 0, 1, 1)
        self.selected_sample_index_label =  QtGui.QLabel(self.view_sample_tab)
        self.sample_view_extra_controls_gridview.addWidget(self.selected_sample_index_label, 0, 0, 1, 1)
        self.sample_view_select_sample_hlayout.addLayout(self.sample_view_extra_controls_gridview)
        self.sample_view_extra_controls_right_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_select_sample_hlayout.addItem(self.sample_view_extra_controls_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_view_select_sample_hlayout)
        #The loop mode combobox
        self.loop_mode_combobox =  QtGui.QComboBox(self.view_sample_tab)
        self.loop_mode_combobox.addItems(["Off", "On"])
        self.loop_mode_label =  QtGui.QLabel(self.view_sample_tab)
        self.loop_mode_label.setText(("Loop Mode"))
        self.sample_view_extra_controls_gridview.addWidget(self.loop_mode_label, 0, 1, 1, 1)
        self.sample_view_extra_controls_gridview.addWidget(self.loop_mode_combobox, 1, 1, 1, 1)
        #connect(self.loop_mode_combobox, SIGNAL(currentIndexChanged(int)), self, SLOT(loopModeChanged(int)))
        #The file select on the 'view' tab
        self.sample_view_file_select_hlayout =  QtGui.QHBoxLayout()
        self.sample_view_file_select_left_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_file_select_hlayout.addItem(self.sample_view_file_select_left_hspacer)
        self.view_file_selector =  LMS_file_select(self.view_sample_tab)
        self.view_file_selector.lms_file_path.setMinimumWidth(400)
        self.sample_view_file_select_hlayout.addLayout(self.view_file_selector.lms_layout)
        self.sample_view_file_select_right_hspacer =  QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.sample_view_file_select_hlayout.addItem(self.sample_view_file_select_right_hspacer)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_view_file_select_hlayout)
        self.view_sample_tab_lower_vspacer =  QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.view_sample_tab_main_vlayout.addItem(self.view_sample_tab_lower_vspacer)
        self.selected_sample_index_label.setText( "Selected Sample" )
        for f_i in range(pydaw_ports.pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.sample_starts[f_i] = 0
            self.sample_ends[f_i] = 0
            self.sample_loop_starts[f_i] = 0
            self.sample_loop_ends[f_i] = 0
            self.sample_loop_modes[f_i] = 0
            self.sample_selected_monofx_groups[f_i] = 0
        #end self.view_sample_tab
        #From Ray-V PolyFX
        f_lfo_types = ["Off" , "Sine" , "Triangle"]
        """This string is generated by running presets_to_qstring.pl script in the packaging folder on the .tsv file associated with self plugin
        in ~/pydaw . If modifying self plugin by changing the number of parameters to be saved by presets, you should comment self out
        and uncomment the section above it."""
        #LMS_style_info * f_info =  LMS_style_info(64)
         #f_info.LMS_set_label_style("background-color: white border: 1px solid black border-radius: 6px QtGui.QComboBox{color:whitebackground-color:black:", 60)
        self.main_layout =  LMS_main_layout(self.poly_fx_tab)
        #From Modulex
        self.fx0 =  pydaw_modulex_single(self, ("FX0"), pydaw_ports.EUPHORIA_FX0_KNOB0, pydaw_ports.EUPHORIA_FX0_KNOB1, pydaw_ports.EUPHORIA_FX0_KNOB2, pydaw_ports.EUPHORIA_FX0_COMBOBOX)
        self.main_layout.lms_add_widget(self.fx0.lms_groupbox.lms_groupbox)
        self.fx1 =  pydaw_modulex_single(self, ("FX1"), pydaw_ports.EUPHORIA_FX1_KNOB0, pydaw_ports.EUPHORIA_FX1_KNOB1, pydaw_ports.EUPHORIA_FX1_KNOB2, pydaw_ports.EUPHORIA_FX1_COMBOBOX)
        self.main_layout.lms_add_widget(self.fx1.lms_groupbox.lms_groupbox)
        self.main_layout.lms_add_layout()
        self.fx2 =  pydaw_modulex_single(self, ("FX2"), pydaw_ports.EUPHORIA_FX2_KNOB0, pydaw_ports.EUPHORIA_FX2_KNOB1, pydaw_ports.EUPHORIA_FX2_KNOB2, pydaw_ports.EUPHORIA_FX2_COMBOBOX)
        self.main_layout.lms_add_widget(self.fx2.lms_groupbox.lms_groupbox)
        self.fx3 =  pydaw_modulex_single(self, ("FX3"), pydaw_ports.EUPHORIA_FX3_KNOB0, pydaw_ports.EUPHORIA_FX3_KNOB1, pydaw_ports.EUPHORIA_FX3_KNOB2, pydaw_ports.EUPHORIA_FX3_COMBOBOX)

        self.main_layout.lms_add_widget(self.fx3.lms_groupbox.lms_groupbox)
        self.main_layout.lms_add_layout()
        #New mod matrix
        f_mod_matrix_columns = [
            "FX0\nCtrl1", "FX0\nCtrl2", "FX0\nCtrl3", "FX1\nCtrl1", "FX1\nCtrl2", "FX1\nCtrl3",
            "FX2\nCtrl1", "FX2\nCtrl2", "FX2\nCtrl3","FX3\nCtrl1", "FX3\nCtrl2", "FX3\nCtrl3"
        ]
        self.polyfx_mod_matrix[0] =  LMS_mod_matrix(self, pydaw_ports.EUPHORIA_MODULATOR_COUNT, f_mod_matrix_columns, pydaw_ports.EUPHORIA_PFXMATRIX_FIRST_PORT, a_style)
        self.polyfx_mod_matrix[0].lms_mod_matrix.setVerticalHeaderLabels(List() , ("ADSR Amp") , ("ADSR 2") , ("Ramp Env") , ("LFO"))
        self.main_layout.lms_add_widget(self.polyfx_mod_matrix[0].lms_mod_matrix)
        self.polyfx_mod_matrix[0].lms_mod_matrix.resizeColumnsToContents()
        self.main_layout.lms_add_layout()
        #End from Modulex
        self.adsr_amp =  LMS_adsr_widget(self, True, pydaw_ports.EUPHORIA_ATTACK, pydaw_ports.EUPHORIA_DECAY, pydaw_ports.EUPHORIA_SUSTAIN, pydaw_ports.EUPHORIA_RELEASE, ("ADSR Amp"))
        self.adsr_amp.lms_release.lms_knob.setMinimum(5) #overriding the default for self, because we want a low minimum default that won't click
        self.main_layout.lms_add_widget(self.adsr_amp.lms_groupbox_adsr.lms_groupbox)
        self.groupbox_noise =  LMS_group_box(self, ("Noise"), a_style)
        self.main_layout.lms_add_widget(self.groupbox_noise.lms_groupbox)
        self.noise_amp =  pydaw_knob_control(("Vol"), -60, 0, 1, 30, (""), self.groupbox_noise.lms_groupbox, kc_integer, pydaw_ports.EUPHORIA_NOISE_AMP)
        self.groupbox_noise.lms_add_h(self.noise_amp)
        #connect(self.noise_amp.lms_knob, SIGNAL(valueChanged(int)), self, SLOT(noiseAmpChanged(int)))
        self.noise_type =  LMS_combobox(("Type"), self, List() , ("Off") , ("White") , ("Pink"), pydaw_ports.EUPHORIA_NOISE_TYPE, a_style)
        self.noise_type.lms_combobox.setMinimumWidth(87)
        self.groupbox_noise.lms_add_h(self.noise_type)
        #connect(self.noise_type.lms_combobox, SIGNAL(currentIndexChanged(int)), self, SLOT(noise_typeChanged(int)))
        self.adsr_filter =  LMS_adsr_widget(self, False, pydaw_ports.EUPHORIA_FILTER_ATTACK, pydaw_ports.EUPHORIA_FILTER_DECAY, pydaw_ports.EUPHORIA_FILTER_SUSTAIN, pydaw_ports.EUPHORIA_FILTER_RELEASE, ("ADSR 2"))
        self.main_layout.lms_add_widget(self.adsr_filter.lms_groupbox_adsr.lms_groupbox)
        self.pitch_env =  LMS_ramp_env(self, pydaw_ports.EUPHORIA_PITCH_ENV_TIME, -1, -1, False, ("Ramp Env"), False)
        self.main_layout.lms_add_widget(self.pitch_env.lms_groupbox.lms_groupbox)
        #connect(self.pitch_env.lms_time_knob.lms_knob, SIGNAL(valueChanged(int)), self, SLOT(pitchEnvTimeChanged(int)))
        self.lfo =  LMS_lfo_widget(self, pydaw_ports.EUPHORIA_LFO_FREQ, pydaw_ports.EUPHORIA_LFO_TYPE, f_lfo_types, ("LFO"))
        self.main_layout.lms_add_widget(self.lfo.lms_groupbox.lms_groupbox)
        #Overriding the default so we can have a faster LFO
        self.lfo.lms_freq_knob.lms_knob.setMaximum(1600)

        self.lfo_pitch =  pydaw_knob_control(("Pitch"), -36, 36, 1, 0, ("0"), self.lfo.lms_groupbox.lms_groupbox,kc_integer, pydaw_ports.EUPHORIA_LFO_PITCH)
        self.lfo.lms_groupbox.lms_add_h(self.lfo_pitch)

        #MonoFX Tab
        self.mono_fx_tab_main_layout =  LMS_main_layout(self.mono_fx_tab)
        self.selected_container =  QtGui.QWidget()
        self.mono_fx_tab_selected_hlayout =  QtGui.QHBoxLayout(self.selected_container)
        self.mono_fx_tab_selected_sample =  QtGui.QComboBox(self.mono_fx_tab)
        self.mono_fx_tab_selected_group =  QtGui.QComboBox(self.mono_fx_tab)
        self.mono_fx_tab_selected_sample_label =  QtGui.QLabel()
        self.mono_fx_tab_selected_sample_label.setText(("Selected Sample:"))
        self.mono_fx_tab_selected_group_label =  QtGui.QLabel()
        self.mono_fx_tab_selected_group_label.setText(("FX Group:"))
        for f_i in range(1, pydaw_ports.EUPHORIA_MONO_FX_GROUPS_COUNT):
            self.mono_fx_tab_selected_group.addItem(str(f_i))
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.mono_fx_tab_selected_sample.addItem("")

        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_sample_label)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_sample)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_group_label)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_group)
        self.mono_fx_tab_main_layout.lms_add_widget(self.selected_container)
        self.mono_fx_tab_main_layout.lms_add_layout()
        self.mono_fx0 =  pydaw_modulex_single(self, ("FX0"), pydaw_ports.EUPHORIA_FX0_KNOB0, pydaw_ports.EUPHORIA_FX0_KNOB1, pydaw_ports.EUPHORIA_FX0_KNOB2, pydaw_ports.EUPHORIA_FX0_COMBOBOX)
        self.mono_fx_tab_main_layout.lms_add_widget(self.mono_fx0.lms_groupbox.lms_groupbox)
        self.mono_fx1 =  pydaw_modulex_single(self, ("FX1"), pydaw_ports.EUPHORIA_FX1_KNOB0, pydaw_ports.EUPHORIA_FX1_KNOB1, pydaw_ports.EUPHORIA_FX1_KNOB2, pydaw_ports.EUPHORIA_FX1_COMBOBOX)
        self.mono_fx_tab_main_layout.lms_add_widget(self.mono_fx1.lms_groupbox.lms_groupbox)
        self.mono_fx_tab_main_layout.lms_add_layout()
        self.mono_fx2 =  pydaw_modulex_single(self, ("FX2"), pydaw_ports.EUPHORIA_FX2_KNOB0, pydaw_ports.EUPHORIA_FX2_KNOB1, pydaw_ports.EUPHORIA_FX2_KNOB2, pydaw_ports.EUPHORIA_FX2_COMBOBOX)
        self.mono_fx_tab_main_layout.lms_add_widget(self.mono_fx2.lms_groupbox.lms_groupbox)
        self.mono_fx3 =  pydaw_modulex_single(self, ("FX3"), pydaw_ports.EUPHORIA_FX3_KNOB0, pydaw_ports.EUPHORIA_FX3_KNOB1, pydaw_ports.EUPHORIA_FX3_KNOB2, pydaw_ports.EUPHORIA_FX3_COMBOBOX)
        self.mono_fx_tab_main_layout.lms_add_widget(self.mono_fx3.lms_groupbox.lms_groupbox)
        self.mono_fx_tab_main_layout.lms_add_layout()
        self.master =  LMS_master_widget(self, pydaw_ports.EUPHORIA_MASTER_VOLUME, -1,
        -1, pydaw_ports.EUPHORIA_MASTER_GLIDE, pydaw_ports.EUPHORIA_MASTER_PITCHBEND_AMT, ("Master"), False)
        self.mono_fx_tab_main_layout.lms_add_widget(self.master.lms_groupbox.lms_groupbox)
        self.master.lms_master_volume.lms_knob.setMinimum(-24)
        self.master.lms_master_volume.lms_knob.setMaximum(24)
        self.mono_fx_tab_main_layout.lms_add_layout()


    def clearAllSamples():
        for i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            set_selected_sample_combobox_item(i, (""))
            self.sample_graph.clearPixmap(i)
            f_item =  QtGui.QTableWidgetItem()
            f_item.setText((""))
            f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
            self.sample_table.lms_mod_matrix.setItem(i, SMP_TB_FILE_PATH_INDEX, f_item)
        generate_files_string()
        self.view_file_selector.lms_set_file((""))
        self.file_selector.lms_set_file((""))

    def mapAllSamplesToOneMonoFXgroup():
        f_orig_index = self.mono_fx_tab_selected_sample.currentIndex()
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.mono_fx_tab_selected_sample.setCurrentIndex(f_i)
            self.mono_fx_tab_selected_group.setCurrentIndex(f_i)
        self.mono_fx_tab_selected_sample.setCurrentIndex(f_orig_index)

    def mapAllSamplesToOneWhiteKey():
        f_current_note = 36
        i_white_notes = 0
        f_white_notes = [2,2,1,2,2,2,1]
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            #((LMS_note_selector*)(self.sample_table.lms_mm_columns[SMP_TB_NOTE_INDEX].controls[f_i])).lms_set_value(f_current_note)
            #((LMS_note_selector*)(self.sample_table.lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX].controls[f_i])).lms_set_value(f_current_note)
            #((LMS_note_selector*)(self.sample_table.lms_mm_columns[SMP_TB_LOW_NOTE_INDEX].controls[f_i])).lms_set_value(f_current_note)
            f_current_note += f_white_notes[i_white_notes]
            i_white_notes+= 1
            if i_white_notes >= 7:
                i_white_notes = 0
    def viewSampleSelectedIndexChanged(a_index):
        if(self.suppress_selected_sample_changed):
            return
        #f_radio_button = (QRadioButton*)m_sample_table.lms_mod_matrix.cellWidget(a_index , SMP_TB_RADIOBUTTON_INDEX)
        f_radio_button.click()

    def loopModeChanged(a_value):
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        self.sample_loop_modes[(self.sample_table.lms_selected_column)] = a_value

    def sampleStartChanged(a_value):
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        f_diff = 10000 - self.sample_ends[(self.sample_table.lms_selected_column)]
        if((f_diff - a_value) < 100):
            self.sample_ends[(self.sample_table.lms_selected_column)] = 9900 - a_value
            if((self.sample_ends[(self.sample_table.lms_selected_column)]) < 0):
                self.sample_ends[(self.sample_table.lms_selected_column)] = 0
                self.sample_starts[(self.sample_table.lms_selected_column)] = 9900
                self.sample_start_hslider.setValue((self.sample_starts[(self.sample_table.lms_selected_column)]))
            else:
                self.sample_starts[(self.sample_table.lms_selected_column)] = a_value
            self.sample_end_hslider.setValue((self.sample_ends[(self.sample_table.lms_selected_column)]))
        else:
            self.sample_starts[(self.sample_table.lms_selected_column)] = a_value

    def sampleEndChanged(a_value):
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        f_diff = 10000 - self.sample_starts[(self.sample_table.lms_selected_column)]
        if((f_diff - a_value) < 100):
            self.sample_starts[(self.sample_table.lms_selected_column)] = 9900 - a_value
            if((self.sample_starts[(self.sample_table.lms_selected_column)]) < 0):
                self.sample_starts[(self.sample_table.lms_selected_column)] = 0
                self.sample_ends[(self.sample_table.lms_selected_column)] = 9900
                self.sample_end_hslider.setValue((self.sample_ends[(self.sample_table.lms_selected_column)]))
            else:
                self.sample_ends[(self.sample_table.lms_selected_column)] = a_value
            self.sample_start_hslider.setValue((self.sample_starts[(self.sample_table.lms_selected_column)]))
        else:
            self.sample_ends[(self.sample_table.lms_selected_column)] = a_value

    def sampleLoopStartChanged(a_value):
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        f_diff = 10000 - self.sample_loop_ends[(self.sample_table.lms_selected_column)]
        if((f_diff - a_value) < 100):
            self.sample_loop_ends[(self.sample_table.lms_selected_column)] = 9900 - a_value
            if((self.sample_loop_ends[(self.sample_table.lms_selected_column)]) < 0):
                self.sample_loop_ends[(self.sample_table.lms_selected_column)] = 0
                self.sample_loop_starts[(self.sample_table.lms_selected_column)] = 9900
                self.sample_loop_start_hslider.setValue((self.sample_loop_starts[(self.sample_table.lms_selected_column)]))
            else:
                self.sample_loop_starts[(self.sample_table.lms_selected_column)] = a_value
            self.sample_loop_end_hslider.setValue((self.sample_loop_ends[(self.sample_table.lms_selected_column)]))
        else:
            self.sample_loop_starts[(self.sample_table.lms_selected_column)] = a_value

    def sampleLoopEndChanged(a_value):
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        f_diff = 10000 - self.sample_loop_starts[(self.sample_table.lms_selected_column)]
        if((f_diff - a_value) < 100):
            self.sample_loop_starts[(self.sample_table.lms_selected_column)] = 9900 - a_value
            if((self.sample_loop_starts[(self.sample_table.lms_selected_column)]) < 0):
                self.sample_loop_starts[(self.sample_table.lms_selected_column)] = 0
                self.sample_loop_ends[(self.sample_table.lms_selected_column)] = 9900
                self.sample_loop_end_hslider.setValue((self.sample_loop_ends[(self.sample_table.lms_selected_column)]))
            else:
                self.sample_loop_ends[(self.sample_table.lms_selected_column)] = a_value
            self.sample_loop_start_hslider.setValue((self.sample_loop_starts[(self.sample_table.lms_selected_column)]))
        else:
            self.sample_loop_ends[(self.sample_table.lms_selected_column)] = a_value

    def setSampleFile( files):
        self.suppressHostUpdate = True
        pydaw_ports.EUPHORIA_cerr , "Calling setSampleFile with string:\n" , files , "\n"
        self.files = files
        files.replace((pydaw_ports.EUPHORIA_FILES_STRING_RELOAD_DELIMITER), (pydaw_ports.EUPHORIA_FILES_STRING_DELIMITER))
        f_file_list = files.split((pydaw_ports.EUPHORIA_FILES_STRING_DELIMITER))
        for f_i in range(len(f_file_list)):
            if f_i >= pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT:
                break
            f_item =  QtGui.QTableWidgetItem()
            f_item.setText(f_file_list[f_i])
            f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
            self.sample_table.lms_mod_matrix.setItem(f_i, SMP_TB_FILE_PATH_INDEX, f_item)
            if f_file_list[f_i] == "":
                continue
            f_path_sections = f_file_list[f_i].split("/")
            set_selected_sample_combobox_item(f_i, f_path_sections[-1])
            self.sample_graph.generatePreview(f_file_list[f_i], f_i)
        self.sample_table.lms_mod_matrix.resizeColumnsToContents()
        selectionChanged()
        self.suppressHostUpdate = False

    def setSelection(a_value):
        self.suppressHostUpdate = True
        #m_selected_sample[a_value].setChecked(True)
        self.suppressHostUpdate = False
    """ def set_selected_sample_combobox_item(
    * int a_index, #Currently, you should only set self to (self.sample_table.lms_selected_column), but I'm leaving it there for when it can be implemented to work otherwise
    *  a_text) #The text of the  item
    """
    def set_selected_sample_combobox_item(a_index,  a_text):
        self.suppress_selected_sample_changed = True
        self.selected_sample_index_combobox.removeItem(a_index)
        self.selected_sample_index_combobox.insertItem(a_index, a_text)
        self.selected_sample_index_combobox.setCurrentIndex(a_index)
        self.mono_fx_tab_selected_sample.removeItem(a_index)
        self.mono_fx_tab_selected_sample.insertItem(a_index, a_text)
        self.mono_fx_tab_selected_sample.setCurrentIndex(a_index)
        self.suppress_selected_sample_changed = False

    def fileSelect():
        paths = self.file_selector.open_button_pressed_multiple(self)
        self.view_file_selector.lms_set_file(self.file_selector.lms_get_file())
        load_files(paths)

    def load_files(paths):
        if len(paths) > 0:
            self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
            f_sample_index_to_load = (self.sample_table.lms_selected_column)
            for i in range(len(paths)):
                path = paths[i]
                if path != "":
                    if( not os.path.isfile(path)):
                        QtGui.QMessageBox.warning(self, ("Error"), ("File cannot be read."))
                        continue
                    f_path_sections = path.split(("/"))
                    set_selected_sample_combobox_item(f_sample_index_to_load, f_path_sections.at((f_path_sections.count() - 1)))
                    self.sample_graph.generatePreview(path, f_sample_index_to_load)
                    f_item =  QtGui.QTableWidgetItem()
                    f_item.setText(path)
                    f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
                    self.sample_table.lms_mod_matrix.setItem(f_sample_index_to_load, SMP_TB_FILE_PATH_INDEX, f_item)
                    f_sample_index_to_load+= 1
                    if(f_sample_index_to_load >= pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
                        break

            generate_files_string()
            #ifndef LMS_DEBUG_STANDALONE
            lo_send(self.host, self.configurePath, "ss", "load", files_string.toLocal8Bit().data())
            lo_send(self.host, self.configurePath, "ss", "lastdir", self.file_selector.lms_last_directory.toLocal8Bit().data())
            #endif
            self.sample_table.lms_mod_matrix.resizeColumnsToContents()
            selectionChanged()

    def generate_files_string():
        generate_files_string(-1)

    def generate_files_string(a_index):
        files_string = ("")
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            files_string.append(self.sample_table.lms_mod_matrix.item(f_i, SMP_TB_FILE_PATH_INDEX).text())
            if((a_index != -1) and (f_i == a_index)):
                files_string.append(pydaw_ports.EUPHORIA_FILES_STRING_RELOAD_DELIMITER)
            else:
                files_string.append(pydaw_ports.EUPHORIA_FILES_STRING_DELIMITER)
        files_string.append(preview_file + pydaw_ports.EUPHORIA_FILES_STRING_DELIMITER)

    def clearFile():
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        self.sample_graph.clearPixmap((self.sample_table.lms_selected_column))
        self.sample_start_hslider.setValue(0)
        self.sample_end_hslider.setValue(0)
        set_selected_sample_combobox_item((self.sample_table.lms_selected_column), (""))
        f_item =  QtGui.QTableWidgetItem()
        f_item.setText((""))
        f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
        self.sample_table.lms_mod_matrix.setItem((self.sample_table.lms_selected_column), SMP_TB_FILE_PATH_INDEX, f_item)
        self.file_selector.clear_button_pressed()
        self.view_file_selector.clear_button_pressed()
        generate_files_string()

    def openInEditor():
        self.file_selector.open_in_editor_button_pressed(self)

    def reloadSample():
        path = self.file_selector.lms_file_path.text()
        if path != "":
            self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
            generate_files_string((self.sample_table.lms_selected_column))

    def selectionChanged(self):
        if(self.suppress_selected_sample_changed):
            return
        self.suppress_selected_sample_changed = True
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        self.selected_sample_index_combobox.setCurrentIndex((self.sample_table.lms_selected_column))
        self.mono_fx_tab_selected_sample.setCurrentIndex((self.sample_table.lms_selected_column))
        self.suppress_selected_sample_changed = False
        setSelectedMonoFX(self.sample_selected_monofx_groups[((self.sample_table.lms_selected_column))])
        self.selected_sample_index_combobox.setCurrentIndex((self.sample_table.lms_selected_column))
        self.mono_fx_tab_selected_sample.setCurrentIndex((self.sample_table.lms_selected_column))
        self.sample_graph.indexChanged((self.sample_table.lms_selected_column))

        self.file_selector.lms_set_file(self.sample_table.lms_mod_matrix.item(self.sample_table.lms_selected_column, SMP_TB_FILE_PATH_INDEX).text())
        self.view_file_selector.lms_set_file(self.sample_table.lms_mod_matrix.item(self.sample_table.lms_selected_column, SMP_TB_FILE_PATH_INDEX).text())
        self.suppressHostUpdate = True
        self.sample_start_hslider.setValue(self.sample_starts[(self.sample_table.lms_selected_column)])
        self.sample_end_hslider.setValue(self.sample_ends[(self.sample_table.lms_selected_column)])
        self.sample_loop_start_hslider.setValue(self.sample_loop_starts[(self.sample_table.lms_selected_column)])
        self.sample_loop_end_hslider.setValue(self.sample_loop_ends[(self.sample_table.lms_selected_column)])
        self.loop_mode_combobox.setCurrentIndex(self.sample_loop_modes[(self.sample_table.lms_selected_column)])
        self.suppressHostUpdate = False

    def file_browser_load_button_pressed():
        f_result = self.file_browser.files_opened()
        for f_i in range(len(f_result)):
            f_temp = (self.file_browser.m_folder_path_lineedit.text() + "/" + f_result.at(f_i))
            f_result.removeAt(f_i)
            f_result.insert(f_i, f_temp)
        load_files(f_result)

    def file_browser_up_button_pressed():
        self.file_browser.up_one_folder()

    def file_browser_preview_button_pressed():
        f_list = self.file_browser.m_files_listWidget.selectedItems()
        if len(f_list) > 0:
            preview_file = self.file_browser.m_folder_path_lineedit.text() + ("/") + f_list[0].text()
            generate_files_string()
            #ifndef LMS_DEBUG_STANDALONE
            #lo_send(self.host, self.configurePath, "ss", "load", files_string.toLocal8Bit().data())
            #lo_send(self.host, self.configurePath, "ss", "lastdir", self.file_browser.m_folder_path_lineedit.text().toLocal8Bit().data())
            #endif
            preview_file = ("")

    def file_browser_folder_clicked(a_item):
        self.file_browser.folder_opened(a_item.text(), True)

    def file_browser_bookmark_clicked(a_item):
        self.file_browser.bookmark_clicked(a_item.text())

    def file_browser_bookmark_button_pressed(self):
        self.file_browser.bookmark_button_pressed()

    def file_browser_bookmark_delete_button_pressed(self):
        self.file_browser.bookmark_delete_button_pressed()

    def moveSamplesToSingleDirectory():
        f_selected_path = ("")
        if(self.creating_instrument_file):
            f_selected_path = self.inst_file_tmp_path
        else:
            f_selected_path = QtGui.QFileDialog.getExistingDirectory(self, "Select a directory to move the samples to...", ".")
        if not f_selected_path.isEmpty():
            #TODO: check that the directory is empty...
            self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
            f_current_radio_button = self.sample_table.lms_selected_column
            for i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
                f_current_file_path = self.sample_table.lms_mod_matrix.item(i, SMP_TB_FILE_PATH_INDEX).text()
                if(f_current_file_path.isNull()):
                    continue
                if((f_current_file_path.isEmpty())):
                    continue
                if(f_current_file_path.startsWith(f_selected_path, QtGui.QtCore.Qt.CaseInsensitive)):
                    continue
                f_current_file =  QtGui.QFile(f_current_file_path)
                f_file_arr = f_current_file.fileName().split("/")
                f__file = f_selected_path + ("/") + f_file_arr[(f_file_arr.count() - 1)]
                #ifdef LMS_DEBUG_STANDALONE
                f_string = f__file.toStdString()
                #endif
                f_current_file.copy(f__file)
                f_item =  QtGui.QTableWidgetItem()
                f_item.setText(f__file)
                f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
                self.sample_table.lms_mod_matrix.setItem(i, SMP_TB_FILE_PATH_INDEX, f_item)
                #f_radio_button = (QRadioButton*)m_sample_table.lms_mod_matrix.cellWidget(i , SMP_TB_RADIOBUTTON_INDEX)
                f_radio_button.setChecked(True)
                generate_files_string()
                #lo_send(self.host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data())
            """Select the radio button that was originally selected"""
            #f_radio_button = (QRadioButton*)m_sample_table.lms_mod_matrix.cellWidget(f_current_radio_button , SMP_TB_RADIOBUTTON_INDEX)
            f_radio_button.setChecked(True)

    def sample_selected_monofx_groupChanged(a_value):
        self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
        self.sample_selected_monofx_groups[(self.sample_table.lms_selected_column)] = a_value
        setmonoFX0knob0(self.mono_fx_values[a_value][0][0])
        setmonoFX0knob1(self.mono_fx_values[a_value][0][1])
        setmonoFX0knob2(self.mono_fx_values[a_value][0][2])
        setmonoFX0combobox(self.mono_fx_values[a_value][0][3])
        setmonoFX1knob0(self.mono_fx_values[a_value][1][0])
        setmonoFX1knob1(self.mono_fx_values[a_value][1][1])
        setmonoFX1knob2(self.mono_fx_values[a_value][1][2])
        setmonoFX1combobox(self.mono_fx_values[a_value][1][3])
        setmonoFX2knob0(self.mono_fx_values[a_value][2][0])
        setmonoFX2knob1(self.mono_fx_values[a_value][2][1])
        setmonoFX2knob2(self.mono_fx_values[a_value][2][2])
        setmonoFX2combobox(self.mono_fx_values[a_value][2][3])
        setmonoFX3knob0(self.mono_fx_values[a_value][3][0])
        setmonoFX3knob1(self.mono_fx_values[a_value][3][1])
        setmonoFX3knob2(self.mono_fx_values[a_value][3][2])
        setmonoFX3combobox(self.mono_fx_values[a_value][3][3])
        if not self.suppressHostUpdate:
            self.sample_table.find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX)
            if(a_value >= 0):
                #ifndef LMS_DEBUG_STANDALONE
                lo_send(self.host, self.controlPath, "if", (pydaw_ports.EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN + (self.sample_table.lms_selected_column)), float(a_value))
                #endif

    def setSelectedMonoFX(val):
        self.suppressHostUpdate = True
        self.mono_fx_tab_selected_group.setCurrentIndex(int(val))
        self.suppressHostUpdate = False
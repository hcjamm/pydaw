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

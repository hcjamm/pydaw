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
    def __init__(self, a_path):
        f_text = pydaw_util.pydaw_read_file_text(a_path)
        f_line_arr = f_text.split("\n")
        self.port_dict = {}
        self.euphoria_samples = []
        for f_line in f_line_arr:
            f_items = f_line.split("|", 1)
            if f_items[0] == "load":
                for f_sample_path in f_items[1].split("~"):
                    self.euphoria_samples.append(f_sample_path)
            else:
                self.port_dict[int(f_items[0])] = float(f_items[1])

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
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion)
        self.control = QtGui.QSlider()
        self.control.setRange(a_min_val, a_max_val)
        self.control.setValue(a_default_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)


class pydaw_spinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion)
        self.control = QtGui.QSpinBox()
        self.control.setRange(a_min_val, a_max_val)
        self.control.setValue(a_default_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)


class pydaw_doublespinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val, \
    a_default_val, a_val_conversion=kc_none, a_port_dict=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_val_conversion)
        self.control = QtGui.QDoubleSpinBox()
        self.control.setRange(a_min_val, a_max_val)
        self.control.setValue(a_default_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)


class pydaw_combobox_control:
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback, a_items_list=[], a_port_dict=None):
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
        if a_port_dict is not None:
            a_port_dict[self.port_num] = self

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
        self.program_combobox = pydaw_combobox_control(150, "", a_rel_callback, a_val_callback, a_port_num=a_port_num)
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
            self.layout.addWidget(f_knob.name_label, 0, f_i)
            self.layout.addWidget(f_knob.control, 1, f_i)
            self.layout.addWidget(f_knob.value_label, 2, f_i)
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


class modulex_plugin_ui:
    def __init__(self, a_rel_callback, a_val_callback):
        self.effects = []
        self.widget = QtGui.QWidget()
        self.layout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.layout)
        self.tab_widget = QtGui.QTabWidget()
        self.layout.addWidget(self.tab_widget)

        self.fx_tab = QtGui.QWidget()
        self.tab_widget.addTab(self.fx_tab, "Effects")
        self.fx_layout = QtGui.QGridLayout()
        self.fx_tab.setLayout(self.fx_layout)

        self.delay_tab = QtGui.QWidget()
        self.tab_widget.addTab(self.delay_tab, "Delay")
        self.delay_layout = QtGui.QVBoxLayout()
        self.delay_tab.setLayout(self.delay_layout)

        self.delay_groupbox = QtGui.QGroupBox("Delay")
        self.delay_groupbox_layout = QtGui.QGridLayout(self.delay_groupbox)

        f_port = 2
        f_column = 0
        f_row = 0
        for f_i in range(8):
            f_effect = pydaw_modulex_single(("FX" + str(f_i)), f_port, a_rel_callback, a_val_callback)
            self.effects.append(f_effect)
            self.fx_layout.addWidget(f_effect.group_box, f_column, f_row)
            f_column += 1
            if f_column > 1:
                f_column = 0
                f_row += 1
            f_port += 4




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


Special note:  Please ignore any bizarre coding style you may see, these
were mostly converted to Python/PyQt from C++/Qt by a script.
"""

import os
from . import pydaw_util, pydaw_ports
from libpydaw.pydaw_project import pydaw_audio_item_fx
from PyQt4 import QtGui, QtCore
import numpy
import gettext

gettext.install("pydaw4" "{}/share/locale".format(pydaw_util.global_pydaw_install_prefix))

_ = gettext.lgettext


global_knob_arc_gradient = QtGui.QLinearGradient(0.0, 0.0, 90.0, 0.0)
global_knob_arc_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(60, 60, 255, 255))
global_knob_arc_gradient.setColorAt(0.25, QtGui.QColor.fromRgb(255, 120, 0, 255))
global_knob_arc_gradient.setColorAt(0.75, QtGui.QColor.fromRgb(255, 0, 0, 255))
global_knob_arc_pen = QtGui.QPen(global_knob_arc_gradient, 5.0,
                                 QtCore.Qt.SolidLine, QtCore.Qt.RoundCap,
                                 QtCore.Qt.RoundJoin)

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
                if f_items[0] == 'c':
                    f_items2 = f_items[1].split("|", 1)
                    self.configure_dict[(f_items2[0])] = f_items2[1]
                else:
                    self.port_dict[int(f_items[0])] = int(float(f_items[1]))

    @staticmethod
    def from_dict(a_port_dict, a_configure_dict):
        f_result = pydaw_plugin_file()
        for k, v in list(a_port_dict.items()):
            f_result.port_dict[int(k)] = v
        for k, v in list(a_configure_dict.items()):
            f_result.configure_dict[k] = v
        return f_result

    def __str__(self):
        f_result = ""
        for k in sorted(self.configure_dict.keys()):
            v = self.configure_dict[k]
            f_result += "c|{}|{}\n".format(k, v)
        for k in sorted(self.port_dict.keys()):
            v = self.port_dict[k]
            f_result += "{}|{}\n".format(int(k), int(v.get_value()))
        return f_result + "\\"

global_pydaw_knob_pixmap = None
global_pydaw_knob_pixmap_cache = {}

def get_scaled_pixmap_knob(a_size):
    global global_pydaw_knob_pixmap, global_pydaw_knob_pixmap_cache
    if global_pydaw_knob_pixmap is None:
        global_pydaw_knob_pixmap = QtGui.QPixmap(
            "{}/pydaw-knob.png".format(pydaw_util.global_stylesheet_dir))

    if not a_size in global_pydaw_knob_pixmap_cache:
        global_pydaw_knob_pixmap_cache[a_size] = \
        global_pydaw_knob_pixmap.scaled(a_size, a_size, \
        QtCore.Qt.KeepAspectRatio, QtCore.Qt.SmoothTransformation)

    return global_pydaw_knob_pixmap_cache[a_size]

global_cc_clipboard = None
global_tempo = 140.0

def set_global_tempo(a_tempo):
    global global_tempo
    global_tempo = a_tempo

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
        f_frac_val = (((float)(self.value() - self.minimum())) /
            ((float)(self.maximum() - self.minimum())))
        f_rotate_value =  f_frac_val * 270.0
        f_rect = self.rect()
        f_rect.setWidth(f_rect.width() - 3)
        f_rect.setHeight(f_rect.height() - 3)
        f_rect.setX(f_rect.x() + 3)
        f_rect.setY(f_rect.y() + 3)
        p.setPen(global_knob_arc_pen)
        p.drawArc(f_rect, -136 * 16, (f_rotate_value + 1.0) * -16)
        p.setRenderHints(
            QtGui.QPainter.HighQualityAntialiasing | QtGui.QPainter.SmoothPixmapTransform)
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
kc_time_decimal = 8
kc_hz_decimal = 9

global_last_tempo_combobox_index = 2

class pydaw_abstract_ui_control:
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback,
                 a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None,
                 a_default_value=None):
        if a_label is None:
            self.name_label = None
        else:
            self.name_label = QtGui.QLabel(str(a_label))
            self.name_label.setAlignment(QtCore.Qt.AlignCenter)
            self.name_label.setMinimumWidth(15)
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
            elif self.val_conversion == kc_decimal or \
            self.val_conversion == kc_time_decimal or \
            self.val_conversion == kc_hz_decimal:
                self.value_label.setText(str(round(f_value * .01, 2)))
            elif self.val_conversion == kc_integer:
                self.value_label.setText(str(int(f_value)))
            elif self.val_conversion == kc_pitch:
                self.value_label.setText(
                    str(int(440.0 * pow(2.0,((float)(f_value - 57.0)) * 0.0833333))))
            elif self.val_conversion == kc_127_pitch:
                self.value_label.setText(
                    str(int(440.0 * pow(2.0, ((float)(((f_value * 0.818897638) + 20.0) -57.0)) * \
                    0.0833333))))
            elif self.val_conversion == kc_127_zero_to_x:
                f_dec_value = (float(f_value) * self.label_value_127_multiply_by) - \
                    self.label_value_127_add_to
                f_dec_value = ((int)(f_dec_value * 10.0)) * 0.1
                self.value_label.setText(str(round(f_dec_value, 2)))
            elif self.val_conversion == kc_127_zero_to_x_int:
                f_dec_value = (float(f_value) * self.label_value_127_multiply_by) - \
                    self.label_value_127_add_to
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

    def set_value_dialog(self):
        def ok_handler(a_self=None, a_val=None):
            self.control.setValue(f_spinbox.value())
            f_dialog.close()
        f_dialog = QtGui.QDialog(self.control)
        f_dialog.setWindowTitle(_("Set Value"))
        f_layout = QtGui.QGridLayout(f_dialog)
        f_layout.addWidget(QtGui.QLabel(_("Value:")), 3, 0)
        f_spinbox = QtGui.QSpinBox()
        f_spinbox.setMinimum(self.control.minimum())
        f_spinbox.setMaximum(self.control.maximum())
        f_spinbox.setValue(self.control.value())
        f_layout.addWidget(f_spinbox, 3, 1)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_cancel_button.pressed.connect(f_dialog.close)
        f_layout.addWidget(f_cancel_button, 6, 0)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_ok_button.pressed.connect(ok_handler)
        f_layout.addWidget(f_ok_button, 6, 1)
        f_dialog.exec_()

    def tempo_sync_dialog(self):
        def sync_button_pressed(a_self=None):
            global global_last_tempo_combobox_index
            f_frac = 1.0
            f_switch = (f_beat_frac_combobox.currentIndex())
            f_dict = {0 : 0.25, 1 : 0.33333, 2 : 0.5, 3 : 0.666666, 4 : 0.75,
                      5 : 1.0, 6 : 2.0, 7 : 4.0}
            f_frac = f_dict[f_switch]
            f_seconds_per_beat = 60 / (f_spinbox.value())
            if self.val_conversion == kc_time_decimal:
                f_result = int(f_seconds_per_beat * f_frac * 100)
            elif self.val_conversion == kc_hz_decimal:
                f_result = int((1.0 / (f_seconds_per_beat * f_frac)) * 100)
            f_result = pydaw_util.pydaw_clip_value(f_result, self.control.minimum(),
                                                   self.control.maximum())
            self.control.setValue(f_result)
            global_last_tempo_combobox_index = f_beat_frac_combobox.currentIndex()
            f_dialog.close()
        f_dialog = QtGui.QDialog(self.control)
        f_dialog.setWindowTitle(_("Tempo Sync"))
        f_groupbox_layout =  QtGui.QGridLayout(f_dialog)
        f_spinbox =  QtGui.QDoubleSpinBox()
        f_spinbox.setDecimals(1)
        f_spinbox.setRange(60, 200)
        f_spinbox.setSingleStep(0.1)
        f_spinbox.setValue(global_tempo)
        f_beat_fracs = ["1/16", "1/12", "1/8", "2/12", "3/16", "1/4", "2/4", "4/4"]
        f_beat_frac_combobox =  QtGui.QComboBox()
        f_beat_frac_combobox.setMinimumWidth(75)
        f_beat_frac_combobox.addItems(f_beat_fracs)
        f_beat_frac_combobox.setCurrentIndex(global_last_tempo_combobox_index)
        f_sync_button =  QtGui.QPushButton(_("Sync"))
        f_sync_button.pressed.connect(sync_button_pressed)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_cancel_button.pressed.connect(f_dialog.close)
        f_groupbox_layout.addWidget(QtGui.QLabel(_("BPM")), 0, 0)
        f_groupbox_layout.addWidget(f_spinbox, 1, 0)
        f_groupbox_layout.addWidget(QtGui.QLabel("Length"), 0, 1)
        f_groupbox_layout.addWidget(f_beat_frac_combobox, 1, 1)
        f_groupbox_layout.addWidget(f_cancel_button, 2, 0)
        f_groupbox_layout.addWidget(f_sync_button, 2, 1)
        f_dialog.exec_()

    def set_note_dialog(self):
        def ok_button_pressed():
            f_value = f_note_selector.get_value()
            f_value = pydaw_util.pydaw_clip_value(f_value, self.control.minimum(),
                                                  self.control.maximum())
            self.set_value(f_value)
            f_dialog.close()
        f_dialog = QtGui.QDialog(self.control)
        f_dialog.setMinimumWidth(210)
        f_dialog.setWindowTitle(_("Set to Note"))
        f_vlayout =  QtGui.QVBoxLayout(f_dialog)
        f_note_selector = pydaw_note_selector_widget(0, None, None)
        f_note_selector.set_value(self.get_value())
        f_vlayout.addWidget(f_note_selector.widget)
        f_ok_button =  QtGui.QPushButton(_("OK"))
        f_ok_button.pressed.connect(ok_button_pressed)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_cancel_button.pressed.connect(f_dialog.close)
        f_ok_cancel_layout.addWidget(f_cancel_button)
        f_ok_cancel_layout.addWidget(f_ok_button)
        f_vlayout.addLayout(f_ok_cancel_layout)
        f_dialog.exec_()


    def copy_automation(self):
        global global_cc_clipboard
        f_value = ((self.get_value() - self.control.minimum()) /
                  (self.control.maximum() - self.control.minimum())) * 127.0
        global_cc_clipboard = pydaw_util.pydaw_clip_value(f_value, 0.0, 127.0)
        print(str(global_cc_clipboard))

    def contextMenuEvent(self, a_event):
        f_menu = QtGui.QMenu(self.control)
        f_reset_action = f_menu.addAction(_("Reset to Default Value"))
        f_reset_action.triggered.connect(self.reset_default_value)
        f_set_value_action = f_menu.addAction(_("Set Raw Controller Value"))
        f_set_value_action.triggered.connect(self.set_value_dialog)
        f_copy_automation_action = f_menu.addAction(_("Copy Automation"))
        f_copy_automation_action.triggered.connect(self.copy_automation)
        if self.val_conversion == kc_time_decimal or self.val_conversion == kc_hz_decimal:
            f_tempo_sync_action = f_menu.addAction(_("Tempo Sync"))
            f_tempo_sync_action.triggered.connect(self.tempo_sync_dialog)
        if self.val_conversion == kc_pitch:
            f_set_note_action = f_menu.addAction(_("Set to Note"))
            f_set_note_action.triggered.connect(self.set_note_dialog)
        f_menu.exec_(QtGui.QCursor.pos())



class pydaw_null_control:
    """ For controls with no visual representation, ie: controls that share a UI widget
    depending on selected index, so that they can participate normally in the data
    representation mechanisms"""
    def __init__(self, a_port_num, a_rel_callback, a_val_callback, a_default_val,
                 a_port_dict, a_preset_mgr=None):
        self.name_label = None
        self.value_label = None
        self.port_num = int(a_port_num)
        self.val_callback = a_val_callback
        self.rel_callback = a_rel_callback
        self.suppress_changes = False
        self.value = a_default_val
        a_port_dict[self.port_num] = self
        self.default_value = a_default_val
        self.control_callback = None
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
        if self.control_callback is not None:
            self.control_callback.set_value(self.value)

    def set_control_callback(self, a_callback=None):
        self.control_callback = a_callback

    def control_released(self):
        if self.rel_callback is not None:
            self.rel_callback(self.port_num, self.value)

    def control_value_changed(self, a_value):
        self.val_callback(self.port_num, self.value)


class pydaw_knob_control(pydaw_abstract_ui_control):
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback,
                 a_min_val, a_max_val, a_default_val, a_val_conversion=kc_none,
                 a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback,
                                           a_val_callback, a_val_conversion,
                                           a_port_dict, a_preset_mgr, a_default_val)
        self.control = pydaw_pixmap_knob(a_size, a_min_val, a_max_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.control.contextMenuEvent = self.contextMenuEvent
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.value_label.setMinimumWidth(15)
        self.set_value(a_default_val)


class pydaw_slider_control(pydaw_abstract_ui_control):
    def __init__(self, a_orientation, a_label, a_port_num, a_rel_callback,
                 a_val_callback, a_min_val, a_max_val,
    a_default_val, a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num,
                                           a_rel_callback, a_val_callback, a_val_conversion,
                                           a_port_dict, a_preset_mgr, a_default_val)
        self.control = QtGui.QSlider()
        self.control.contextMenuEvent = self.contextMenuEvent
        self.control.setRange(a_min_val, a_max_val)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.sliderReleased.connect(self.control_released)
        self.value_label = QtGui.QLabel("")
        self.value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.value_label.setMinimumWidth(15)
        self.set_value(a_default_val)


class pydaw_spinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val,
                 a_default_val, a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback,
                                           a_val_callback, a_val_conversion,
                                           a_port_dict, a_preset_mgr, a_default_val)
        self.control = QtGui.QSpinBox()
        self.widget = self.control
        self.control.setRange(a_min_val, a_max_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)
        self.value_label = None
        self.set_value(a_default_val)


class pydaw_doublespinbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback, a_min_val, a_max_val,
    a_default_val, a_val_conversion=kc_none, a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, a_label, a_port_num, a_rel_callback,
                                           a_val_callback, a_val_conversion,
                                           a_port_dict, a_preset_mgr, a_default_val)
        self.control = QtGui.QDoubleSpinBox()
        self.widget = self.control
        self.control.setRange(a_min_val, a_max_val)
        self.control.setKeyboardTracking(False)
        self.control.valueChanged.connect(self.control_value_changed)
        self.control.valueChanged.connect(self.control_released)
        self.value_label = None
        self.set_value(a_default_val)


class pydaw_checkbox_control(pydaw_abstract_ui_control):
    def __init__(self, a_label, a_port_num, a_rel_callback, a_val_callback,
                 a_port_dict=None, a_preset_mgr=None):
        pydaw_abstract_ui_control.__init__(self, None, a_port_num, a_rel_callback, a_val_callback,
                                           a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr,
                                           a_default_value=0)
        self.control = QtGui.QCheckBox(a_label)
        self.widget = self.control
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
    def __init__(self, a_size, a_label, a_port_num, a_rel_callback, a_val_callback,
                 a_items_list=[], a_port_dict=None, a_default_index=None, a_preset_mgr=None):
        self.suppress_changes = True
        self.name_label = QtGui.QLabel(str(a_label))
        self.name_label.setAlignment(QtCore.Qt.AlignCenter)
        self.control = QtGui.QComboBox()
        self.control.wheelEvent = self.wheel_event
        self.widget = self.control
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

    def wheel_event(self, a_event=None):
        pass

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
    def __init__(self, a_size, a_sustain_in_db, a_attack_port, a_decay_port,
                 a_sustain_port, a_release_port, a_label, a_rel_callback, a_val_callback,
                 a_port_dict=None, a_preset_mgr=None, a_attack_default=10):
        self.attack_knob = pydaw_knob_control(a_size, _("Attack"), a_attack_port, a_rel_callback,
                                              a_val_callback, 0, 200, a_attack_default,
                                              kc_time_decimal, a_port_dict, a_preset_mgr)
        self.decay_knob = pydaw_knob_control(a_size, _("Decay"), a_decay_port, a_rel_callback,
                                             a_val_callback, 10, 200, 50, kc_time_decimal,
                                             a_port_dict, a_preset_mgr)
        if a_sustain_in_db:
            self.sustain_knob = pydaw_knob_control(a_size, _("Sustain"), a_sustain_port,
                                                   a_rel_callback, a_val_callback,
                                                   -30, 0, 0, kc_integer, a_port_dict,
                                                   a_preset_mgr)
        else:
            self.sustain_knob = pydaw_knob_control(a_size, _("Sustain"), a_sustain_port,
                                                   a_rel_callback, a_val_callback,
                                                   0, 100, 100, kc_decimal, a_port_dict,
                                                   a_preset_mgr)
        self.release_knob = pydaw_knob_control(a_size, _("Release"), a_release_port,
                                               a_rel_callback, a_val_callback, 10,
                                               400, 50, kc_time_decimal, a_port_dict, a_preset_mgr)
        self.groupbox = QtGui.QGroupBox(a_label)
        self.groupbox.setObjectName("plugin_groupbox")
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.attack_knob.add_to_grid_layout(self.layout, 0)
        self.decay_knob.add_to_grid_layout(self.layout, 1)
        self.sustain_knob.add_to_grid_layout(self.layout, 2)
        self.release_knob.add_to_grid_layout(self.layout, 3)

class pydaw_filter_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict,
                 a_cutoff_port, a_res_port, a_type_port=None, a_label=_("Filter"),
                 a_preset_mgr=None):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.groupbox.setObjectName("plugin_groupbox")
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.cutoff_knob = pydaw_knob_control(a_size, _("Cutoff"), a_cutoff_port,
                                              a_rel_callback, a_val_callback,
                                              20, 124, 124, kc_pitch, a_port_dict,
                                              a_preset_mgr)
        self.cutoff_knob.add_to_grid_layout(self.layout, 0)
        self.res_knob = pydaw_knob_control(a_size, _("Res"), a_res_port, a_rel_callback,
                                           a_val_callback, -30, 0, -12, kc_integer,
                                           a_port_dict, a_preset_mgr)
        self.res_knob.add_to_grid_layout(self.layout, 1)
        if a_type_port is not None:
            self.type_combobox = pydaw_combobox_control(150, _("Type"), a_type_port,
                                                        a_rel_callback, a_val_callback,
                                                        ["LP 2", "HP 2", "BP2", "LP 4",
                                                        "HP 4", "BP4", _("Off")],
                                                        a_port_dict, a_preset_mgr=a_preset_mgr)
            self.layout.addWidget(self.type_combobox.name_label, 2, 0)
            self.layout.addWidget(self.type_combobox.control, 2, 1)


class pydaw_perc_env_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict,
                 a_time1_port, a_pitch1_port, a_time2_port, a_pitch2_port, a_on_port,
                 a_label=_("Perc Env"), a_preset_mgr=None):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.groupbox.setObjectName("plugin_groupbox")
        self.layout = QtGui.QGridLayout(self.groupbox)

        self.time1_knob = pydaw_knob_control(a_size, _("Time1"), a_time1_port,
                                              a_rel_callback, a_val_callback,
                                              2, 40, 10, kc_integer, a_port_dict,
                                              a_preset_mgr)
        self.time1_knob.add_to_grid_layout(self.layout, 0)

        self.pitch1_knob = pydaw_knob_control(a_size, _("Pitch1"), a_pitch1_port, a_rel_callback,
                                           a_val_callback, 42, 120, 66, kc_pitch,
                                           a_port_dict, a_preset_mgr)
        self.pitch1_knob.add_to_grid_layout(self.layout, 1)

        self.time2_knob = pydaw_knob_control(a_size, _("Time2"), a_time2_port,
                                              a_rel_callback, a_val_callback,
                                              20, 400, 100, kc_integer, a_port_dict,
                                              a_preset_mgr)
        self.time2_knob.add_to_grid_layout(self.layout, 2)

        self.pitch2_knob = pydaw_knob_control(a_size, _("Pitch2"), a_pitch2_port, a_rel_callback,
                                           a_val_callback, 33, 63, 48, kc_pitch,
                                           a_port_dict, a_preset_mgr)
        self.pitch2_knob.add_to_grid_layout(self.layout, 3)

        self.on_switch = pydaw_checkbox_control(_("On"), a_on_port,
                                                a_rel_callback, a_val_callback,
                                                a_port_dict, a_preset_mgr)
        self.on_switch.add_to_grid_layout(self.layout, 4)


class pydaw_ramp_env_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict,
                 a_time_port, a_amt_port,
                 a_label=_("Ramp Env"), a_preset_mgr=None, a_curve_port=None):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.groupbox.setObjectName("plugin_groupbox")
        self.layout = QtGui.QGridLayout(self.groupbox)
        if a_amt_port is not None:
            self.amt_knob = pydaw_knob_control(a_size, _("Amt"), a_amt_port,
                                               a_rel_callback, a_val_callback,
                                               -36, 36, 0, kc_integer, a_port_dict,
                                               a_preset_mgr)
            self.amt_knob.add_to_grid_layout(self.layout, 0)
        self.time_knob = pydaw_knob_control(a_size, _("Time"), a_time_port,
                                            a_rel_callback, a_val_callback,
                                            1, 600, 100, kc_time_decimal, a_port_dict,
                                            a_preset_mgr)
        self.time_knob.add_to_grid_layout(self.layout, 1)
        if a_curve_port is not None:
            self.curve_knob = pydaw_knob_control(a_size, _("Curve"), a_curve_port,
                                                 a_rel_callback, a_val_callback,
                                                 0, 100, 50, kc_none, a_port_dict,
                                                 a_preset_mgr)
            self.curve_knob.add_to_grid_layout(self.layout, 2)

class pydaw_lfo_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_port_dict,
                 a_freq_port, a_type_port, a_type_list,
                 a_label=_("LFO"), a_preset_mgr=None):
        self.groupbox = QtGui.QGroupBox(str(a_label))
        self.groupbox.setObjectName("plugin_groupbox")
        self.layout = QtGui.QGridLayout(self.groupbox)
        self.freq_knob = pydaw_knob_control(a_size, _("Freq"), a_freq_port,
                                            a_rel_callback, a_val_callback,
                                            10, 1600, 200, kc_hz_decimal, a_port_dict,
                                            a_preset_mgr)
        self.freq_knob.add_to_grid_layout(self.layout, 0)
        self.type_combobox = pydaw_combobox_control(120, _("Type"), a_type_port,
                                                    a_rel_callback, a_val_callback,
                                                    a_type_list, a_port_dict, 0,
                                                    a_preset_mgr=a_preset_mgr)
        self.layout.addWidget(self.type_combobox.name_label, 0, 1)
        self.layout.addWidget(self.type_combobox.control, 1, 1)

class pydaw_osc_widget:
    def __init__(self, a_size, a_pitch_port, a_fine_port, a_vol_port, a_type_port,
                 a_osc_types_list, a_rel_callback, a_val_callback, a_label,
                 a_port_dict=None, a_preset_mgr=None, a_default_type=0):
        self.pitch_knob = pydaw_knob_control(a_size, _("Pitch"), a_pitch_port,
                                             a_rel_callback, a_val_callback, -36, 36,
                                             0, a_val_conversion=kc_integer,
                                             a_port_dict=a_port_dict,
                                             a_preset_mgr=a_preset_mgr)
        self.fine_knob = pydaw_knob_control(a_size, _("Fine"), a_fine_port, a_rel_callback,
                                            a_val_callback, -100, 100,
                                            0, a_val_conversion=kc_decimal,
                                            a_port_dict=a_port_dict,
                                            a_preset_mgr=a_preset_mgr)
        self.vol_knob = pydaw_knob_control(a_size, _("Vol"), a_vol_port, a_rel_callback,
                                           a_val_callback, -30, 0, -6,
                                           a_val_conversion=kc_integer,
                                           a_port_dict=a_port_dict,
                                           a_preset_mgr=a_preset_mgr)
        self.osc_type_combobox = pydaw_combobox_control(139, _("Type"), a_type_port,
                                                        a_rel_callback, a_val_callback,
                                                        a_osc_types_list, a_port_dict,
                                                        a_preset_mgr=a_preset_mgr,
                                                        a_default_index=a_default_type)
        self.grid_layout = QtGui.QGridLayout()
        self.group_box = QtGui.QGroupBox(str(a_label))
        self.group_box.setObjectName("plugin_groupbox")
        self.group_box.setLayout(self.grid_layout)
        self.pitch_knob.add_to_grid_layout(self.grid_layout, 0)
        self.fine_knob.add_to_grid_layout(self.grid_layout, 1)
        self.vol_knob.add_to_grid_layout(self.grid_layout, 2)
        self.grid_layout.addWidget(self.osc_type_combobox.name_label, 0, 3)
        self.grid_layout.addWidget(self.osc_type_combobox.control, 1, 3)

class pydaw_note_selector_widget:
    def __init__(self, a_port_num, a_rel_callback, a_val_callback,
                 a_port_dict=None, a_default_value=None, a_preset_mgr=None):
        self.control = self
        self.port_num = a_port_num
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.note_combobox = QtGui.QComboBox()
        self.note_combobox.wheelEvent = self.wheel_event
        self.note_combobox.setMinimumWidth(60)
        self.note_combobox.addItems(["C", "C#", "D", "D#", "E", "F", "F#",
                                     "G", "G#", "A", "A#", "B"])
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
            self.selected_note = a_default_value
            self.set_value(a_default_value)
        else:
            self.selected_note = 60
        if a_preset_mgr is not None:
            a_preset_mgr.add_control(self)

    def wheel_event(self, a_event=None):
        pass

    def control_value_changed(self, a_val=None):
        self.selected_note = (self.note_combobox.currentIndex()) + \
                             (((self.octave_spinbox.value()) + 2) * 12)
        if not self.suppress_changes:
            if self.val_callback is not None:
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
    """
    a_load_callback : function to call when loading that accepts a single argument
    of [list of paths,...]
    """
    def __init__(self, a_load_callback):
        self.load_callback = a_load_callback
        self.layout =  QtGui.QHBoxLayout()
        self.clear_button =  QtGui.QPushButton(_("Clear"))
        self.clear_button.setMaximumWidth(60)
        self.copy_to_clipboard =  QtGui.QPushButton(_("Copy"))
        self.copy_to_clipboard.setToolTip(_("Copy file path to clipboard"))
        self.copy_to_clipboard.pressed.connect(self.copy_to_clipboard_pressed)
        self.copy_to_clipboard.setMaximumWidth(60)
        self.paste_from_clipboard =  QtGui.QPushButton(_("Paste"))
        self.paste_from_clipboard.setToolTip(_("Paste file path from clipboard"))
        self.paste_from_clipboard.pressed.connect(self.paste_from_clipboard_pressed)
        self.paste_from_clipboard.setMaximumWidth(60)
        self.reload_button =  QtGui.QPushButton(_("Reload"))
        self.reload_button.setMaximumWidth(60)
        self.file_path =  QtGui.QLineEdit()
        self.file_path.setSizePolicy(QtGui.QSizePolicy.Expanding,
                                     QtGui.QSizePolicy.Minimum)
        self.file_path.setReadOnly(True)
        self.file_path.setMinimumWidth(210)
        self.last_directory = ("")
        self.layout.addWidget(self.file_path)
        self.layout.addWidget(self.clear_button)
        self.layout.addWidget(self.copy_to_clipboard)
        self.layout.addWidget(self.paste_from_clipboard)
        self.layout.addWidget(self.reload_button)


    def clear_button_pressed(self):
        self.file_path.setText("")

    def get_file(self):
        return self.file_path.text()

    def set_file(self, a_file):
        self.file_path.setText(str(a_file))

    def copy_to_clipboard_pressed(self):
        f_text = str(self.file_path.text())
        if f_text != "":
            f_clipboard = QtGui.QApplication.clipboard()
            f_clipboard.setText(f_text)

    def paste_from_clipboard_pressed(self):
        f_clipboard = QtGui.QApplication.clipboard()
        f_text = f_clipboard.text()
        if f_text is None:
            QtGui.QMessageBox.warning(self.paste_from_clipboard, _("Error"),
            _("No file path in the system clipboard."))
        else:
            f_text = str(f_text)
            if os.path.isfile(f_text):
                self.set_file(f_text)
                self.load_callback([f_text])
            else:
                #Don't show more than 100 chars just in case somebody had an
                #entire book copied to the clipboard
                f_str = f_text[100:]
                QtGui.QMessageBox.warning(self.paste_from_clipboard, _("Error"),
                                          _("{} does not exist.").format(f_str))



class pydaw_file_browser_widget:
    def __init__(self):
        if os.path.isdir("/media/pydaw_data") and os.path.isdir("/home/ubuntu"):
            self.home_path = ("/media/pydaw_data")
        else:
            self.home_path = os.path.expanduser("~")
        self.file_browser_vsplitter =  QtGui.QSplitter(QtCore.Qt.Vertical)
        self.file_browser_vsplitter.setMinimumWidth(240)
        self.file_browser_vsplitter.setContentsMargins(0, 0, 0, 0)
        self.bookmark_widget = QtGui.QWidget()
        self.file_browser_vsplitter.addWidget(self.bookmark_widget)
        self.bookmark_layout = QtGui.QVBoxLayout(self.bookmark_widget)
        self.bookmarks_label =  QtGui.QLabel(_("Bookmarks"))
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
        self.bookmarks_listWidget.setToolTip(_("Press the 'bookmark' button to add folders here."))
        self.bookmarks_listWidget.setDragDropMode(QtGui.QAbstractItemView.DropOnly)
        self.bookmark_layout.addWidget(self.bookmarks_listWidget)
        self.bookmarks_button_widget = QtGui.QWidget()
        self.bookmarks_hlayout0 =  QtGui.QHBoxLayout(self.bookmarks_button_widget)
        self.bookmarks_delete_button =  QtGui.QPushButton(_("Delete"))
        self.bookmarks_hlayout0.addWidget(self.bookmarks_delete_button)
        self.bookmarks_delete_button.pressed.connect(self.bookmark_delete_button_pressed)
        self.bookmark_layout.addWidget(self.bookmarks_button_widget)
        self.folders_widget = QtGui.QWidget()
        self.file_browser_vsplitter.addWidget(self.folders_widget)
        self.folders_layout = QtGui.QVBoxLayout(self.folders_widget)
        self.folders_label =  QtGui.QLabel(_("Folders"))
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
        self.up_pushButton =  QtGui.QPushButton(_("Up"))
        self.up_pushButton.pressed.connect(self.up_button_pressed)
        self.folders_hlayout0.addWidget(self.up_pushButton)
        self.bookmark_button =  QtGui.QPushButton(_("Bookmark"))
        self.folders_hlayout0.addWidget(self.bookmark_button)
        self.bookmark_button.pressed.connect(self.bookmark_button_pressed)
        self.folders_layout.addWidget(self.folder_buttons_widget)
        self.files_widget = QtGui.QWidget()
        self.file_browser_vsplitter.addWidget(self.files_widget)
        self.files_layout = QtGui.QVBoxLayout(self.files_widget)
        self.files_label =  QtGui.QLabel(_("Files"))
        self.files_label.setAlignment(QtCore.Qt.AlignCenter)
        self.files_layout.addWidget(self.files_label)
        self.files_listWidget =  QtGui.QListWidget()
        self.files_listWidget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.files_listWidget.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        self.files_listWidget.setToolTip(
            _("Select the file(s) you wish to load and click the 'load' button.\n"
            "The samples will be loaded sequentially starting from the currently selected row."))
        self.files_layout.addWidget(self.files_listWidget)
        self.file_buttons_widget = QtGui.QWidget()
        self.files_hlayout0 =  QtGui.QHBoxLayout(self.file_buttons_widget)
        self.load_pushButton =  QtGui.QPushButton(_("Load"))
        self.files_hlayout0.addWidget(self.load_pushButton)
        self.preview_pushButton =  QtGui.QPushButton(_("Preview"))
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
                self.enumerate_folders_and_files("/{}".format(a_folder))
            else:
                self.enumerate_folders_and_files(
                    "{}/{}".format(self.folder_path_lineedit.text(), a_folder))
        else:
            self.enumerate_folders_and_files(a_folder)

    def up_one_folder(self):
        self.set_folder("..")

    def bookmark_delete_button_pressed(self):
        f_items = self.bookmarks_listWidget.selectedItems()
        if len(f_items) > 0:
            pydaw_util.global_delete_file_bookmark(str(f_items[0].text()))
            self.bookmarks_listWidget.clear()
            self.open_bookmarks()

    def files_selected(self):
        f_dir_path = "{}/".format(self.folder_path_lineedit.text())
        f_result = []
        for f_item in self.files_listWidget.selectedItems():
            f_result.append("{}{}".format(f_dir_path, f_item.text()))
        return f_result

    def open_bookmarks(self):
        self.bookmarks_listWidget.clear()
        f_dict = pydaw_util.global_get_file_bookmarks()
        for k in sorted(f_dict.keys()):
            self.bookmarks_listWidget.addItem(str(k))

    def bookmark_button_pressed(self):
        pydaw_util.global_add_file_bookmark(self.last_open_dir)
        self.open_bookmarks()

    def bookmark_clicked(self, a_item):
        f_dict = pydaw_util.global_get_file_bookmarks()
        f_folder_name = str(a_item.text())
        if f_folder_name in f_dict:
            f_full_path = "{}/{}".format(f_dict[f_folder_name], f_folder_name)
            self.set_folder(f_full_path, True)
        else:
            QtGui.QMessageBox.warning(self.files_listWidget, _("Error"),
                                      _("This bookmark no longer exists.  You may have deleted "
                                      "it in another windows."))
            self.open_bookmarks()

    def file_mouse_press_event(self, a_event):
        QtGui.QListWidget.mousePressEvent(self.files_listWidget, a_event)
        global global_audio_items_to_drop
        global_audio_items_to_drop = []
        for f_item in self.files_listWidget.selectedItems():
            global_audio_items_to_drop.append(
                "{}/{}".format(self.last_open_dir, f_item.text()))

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
            self.last_open_dir = os.path.abspath("{}/{}".format(self.last_open_dir, a_folder))
        self.last_open_dir = self.last_open_dir.replace("//", "/")
        self.folder_path_lineedit.setText(self.last_open_dir)
        f_list = os.listdir(self.last_open_dir)
        f_list.sort(key=str.lower)
        for f_file in f_list:
            f_full_path = "{}/{}".format(self.last_open_dir, f_file)
            if  not f_file.startswith("."):
                if os.path.isdir(f_full_path):
                    self.folders_listWidget.addItem(f_file)
                elif pydaw_util.is_audio_file(f_file) and os.path.isfile(f_full_path):
                    if not pydaw_util.pydaw_str_has_bad_chars(f_full_path):
                        self.files_listWidget.addItem(f_file)
                    else:
                        print("Not adding '{}' because it contains bad chars, "
                        "you must rename this file path without:\n{}".format(
                        f_full_path, "\n".join(pydaw_util.pydaw_bad_chars)))

global_preset_file_dialog_string = 'PyDAW Presets (*.pypresets)'
global_plugin_settings_clipboard = {}
global_plugin_configure_clipboard = None

class pydaw_preset_manager_widget:
    def __init__(self, a_plugin_name, a_configure_dict=None, a_reconfigure_callback=None):
        self.plugin_name = str(a_plugin_name)
        self.configure_dict = a_configure_dict
        self.reconfigure_callback = a_reconfigure_callback
        self.factory_preset_path = "{}/lib/{}/presets/{}.pypresets".format(
            pydaw_util.global_pydaw_install_prefix, pydaw_util.global_pydaw_version_string,
            a_plugin_name)
        self.bank_file = "{}/{}.bank".format(pydaw_util.global_pydaw_home, a_plugin_name)
        self.preset_path = "{}/{}.pypresets".format(pydaw_util.global_pydaw_home, a_plugin_name)

        if os.path.isfile(self.bank_file):
            f_text = pydaw_util.pydaw_read_file_text(self.bank_file)
            if os.path.isfile(f_text):
                self.preset_path = f_text

        self.group_box = QtGui.QGroupBox()
        self.group_box.setObjectName("plugin_groupbox")
        self.layout = QtGui.QHBoxLayout(self.group_box)
        self.program_combobox = QtGui.QComboBox()
        self.program_combobox.setEditable(True)
        self.program_combobox.setMinimumWidth(190)
        self.layout.addWidget(self.program_combobox)
        self.save_button = QtGui.QPushButton("Save")
        self.save_button.setToolTip(_("Save the current settings to a preset.  "
        "Plugin settings are saved to the project automatically\n"
        "when you close the plugin window, this button is only for presets."))
        self.save_button.pressed.connect(self.save_presets)
        self.layout.addWidget(self.save_button)
        self.reset_button = QtGui.QPushButton(_("Reset"))
        self.reset_button.setToolTip(_("Resets all controls to their default value"))
        self.reset_button.pressed.connect(self.reset_controls)
        self.layout.addWidget(self.reset_button)
        self.more_button = QtGui.QPushButton(_("More"))
        self.more_button.pressed.connect(self.on_more)

        self.more_menu = QtGui.QMenu(self.more_button)

        f_save_as_action = self.more_menu.addAction(_("Save bank as..."))
        f_save_as_action.triggered.connect(self.on_save_as)
        f_open_action = self.more_menu.addAction(_("Open bank..."))
        f_open_action.triggered.connect(self.on_open_bank)
        f_restore_action = self.more_menu.addAction(_("Restore factory bank..."))
        f_restore_action.triggered.connect(self.on_restore_bank)
        self.more_menu.addSeparator()
        f_copy_action = self.more_menu.addAction(_("Copy plugin settings..."))
        f_copy_action.triggered.connect(self.on_copy)
        f_paste_action = self.more_menu.addAction(_("Paste plugin settings..."))
        f_paste_action.triggered.connect(self.on_paste)

        self.more_button.setMenu(self.more_menu)
        self.layout.addWidget(self.more_button)
        self.presets_delimited = []
        self.controls = {}
        for f_i in range(128):
            self.program_combobox.addItem("empty")
        self.load_presets()
        self.program_combobox.currentIndexChanged.connect(self.program_changed)

    def on_copy(self):
        f_result = {}
        for k, v in self.controls.items():
            f_result[k] = v.get_value()
        global_plugin_settings_clipboard[self.plugin_name] = f_result
        global global_plugin_configure_clipboard
        if self.configure_dict is None:
            global_plugin_configure_clipboard = None
        else:
            global_plugin_configure_clipboard = self.configure_dict.copy()

    def on_paste(self):
        if not self.plugin_name in global_plugin_settings_clipboard:
            QtGui.QMessageBox.warning(self.group_box, _("Error"),
                _("Nothing copied to clipboard for {}").format(self.plugin_name))
            return
        f_dict = global_plugin_settings_clipboard[self.plugin_name]
        for k, v in f_dict.items():
            self.controls[k].set_value(v)
            self.controls[k].control_value_changed(v)
        if global_plugin_configure_clipboard is not None:
            self.reconfigure_callback(global_plugin_configure_clipboard)

    def on_more(self):
        self.more_button.showMenu()

    def on_save_as(self):
        f_file = QtGui.QFileDialog.getSaveFileName(parent=self.group_box,
                                                   caption=_('Save preset bank...'),
                                                   directory=pydaw_util.global_home,
                                                   filter=global_preset_file_dialog_string)
        if not f_file is None and not str(f_file) == "":
            f_file = str(f_file)
            if not f_file.endswith(".pypresets"):
                f_file += ".pypresets"
            os.system('cp "{}" "{}"'.format(self.preset_path, f_file))
            self.preset_path = f_file
            pydaw_util.pydaw_write_file_text(self.bank_file, self.preset_path)

    def on_open_bank(self):
        f_file = QtGui.QFileDialog.getOpenFileName(parent=self.group_box,
                                                   caption=_('Open preset bank...'),
                                                   directory=pydaw_util.global_home,
                                                   filter=global_preset_file_dialog_string)
        if not f_file is None and not str(f_file) == "":
            f_file = str(f_file)
            self.preset_path = f_file
            pydaw_util.pydaw_write_file_text(self.bank_file, self.preset_path)
            self.load_presets()

    def on_restore_bank(self):
        if os.path.isfile(self.bank_file):
            os.system('rm "{}"'.format(self.bank_file))
        self.preset_path = "{}/{}.pypresets".format(pydaw_util.global_pydaw_home,
                                                    self.plugin_name)
        os.system('rm "{}"'.format(self.preset_path))
        self.load_presets()

    def reset_controls(self):
        for v in self.controls.values():
            v.reset_default_value()
        if self.reconfigure_callback is not None:
            self.reconfigure_callback({})

    def load_presets(self):
        if os.path.isfile(self.preset_path):
            print("loading presets from file")
            f_text = pydaw_util.pydaw_read_file_text(self.preset_path)
            f_line_arr = f_text.split("\n")
        elif os.path.isfile(self.factory_preset_path):
            os.system('cp "{}" "{}"'.format(self.factory_preset_path, self.preset_path))
            print("loading factory presets")
            f_text = pydaw_util.pydaw_read_file_text(self.preset_path)
            f_line_arr = f_text.split("\n")
        else:
            f_line_arr = []

        if f_line_arr:
            if f_line_arr[0].strip() != self.plugin_name:
                QtGui.QMessageBox.warning(self.group_box, _("Error"),
                _("The selected preset bank is for {}, please select one for {}").format(
                f_line_arr[0], self.plugin_name))
                if os.path.isfile(self.bank_file):
                    os.system('rm "{}"'.format(self.bank_file))
                return

        f_line_arr = f_line_arr[1:]
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
            QtGui.QMessageBox.warning(self.group_box, _("Error"),
                                      _("Preset name cannot be 'empty'"))
            return
        if self.program_combobox.currentIndex() == 0:
            QtGui.QMessageBox.warning(self.group_box, _("Error"),
                                      _("The first preset must be empty"))
            return
        f_result_values = [str(self.program_combobox.currentText())]
        for k in sorted(self.controls.keys()):
            f_control = self.controls[k]
            f_result_values.append("{}:{}".format(f_control.port_num, f_control.get_value()))
        if self.configure_dict is not None:
            for k in self.configure_dict.keys():
                v = self.configure_dict[k]
                f_result_values.append("c:{}:{}".format(k, v.replace("|", ":")))
        self.presets_delimited[(self.program_combobox.currentIndex())] = f_result_values
        f_result = "{}\n".format(self.plugin_name)
        for f_list in self.presets_delimited:
            f_result += "{}\n".format("|".join(f_list))
        pydaw_util.pydaw_write_file_text(self.preset_path, f_result)
        self.load_presets()

    def program_changed(self, a_val=None):
        if str(self.program_combobox.currentText()) == "empty":
            print("empty")
        else:
            f_preset = self.presets_delimited[self.program_combobox.currentIndex()]
            print("setting preset {}".format(f_preset))
            f_preset_dict = {}
            f_configure_dict = {}
            for f_i in range(1, len(f_preset)):
                f_list = f_preset[f_i].split(":")
                if f_list[0] == "c":
                    f_configure_dict[f_list[1]] = "|".join(f_list[2:])
                else:
                    f_preset_dict[int(f_list[0])] = int(f_list[1])

            for k, v in self.controls.items():
                if int(k) in f_preset_dict:
                    v.set_value(f_preset_dict[k])
                    v.control_value_changed(f_preset_dict[k])
                else:
                    v.reset_default_value()
            if self.reconfigure_callback is not None:
                self.reconfigure_callback(f_configure_dict)

    def add_control(self, a_control):
        self.controls[a_control.port_num] = a_control

class pydaw_master_widget:
    def __init__(self, a_size, a_rel_callback, a_val_callback, a_master_vol_port,
                 a_master_glide_port, a_master_pitchbend_port, a_port_dict,
                 a_title=_("Master"), a_master_uni_voices_port=None,
                 a_master_uni_spread_port=None, a_preset_mgr=None):
        self.group_box = QtGui.QGroupBox()
        self.group_box.setObjectName("plugin_groupbox")
        self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout(self.group_box)
        self.vol_knob = pydaw_knob_control(a_size, _("Vol"), a_master_vol_port,
                                           a_rel_callback, a_val_callback, -30,
                                           12, -6, kc_integer, a_port_dict, a_preset_mgr)
        self.vol_knob.add_to_grid_layout(self.layout, 0)
        if a_master_uni_voices_port is not None and a_master_uni_spread_port is not None:
            self.uni_voices_knob = pydaw_knob_control(a_size, _("Unison"), a_master_uni_voices_port,
                                                      a_rel_callback, a_val_callback,
                                                      1, 7, 4, kc_integer, a_port_dict,
                                                      a_preset_mgr)
            self.uni_voices_knob.add_to_grid_layout(self.layout, 1)
            self.uni_spread_knob = pydaw_knob_control(a_size, _("Spread"), a_master_uni_spread_port,
                                                      a_rel_callback, a_val_callback,
                                                      10, 100, 50, kc_decimal, a_port_dict,
                                                      a_preset_mgr)
            self.uni_spread_knob.add_to_grid_layout(self.layout, 2)
        self.glide_knob = pydaw_knob_control(a_size, _("Glide"), a_master_glide_port,
                                             a_rel_callback, a_val_callback,
                                             0, 200, 0, kc_time_decimal, a_port_dict, a_preset_mgr)
        self.glide_knob.add_to_grid_layout(self.layout, 3)
        self.pb_knob = pydaw_knob_control(a_size, _("Pitchbend"), a_master_pitchbend_port,
                                          a_rel_callback, a_val_callback, 1, 36, 18,
                                          kc_integer, a_port_dict, a_preset_mgr)
        self.pb_knob.add_to_grid_layout(self.layout, 4)



global_eq_point_diameter = 12.0
global_eq_point_radius = global_eq_point_diameter * 0.5
global_eq_width = 600
global_eq_height = 300
global_eq_octave_px = (global_eq_width / (100.0 / 12.0))

global_eq_gradient = QtGui.QLinearGradient(0, 0, global_eq_point_diameter,
                                           global_eq_point_diameter)
global_eq_gradient.setColorAt(0, QtGui.QColor(255, 255, 255))
global_eq_gradient.setColorAt(1, QtGui.QColor(240, 240, 240))

global_eq_fill = QtGui.QLinearGradient(0.0, 0.0, 0.0, global_eq_height)

global_eq_fill.setColorAt(0.0, QtGui.QColor(255, 0, 0, 90)) #red
global_eq_fill.setColorAt(0.14285, QtGui.QColor(255, 123, 0, 90)) #orange
global_eq_fill.setColorAt(0.2857, QtGui.QColor(255, 255, 0, 90)) #yellow
global_eq_fill.setColorAt(0.42855, QtGui.QColor(0, 255, 0, 90)) #green
global_eq_fill.setColorAt(0.5714, QtGui.QColor(0, 123, 255, 90)) #blue
global_eq_fill.setColorAt(0.71425, QtGui.QColor(0, 0, 255, 90)) #indigo
global_eq_fill.setColorAt(0.8571, QtGui.QColor(255, 0, 255, 90)) #violet

global_eq_background = QtGui.QLinearGradient(0.0, 0.0, 0.0, global_eq_height)

global_eq_background.setColorAt(0.0, QtGui.QColor(40, 40, 40))
global_eq_background.setColorAt(0.1, QtGui.QColor(20, 20, 20))
global_eq_background.setColorAt(0.9, QtGui.QColor(30, 30, 30))
global_eq_background.setColorAt(1.0, QtGui.QColor(40, 40, 40))

class eq_item(QtGui.QGraphicsEllipseItem):
    def __init__(self, a_eq, a_num, a_val_callback):
        QtGui.QGraphicsEllipseItem.__init__(self, 0, 0, global_eq_point_diameter,
                                            global_eq_point_diameter)
        self.val_callback = a_val_callback
        self.eq = a_eq
        self.num = a_num
        self.setToolTip("EQ{}".format(self.num))
        self.setBrush(global_eq_gradient)
        self.mapToScene(0.0, 0.0)
        self.path_item = None
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseMoveEvent(self, a_event)
        f_pos = self.pos()
        f_pos_x = pydaw_util.pydaw_clip_value(f_pos.x(), -global_eq_point_radius, global_eq_width)
        f_pos_y = pydaw_util.pydaw_clip_value(f_pos.y(), -global_eq_point_radius, global_eq_height)

        if f_pos_x != f_pos.x() or f_pos_y != f_pos.y():
            self.setPos(f_pos_x, f_pos_y)

        f_freq, f_gain = self.get_value()
        self.val_callback(self.eq.freq_knob.port_num, f_freq)
        self.val_callback(self.eq.gain_knob.port_num, f_gain)
        self.eq.freq_knob.set_value(f_freq)
        self.eq.gain_knob.set_value(f_gain)
        self.draw_path_item()

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseReleaseEvent(self, a_event)
        self.eq.freq_knob.control_value_changed(self.eq.freq_knob.get_value())
        self.eq.gain_knob.control_value_changed(self.eq.gain_knob.get_value())

    def set_pos(self):
        f_freq = self.eq.freq_knob.get_value()
        f_gain = self.eq.gain_knob.get_value()
        f_x = (((f_freq - 20.0) * 0.01) * global_eq_width) - global_eq_point_radius
        f_y = ((1.0 - ((f_gain + 24.0) / 48.0)) * global_eq_height) - global_eq_point_radius
        self.setPos(f_x, f_y)
        self.draw_path_item()

    def get_value(self):
        f_pos = self.pos()
        f_freq = (((f_pos.x() + global_eq_point_radius) / global_eq_width) * 100.0) + 20.0
        f_gain = ((1.0 - ((f_pos.y() + global_eq_point_radius) / global_eq_height)) * 48.0) - 24.0
        return round(f_freq, 2), round(f_gain, 2)

    def __lt__(self, other):
        return self.pos().x() < other.pos().x()

    def draw_path_item(self):
        f_res = self.eq.res_knob.get_value()

        if self.path_item is not None:
            self.scene().removeItem(self.path_item)

        f_line_pen = QtGui.QPen(QtGui.QColor(255, 255, 255, 210), 2.0)
        f_path = QtGui.QPainterPath()

        f_pos = self.pos()
        f_bw = (f_res * 0.01)
        f_point_x = f_pos.x() + global_eq_point_radius
        f_point_y = f_pos.y() + global_eq_point_radius
        f_start_x = f_point_x - ((f_bw * 0.5 * global_eq_octave_px))
        f_end_x = f_point_x + ((f_bw * 0.5 * global_eq_octave_px))

        f_path.moveTo(f_start_x, global_eq_height * 0.5)

        f_path.lineTo(f_point_x, f_point_y)

        f_path.lineTo(f_end_x, global_eq_height * 0.5)

        self.path_item = QtGui.QGraphicsPathItem(f_path)
        self.path_item.setPen(f_line_pen)
        self.path_item.setBrush(global_eq_fill)
        self.scene().addItem(self.path_item)


class eq_viewer(QtGui.QGraphicsView):
    def __init__(self, a_val_callback):
        QtGui.QGraphicsView.__init__(self)
        self.val_callback = a_val_callback
        self.eq_points = []
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(global_eq_background)
        self.setScene(self.scene)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setResizeAnchor(QtGui.QGraphicsView.AnchorViewCenter)
        self.last_x_scale = 1.0
        self.last_y_scale = 1.0
        self.eq_points = []
        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.setSceneRect(-global_eq_point_radius, -global_eq_point_radius,
                          global_eq_width + global_eq_point_radius,
                          global_eq_height + global_eq_point_diameter)

    def draw_eq(self, a_eq_list=[]):
        f_hline_pen = QtGui.QPen(QtGui.QColor(255, 255, 255, 90), 1.0)
        f_vline_pen = QtGui.QPen(QtGui.QColor(255, 255, 255, 150), 2.0)

        f_label_pos = 0.0

        self.scene.clear()

        f_y_pos = 0.0
        f_db = 24.0
        f_inc = (global_eq_height * 0.5) * 0.25

        for i in range(4):
            self.scene.addLine(0.0, f_y_pos, global_eq_width, f_y_pos, f_hline_pen)
            f_label = QtGui.QGraphicsSimpleTextItem("{}".format(f_db), scene=self.scene)
            f_label.setPos(global_eq_width - 36.0, f_y_pos + 3.0)
            f_label.setBrush(QtCore.Qt.white)
            f_db -= 6.0
            f_y_pos += f_inc

        self.scene.addLine(0.0, global_eq_height * 0.5, global_eq_width,
                           global_eq_height * 0.5,
                           QtGui.QPen(QtGui.QColor(255, 255, 255, 210), 2.0))

        f_y_pos = global_eq_height
        f_db = -24.0

        for i in range(4):
            self.scene.addLine(0.0, f_y_pos, global_eq_width, f_y_pos, f_hline_pen)
            f_label = QtGui.QGraphicsSimpleTextItem("{}".format(f_db), scene=self.scene)
            f_label.setPos(global_eq_width - 36.0, f_y_pos - 24.0)
            f_label.setBrush(QtCore.Qt.white)
            f_db += 6.0
            f_y_pos -= f_inc

        f_label_pos = 0.0
        f_pitch = 20.0
        f_pitch_inc = 17.0
        f_label_inc = global_eq_width / (100.0 / f_pitch_inc)

        for i in range(6):
            f_hz = int(pydaw_util.pydaw_pitch_to_hz(f_pitch))
            if f_hz > 950:
                f_hz = round(f_hz, -1)
                f_hz = "{}khz".format(round(f_hz / 1000, 1))
            f_label = QtGui.QGraphicsSimpleTextItem("{}".format(f_hz), scene=self.scene)
            f_label.setPos(f_label_pos + 4.0, global_eq_height - 30.0)
            self.scene.addLine(f_label_pos, 0.0, f_label_pos, global_eq_height, f_vline_pen)
            f_label.setBrush(QtCore.Qt.white)
            f_label_pos += f_label_inc
            f_pitch += f_pitch_inc

        self.eq_points = []

        for f_eq, f_num in zip(a_eq_list, range(1, len(a_eq_list) + 1)):
            f_eq_point = eq_item(f_eq, f_num, self.val_callback)
            self.eq_points.append(f_eq_point)
            self.scene.addItem(f_eq_point)
            f_eq_point.set_pos()


    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / (global_eq_width + global_eq_point_diameter + 3.0)
        self.last_y_scale = f_rect.height() / (global_eq_height + global_eq_point_diameter + 3.0)
        self.scale(self.last_x_scale, self.last_y_scale)



class eq_widget:
    def __init__(self, a_number, a_freq_port, a_res_port, a_gain_port, a_rel_callback,
                 a_val_callback, a_default_value, a_port_dict=None, a_preset_mgr=None, a_size=48):
        self.groupbox = QtGui.QGroupBox("EQ{}".format(a_number))
        self.groupbox.setObjectName("plugin_groupbox")
        self.layout = QtGui.QGridLayout(self.groupbox)

        self.freq_knob = pydaw_knob_control(a_size, "Freq", a_freq_port, a_rel_callback,
                                            a_val_callback, 20.0, 120.0, a_default_value,
                                            kc_pitch, a_port_dict, a_preset_mgr)
        self.freq_knob.add_to_grid_layout(self.layout, 0)

        self.res_knob = pydaw_knob_control(a_size, "BW", a_res_port, a_rel_callback,
                                            a_val_callback, 100.0, 600.0, 300.0, kc_decimal,
                                            a_port_dict, a_preset_mgr)
        self.res_knob.add_to_grid_layout(self.layout, 1)

        self.gain_knob = pydaw_knob_control(a_size, _("Gain"), a_gain_port, a_rel_callback,
                                            a_val_callback, -24.0, 24.0, 0.0, kc_integer,
                                            a_port_dict, a_preset_mgr)
        self.gain_knob.add_to_grid_layout(self.layout, 2)


class eq6_widget:
    def __init__(self, a_first_port, a_rel_callback, a_val_callback,
                 a_port_dict=None, a_preset_mgr=None, a_size=48):
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.widget = QtGui.QWidget()
        self.widget.setObjectName("plugin_ui")
        self.vlayout = QtGui.QVBoxLayout(self.widget)
        self.combobox = pydaw_combobox_control(120, None, a_first_port, a_rel_callback,
                                               a_val_callback, ["Off", "Pre-FX", "Post-FX"],
                                               a_port_dict, 0, a_preset_mgr)
        self.combobox_hlayout = QtGui.QHBoxLayout()
        self.combobox_hlayout.addWidget(self.combobox.control)
        self.combobox_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.reset_button = QtGui.QPushButton(_("Reset"))
        self.combobox_hlayout.addWidget(self.reset_button)
        self.reset_button.pressed.connect(self.reset_controls)
        self.vlayout.addLayout(self.combobox_hlayout)

        self.eq_viewer = eq_viewer(a_val_callback)
        self.vlayout.addWidget(self.eq_viewer)

        self.grid_layout = QtGui.QGridLayout()
        self.vlayout.addLayout(self.grid_layout)
        self.eqs = []
        f_port = a_first_port + 1
        f_default_value = 24
        for f_i in range(1, 7):
            f_eq = eq_widget(f_i, f_port, f_port + 1, f_port + 2,
                             a_rel_callback, self.knob_callback,
                             f_default_value, a_port_dict, a_preset_mgr, a_size)
            self.eqs.append(f_eq)
            if f_i <= 3:
                self.grid_layout.addWidget(f_eq.groupbox, 0, f_i)
            else:
                self.grid_layout.addWidget(f_eq.groupbox, 1, f_i - 3)
            f_port += 3
            f_default_value += 18
        self.update_viewer()

    def knob_callback(self, a_port, a_val):
        self.val_callback(a_port, a_val)
        self.update_viewer()

    def update_viewer(self):
        self.eq_viewer.draw_eq(self.eqs)

    def reset_controls(self):
        for f_eq in self.eqs:
            f_eq.freq_knob.reset_default_value()
            f_eq.res_knob.reset_default_value()
            f_eq.gain_knob.reset_default_value()


# Custom oscillator widgets

class pydaw_abstract_custom_oscillator:
    def __init__(self):
        self.widget = QtGui.QWidget()
        self.widget.setObjectName("plugin_ui")
        self.layout = QtGui.QVBoxLayout(self.widget)

    def open_settings(self, a_settings):
        pass


global_additive_osc_height = 310
global_additive_osc_inc = 10
global_additive_max_y_pos = global_additive_osc_height - global_additive_osc_inc
global_additive_osc_harmonic_count = 32
global_additive_osc_bar_width = 20
global_additive_osc_width = global_additive_osc_harmonic_count * global_additive_osc_bar_width
global_additive_wavetable_size = 1024 #pow(2, global_additive_osc_harmonic_count)
#global_additive_osc_height_div2 = global_additive_osc_height * 0.5


global_add_osc_fill = QtGui.QLinearGradient(0.0, 0.0, 0.0, global_additive_osc_height)

global_add_osc_fill.setColorAt(0.0, QtGui.QColor(255, 0, 0, 90)) #red
global_add_osc_fill.setColorAt(0.14285, QtGui.QColor(255, 123, 0, 90)) #orange
global_add_osc_fill.setColorAt(0.2857, QtGui.QColor(255, 255, 0, 90)) #yellow
global_add_osc_fill.setColorAt(0.42855, QtGui.QColor(0, 255, 0, 90)) #green
global_add_osc_fill.setColorAt(0.5714, QtGui.QColor(0, 123, 255, 90)) #blue
global_add_osc_fill.setColorAt(0.71425, QtGui.QColor(0, 0, 255, 90)) #indigo
global_add_osc_fill.setColorAt(0.8571, QtGui.QColor(255, 0, 255, 90)) #violet

global_add_osc_background = QtGui.QLinearGradient(0.0, 0.0, 10.0, global_additive_osc_height)
global_add_osc_background.setColorAt(0.0, QtGui.QColor(40, 40, 40))
global_add_osc_background.setColorAt(0.2, QtGui.QColor(20, 20, 20))
global_add_osc_background.setColorAt(0.7, QtGui.QColor(30, 30, 30))
global_add_osc_background.setColorAt(1.0, QtGui.QColor(40, 40, 40))

global_add_osc_sine_cache = {}

def global_get_sine(a_size):
    if a_size in global_add_osc_sine_cache:
        return numpy.copy(global_add_osc_sine_cache[a_size])
    else:
        f_lin = numpy.linspace(0.0, 2.0 * numpy.pi, a_size)
        f_sin = numpy.sin(f_lin)
        global_add_osc_sine_cache[a_size] = f_sin
        return numpy.copy(f_sin)



class pydaw_additive_osc_amp_bar(QtGui.QGraphicsRectItem):
    def __init__(self, a_x_pos):
        QtGui.QGraphicsRectItem.__init__(self)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setBrush(global_add_osc_fill)
        self.setPen(QtGui.QPen(QtCore.Qt.white))
        self.x_pos = a_x_pos
        self.setPos(a_x_pos, global_additive_osc_height - global_additive_osc_inc)
        self.setRect(0.0, 0.0, global_additive_osc_bar_width, global_additive_osc_inc)
        self.value = -30
        self.extend_to_bottom()

    def set_value(self, a_value):
        if self.value != a_value:
            self.value = a_value
            f_y_pos = (a_value * global_additive_osc_inc * -1.0)
            self.setPos(self.x_pos, f_y_pos)
            self.extend_to_bottom()
            return True
        else:
            return False

    def get_value(self):
        return self.value

    def extend_to_bottom(self):
        f_pos_y = pydaw_util.pydaw_clip_value(round(self.pos().y(), -1), 10,
                                              global_additive_max_y_pos)
        self.setPos(self.x_pos, f_pos_y)
        self.setRect(0.0, 0.0, global_additive_osc_bar_width,
                     global_additive_osc_height - f_pos_y - 1.0)

class pydaw_additive_wav_viewer(QtGui.QGraphicsView):
    def __init__(self):
        QtGui.QGraphicsView.__init__(self)
        self.last_x_scale = 1.0
        self.last_y_scale = 1.0
        self.scene = QtGui.QGraphicsScene()
        self.setScene(self.scene)
        self.scene.setBackgroundBrush(global_add_osc_background)
        self.setSceneRect(0.0, 0.0, global_additive_wavetable_size,
                          global_additive_osc_height)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)

    def draw_array(self, a_np_array):
        f_path = QtGui.QPainterPath(QtCore.QPointF(0.0, global_additive_osc_height * 0.5))
        f_x = 1.0
        f_half = global_additive_osc_height * 0.5
        for f_point in a_np_array:
            f_path.lineTo(f_x, (f_point * f_half) + f_half)
            f_x += 1.0
        self.scene.clear()
        f_path_item = self.scene.addPath(f_path, QtGui.QPen(QtCore.Qt.white, 1.0))
        f_path_item.setBrush(global_add_osc_fill)

    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / global_additive_wavetable_size
        self.last_y_scale = f_rect.height() / global_additive_osc_height
        self.scale(self.last_x_scale, self.last_y_scale)


class pydaw_additive_osc_viewer(QtGui.QGraphicsView):
    def __init__(self, a_draw_callback, a_configure_callback=None):
        QtGui.QGraphicsView.__init__(self)
        self.configure_callback = a_configure_callback
        self.osc_num = 0
        self.draw_callback = a_draw_callback
        self.last_x_scale = 1.0
        self.last_y_scale = 1.0
        self.is_drawing = False
        self.edit_mode = 0
        self.setMinimumSize(global_additive_osc_width, global_additive_osc_height)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.scene = QtGui.QGraphicsScene()
        self.setScene(self.scene)
        self.scene.mousePressEvent = self.scene_mousePressEvent
        self.scene.mouseReleaseEvent = self.scene_mouseReleaseEvent
        self.scene.mouseMoveEvent = self.scene_mouseMoveEvent
        self.scene.setBackgroundBrush(global_add_osc_background)
        self.setSceneRect(0.0, 0.0, global_additive_osc_width, global_additive_osc_height)
        self.bars = []
        for f_i in range(0, global_additive_osc_width, int(global_additive_osc_bar_width)):
            f_bar = pydaw_additive_osc_amp_bar(f_i)
            self.bars.append(f_bar)
            self.scene.addItem(f_bar)

    def set_edit_mode(self, a_mode):
        self.edit_mode = a_mode

    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / global_additive_osc_width
        self.last_y_scale = f_rect.height() / global_additive_osc_height
        self.scale(self.last_x_scale, self.last_y_scale)

    def scene_mousePressEvent(self, a_event):
        QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)
        self.is_drawing = True
        self.draw_harmonics(a_event.scenePos())

    def scene_mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsScene.mouseReleaseEvent(self.scene, a_event)
        self.is_drawing = False
        self.get_wav(True)

    def get_wav(self, a_configure=False):
        f_result = numpy.zeros(global_additive_wavetable_size)
        f_recall_list = []
        for f_i in range(1, global_additive_osc_harmonic_count + 1):
            f_size = int(global_additive_wavetable_size / f_i)
            f_db = self.bars[f_i - 1].get_value()
            if a_configure:
                f_recall_list.append("{}".format(f_db))
            if f_db > -29:
                f_sin = global_get_sine(f_size) * pydaw_util.pydaw_db_to_lin(f_db)
                for f_i2 in range(int(global_additive_wavetable_size / f_size)):
                    f_start = (f_i2) * f_size
                    f_end = f_start + f_size
                    f_result[f_start:f_end] += f_sin
        f_max = numpy.max(numpy.abs(f_result), axis=0)
        if f_max > 0.0:
            f_normalize = 0.99 / f_max
            f_result *= f_normalize
        self.draw_callback(f_result)
        if a_configure and self.configure_callback is not None:
            f_engine_list = []
            for f_float in f_result:
                f_engine_list.append("{}".format(round(f_float, 6)))
            f_engine_str = "{}|{}".format(global_additive_wavetable_size,
                "|".join(f_engine_list))
            self.configure_callback("wayv_add_eng{}".format(self.osc_num), f_engine_str)
            f_recall_str = "|".join(f_recall_list)
            self.configure_callback("wayv_add_ui{}".format(self.osc_num), f_recall_str)

    def scene_mouseMoveEvent(self, a_event):
        if self.is_drawing:
            QtGui.QGraphicsScene.mouseMoveEvent(self.scene, a_event)
            self.draw_harmonics(a_event.scenePos())

    def draw_harmonics(self, a_pos):
        f_pos = a_pos
        f_pos_x = f_pos.x()
        f_pos_y = f_pos.y()
        f_db = (f_pos_y / global_additive_osc_height) * -30.0
        f_harmonic = int((f_pos_x / global_additive_osc_width) * \
            global_additive_osc_harmonic_count)
        if f_harmonic < 0:
            f_harmonic = 0
        elif f_harmonic >= global_additive_osc_harmonic_count:
            f_harmonic = global_additive_osc_harmonic_count - 1
        if self.edit_mode == 1 and (f_harmonic % 2) != 0:
            return
        if f_db > 0:
            f_db = 0
        elif f_db < -30:
            f_db = -30
        if self.bars[int(f_harmonic)].set_value(int(f_db)):
            self.get_wav()

    def clear_osc(self):
        for f_point in self.bars:
            f_point.set_value(-30)
        self.get_wav()

    def set_saw(self):
        f_db = 0
        for f_point in self.bars:
            f_point.set_value(f_db)
            f_db -= 1
        self.get_wav(True)

    def set_square(self):
        f_db = 0
        f_odd = True
        for f_point in self.bars:
            if f_odd:
                f_odd = False
                f_point.set_value(f_db)
                f_db -= 2
            else:
                f_odd = True
                f_point.set_value(-30)
        self.get_wav(True)

    def set_sine(self):
        self.bars[0].set_value(0)
        for f_point in self.bars[1:]:
            f_point.set_value(-30)
        self.get_wav(True)

    def open_osc(self, a_arr):
        for f_val, f_point in zip(a_arr, self.bars):
            f_point.set_value(int(f_val))
        self.get_wav()


class pydaw_custom_additive_oscillator(pydaw_abstract_custom_oscillator):
    def __init__(self, a_configure_callback=None, a_osc_count=3):
        pydaw_abstract_custom_oscillator.__init__(self)
        self.configure_callback = a_configure_callback
        self.hlayout = QtGui.QHBoxLayout()
        self.layout.addLayout(self.hlayout)
        self.hlayout.addWidget(QtGui.QLabel(_("Oscillator#:")))
        self.osc_num_combobox = QtGui.QComboBox()
        self.osc_num_combobox.setMinimumWidth(66)
        self.hlayout.addWidget(self.osc_num_combobox)
        for f_i in range(1, a_osc_count + 1):
            self.osc_num_combobox.addItem(str(f_i))
        self.osc_num_combobox.currentIndexChanged.connect(self.osc_index_changed)
        self.hlayout.addWidget(QtGui.QLabel(_("Edit Mode:")))
        self.edit_mode_combobox = QtGui.QComboBox()
        self.edit_mode_combobox.setMinimumWidth(90)
        self.hlayout.addWidget(self.edit_mode_combobox)
        self.edit_mode_combobox.addItems([_("All"), _("Odd")])
        self.edit_mode_combobox.currentIndexChanged.connect(
            self.edit_mode_combobox_changed)
        self.tools_button = QtGui.QPushButton(_("Tools"))
        self.hlayout.addWidget(self.tools_button)
        self.tools_menu = QtGui.QMenu(self.tools_button)
        self.tools_button.setMenu(self.tools_menu)

        self.hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.hlayout.addWidget(QtGui.QLabel(_("Select (Additive [n]) as your osc type to use")))
        self.wav_viewer = pydaw_additive_wav_viewer()
        self.viewer = pydaw_additive_osc_viewer(self.wav_viewer.draw_array,
                                                self.configure_wrapper)
        self.layout.addWidget(self.viewer)
        self.layout.addWidget(self.wav_viewer)

        f_saw_action = self.tools_menu.addAction(_("Set Saw"))
        f_saw_action.triggered.connect(self.viewer.set_saw)
        f_square_action = self.tools_menu.addAction(_("Set Square"))
        f_square_action.triggered.connect(self.viewer.set_square)
        f_sine_action = self.tools_menu.addAction(_("Set Sine"))
        f_sine_action.triggered.connect(self.viewer.set_sine)
        self.osc_values = {0 : None, 1 : None, 2 : None}

    def configure_wrapper(self, a_key, a_val):
        if self.configure_callback is not None:
            self.configure_callback(a_key, a_val)
        if a_key.startswith("wayv_add_ui"):
            self.osc_values[int(a_key[-1])] = a_val.split("|")

    def osc_index_changed(self, a_event):
        self.viewer.osc_num = self.osc_num_combobox.currentIndex()
        if self.osc_values[self.viewer.osc_num] is None:
            self.viewer.clear_osc()
        else:
            self.viewer.open_osc(self.osc_values[self.viewer.osc_num])

    def edit_mode_combobox_changed(self, a_event):
        self.viewer.set_edit_mode(self.edit_mode_combobox.currentIndex())

    def set_values(self, a_num, a_val):
        self.osc_values[int(a_num)] = a_val
        if self.osc_num_combobox.currentIndex() == int(a_num):
            self.osc_index_changed(None)



pydaw_audio_item_scene_height = 1200.0
pydaw_audio_item_scene_width = 6000.0
pydaw_audio_item_scene_rect = QtCore.QRectF(0.0, 0.0, pydaw_audio_item_scene_width,
                                            pydaw_audio_item_scene_height)

pydaw_start_end_gradient = QtGui.QLinearGradient(0.0, 0.0, 66.0, 66.0)
pydaw_start_end_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(246, 30, 30))
pydaw_start_end_gradient.setColorAt(1.0, QtGui.QColor.fromRgb(226, 42, 42))
pydaw_start_end_pen = QtGui.QPen(QtGui.QColor.fromRgb(246, 30, 30), 12.0)

pydaw_fade_pen = QtGui.QPen(QtGui.QColor.fromRgb(246, 30, 30), 6.0)

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
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, self.audio_item_marker_height,
                                         self.audio_item_marker_height)
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
        self.fade_marker = None
        if a_type == 0:
            self.min_x = 0.0
            self.y_pos = 0.0 + (a_offset * self.audio_item_marker_height)
            self.line.setPos(0.0, self.y_pos * -1.0)
        elif a_type == 1:
            self.min_x = 66.0
            self.y_pos = pydaw_audio_item_scene_height - self.audio_item_marker_height - \
            (a_offset * self.audio_item_marker_height)
            self.line.setPos(self.audio_item_marker_height, self.y_pos * -1.0)
        self.setPen(a_pen)
        self.setBrush(a_brush)
        self.text_item = QtGui.QGraphicsTextItem(a_label)
        self.text_item.setParentItem(self)
        self.text_item.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)

    def set_pos(self):
        if self.marker_type == 0:
            f_new_val = self.value * 6.0
        elif self.marker_type == 1:
            f_new_val = (self.value * 6.0) - self.audio_item_marker_height
        f_new_val = pydaw_util.pydaw_clip_value(f_new_val, self.min_x, self.max_x)
        self.setPos(f_new_val, self.y_pos)

    def set_other(self, a_other, a_fade_marker=None):
        self.other = a_other
        self.fade_marker = a_fade_marker

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        self.pos_x = a_event.scenePos().x()
        self.pos_x = pydaw_util.pydaw_clip_value(self.pos_x, self.min_x, self.max_x)
        self.setPos(self.pos_x, self.y_pos)
        if self.marker_type == 0:
            f_new_val = self.pos_x * .1666666
            if self.fade_marker is not None and self.fade_marker.pos().x() < self.pos_x:
                self.fade_marker.value = f_new_val
                self.fade_marker.set_pos()
        elif self.marker_type == 1:
            f_new_val = (self.pos_x + self.audio_item_marker_height) * .1666666
            if self.fade_marker is not None and self.fade_marker.pos().x() > self.pos_x:
                self.fade_marker.value = f_new_val
                self.fade_marker.set_pos()
        f_new_val = pydaw_util.pydaw_clip_value(f_new_val, 0.0, 994.0)
        self.value = f_new_val
        if self.other is not None:
            if self.marker_type == 0:
                if self.value > self.other.value - 6.0:
                    self.other.value = self.value + 6.0
                    self.other.value = pydaw_util.pydaw_clip_value(self.other.value,
                                                                   6.0, 1000.0, a_round=True)
                    self.other.set_pos()
            elif self.marker_type == 1:
                if self.other.value > self.value - 6.0:
                    self.other.value = self.value - 6.0
                    self.other.value = pydaw_util.pydaw_clip_value(self.other.value,
                                                                   0.0, 994.0, a_round=True)
                    self.other.set_pos()
        if self.fade_marker is not None:
            self.fade_marker.draw_lines()

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        if self.callback is not None:
            self.callback(self.value)
        if self.other.callback is not None:
            self.other.callback(self.other.value)
        if self.fade_marker is not None:
            self.fade_marker.callback(self.fade_marker.value)


class pydaw_audio_fade_marker_widget(QtGui.QGraphicsRectItem):
    def __init__(self, a_type, a_val, a_pen, a_brush, a_label, a_channel_count,
                 a_offset=0, a_callback=None):
        """ a_type:  0 == start, 1 == end, more types eventually... """
        self.audio_item_marker_height = 66.0
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, self.audio_item_marker_height,
                                         self.audio_item_marker_height)
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
        self.start_end_marker = None
        if a_type == 0:
            self.min_x = 0.0
            self.y_pos = 0.0 + (a_offset * self.audio_item_marker_height)
            self.line.setPos(0.0, self.y_pos * -1.0)
        elif a_type == 1:
            self.min_x = 66.0
            self.y_pos = pydaw_audio_item_scene_height - self.audio_item_marker_height - \
            (a_offset * self.audio_item_marker_height)
            self.line.setPos(self.audio_item_marker_height, self.y_pos * -1.0)
        self.setPen(a_pen)
        self.setBrush(a_brush)
        self.text_item = QtGui.QGraphicsTextItem(a_label)
        self.text_item.setParentItem(self)
        self.text_item.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
        self.amp_lines = []
        self.channel_count = a_channel_count
        for f_i in range(a_channel_count * 2):
            f_line = QtGui.QGraphicsLineItem()
            self.amp_lines.append(f_line)
            f_line.setPen(pydaw_fade_pen)

    def draw_lines(self):
        f_inc = pydaw_audio_item_scene_height / float(len(self.amp_lines))
        f_y_pos = 0
        f_x_inc = 0
        if self.marker_type == 0:
            f_x_list = [self.scenePos().x(), self.start_end_marker.scenePos().x()]
        elif self.marker_type == 1:
            f_x_list = [self.scenePos().x() + self.audio_item_marker_height,
                        self.start_end_marker.scenePos().x() + self.audio_item_marker_height]
        for f_line in self.amp_lines:
            if f_x_inc == 0:
                f_line.setLine(f_x_list[0], f_y_pos, f_x_list[1], f_y_pos + f_inc)
            else:
                f_line.setLine(f_x_list[1], f_y_pos, f_x_list[0], f_y_pos + f_inc)
            f_y_pos += f_inc
            f_x_inc += 1
            if f_x_inc > 1:
                f_x_inc = 0

    def set_pos(self):
        if self.marker_type == 0:
            f_new_val = self.value * 6.0
        elif self.marker_type == 1:
            f_new_val = (self.value * 6.0) - self.audio_item_marker_height
        f_new_val = pydaw_util.pydaw_clip_value(f_new_val, self.min_x, self.max_x)
        self.setPos(f_new_val, self.y_pos)

    def set_other(self, a_other, a_start_end_marker):
        self.other = a_other
        self.start_end_marker = a_start_end_marker

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        self.pos_x = a_event.scenePos().x()
        self.pos_x = pydaw_util.pydaw_clip_value(self.pos_x, self.min_x, self.max_x)
        if self.marker_type == 0:
            self.pos_x = pydaw_util.pydaw_clip_max(self.pos_x, self.other.scenePos().x())
        elif self.marker_type == 1:
            self.pos_x = pydaw_util.pydaw_clip_min(self.pos_x, self.other.scenePos().x())
        self.setPos(self.pos_x, self.y_pos)
        if self.marker_type == 0:
            f_new_val = self.pos_x * .1666666
            if self.pos_x < self.start_end_marker.scenePos().x():
                self.start_end_marker.value = f_new_val
                self.start_end_marker.set_pos()
        elif self.marker_type == 1:
            f_new_val = (self.pos_x + self.audio_item_marker_height) * .1666666
            if self.pos_x > self.start_end_marker.scenePos().x():
                self.start_end_marker.value = f_new_val
                self.start_end_marker.set_pos()
        f_new_val = pydaw_util.pydaw_clip_value(f_new_val, 0.0, 1000.0)
        self.value = f_new_val
        self.draw_lines()

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        if self.callback is not None:
            self.callback(self.value)
        if self.start_end_marker is not None:
            self.start_end_marker.callback(self.start_end_marker.value)


class pydaw_audio_item_viewer_widget(QtGui.QGraphicsView):
    def __init__(self, a_start_callback, a_end_callback, a_loop_start_callback,
                 a_loop_end_callback, a_fade_in_callback, a_fade_out_callback):
        QtGui.QGraphicsView.__init__(self)
        self.start_callback = a_start_callback
        self.end_callback = a_end_callback
        self.loop_start_callback = a_loop_start_callback
        self.loop_end_callback = a_loop_end_callback
        self.fade_in_callback = a_fade_in_callback
        self.fade_out_callback = a_fade_out_callback
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

    def draw_item(self, a_path_list, a_start, a_end, a_loop_start, a_loop_end,
                  a_fade_in, a_fade_out):
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
        self.start_marker = pydaw_audio_marker_widget(0, a_start, pydaw_start_end_pen,
                                                      pydaw_start_end_gradient,
                                                      "S", 1, self.start_callback)
        self.scene.addItem(self.start_marker)
        self.end_marker = pydaw_audio_marker_widget(1, a_end, pydaw_start_end_pen,
                                                    pydaw_start_end_gradient, "E", 1,
                                                    self.end_callback)
        self.scene.addItem(self.end_marker)
        self.loop_start_marker = pydaw_audio_marker_widget(0, a_loop_start, pydaw_loop_pen,
                                                           pydaw_loop_gradient, "L",
                                                           2, self.loop_start_callback)
        self.scene.addItem(self.loop_start_marker)
        self.loop_end_marker = pydaw_audio_marker_widget(1, a_loop_end, pydaw_loop_pen,
                                                         pydaw_loop_gradient, "L", 2,
                                                         self.loop_end_callback)
        self.scene.addItem(self.loop_end_marker)
        #new:  fade stuff
        self.fade_in_marker = pydaw_audio_fade_marker_widget(0, a_fade_in, pydaw_start_end_pen,
                                                             pydaw_start_end_gradient,
                                                             "I", len(a_path_list), 0,
                                                             self.fade_in_callback)
        self.scene.addItem(self.fade_in_marker)
        for f_line in self.fade_in_marker.amp_lines:
            self.scene.addItem(f_line)
        self.fade_out_marker = pydaw_audio_fade_marker_widget(1, a_fade_out, pydaw_start_end_pen,
                                                              pydaw_start_end_gradient, "O",
                                                              len(a_path_list), 0,
                                                              self.fade_out_callback)
        self.scene.addItem(self.fade_out_marker)
        for f_line in self.fade_out_marker.amp_lines:
            self.scene.addItem(f_line)
        self.fade_in_marker.set_other(self.fade_out_marker, self.start_marker)
        self.fade_out_marker.set_other(self.fade_in_marker, self.end_marker)
        #end fade stuff
        self.start_marker.set_other(self.end_marker, self.fade_in_marker)
        self.end_marker.set_other(self.start_marker, self.fade_out_marker)
        self.loop_start_marker.set_other(self.loop_end_marker)
        self.loop_end_marker.set_other(self.loop_start_marker)
        self.start_marker.set_pos()
        self.end_marker.set_pos()
        self.loop_start_marker.set_pos()
        self.loop_end_marker.set_pos()
        self.fade_in_marker.set_pos()
        self.fade_out_marker.set_pos()
        self.fade_in_marker.draw_lines()
        self.fade_out_marker.draw_lines()

    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / pydaw_audio_item_scene_width
        self.last_y_scale = (f_rect.height() - self.scroll_bar_height) / \
            pydaw_audio_item_scene_height
        self.scale(self.last_x_scale, self.last_y_scale)


global_modulex_clipboard = None

class pydaw_modulex_single:
    def __init__(self, a_title, a_port_k1, a_rel_callback, a_val_callback,
                 a_port_dict=None, a_preset_mgr=None, a_knob_size=51):
        self.group_box = QtGui.QGroupBox()
        self.group_box.contextMenuEvent = self.contextMenuEvent
        self.group_box.setObjectName("plugin_groupbox")
        if a_title is not None:
            self.group_box.setTitle(str(a_title))
        self.layout = QtGui.QGridLayout()
        self.group_box.setLayout(self.layout)
        self.knobs = []
        for f_i in range(3):
            f_knob = pydaw_knob_control(a_knob_size, "", a_port_k1 + f_i,
                                        a_rel_callback, a_val_callback, 0, 127, 64,
                                        a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr)
            f_knob.add_to_grid_layout(self.layout, f_i)
            self.knobs.append(f_knob)
        self.combobox = pydaw_combobox_control(
            132, "Type", a_port_k1 + 3, a_rel_callback, a_val_callback,
               ["Off", "LP2" , "LP4", "HP2", "HP4", "BP2", "BP4" , "Notch2",
               "Notch4", "EQ" , "Distortion", "Comb Filter", "Amp/Pan",
               "Limiter" , "Saturator", "Formant", "Chorus", "Glitch" ,
               "RingMod", "LoFi", "S/H", "LP-Dry/Wet" , "HP-Dry/Wet",
               "Monofier", "LP<-->HP", "Growl Filter",
                "Screech LP", "Metal Comb"],
                a_port_dict=a_port_dict, a_preset_mgr=a_preset_mgr, a_default_index=0)
        self.layout.addWidget(self.combobox.name_label, 0, 3)
        self.combobox.control.currentIndexChanged.connect(self.type_combobox_changed)
        self.layout.addWidget(self.combobox.control, 1, 3)

    def contextMenuEvent(self, a_event):
        f_menu = QtGui.QMenu(self.group_box)
        f_copy_action = QtGui.QAction("Copy", self.group_box)
        f_copy_action.triggered.connect(self.copy_settings)
        f_menu.addAction(f_copy_action)
        f_paste_action = QtGui.QAction("Paste", self.group_box)
        f_paste_action.triggered.connect(self.paste_settings)
        f_menu.addAction(f_paste_action)
        f_paste_and_copy_action = QtGui.QAction("Paste and Copy", self.group_box)
        f_paste_and_copy_action.triggered.connect(self.paste_and_copy)
        f_menu.addAction(f_paste_and_copy_action)
        f_reset_action = QtGui.QAction("Reset", self.group_box)
        f_reset_action.triggered.connect(self.reset_settings)
        f_menu.addAction(f_reset_action)
        f_menu.exec_(QtGui.QCursor.pos())

    def copy_settings(self):
        global global_modulex_clipboard
        global_modulex_clipboard = self.get_class()

    def paste_and_copy(self):
        """ Copy the existing setting and then paste, for rearranging effects """
        self.paste_settings(True)

    def paste_settings(self, a_copy=False):
        global global_modulex_clipboard
        if global_modulex_clipboard is None:
            QtGui.QMessageBox.warning(self.group_box, "Error",
            "Nothing copied to clipboard")
        else:
            f_class = self.get_class()
            self.set_from_class(global_modulex_clipboard)
            self.update_all_values()
            if a_copy:
                global_modulex_clipboard = f_class

    def update_all_values(self):
        for f_knob in self.knobs:
            f_knob.control_value_changed(f_knob.get_value())
        self.combobox.control_value_changed(self.combobox.get_value())

    def reset_settings(self):
        self.set_from_class(pydaw_audio_item_fx(64, 64, 64, 0))
        self.update_all_values()

    def set_from_class(self, a_class):
        """ a_class is a pydaw_audio_item_fx instance """
        self.knobs[0].set_value(a_class.knobs[0])
        self.knobs[1].set_value(a_class.knobs[1])
        self.knobs[2].set_value(a_class.knobs[2])
        self.combobox.set_value(a_class.fx_type)

    def get_class(self):
        """ return a pydaw_audio_item_fx instance """
        return pydaw_audio_item_fx(self.knobs[0].control.value(),
                                   self.knobs[1].control.value(),
                                   self.knobs[2].control.value(),
                                   self.combobox.control.currentIndex())

    def type_combobox_changed(self, a_val):
        if a_val == 0: #Off
            self.knobs[0].name_label.setText("")
            self.knobs[1].name_label.setText("")
            self.knobs[2].name_label.setText("")
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_none
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].value_label.setText("")
            self.knobs[2].value_label.setText("")
        elif a_val == 1: #LP2
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 2: #LP4
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 3: #HP2
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 4: #HP4
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 5: #BP2
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 6: #BP4
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 7: #Notch2
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 8: #Notch4
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 9: #EQ
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Q"))
            self.knobs[2].name_label.setText(_("Gain"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_none
            self.knobs[2].val_conversion = kc_127_zero_to_x
            self.knobs[2].set_127_min_max(-24.0, 24.0)
            self.knobs[1].value_label.setText("")
        elif a_val == 10: #Distortion
            self.knobs[0].name_label.setText(_("Gain"))
            self.knobs[1].name_label.setText(_("Dry/Wet"))
            self.knobs[2].name_label.setText(_("Out Gain"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(0.0, 48.0)
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_127_zero_to_x
            self.knobs[2].set_127_min_max(-30.0, 0.0)
        elif a_val == 11: #Comb Filter
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Amt"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_none
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].value_label.setText("")
            self.knobs[2].value_label.setText("")
        elif a_val == 12: #Amp/Panner
            self.knobs[0].name_label.setText(_("Pan"))
            self.knobs[1].name_label.setText(_("Amp"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-40.0, 24.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].value_label.setText("")
            self.knobs[2].value_label.setText("")
        elif a_val == 13: #Limiter
            self.knobs[0].name_label.setText(_("Thresh"))
            self.knobs[1].name_label.setText(_("Ceiling"))
            self.knobs[2].name_label.setText(_("Release"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(-30.0, 0.0)
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-12.0, -0.1)
            self.knobs[2].val_conversion = kc_127_zero_to_x_int
            self.knobs[2].set_127_min_max(150.0, 400.0)
        elif a_val == 14: #Saturator
            self.knobs[0].name_label.setText(_("InGain"))
            self.knobs[1].name_label.setText(_("Amt"))
            self.knobs[2].name_label.setText(_("OutGain"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(-12.0, 12.0)
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_127_zero_to_x
            self.knobs[2].set_127_min_max(-12.0, 12.0)
        elif a_val == 15: #Formant Filter
            self.knobs[0].name_label.setText(_("Vowel"))
            self.knobs[1].name_label.setText(_("Wet"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 16: #Chorus
            self.knobs[0].name_label.setText(_("Rate"))
            self.knobs[1].name_label.setText(_("Wet"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(0.3, 6.0)
            self.knobs[0].value_label.setText("")
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 17: #Glitch
            self.knobs[0].name_label.setText(_("Pitch"))
            self.knobs[1].name_label.setText(_("Glitch"))
            self.knobs[2].name_label.setText(_("Wet"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 18: #RingMod
            self.knobs[0].name_label.setText(_("Pitch"))
            self.knobs[1].name_label.setText(_("Wet"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 19: #LoFi
            self.knobs[0].name_label.setText(_("Bits"))
            self.knobs[1].name_label.setText(_("unused"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_zero_to_x
            self.knobs[0].set_127_min_max(4.0, 16.0)
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 20: #Sample and Hold
            self.knobs[0].name_label.setText(_("Pitch"))
            self.knobs[1].name_label.setText(_("Wet"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 21: #LP2-Dry/Wet
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("Wet"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 22: #HP2-Dry/Wet
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("Wet"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 23: #Monofier
            self.knobs[0].name_label.setText(_("Pan"))
            self.knobs[1].name_label.setText(_("Amp"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 6.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].value_label.setText("")
            self.knobs[2].value_label.setText("")
        elif a_val == 24: #LP<-->HP
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(("LP<->HP"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 25: #Growl Filter
            self.knobs[0].name_label.setText(_("Vowel"))
            self.knobs[1].name_label.setText(_("Wet"))
            self.knobs[2].name_label.setText(_("Type"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].val_conversion = kc_none
            self.knobs[1].value_label.setText("")
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 26: #Screech LP
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Res"))
            self.knobs[2].name_label.setText(_("unused"))
            self.knobs[0].val_conversion = kc_127_pitch
            self.knobs[1].val_conversion = kc_127_zero_to_x
            self.knobs[1].set_127_min_max(-30.0, 0.0)
            self.knobs[2].val_conversion = kc_none
            self.knobs[2].value_label.setText("")
        elif a_val == 27: #Metal Comb
            self.knobs[0].name_label.setText(_("Cutoff"))
            self.knobs[1].name_label.setText(_("Amt"))
            self.knobs[2].name_label.setText(_("Detune"))
            self.knobs[0].val_conversion = kc_none
            self.knobs[1].val_conversion = kc_none
            self.knobs[2].val_conversion = kc_none
            self.knobs[0].value_label.setText("")
            self.knobs[1].value_label.setText("")
            self.knobs[2].value_label.setText("")

        self.knobs[0].set_value(self.knobs[0].control.value())
        self.knobs[1].set_value(self.knobs[1].control.value())
        self.knobs[2].set_value(self.knobs[2].control.value())


class pydaw_per_audio_item_fx_widget:
    def __init__(self, a_rel_callback, a_val_callback):
        self.effects = []
        self.widget = QtGui.QWidget()
        self.widget.setObjectName("plugin_ui")
        self.layout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.layout)
        f_port = 0
        for f_i in range(8):
            f_effect = pydaw_modulex_single(_("FX{}").format(f_i), f_port,
                                            a_rel_callback, a_val_callback)
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
    def __init__(self, a_rel_callback, a_val_callback, a_track_num,
                 a_project, a_track_type, a_stylesheet, a_close_callback,
                 a_configure_callback, a_can_resize=False):
        self.can_resize = a_can_resize
        self.track_num = int(a_track_num)
        self.pydaw_project = a_project
        self.rel_callback = a_rel_callback
        self.val_callback = a_val_callback
        self.configure_callback = a_configure_callback
        self.track_type = int(a_track_type)
        self.widget = QtGui.QScrollArea()
        self.widget.setObjectName("plugin_ui")
        self.widget.setMinimumSize(500, 500)
        self.widget.setStyleSheet(str(a_stylesheet))
        self.widget.closeEvent = self.widget_close_event

        self.widget.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.widget.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.scrollarea_widget = QtGui.QWidget()
        self.scrollarea_widget.setObjectName("plugin_ui")
        self.widget.setWidgetResizable(True)
        self.widget.setWidget(self.scrollarea_widget)

        self.layout = QtGui.QVBoxLayout()
        self.scrollarea_widget.setLayout(self.layout)
        self.port_dict = {}
        self.effects = []
        self.close_callback = a_close_callback
        self.configure_dict = {}
        self.save_file_on_exit = True
        self.is_quitting = False

    def set_default_size(self):
        """ Override this for plugins that can't properly resize automatically and
            can be resized
        """
        pass

    def delete_plugin_file(self):
        self.save_file_on_exit = False

    def show_widget(self):
        self.layout.update()
        self.layout.activate()
        f_size = self.scrollarea_widget.size()
        f_desktop_size = QtGui.QApplication.desktop().screen().rect()

        f_x = f_size.width() + 21
        f_y = f_size.height()

        if self.can_resize or \
        f_x > f_desktop_size.width() - 40 or \
        f_y > f_desktop_size.height() - 40:
            f_y += 21
            f_x = pydaw_util.pydaw_clip_value(f_x, 400, f_desktop_size.width())
            f_y = pydaw_util.pydaw_clip_value(f_y, 400, f_desktop_size.height())
            self.widget.resize(f_x, f_y)
            self.set_default_size()
        else:
            self.widget.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
            self.widget.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
            self.widget.setFixedSize(f_x, f_y)
        self.widget.show()

    def open_plugin_file(self):
        f_file_path = "{}/{}/{}".format(self.pydaw_project.project_folder, self.folder, self.file)
        if os.path.isfile(f_file_path):
            f_file = pydaw_plugin_file(f_file_path)
            for k, v in list(f_file.port_dict.items()):
                self.set_control_val(int(k), v)
            for k, v in list(f_file.configure_dict.items()):
                self.set_configure(k, v)
        else:
            print("pydaw_abstract_plugin_ui.open_plugin_file():"
                " '{}' did not exist, not loading.".format(f_file_path))

    def save_plugin_file(self):
        f_file = pydaw_plugin_file.from_dict(self.port_dict, self.configure_dict)
        self.pydaw_project.save_file(self.folder, self.file, str(f_file))
        self.pydaw_project.commit(_("Update controls for {}").format(self.track_name))
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
            print("pydaw_abstract_plugin_ui.set_control_val():  "
                "Did not have port {}".format(f_port))

    def configure_plugin(self, a_key, a_message):
        """ Override this function to allow str|str key/value pair
            messages to be sent to the back-end
        """
        pass

    def set_configure(self, a_key, a_message):
        """ Override this function to configure the plugin from the state file """
        pass

    def reconfigure_plugin(self, a_dict):
        """ Override this to re-configure a plugin from scratch with the
            values in a_dict
        """
        pass

    def set_window_title(self, a_track_name):
        pass  #Override this function



class pydaw_modulex_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project,
                 a_folder, a_track_type, a_track_name, a_stylesheet,
                 a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback,
                                          a_track_num, a_project, a_track_type,
                                          a_stylesheet, a_close_callback,
                                          a_configure_callback)
        self.folder = str(a_folder)
        self.file =  "{}.pyfx".format(self.track_num)
        self.set_window_title(a_track_name)
        self.is_instrument = False

        self.preset_manager =  pydaw_preset_manager_widget("MODULEX")
        self.presets_hlayout = QtGui.QHBoxLayout()
        self.presets_hlayout.addWidget(self.preset_manager.group_box)
        self.presets_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.layout.addLayout(self.presets_hlayout)
        self.tab_widget = QtGui.QTabWidget()
        self.layout.addWidget(self.tab_widget)
        self.layout.setSizeConstraint(QtGui.QLayout.SetFixedSize)

        self.fx_tab = QtGui.QWidget()
        self.tab_widget.addTab(self.fx_tab, _("Effects"))
        self.fx_layout = QtGui.QGridLayout()
        self.fx_hlayout = QtGui.QHBoxLayout(self.fx_tab)
        self.fx_hlayout.addLayout(self.fx_layout)

        self.delay_tab = QtGui.QWidget()
        self.tab_widget.addTab(self.delay_tab, _("Delay"))
        self.delay_vlayout = QtGui.QVBoxLayout()
        self.delay_tab.setLayout(self.delay_vlayout)
        self.delay_hlayout = QtGui.QHBoxLayout()
        self.delay_vlayout.addLayout(self.delay_hlayout)

        self.delay_groupbox = QtGui.QGroupBox(_("Delay"))
        self.delay_groupbox_layout = QtGui.QGridLayout(self.delay_groupbox)
        f_knob_size = 48

        f_port = 4
        f_column = 0
        f_row = 0
        for f_i in range(8):
            f_effect = pydaw_modulex_single("FX{}".format(f_i), f_port,
                                            self.plugin_rel_callback,
                                            self.plugin_val_callback,
                                            self.port_dict, self.preset_manager,
                                            a_knob_size=f_knob_size)
            self.effects.append(f_effect)
            self.fx_layout.addWidget(f_effect.group_box, f_row, f_column)
            f_column += 1
            if f_column > 1:
                f_column = 0
                f_row += 1
            f_port += 4

        self.volume_gridlayout = QtGui.QGridLayout()
        self.fx_hlayout.addLayout(self.volume_gridlayout)
        self.volume_slider = pydaw_slider_control(QtCore.Qt.Vertical, "Vol",
                                                  pydaw_ports.MODULEX_VOL_SLIDER,
                                                  self.plugin_rel_callback,
                                                  self.plugin_val_callback,
                                                  -50, 0, 0,
                                                  kc_integer, self.port_dict)
        self.volume_slider.add_to_grid_layout(self.volume_gridlayout, 0)

        delay_groupbox =  QtGui.QGroupBox(_("Delay"))
        delay_groupbox.setObjectName("plugin_groupbox")
        delay_groupbox.setMaximumSize(387, 126)
        self.delay_hlayout.addWidget(delay_groupbox)
        delay_groupbox.setGeometry(0, 0, 10, 10)
        delay_gridlayout = QtGui.QGridLayout(delay_groupbox)
        self.delay_hlayout.addWidget(delay_groupbox)
        self.delay_time_knob =  pydaw_knob_control(f_knob_size, _("Time"),
                                                   pydaw_ports.MODULEX_DELAY_TIME,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   10, 100, 50,
                                                   kc_time_decimal, self.port_dict,
                                                   self.preset_manager)
        self.delay_time_knob.add_to_grid_layout(delay_gridlayout, 0)
        m_feedback =  pydaw_knob_control(f_knob_size, _("Fdbk"), pydaw_ports.MODULEX_FEEDBACK,
                                         self.plugin_rel_callback, self.plugin_val_callback,
                                         -20, 0, -12, kc_integer,
                                         self.port_dict, self.preset_manager)
        m_feedback.add_to_grid_layout(delay_gridlayout, 1)
        m_dry =  pydaw_knob_control(f_knob_size, _("Dry"), pydaw_ports.MODULEX_DRY,
                                    self.plugin_rel_callback, self.plugin_val_callback,
                                    -30, 0, 0, kc_integer, self.port_dict,
                                    self.preset_manager)
        m_dry.add_to_grid_layout(delay_gridlayout, 2)
        m_wet =  pydaw_knob_control(f_knob_size, _("Wet"), pydaw_ports.MODULEX_WET,
                                    self.plugin_rel_callback,
                                    self.plugin_val_callback, -30, 0, -30,
                                    kc_integer, self.port_dict,
                                    self.preset_manager)
        m_wet.add_to_grid_layout(delay_gridlayout, 3)
        m_duck =  pydaw_knob_control(f_knob_size, _("Duck"), pydaw_ports.MODULEX_DUCK,
                                     self.plugin_rel_callback, self.plugin_val_callback,
                                     -40, 0, 0, kc_integer, self.port_dict,
                                     self.preset_manager)
        m_duck.add_to_grid_layout(delay_gridlayout, 4)
        m_cutoff =  pydaw_knob_control(f_knob_size, _("Cutoff"), pydaw_ports.MODULEX_CUTOFF,
                                       self.plugin_rel_callback, self.plugin_val_callback,
                                       40, 118, 90, kc_pitch, self.port_dict,
                                       self.preset_manager)
        m_cutoff.add_to_grid_layout(delay_gridlayout, 5)
        m_stereo =  pydaw_knob_control(f_knob_size, _("Stereo"), pydaw_ports.MODULEX_STEREO,
                                       self.plugin_rel_callback, self.plugin_val_callback,
                                       0, 100, 100, kc_decimal, self.port_dict,
                                       self.preset_manager)
        m_stereo.add_to_grid_layout(delay_gridlayout, 6)
        self.bpm_groupbox =  QtGui.QGroupBox()
        self.bpm_groupbox.setObjectName("plugin_groupbox")
        self.delay_hlayout.addWidget(self.bpm_groupbox)
        self.delay_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.bpm_groupbox.setGeometry(0, 0, 10, 10)
        self.bpm_groupbox.setTitle(_("Tempo Sync"))
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
        self.bpm_sync_button.setText(_("Sync"))
        self.bpm_sync_button.setMaximumWidth(60)
        self.bpm_sync_button.pressed.connect(self.bpmSyncPressed)
        f_bpm_label =  QtGui.QLabel(_("BPM"))
        f_bpm_label.setMinimumWidth(60)
        f_beat_label =  QtGui.QLabel(_("Beats"))
        f_beat_label.setMinimumWidth(60)
        self.bpm_groupbox_layout.addWidget(f_bpm_label, 0, 0, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(self.bpm_spinbox, 1, 0, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(f_beat_label, 0, 1, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(self.beat_frac_combobox, 1, 1, QtCore.Qt.AlignCenter)
        self.bpm_groupbox_layout.addWidget(self.bpm_sync_button, 2, 1, QtCore.Qt.AlignCenter)
        reverb_groupbox =  QtGui.QGroupBox(_("Reverb"))
        reverb_groupbox.setObjectName("plugin_groupbox")
        reverb_groupbox.setMaximumSize(186, 126)
        self.reverb_groupbox_gridlayout = QtGui.QGridLayout(reverb_groupbox)
        self.reverb_hlayout = QtGui.QHBoxLayout()
        self.delay_vlayout.addLayout(self.reverb_hlayout)
        self.reverb_hlayout.addWidget(reverb_groupbox)
        self.reverb_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        m_reverb_time =  pydaw_knob_control(f_knob_size, _("Time"), pydaw_ports.MODULEX_REVERB_TIME,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 50,
                                            kc_decimal, self.port_dict, self.preset_manager)
        m_reverb_time.add_to_grid_layout(self.reverb_groupbox_gridlayout, 0)
        m_reverb_wet =  pydaw_knob_control(f_knob_size, _("Wet"), pydaw_ports.MODULEX_REVERB_WET,
                                           self.plugin_rel_callback, self.plugin_val_callback,
                                           0, 100, 0, kc_decimal,
                                           self.port_dict, self.preset_manager)
        m_reverb_wet.add_to_grid_layout(self.reverb_groupbox_gridlayout, 1)
        m_reverb_color =  pydaw_knob_control(f_knob_size, _("Color"),
                                             pydaw_ports.MODULEX_REVERB_COLOR,
                                             self.plugin_rel_callback, self.plugin_val_callback,
                                             0, 100, 50,
                                             kc_decimal, self.port_dict, self.preset_manager)
        m_reverb_color.add_to_grid_layout(self.reverb_groupbox_gridlayout, 2)
        self.delay_spacer_layout = QtGui.QVBoxLayout()
        self.delay_vlayout.addLayout(self.delay_spacer_layout)
        self.delay_spacer_layout.addItem(
            QtGui.QSpacerItem(1, 1, vPolicy=QtGui.QSizePolicy.Expanding))

        self.eq6 = eq6_widget(pydaw_ports.MODULEX_EQ_ON,
                              self.plugin_rel_callback, self.plugin_val_callback,
                              self.port_dict, a_preset_mgr=self.preset_manager,
                              a_size=f_knob_size)

        self.tab_widget.addTab(self.eq6.widget, _("EQ"))

        self.open_plugin_file()

    def open_plugin_file(self):
        pydaw_abstract_plugin_ui.open_plugin_file(self)
        self.eq6.update_viewer()

    def bpmSyncPressed(self):
        f_frac = 1.0
        f_switch = (self.beat_frac_combobox.currentIndex())
        f_dict = {0 : 0.25, 1 : 0.33333, 2 : 0.5, 3 : 0.666666, 4 : 0.75, 5 : 1.0}
        f_frac = f_dict[f_switch]
        f_seconds_per_beat = 60/(self.bpm_spinbox.value())
        f_result = int(f_seconds_per_beat * f_frac * 100)
        self.delay_time_knob.set_value(f_result)

    def set_window_title(self, a_track_name):
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Modulex - {}".format(self.track_name))


class pydaw_rayv_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project,
                 a_folder, a_track_type, a_track_name,
                 a_stylesheet, a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback,
                                          a_track_num, a_project, a_track_type,
                                          a_stylesheet, a_close_callback, a_configure_callback)
        self.folder = str(a_folder)
        self.file = "{}.pyinst".format(self.track_num)
        self.set_window_title(a_track_name)
        self.is_instrument = True
        f_osc_types = [_("Saw"), _("Square"), _("Triangle"), _("Sine"), _("Off")]
        f_lfo_types = [_("Off"), _("Sine"), _("Triangle")]
        self.preset_manager =  pydaw_preset_manager_widget("RAYV")
        self.main_layout = QtGui.QVBoxLayout()
        self.layout.addLayout(self.main_layout)
        self.layout.setSizeConstraint(QtGui.QLayout.SetFixedSize)
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout0)
        self.hlayout0.addWidget(self.preset_manager.group_box)
        self.hlayout0.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        f_logo_label =  QtGui.QLabel()
        f_pixmap = QtGui.QPixmap(
            "{}/lib/{}/themes/default/rayv.png".format(pydaw_util.global_pydaw_install_prefix,
            pydaw_util.global_pydaw_version_string)).scaled(
                120, 60, transformMode=QtCore.Qt.SmoothTransformation)
        f_logo_label.setMinimumSize(90, 30)
        f_logo_label.setPixmap(f_pixmap)
        f_knob_size = 55

        self.hlayout0.addWidget(f_logo_label)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout1)
        self.osc1 =  pydaw_osc_widget(f_knob_size, pydaw_ports.RAYV_OSC1_PITCH,
                                      pydaw_ports.RAYV_OSC1_TUNE, pydaw_ports.RAYV_OSC1_VOLUME,
                                      pydaw_ports.RAYV_OSC1_TYPE, f_osc_types,
                                      self.plugin_rel_callback, self.plugin_val_callback,
                                      _("Oscillator 1"), self.port_dict,
                                      a_preset_mgr=self.preset_manager)
        self.hlayout1.addWidget(self.osc1.group_box)
        self.adsr_amp = pydaw_adsr_widget(f_knob_size, True,
                                          pydaw_ports.RAYV_ATTACK, pydaw_ports.RAYV_DECAY,
                                          pydaw_ports.RAYV_SUSTAIN, pydaw_ports.RAYV_RELEASE,
                                          _("ADSR Amp"),
                                          self.plugin_rel_callback, self.plugin_val_callback,
                                          self.port_dict, self.preset_manager)
        self.hlayout1.addWidget(self.adsr_amp.groupbox)
        self.groupbox_distortion =  QtGui.QGroupBox(_("Distortion"))
        self.groupbox_distortion.setObjectName("plugin_groupbox")
        self.groupbox_distortion_layout = QtGui.QGridLayout(self.groupbox_distortion)
        self.hlayout1.addWidget(self.groupbox_distortion)
        self.dist =  pydaw_knob_control(f_knob_size, _("Gain"), pydaw_ports.RAYV_DIST,
                                        self.plugin_rel_callback, self.plugin_val_callback,
                                        0, 48, 15, kc_integer, self.port_dict,
                                        self.preset_manager)
        self.dist.add_to_grid_layout(self.groupbox_distortion_layout, 0)
        self.dist_wet =  pydaw_knob_control(f_knob_size, _("Wet"), pydaw_ports.RAYV_DIST_WET,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_none, self.port_dict,
                                            self.preset_manager)
        self.dist_wet.add_to_grid_layout(self.groupbox_distortion_layout, 1)
        self.groupbox_noise =  QtGui.QGroupBox(_("Noise"))
        self.groupbox_noise.setObjectName("plugin_groupbox")
        self.noise_layout = QtGui.QGridLayout(self.groupbox_noise)
        self.hlayout1.addWidget(self.groupbox_noise)
        self.noise_amp =  pydaw_knob_control(f_knob_size, _("Vol"), pydaw_ports.RAYV_NOISE_AMP,
                                             self.plugin_rel_callback, self.plugin_val_callback,
                                             -60, 0, -30,
                                             kc_integer, self.port_dict, self.preset_manager)
        self.noise_amp.add_to_grid_layout(self.noise_layout, 0)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout2)
        self.osc2 =  pydaw_osc_widget(f_knob_size, pydaw_ports.RAYV_OSC2_PITCH,
                                      pydaw_ports.RAYV_OSC2_TUNE, pydaw_ports.RAYV_OSC2_VOLUME,
                                      pydaw_ports.RAYV_OSC2_TYPE, f_osc_types,
                                      self.plugin_rel_callback, self.plugin_val_callback,
                                      _("Oscillator 2"),
                                      self.port_dict, self.preset_manager, 4)
        self.hlayout2.addWidget(self.osc2.group_box)
        self.sync_groupbox =  QtGui.QGroupBox(_("Sync"))
        self.sync_groupbox.setObjectName("plugin_groupbox")
        self.hlayout2.addWidget(self.sync_groupbox)
        self.sync_gridlayout = QtGui.QGridLayout(self.sync_groupbox)
        self.hard_sync =  pydaw_checkbox_control("On", pydaw_ports.RAYV_OSC_HARD_SYNC,
                                                 self.plugin_rel_callback,
                                                 self.plugin_val_callback,
                                                 self.port_dict, self.preset_manager)
        self.hard_sync.control.setToolTip(
            _("Setting self hard sync's Osc1 to Osc2. Usually you would want to "
            "distort and pitchbend if this is enabled."))
        self.sync_gridlayout.addWidget(self.hard_sync.control, 1, 0, QtCore.Qt.AlignCenter)
        self.adsr_filter =  pydaw_adsr_widget(f_knob_size, False, pydaw_ports.RAYV_FILTER_ATTACK,
                                              pydaw_ports.RAYV_FILTER_DECAY,
                                              pydaw_ports.RAYV_FILTER_SUSTAIN,
                                              pydaw_ports.RAYV_FILTER_RELEASE, _("ADSR Filter"),
                                              self.plugin_rel_callback, self.plugin_val_callback,
                                              self.port_dict, self.preset_manager)
        self.hlayout2.addWidget(self.adsr_filter.groupbox)
        self.filter =  pydaw_filter_widget(f_knob_size,
                                           self.plugin_rel_callback, self.plugin_val_callback,
                                           self.port_dict, pydaw_ports.RAYV_TIMBRE,
                                           pydaw_ports.RAYV_RES,
                                           a_preset_mgr=self.preset_manager)
        self.hlayout2.addWidget(self.filter.groupbox)
        self.filter_env_amt =  pydaw_knob_control(f_knob_size, _("Env Amt"),
                                                  pydaw_ports.RAYV_FILTER_ENV_AMT,
                                                  self.plugin_rel_callback,
                                                  self.plugin_val_callback,
                                                  -36, 36, 0, kc_integer,
                                                  self.port_dict, self.preset_manager)
        self.filter_env_amt.add_to_grid_layout(self.filter.layout, 2)
        self.filter_keytrk = pydaw_knob_control(f_knob_size, _("KeyTrk"),
                                                pydaw_ports.RAYV_FILTER_KEYTRK,
                                                self.plugin_rel_callback,
                                                self.plugin_val_callback,
                                                0, 100, 0, kc_none,
                                                self.port_dict, self.preset_manager)
        self.filter_keytrk.add_to_grid_layout(self.filter.layout, 3)
        self.hlayout3 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout3)
        self.master =  pydaw_master_widget(f_knob_size,
                                           self.plugin_rel_callback, self.plugin_val_callback,
                                           pydaw_ports.RAYV_MASTER_VOLUME,
                                           pydaw_ports.RAYV_MASTER_GLIDE,
                                           pydaw_ports.RAYV_MASTER_PITCHBEND_AMT,
                                           self.port_dict, _("Master"),
                                           pydaw_ports.RAYV_MASTER_UNISON_VOICES,
                                           pydaw_ports.RAYV_MASTER_UNISON_SPREAD,
                                           self.preset_manager)
        self.hlayout3.addWidget(self.master.group_box)
        self.pitch_env =  pydaw_ramp_env_widget(f_knob_size,
                                                self.plugin_rel_callback,
                                                self.plugin_val_callback,
                                                self.port_dict,
                                                pydaw_ports.RAYV_PITCH_ENV_TIME,
                                                pydaw_ports.RAYV_PITCH_ENV_AMT,
                                                _("Pitch Env"), self.preset_manager,
                                                pydaw_ports.RAYV_RAMP_CURVE)
        self.hlayout3.addWidget(self.pitch_env.groupbox)
        self.lfo =  pydaw_lfo_widget(f_knob_size,
                                     self.plugin_rel_callback, self.plugin_val_callback,
                                     self.port_dict,
                                     pydaw_ports.RAYV_LFO_FREQ,
                                     pydaw_ports.RAYV_LFO_TYPE,
                                     f_lfo_types, _("LFO"),
                                     self.preset_manager)
        self.hlayout3.addWidget(self.lfo.groupbox)

        self.lfo_amp =  pydaw_knob_control(f_knob_size, _("Amp"),
                                           pydaw_ports.RAYV_LFO_AMP, self.plugin_rel_callback,
                                           self.plugin_val_callback, -24, 24, 0, kc_integer,
                                           self.port_dict, self.preset_manager)
        self.lfo_amp.add_to_grid_layout(self.lfo.layout, 2)
        self.lfo_pitch =  pydaw_knob_control(f_knob_size, _("Pitch"), pydaw_ports.RAYV_LFO_PITCH,
                                             self.plugin_rel_callback, self.plugin_val_callback,
                                             -36, 36, 0,
                                             kc_integer, self.port_dict, self.preset_manager)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 3)
        self.lfo_cutoff =  pydaw_knob_control(f_knob_size, _("Filter"),
                                              pydaw_ports.RAYV_LFO_FILTER,
                                              self.plugin_rel_callback, self.plugin_val_callback,
                                              -48, 48, 0,
                                              kc_integer, self.port_dict, self.preset_manager)
        self.lfo_cutoff.add_to_grid_layout(self.lfo.layout, 4)

        self.open_plugin_file()

    def set_window_title(self, a_track_name):
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Ray-V - {}".format(self.track_name))



class pydaw_wayv_plugin_ui(pydaw_abstract_plugin_ui):
    def __init__(self, a_rel_callback, a_val_callback, a_track_num, a_project,
                 a_folder, a_track_type, a_track_name, a_stylesheet,
                 a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback,
                                          a_track_num, a_project, a_track_type,
                                          a_stylesheet, a_close_callback, a_configure_callback)
        self.folder = str(a_folder)
        self.file = "{}.pyinst".format(self.track_num)
        self.set_window_title(a_track_name)
        self.is_instrument = True

        f_osc_types = [_("Off"),
            #Saw-like waves
            _("Plain Saw"), _("SuperbSaw"), _("Viral Saw"), _("Soft Saw"),
            _("Mid Saw"), _("Lush Saw"),
            #Square-like waves
            _("Evil Square"), _("Punchy Square"), _("Soft Square"),
            #Glitchy and distorted waves
            _("Pink Glitch"), _("White Glitch"), _("Acid"), _("Screetch"),
            #Sine and triangle-like waves
            _("Thick Bass"), _("Rattler"), _("Deep Saw"), _("Sine"),
            #The custom additive oscillator tab
            _("(Additive 1)"), _("(Additive 2)"), _("(Additive 3)")
        ]
        f_lfo_types = [_("Off"), _("Sine"), _("Triangle")]
        self.tab_widget =  QtGui.QTabWidget()
        self.layout.addWidget(self.tab_widget)
        self.layout.setSizeConstraint(QtGui.QLayout.SetFixedSize)
        self.osc_tab =  QtGui.QWidget()
        self.tab_widget.addTab(self.osc_tab, _("Oscillators"))
        self.poly_fx_tab =  QtGui.QWidget()
        self.tab_widget.addTab(self.poly_fx_tab, _("PolyFX"))
        self.oscillator_layout =  QtGui.QVBoxLayout(self.osc_tab)
        self.preset_manager =  pydaw_preset_manager_widget("WAYV", self.configure_dict,
                                                           self.reconfigure_plugin)
        self.hlayout0 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout0)
        self.hlayout0.addWidget(self.preset_manager.group_box)
        self.hlayout0.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        f_knob_size = 48

        self.hlayout1 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout1)
        self.osc1 =  pydaw_osc_widget(f_knob_size, pydaw_ports.WAYV_OSC1_PITCH,
                                      pydaw_ports.WAYV_OSC1_TUNE,
                                      pydaw_ports.WAYV_OSC1_VOLUME,
                                      pydaw_ports.WAYV_OSC1_TYPE, f_osc_types,
                                      self.plugin_rel_callback,
                                      self.plugin_val_callback, _("Oscillator 1"),
                                      self.port_dict, self.preset_manager, 1)

        self.osc1_uni_voices =  pydaw_knob_control(f_knob_size, _("Unison"),
                                                   pydaw_ports.WAYV_OSC1_UNISON_VOICES,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   1, 7, 4, kc_integer,
                                                   self.port_dict, self.preset_manager)
        self.osc1_uni_voices.add_to_grid_layout(self.osc1.grid_layout, 4)
        self.osc1_uni_spread =  pydaw_knob_control(f_knob_size, _("Spread"),
                                                   pydaw_ports.WAYV_OSC1_UNISON_SPREAD,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   0, 100, 50, kc_decimal,
                                                   self.port_dict, self.preset_manager)
        self.osc1_uni_spread.add_to_grid_layout(self.osc1.grid_layout, 5)

        self.hlayout1.addWidget(self.osc1.group_box)

        self.adsr_amp1 =  pydaw_adsr_widget(f_knob_size,  True,
                                            pydaw_ports.WAYV_ATTACK1, pydaw_ports.WAYV_DECAY1,
                                            pydaw_ports.WAYV_SUSTAIN1, pydaw_ports.WAYV_RELEASE1,
                                            _("ADSR Osc1"),
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            self.port_dict, self.preset_manager)
        self.hlayout1.addWidget(self.adsr_amp1.groupbox)

        self.adsr_amp1_checkbox =  pydaw_checkbox_control(_("On"), pydaw_ports.WAYV_ADSR1_CHECKBOX,
                                                          self.plugin_rel_callback,
                                                          self.plugin_val_callback,
                                                          self.port_dict, self.preset_manager)
        self.adsr_amp1_checkbox.add_to_grid_layout(self.adsr_amp1.layout, 4)

        self.groupbox_osc1_fm =  QtGui.QGroupBox(_("Osc1 FM"))
        self.groupbox_osc1_fm_layout = QtGui.QGridLayout(self.groupbox_osc1_fm)
        self.groupbox_osc1_fm.setObjectName("plugin_groupbox")

        self.osc1_fm1 =  pydaw_knob_control(f_knob_size, _("Osc1"), pydaw_ports.WAYV_OSC1_FM1,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer, self.port_dict,
                                            self.preset_manager)
        self.osc1_fm1.add_to_grid_layout(self.groupbox_osc1_fm_layout, 0)

        self.osc1_fm2 =  pydaw_knob_control(f_knob_size, _("Osc2"), pydaw_ports.WAYV_OSC1_FM2,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer, self.port_dict,
                                            self.preset_manager)
        self.osc1_fm2.add_to_grid_layout(self.groupbox_osc1_fm_layout, 1)

        self.osc1_fm3 =  pydaw_knob_control(f_knob_size, _("Osc3"), pydaw_ports.WAYV_OSC1_FM3,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer, self.port_dict,
                                            self.preset_manager)
        self.osc1_fm3.add_to_grid_layout(self.groupbox_osc1_fm_layout, 2)

        self.hlayout1.addWidget(self.groupbox_osc1_fm)

        #Osc2
        self.hlayout2 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout2)
        self.osc2 =  pydaw_osc_widget(f_knob_size, pydaw_ports.WAYV_OSC2_PITCH,
                                      pydaw_ports.WAYV_OSC2_TUNE, pydaw_ports.WAYV_OSC2_VOLUME,
                                      pydaw_ports.WAYV_OSC2_TYPE, f_osc_types,
                                      self.plugin_rel_callback, self.plugin_val_callback,
                                      _("Oscillator 2"),
                                      self.port_dict, self.preset_manager)

        self.osc2_uni_voices =  pydaw_knob_control(f_knob_size, _("Unison"),
                                                   pydaw_ports.WAYV_OSC2_UNISON_VOICES,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   1, 7, 4, kc_integer, self.port_dict,
                                                   self.preset_manager)
        self.osc2_uni_voices.add_to_grid_layout(self.osc2.grid_layout, 4)
        self.osc2_uni_spread =  pydaw_knob_control(f_knob_size, _("Spread"),
                                                   pydaw_ports.WAYV_OSC2_UNISON_SPREAD,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   0, 100, 50, kc_decimal,
                                                   self.port_dict, self.preset_manager)
        self.osc2_uni_spread.add_to_grid_layout(self.osc2.grid_layout, 5)

        self.hlayout2.addWidget(self.osc2.group_box)

        self.adsr_amp2 =  pydaw_adsr_widget(f_knob_size,  True,
                                            pydaw_ports.WAYV_ATTACK2,
                                            pydaw_ports.WAYV_DECAY2,
                                            pydaw_ports.WAYV_SUSTAIN2,
                                            pydaw_ports.WAYV_RELEASE2,
                                            _("ADSR Osc2"),
                                            self.plugin_rel_callback,
                                            self.plugin_val_callback,
                                            self.port_dict, self.preset_manager)
        self.hlayout2.addWidget(self.adsr_amp2.groupbox)

        self.adsr_amp2_checkbox =  pydaw_checkbox_control(_("On"),
                                                          pydaw_ports.WAYV_ADSR2_CHECKBOX,
                                                          self.plugin_rel_callback,
                                                          self.plugin_val_callback,
                                                          self.port_dict, self.preset_manager)
        self.adsr_amp2_checkbox.add_to_grid_layout(self.adsr_amp2.layout, 4)

        self.groupbox_osc2_fm =  QtGui.QGroupBox(_("Osc2 FM"))
        self.groupbox_osc2_fm_layout = QtGui.QGridLayout(self.groupbox_osc2_fm)
        self.groupbox_osc2_fm.setObjectName("plugin_groupbox")

        self.osc2_fm1 =  pydaw_knob_control(f_knob_size, _("Osc1"), pydaw_ports.WAYV_OSC2_FM1,
                                            self.plugin_rel_callback,
                                            self.plugin_val_callback,
                                            0, 100, 0, kc_integer,
                                            self.port_dict, self.preset_manager)
        self.osc2_fm1.add_to_grid_layout(self.groupbox_osc2_fm_layout, 0)

        self.osc2_fm2 =  pydaw_knob_control(f_knob_size, _("Osc2"), pydaw_ports.WAYV_OSC2_FM2,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer,
                                            self.port_dict, self.preset_manager)
        self.osc2_fm2.add_to_grid_layout(self.groupbox_osc2_fm_layout, 1)

        self.osc2_fm3 =  pydaw_knob_control(f_knob_size, _("Osc3"), pydaw_ports.WAYV_OSC2_FM3,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer,
                                            self.port_dict, self.preset_manager)
        self.osc2_fm3.add_to_grid_layout(self.groupbox_osc2_fm_layout, 2)

        self.hlayout2.addWidget(self.groupbox_osc2_fm)

        #osc3
        self.hlayout3 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout3)
        self.osc3 =  pydaw_osc_widget(f_knob_size,
                                      pydaw_ports.WAYV_OSC3_PITCH,
                                      pydaw_ports.WAYV_OSC3_TUNE,
                                      pydaw_ports.WAYV_OSC3_VOLUME,
                                      pydaw_ports.WAYV_OSC3_TYPE,
                                      f_osc_types,
                                      self.plugin_rel_callback, self.plugin_val_callback,
                                      _("Oscillator 3"),
                                      self.port_dict, self.preset_manager)

        self.osc3_uni_voices =  pydaw_knob_control(f_knob_size, _("Unison"),
                                                   pydaw_ports.WAYV_OSC3_UNISON_VOICES,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   1, 7, 4, kc_integer,
                                                   self.port_dict, self.preset_manager)
        self.osc3_uni_voices.add_to_grid_layout(self.osc3.grid_layout, 4)
        self.osc3_uni_spread =  pydaw_knob_control(f_knob_size, _("Spread"),
                                                   pydaw_ports.WAYV_OSC3_UNISON_SPREAD,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   0, 100, 50, kc_decimal,
                                                   self.port_dict, self.preset_manager)
        self.osc3_uni_spread.add_to_grid_layout(self.osc3.grid_layout, 5)

        self.hlayout3.addWidget(self.osc3.group_box)

        self.adsr_amp3 =  pydaw_adsr_widget(f_knob_size,  True,
                                            pydaw_ports.WAYV_ATTACK3,
                                            pydaw_ports.WAYV_DECAY3,
                                            pydaw_ports.WAYV_SUSTAIN3,
                                            pydaw_ports.WAYV_RELEASE3,
                                            _("ADSR Osc3"),
                                            self.plugin_rel_callback,
                                            self.plugin_val_callback,
                                            self.port_dict, self.preset_manager)

        self.hlayout3.addWidget(self.adsr_amp3.groupbox)

        self.adsr_amp3_checkbox =  pydaw_checkbox_control(_("On"),
                                                          pydaw_ports.WAYV_ADSR3_CHECKBOX,
                                                          self.plugin_rel_callback,
                                                          self.plugin_val_callback,
                                                          self.port_dict, self.preset_manager)
        self.adsr_amp3_checkbox.add_to_grid_layout(self.adsr_amp3.layout, 4)

        self.groupbox_osc3_fm =  QtGui.QGroupBox(_("Osc3 FM"))
        self.groupbox_osc3_fm_layout = QtGui.QGridLayout(self.groupbox_osc3_fm)
        self.groupbox_osc3_fm.setObjectName("plugin_groupbox")

        self.osc3_fm1 =  pydaw_knob_control(f_knob_size, _("Osc1"), pydaw_ports.WAYV_OSC3_FM1,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer, self.port_dict,
                                            self.preset_manager)
        self.osc3_fm1.add_to_grid_layout(self.groupbox_osc3_fm_layout, 0)

        self.osc3_fm2 =  pydaw_knob_control(f_knob_size, _("Osc2"), pydaw_ports.WAYV_OSC3_FM2,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer, self.port_dict,
                                            self.preset_manager)
        self.osc3_fm2.add_to_grid_layout(self.groupbox_osc3_fm_layout, 1)

        self.osc3_fm3 =  pydaw_knob_control(f_knob_size, _("Osc3"), pydaw_ports.WAYV_OSC3_FM3,
                                            self.plugin_rel_callback, self.plugin_val_callback,
                                            0, 100, 0, kc_integer, self.port_dict,
                                            self.preset_manager)
        self.osc3_fm3.add_to_grid_layout(self.groupbox_osc3_fm_layout, 2)

        self.hlayout3.addWidget(self.groupbox_osc3_fm)

        self.hlayout4 = QtGui.QHBoxLayout()
        self.oscillator_layout.addLayout(self.hlayout4)
        self.master =  pydaw_master_widget(f_knob_size,  self.plugin_rel_callback,
                                           self.plugin_val_callback,
                                           pydaw_ports.WAYV_MASTER_VOLUME,
                                           pydaw_ports.WAYV_MASTER_GLIDE,
                                           pydaw_ports.WAYV_MASTER_PITCHBEND_AMT,
                                           self.port_dict, a_preset_mgr=self.preset_manager)

        self.hlayout4.addWidget(self.master.group_box)

        self.adsr_amp_main =  pydaw_adsr_widget(f_knob_size, True,
                                                pydaw_ports.WAYV_ATTACK_MAIN,
                                                pydaw_ports.WAYV_DECAY_MAIN,
                                                pydaw_ports.WAYV_SUSTAIN_MAIN,
                                                pydaw_ports.WAYV_RELEASE_MAIN,
                                                _("ADSR Master"),
                                                self.plugin_rel_callback,
                                                self.plugin_val_callback,
                                                self.port_dict, self.preset_manager)
        self.hlayout4.addWidget(self.adsr_amp_main.groupbox)

        self.groupbox_noise =  QtGui.QGroupBox(_("Noise"))
        self.groupbox_noise.setObjectName("plugin_groupbox")
        self.groupbox_noise_layout = QtGui.QGridLayout(self.groupbox_noise)
        self.hlayout4.addWidget(self.groupbox_noise)
        self.noise_amp =  pydaw_knob_control(f_knob_size, _("Vol"),
                                             pydaw_ports.WAYV_NOISE_AMP,
                                             self.plugin_rel_callback,
                                             self.plugin_val_callback,
                                             -60, 0, -30, kc_integer,
                                             self.port_dict, self.preset_manager)
        self.noise_amp.add_to_grid_layout(self.groupbox_noise_layout, 0)

        self.noise_type =  pydaw_combobox_control(87, _("Type"), pydaw_ports.LMS_NOISE_TYPE,
                                                  self.plugin_rel_callback,
                                                  self.plugin_val_callback,
                                                  [_("Off"), _("White"), _("Pink")],
                                                  self.port_dict,
                                                  a_preset_mgr=self.preset_manager)
        self.noise_type.control.setMaximumWidth(87)
        self.noise_type.add_to_grid_layout(self.groupbox_noise_layout, 1)

        self.perc_env = pydaw_perc_env_widget(f_knob_size,
                                              self.plugin_rel_callback,
                                              self.plugin_val_callback,
                                              self.port_dict,
                                              pydaw_ports.WAYV_PERC_ENV_TIME1,
                                              pydaw_ports.WAYV_PERC_ENV_PITCH1,
                                              pydaw_ports.WAYV_PERC_ENV_TIME2,
                                              pydaw_ports.WAYV_PERC_ENV_PITCH2,
                                              pydaw_ports.WAYV_PERC_ENV_ON,
                                              a_preset_mgr=self.preset_manager
                                              )

        self.hlayout4.addWidget(self.perc_env.groupbox)

        self.main_layout =  QtGui.QVBoxLayout(self.poly_fx_tab)
        self.hlayout5 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout5)
        self.hlayout6 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout6)
        #From Modulex
        self.fx0 =  pydaw_modulex_single(_("FX0"), pydaw_ports.WAYV_FX0_KNOB0,
                                         self.plugin_rel_callback, self.plugin_val_callback,
                                         self.port_dict, self.preset_manager,
                                         a_knob_size=f_knob_size)
        self.hlayout5.addWidget(self.fx0.group_box)
        self.fx1 =  pydaw_modulex_single(_("FX1"), pydaw_ports.WAYV_FX1_KNOB0,
                                         self.plugin_rel_callback, self.plugin_val_callback,
                                         self.port_dict, self.preset_manager,
                                         a_knob_size=f_knob_size)
        self.hlayout5.addWidget(self.fx1.group_box)
        self.fx2 =  pydaw_modulex_single(_("FX2"), pydaw_ports.WAYV_FX2_KNOB0,
                                         self.plugin_rel_callback, self.plugin_val_callback,
                                         self.port_dict, self.preset_manager,
                                         a_knob_size=f_knob_size)
        self.hlayout6.addWidget(self.fx2.group_box)
        self.fx3 =  pydaw_modulex_single(_("FX3"), pydaw_ports.WAYV_FX3_KNOB0,
                                         self.plugin_rel_callback, self.plugin_val_callback,
                                         self.port_dict, self.preset_manager,
                                         a_knob_size=f_knob_size)
        self.hlayout6.addWidget(self.fx3.group_box)

        self.mod_matrix = QtGui.QTableWidget()
        self.mod_matrix.setRowCount(6)
        self.mod_matrix.setColumnCount(12)
        self.mod_matrix.setFixedHeight(222)
        self.mod_matrix.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setHorizontalHeaderLabels([_("FX0\nCtrl1"), _("FX0\nCtrl2"),
            _("FX0\nCtrl3"), _("FX1\nCtrl1"), _("FX1\nCtrl2"), _("FX1\nCtrl3"), _("FX2\nCtrl1"),
            _("FX2\nCtrl2"), _("FX2\nCtrl3"), _("FX3\nCtrl1"), _("FX3\nCtrl2"), _("FX3\nCtrl3")])
        self.mod_matrix.setVerticalHeaderLabels([_("ADSR 1"), _("ADSR 2"), _("Ramp Env"),
                                                 _("LFO"), _("Pitch"), _("Velocity")])

        f_port_num = pydaw_ports.WAVV_PFXMATRIX_FIRST_PORT

        for f_i_dst in range(4):
            for f_i_src in range(4):
                for f_i_ctrl in range(3):
                    f_ctrl = pydaw_spinbox_control(None, f_port_num, self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   -100, 100, 0, kc_none,
                                                   self.port_dict, self.preset_manager)
                    f_x = (f_i_dst * 3) + f_i_ctrl
                    self.mod_matrix.setCellWidget(f_i_src, f_x, f_ctrl.control)
                    f_port_num += 1

        #The new pitch and velocity tracking controls
        f_port_num = pydaw_ports.WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0

        for f_i_src in range(4, 6):
            for f_i_dst in range(4):
                for f_i_ctrl in range(3):
                    f_ctrl = pydaw_spinbox_control(None, f_port_num,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   -100, 100, 0, kc_none,
                                                   self.port_dict, self.preset_manager)
                    f_x = (f_i_dst * 3) + f_i_ctrl
                    self.mod_matrix.setCellWidget(f_i_src, f_x, f_ctrl.control)
                    f_port_num += 1

        self.main_layout.addWidget(self.mod_matrix)
        self.mod_matrix.resizeColumnsToContents()

        self.hlayout7 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout7)

        self.adsr_amp =  pydaw_adsr_widget(f_knob_size, True,
                                           pydaw_ports.WAYV_ATTACK_PFX1,
                                           pydaw_ports.WAYV_DECAY_PFX1,
                                           pydaw_ports.WAYV_SUSTAIN_PFX1,
                                           pydaw_ports.WAYV_RELEASE_PFX1,
                                           _("ADSR 1"),
                                           self.plugin_rel_callback,
                                           self.plugin_val_callback,
                                           self.port_dict, self.preset_manager)
        #self.adsr_amp.lms_release.lms_knob.setMinimum(5) #overriding the default for self,
       #because we want a low minimum default that won't click
        self.hlayout7.addWidget(self.adsr_amp.groupbox)

        self.adsr_filter =  pydaw_adsr_widget(f_knob_size,  False,
                                              pydaw_ports.WAYV_ATTACK_PFX2,
                                              pydaw_ports.WAYV_DECAY_PFX2,
                                              pydaw_ports.WAYV_SUSTAIN_PFX2,
                                              pydaw_ports.WAYV_RELEASE_PFX2, _("ADSR 2"),
                                              self.plugin_rel_callback,
                                              self.plugin_val_callback,
                                              self.port_dict,
                                              self.preset_manager)
        self.hlayout7.addWidget(self.adsr_filter.groupbox)

        self.pitch_env =  pydaw_ramp_env_widget(f_knob_size,
                                                self.plugin_rel_callback,
                                                self.plugin_val_callback,
                                                self.port_dict,
                                                pydaw_ports.WAYV_RAMP_ENV_TIME,
                                                pydaw_ports.WAYV_PITCH_ENV_AMT,
                                                _("Ramp Env"), self.preset_manager,
                                                pydaw_ports.WAYV_RAMP_CURVE)
        self.pitch_env.amt_knob.name_label.setText(_("Pitch"))
        self.pitch_env.amt_knob.control.setRange(-60, 60)
        self.hlayout7.addWidget(self.pitch_env.groupbox)

        self.lfo =  pydaw_lfo_widget(f_knob_size,
                                     self.plugin_rel_callback, self.plugin_val_callback,
                                     self.port_dict, pydaw_ports.WAYV_LFO_FREQ,
                                     pydaw_ports.WAYV_LFO_TYPE, f_lfo_types,
                                     _("LFO"), self.preset_manager)
        self.hlayout7.addWidget(self.lfo.groupbox)

        self.lfo_amount =  pydaw_knob_control(f_knob_size, _("Amount"),
                                              pydaw_ports.WAYV_LFO_AMOUNT,
                                              self.plugin_rel_callback,
                                              self.plugin_val_callback,
                                              0, 100, 100, kc_decimal,
                                              self.port_dict, self.preset_manager)
        self.lfo_amount.add_to_grid_layout(self.lfo.layout, 2)

        self.lfo_amp =  pydaw_knob_control(f_knob_size, _("Amp"),
                                           pydaw_ports.WAYV_LFO_AMP,
                                           self.plugin_rel_callback,
                                           self.plugin_val_callback,
                                           -24, 24, 0, kc_integer,
                                           self.port_dict, self.preset_manager)
        self.lfo_amp.add_to_grid_layout(self.lfo.layout, 3)

        self.lfo_pitch =  pydaw_knob_control(f_knob_size, _("Pitch"),
                                             pydaw_ports.WAYV_LFO_PITCH,
                                             self.plugin_rel_callback,
                                             self.plugin_val_callback,
                                             -36, 36, 0,  kc_integer,
                                             self.port_dict, self.preset_manager)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 4)

        self.additive_osc = pydaw_custom_additive_oscillator(self.configure_plugin)
        self.tab_widget.addTab(self.additive_osc.widget, "Additive")

        self.open_plugin_file()

    def configure_plugin(self, a_key, a_message):
        self.configure_dict[a_key] = a_message
        self.configure_callback(True, 0, self.track_num, a_key, a_message)

    def set_configure(self, a_key, a_message):
        self.configure_dict[a_key] = a_message
        if a_key.startswith("wayv_add_ui"):
            self.configure_dict[a_key] = a_message
            f_arr = a_message.split("|")
            self.additive_osc.set_values(int(a_key[-1]), f_arr)
        elif a_key.startswith("wayv_add_eng"):
            pass
        else:
            print("Way-V: Unknown configure message '{}'".format(a_key))

    def reconfigure_plugin(self, a_dict):
        # Clear existing sample tables
        f_ui_config_keys = ["wayv_add_ui0", "wayv_add_ui1", "wayv_add_ui2"]
        f_eng_config_keys = ["wayv_add_eng0", "wayv_add_eng1", "wayv_add_eng2"]
        f_empty_ui_val = "|".join(["-30"] * global_additive_osc_harmonic_count)
        f_empty_eng_val = "{}|{}".format(global_additive_wavetable_size,
            "|".join(["0.0"] * global_additive_wavetable_size))
        for f_key in f_ui_config_keys:
            if f_key in a_dict:
                self.configure_plugin(f_key, a_dict[f_key])
                self.set_configure(f_key, a_dict[f_key])
            else:
                self.configure_plugin(f_key, f_empty_ui_val)
                self.set_configure(f_key, f_empty_ui_val)
        for f_key in f_eng_config_keys:
            if f_key in a_dict:
                self.configure_plugin(f_key, a_dict[f_key])
            else:
                self.configure_plugin(f_key, f_empty_eng_val)

    def set_window_title(self, a_track_name):
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Way-V - {}".format(self.track_name))


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
    def __init__(self, a_rel_callback, a_val_callback, a_track_num,
                 a_project, a_folder, a_track_type, a_track_name,
                 a_stylesheet, a_close_callback, a_configure_callback):
        pydaw_abstract_plugin_ui.__init__(self, a_rel_callback, a_val_callback,
                                          a_track_num, a_project, a_track_type,
                                          a_stylesheet, a_close_callback,
                                          a_configure_callback, a_can_resize=True)
        self.folder = str(a_folder)
        self.file = "{}.pyinst".format(self.track_num)
        self.set_window_title(a_track_name)
        self.track_name = str(a_track_name)
        self.widget.setWindowTitle("PyDAW Euphoria - {}".format(self.track_name))
        self.is_instrument = True

        self.selected_row_index = 0
        self.handle_control_updates = True
        self.suppress_selected_sample_changed = False
        self.interpolation_modes_list = [_("Pitched"), _("Percussion"), _("No Pitch")]
        f_sample_table_columns = [
            "", #Selected row
            _("Path"), #File path
            _("Sample Pitch"), #Sample base pitch
            _("Low Note"), #Low Note
            _("High Note"), #High Note
            _("Volume"), #Volume
            _("Vel. Sens."), #Velocity Sensitivity
            _("Low Vel."), #Low Velocity
            _("High Vel."), #High Velocity
            _("Pitch"), #Pitch
            _("Tune"), #Tune
            _("Mode"), #Interpolation Mode
            _("Noise Type"),
            _("Noise Amp"),
        ]

        self.noise_types_list = [_("Off"), _("White"), _("Pink")]

        self.selected_sample_port = pydaw_null_control(pydaw_ports.EUPHORIA_SELECTED_SAMPLE,
                                                       self.plugin_rel_callback,
                                                       self.plugin_val_callback,
                                                       0, self.port_dict)

        self.sample_table = QtGui.QTableWidget(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT,
                                               len(f_sample_table_columns))
        self.sample_table.setAlternatingRowColors(True)
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
            f_sample_pitch = pydaw_note_selector_widget(f_port_start + f_i,
                                                        self.plugin_rel_callback,
                                                        self.plugin_val_callback,
                                                        self.port_dict, 60)
            self.sample_table.setCellWidget(f_i, 2, f_sample_pitch.widget)
            self.sample_base_pitches.append(f_sample_pitch)

        self.sample_low_notes = []
        f_port_start = pydaw_ports.EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_low_pitch = pydaw_note_selector_widget(f_port_start + f_i,
                                                     self.plugin_rel_callback,
                                                     self.plugin_val_callback,
                                                     self.port_dict, 0)
            self.sample_table.setCellWidget(f_i, 3, f_low_pitch.widget)
            self.sample_low_notes.append(f_low_pitch)

        self.sample_high_notes = []
        f_port_start = pydaw_ports.EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_high_pitch = pydaw_note_selector_widget(f_port_start + f_i,
                                                      self.plugin_rel_callback,
                                                      self.plugin_val_callback,
                                                      self.port_dict, 120)
            self.sample_table.setCellWidget(f_i, 4, f_high_pitch.widget)
            self.sample_high_notes.append(f_high_pitch)

        self.sample_vols = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_vol = pydaw_spinbox_control(None, f_port_start + f_i,
                                                 self.plugin_rel_callback,
                                                 self.plugin_val_callback,
                                                 -50.0, 36.0, 0.0, kc_none,
                                                 self.port_dict)
            self.sample_table.setCellWidget(f_i, 5, f_sample_vol.control)
            self.sample_vols.append(f_sample_vol)

        self.sample_vel_sens = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_vel_sens = pydaw_spinbox_control(None, f_port_start + f_i,
                                               self.plugin_rel_callback,
                                               self.plugin_val_callback,
                                               0, 20, 10, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 6, f_vel_sens.control)
            self.sample_vel_sens.append(f_vel_sens)

        self.sample_low_vels = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_vel_low = pydaw_spinbox_control(None, f_port_start + f_i,
                                              self.plugin_rel_callback,
                                              self.plugin_val_callback,
                                              1, 127, 1, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 7, f_vel_low.control)
            self.sample_low_vels.append(f_vel_low)

        self.sample_high_vels = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_vel_high = pydaw_spinbox_control(None, f_port_start + f_i,
                                               self.plugin_rel_callback, self.plugin_val_callback,
                                               1, 128, 128, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 8, f_vel_high.control)
            self.sample_high_vels.append(f_vel_high)

        self.sample_pitches = []
        f_port_start = pydaw_ports.EUPHORIA_PITCH_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_pitch = pydaw_spinbox_control(None, f_port_start + f_i,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   -36, 36, 0, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 9, f_sample_pitch.control)
            self.sample_pitches.append(f_sample_pitch)

        self.sample_tunes = []
        f_port_start = pydaw_ports.EUPHORIA_TUNE_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_tune = pydaw_spinbox_control(None, f_port_start + f_i,
                                                  self.plugin_rel_callback,
                                                  self.plugin_val_callback,
                                                  -100, 100, 0, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 10, f_sample_tune.control)
            self.sample_tunes.append(f_sample_tune)

        self.sample_modes = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_mode = pydaw_combobox_control(120, None, f_port_start + f_i,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   self.interpolation_modes_list,
                                                   self.port_dict, 1)
            self.sample_table.setCellWidget(f_i, 11, f_sample_mode.control)
            self.sample_modes.append(f_sample_mode)

        self.noise_types = []
        f_port_start = pydaw_ports.EUPHORIA_NOISE_TYPE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_noise_type = pydaw_combobox_control(75, None, f_port_start + f_i,
                                                  self.plugin_rel_callback,
                                                  self.plugin_val_callback,
                                                  self.noise_types_list, self.port_dict, 0)
            self.sample_table.setCellWidget(f_i, 12, f_noise_type.control)
            self.noise_types.append(f_noise_type)

        self.noise_amps = []
        f_port_start = pydaw_ports.EUPHORIA_NOISE_AMP_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_noise_amp = pydaw_spinbox_control(None, f_port_start + f_i,
                                                self.plugin_rel_callback, self.plugin_val_callback,
                                                -60, 0, -30, kc_none, self.port_dict)
            self.sample_table.setCellWidget(f_i, 13, f_noise_amp.control)
            self.noise_amps.append(f_noise_amp)

        self.sample_starts = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_START_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_start = pydaw_null_control(f_port_start + f_i,
                                                self.plugin_rel_callback,
                                                self.plugin_val_callback, 0,
                                                self.port_dict)
            self.sample_starts.append(f_sample_start)

        self.sample_ends = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_END_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_sample_end = pydaw_null_control(f_port_start + f_i,
                                              self.plugin_rel_callback,
                                              self.plugin_val_callback, 1000,
                                              self.port_dict)
            self.sample_ends.append(f_sample_end)

        self.loop_starts = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_loop_start = pydaw_null_control(f_port_start + f_i,
                                              self.plugin_rel_callback,
                                              self.plugin_val_callback, 0,
                                              self.port_dict)
            self.loop_starts.append(f_loop_start)

        self.loop_modes = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_loop_mode = pydaw_null_control(f_port_start + f_i,
                                             self.plugin_rel_callback,
                                             self.plugin_val_callback, 0,
                                             self.port_dict)
            self.loop_modes.append(f_loop_mode)

        self.loop_ends = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_loop_end = pydaw_null_control(f_port_start + f_i,
                                            self.plugin_rel_callback,
                                            self.plugin_val_callback, 1000,
                                            self.port_dict)
            self.loop_ends.append(f_loop_end)

        self.fade_in_ends = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_FADE_IN_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_fade_in = pydaw_null_control(f_port_start + f_i,
                                           self.plugin_rel_callback,
                                           self.plugin_val_callback, 0,
                                           self.port_dict)
            self.fade_in_ends.append(f_fade_in)

        self.fade_out_starts = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_FADE_OUT_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_fade_out = pydaw_null_control(f_port_start + f_i,
                                            self.plugin_rel_callback,
                                            self.plugin_val_callback, 1000,
                                            self.port_dict)
            self.fade_out_starts.append(f_fade_out)

        #MonoFX0
        self.monofx0knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx0knob0_ctrls.append(f_ctrl)

        self.monofx0knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx0knob1_ctrls.append(f_ctrl)

        self.monofx0knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx0knob2_ctrls.append(f_ctrl)

        self.monofx0comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 0,
                                        self.port_dict)
            self.monofx0comboboxes.append(f_ctrl)
        #MonoFX1
        self.monofx1knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx1knob0_ctrls.append(f_ctrl)

        self.monofx1knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx1knob1_ctrls.append(f_ctrl)

        self.monofx1knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx1knob2_ctrls.append(f_ctrl)

        self.monofx1comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 0,
                                        self.port_dict)
            self.monofx1comboboxes.append(f_ctrl)
        #MonoFX2
        self.monofx2knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx2knob0_ctrls.append(f_ctrl)

        self.monofx2knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx2knob1_ctrls.append(f_ctrl)

        self.monofx2knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx2knob2_ctrls.append(f_ctrl)

        self.monofx2comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 0,
                                        self.port_dict)
            self.monofx2comboboxes.append(f_ctrl)
        #MonoFX3
        self.monofx3knob0_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx3knob0_ctrls.append(f_ctrl)

        self.monofx3knob1_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx3knob1_ctrls.append(f_ctrl)

        self.monofx3knob2_ctrls = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 64,
                                        self.port_dict)
            self.monofx3knob2_ctrls.append(f_ctrl)

        self.monofx3comboboxes = []
        f_port_start = pydaw_ports.EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_ctrl = pydaw_null_control(f_port_start + f_i,
                                        self.plugin_rel_callback,
                                        self.plugin_val_callback, 0,
                                        self.port_dict)
            self.monofx3comboboxes.append(f_ctrl)

        self.monofx_groups = []
        f_port_start = pydaw_ports.EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_monofx_group = pydaw_null_control(f_port_start + f_i,
                                                self.plugin_rel_callback,
                                                self.plugin_val_callback, 0,
                                                self.port_dict)
            self.monofx_groups.append(f_monofx_group)

        self.monofx_null_controls_tuple = (self.monofx0knob0_ctrls, self.monofx0knob1_ctrls,
                                           self.monofx0knob2_ctrls, self.monofx1knob0_ctrls,
                                           self.monofx1knob1_ctrls, self.monofx1knob2_ctrls,
                                           self.monofx2knob0_ctrls, self.monofx2knob1_ctrls,
                                           self.monofx2knob2_ctrls, self.monofx3knob0_ctrls,
                                           self.monofx3knob1_ctrls, self.monofx3knob2_ctrls)

        self.sample_table.setHorizontalHeaderLabels(f_sample_table_columns)
        self.sample_table.verticalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.sample_table.horizontalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.sample_table.resizeRowsToContents()

        self.file_selector = pydaw_file_select_widget(self.load_files)
        self.file_selector.clear_button.pressed.connect(self.clearFile)
        self.file_selector.reload_button.pressed.connect(self.reloadSample)

        self.main_tab =  QtGui.QTabWidget()

        self.sample_tab =  QtGui.QWidget()
        self.sample_tab.setObjectName("plugin_widget")
        self.sample_tab_layout = QtGui.QVBoxLayout(self.sample_tab)
        self.sample_tab_horizontal_splitter = QtGui.QSplitter(QtCore.Qt.Horizontal)
        self.sample_tab_layout.addWidget(self.sample_tab_horizontal_splitter)

        self.file_browser =  pydaw_file_browser_widget()
        self.file_browser.load_pushButton.pressed.connect(self.file_browser_load_button_pressed)
        self.file_browser.preview_pushButton.pressed.connect(
            self.file_browser_preview_button_pressed)
        self.sample_tab_horizontal_splitter.addWidget(self.file_browser.file_browser_vsplitter)

        self.smp_tab_main_widget = QtGui.QWidget()
        self.smp_tab_main_widget.setMinimumWidth(420)
        self.smp_tab_main_verticalLayout = QtGui.QVBoxLayout(self.smp_tab_main_widget)
        self.sample_tab_horizontal_splitter.addWidget(self.smp_tab_main_widget)

        self.smp_tab_main_verticalLayout.addWidget(self.sample_table, QtCore.Qt.AlignCenter)

        menubar =  QtGui.QPushButton(_("Menu"))
        self.menubar_layout = QtGui.QHBoxLayout()
        self.main_bottom_layout = QtGui.QVBoxLayout()
        self.main_bottom_hlayout = QtGui.QHBoxLayout()
        self.main_bottom_hlayout.addLayout(self.main_bottom_layout)
        self.main_bottom_layout.addLayout(self.menubar_layout)
        self.main_bottom_layout.addLayout(self.file_selector.layout)
        self.menubar_layout.addWidget(menubar)
        self.menubar_layout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.smp_tab_main_verticalLayout.addLayout(self.main_bottom_hlayout)

        f_logo_label =  QtGui.QLabel()
        f_pixmap = QtGui.QPixmap("{}/lib/{}/themes/default/euphoria.png".format(
        pydaw_util.global_pydaw_install_prefix, pydaw_util.global_pydaw_version_string)).scaled(
            80, 80, transformMode=QtCore.Qt.SmoothTransformation)
        f_logo_label.setPixmap(f_pixmap)
        f_logo_label.setAlignment(QtCore.Qt.AlignCenter)
        f_logo_label.setSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Minimum)
        self.main_bottom_hlayout.addWidget(f_logo_label, alignment=QtCore.Qt.AlignRight)

        menuFile =  QtGui.QMenu("Menu", menubar)
        actionSave_instrument_to_file =  menuFile.addAction(_("Save instrument to file"))
        actionOpen_instrument_from_file = menuFile.addAction(_("Open instrument from file"))
        actionMapToWhiteKeys =  menuFile.addAction(_("Map All Samples to 1 White Key"))
        actionMapToMonoFX =  menuFile.addAction(_("Map All Samples to Own MonoFX Group"))
        actionClearAllSamples =  menuFile.addAction(_("Clear All Samples"))
        actionImportSfz = menuFile.addAction(_("Import SFZ"))

        menubar.setMenu(menuFile)
        menuSetAll = menuFile.addMenu("Set all...")

        actionSetAllHighPitches = menuSetAll.addAction(_("High Notes"))
        actionSetAllLowPitches = menuSetAll.addAction(_("Low Notes"))
        actionSetAllVolumes = menuSetAll.addAction(_("Volumes"))
        actionSetAllVelSens = menuSetAll.addAction(_("Velocity Sensitivity"))
        actionSetAllHighVels = menuSetAll.addAction(_("High Velocities"))
        actionSetAllLowVels = menuSetAll.addAction(_("Low Velocities"))
        actionSetAllPitches = menuSetAll.addAction(_("Pitches"))
        actionSetAllTunes = menuSetAll.addAction(_("Tunes"))
        actionSetAllModes = menuSetAll.addAction(_("Interpolation Modes"))
        actionSetAllNoiseTypes = menuSetAll.addAction(_("Noise Types"))
        actionSetAllNoiseAmps = menuSetAll.addAction(_("Noise Amps"))

        actionSave_instrument_to_file.triggered.connect(self.saveToFile)
        actionOpen_instrument_from_file.triggered.connect(self.openFromFile)
        actionMapToWhiteKeys.triggered.connect(self.mapAllSamplesToOneWhiteKey)
        actionMapToMonoFX.triggered.connect(self.mapAllSamplesToOneMonoFXgroup)
        actionClearAllSamples.triggered.connect(self.clearAllSamples)
        actionImportSfz.triggered.connect(self.sfz_dialog)

        actionSetAllHighPitches.triggered.connect(self.set_all_high_notes)
        actionSetAllLowPitches.triggered.connect(self.set_all_low_notes)
        actionSetAllVolumes.triggered.connect(self.set_all_volumes)
        actionSetAllVelSens.triggered.connect(self.set_all_vel_sens)
        actionSetAllHighVels.triggered.connect(self.set_all_high_vels)
        actionSetAllLowVels.triggered.connect(self.set_all_low_vels)
        actionSetAllPitches.triggered.connect(self.set_all_pitches)
        actionSetAllTunes.triggered.connect(self.set_all_tunes)
        actionSetAllModes.triggered.connect(self.set_all_interpolation_modes)
        actionSetAllNoiseTypes.triggered.connect(self.set_all_noise_types)
        actionSetAllNoiseAmps.triggered.connect(self.set_all_noise_amps)

        self.main_tab.addTab(self.sample_tab, _("Samples"))
        self.poly_fx_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.poly_fx_tab, _("Poly FX"))
        self.mono_fx_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.mono_fx_tab, _("Mono FX"))
        self.layout.addWidget(self.main_tab)
        self.main_tab.setCurrentIndex(0)
        self.sample_table.resizeColumnsToContents()
        #m_view_sample_tab
        self.view_sample_tab =  QtGui.QWidget()
        self.main_tab.addTab(self.view_sample_tab, _("View"))
        self.view_sample_tab_main_vlayout =  QtGui.QVBoxLayout(self.view_sample_tab)
        self.view_sample_tab_main_vlayout.setContentsMargins(0, 0, 0, 0)

        #Sample Graph
        self.sample_graph = pydaw_audio_item_viewer_widget(self.sample_start_callback,
                                                           self.sample_end_callback,
                                                           self.loop_start_callback,
                                                           self.loop_end_callback,
                                                           self.fade_in_callback,
                                                           self.fade_out_callback)
        self.view_sample_tab_main_vlayout.addWidget(self.sample_graph)
        #The combobox for selecting the sample on the 'view' tab
        self.sample_view_select_sample_widget = QtGui.QWidget()
        self.sample_view_select_sample_widget.setMaximumHeight(200)
        self.sample_view_select_sample_hlayout =  QtGui.QHBoxLayout(
            self.sample_view_select_sample_widget)

        self.sample_view_select_sample_hlayout.addItem(QtGui.QSpacerItem(
            40, 20, QtGui.QSizePolicy.Expanding))
        self.sample_view_extra_controls_gridview =  QtGui.QGridLayout()
        self.selected_sample_index_combobox =  QtGui.QComboBox()
        sizePolicy1 = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(
            self.selected_sample_index_combobox.sizePolicy().hasHeightForWidth())
        self.selected_sample_index_combobox.setSizePolicy(sizePolicy1)
        self.selected_sample_index_combobox.setMinimumWidth(320)
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.selected_sample_index_combobox.addItem("")
        self.selected_sample_index_combobox.currentIndexChanged.connect(
            self.viewSampleSelectedIndexChanged)
        self.sample_view_extra_controls_gridview.addWidget(
            self.selected_sample_index_combobox, 1, 0, 1, 1)
        self.selected_sample_index_label =  QtGui.QLabel(_("Selected Sample"))
        self.sample_view_extra_controls_gridview.addWidget(
            self.selected_sample_index_label, 0, 0, 1, 1)
        self.sample_view_select_sample_hlayout.addItem(QtGui.QSpacerItem(30, 1))
        self.sample_view_select_sample_hlayout.addLayout(
            self.sample_view_extra_controls_gridview)
        self.sample_view_select_sample_hlayout.addItem(
            QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding))
        self.view_sample_tab_main_vlayout.addWidget(self.sample_view_select_sample_widget)
        #The loop mode combobox
        self.loop_mode_combobox =  QtGui.QComboBox(self.view_sample_tab)
        self.loop_mode_combobox.addItems([_("Off"), _("On")])
        self.loop_mode_combobox.currentIndexChanged.connect(self.loopModeChanged)
        self.loop_tune_note_selector = pydaw_note_selector_widget(0, None, None)
        self.loop_tune_button = QtGui.QPushButton(_("Tune"))
        self.loop_tune_button.pressed.connect(self.on_loop_tune)
        self.sample_view_extra_controls_gridview.addWidget(QtGui.QLabel(_("Loop Mode")), 0, 1)
        self.sample_view_extra_controls_gridview.addWidget(self.loop_mode_combobox, 1, 1)
        self.sample_view_extra_controls_gridview.addItem(QtGui.QSpacerItem(30, 1), 1, 2)
        self.sample_view_extra_controls_gridview.addWidget(QtGui.QLabel(_("Loop Tune")), 0, 3)
        self.sample_view_extra_controls_gridview.addWidget(
            self.loop_tune_note_selector.widget, 1, 3)
        self.sample_view_extra_controls_gridview.addWidget(self.loop_tune_button, 2, 3)

        #The file select on the 'view' tab
        self.sample_view_file_select_hlayout =  QtGui.QHBoxLayout()
        self.view_file_selector =  pydaw_file_select_widget(self.load_files)
        self.view_file_selector.clear_button.pressed.connect(self.clearFile)
        self.view_file_selector.reload_button.pressed.connect(self.reloadSample)
        self.sample_view_file_select_hlayout.addLayout(self.view_file_selector.layout)
        self.view_sample_tab_main_vlayout.addLayout(self.sample_view_file_select_hlayout)

        f_lfo_types = [_("Off"), _("Sine"), _("Triangle")]

        f_knob_size = 46

        self.main_layout =  QtGui.QVBoxLayout(self.poly_fx_tab)
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout0)
        self.fx0 =  pydaw_modulex_single(_("FX0"), pydaw_ports.EUPHORIA_FX0_KNOB0,
                                         self.plugin_rel_callback,
                                         self.plugin_val_callback, self.port_dict,
                                         a_knob_size=f_knob_size)
        self.hlayout0.addWidget(self.fx0.group_box)
        self.fx1 =  pydaw_modulex_single(_("FX1"), pydaw_ports.EUPHORIA_FX1_KNOB0,
                                         self.plugin_rel_callback, self.plugin_val_callback,
                                         self.port_dict, a_knob_size=f_knob_size)
        self.hlayout0.addWidget(self.fx1.group_box)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout1)
        self.fx2 =  pydaw_modulex_single(_("FX2"), pydaw_ports.EUPHORIA_FX2_KNOB0,
                                         self.plugin_rel_callback,
                                         self.plugin_val_callback, self.port_dict,
                                         a_knob_size=f_knob_size)
        self.hlayout1.addWidget(self.fx2.group_box)
        self.fx3 =  pydaw_modulex_single(_("FX3"), pydaw_ports.EUPHORIA_FX3_KNOB0,
                                         self.plugin_rel_callback,
                                         self.plugin_val_callback, self.port_dict,
                                         a_knob_size=f_knob_size)
        self.hlayout1.addWidget(self.fx3.group_box)

        self.mod_matrix = QtGui.QTableWidget()
        self.mod_matrix.setRowCount(6)
        self.mod_matrix.setColumnCount(12)
        self.mod_matrix.setFixedHeight(222)
        self.mod_matrix.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.mod_matrix.setHorizontalHeaderLabels([_("FX0\nCtrl1"), _("FX0\nCtrl2"),
            _("FX0\nCtrl3"), _("FX1\nCtrl1"), _("FX1\nCtrl2"), _("FX1\nCtrl3"), _("FX2\nCtrl1"),
            _("FX2\nCtrl2"), _("FX2\nCtrl3"), _("FX3\nCtrl1"), _("FX3\nCtrl2"), _("FX3\nCtrl3")])
        self.mod_matrix.setVerticalHeaderLabels([_("ADSR 1"), _("ADSR 2"), _("Ramp Env"),
                                                 _("LFO"), _("Pitch"), _("Velocity")])
        f_port_num = pydaw_ports.EUPHORIA_PFXMATRIX_FIRST_PORT

        for f_i_dst in range(4):
            for f_i_src in range(4):
                for f_i_ctrl in range(3):
                    f_ctrl = pydaw_spinbox_control(None, f_port_num,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   -100, 100, 0,
                                                   kc_none, self.port_dict)
                    f_x = (f_i_dst * 3) + f_i_ctrl
                    self.mod_matrix.setCellWidget(f_i_src, f_x, f_ctrl.control)
                    f_port_num += 1

        #The new pitch and velocity tracking controls
        f_port_num = pydaw_ports.EUPHORIA_PFXMATRIX_GRP0DST0SRC4CTRL0

        for f_i_src in range(4, 6):
            for f_i_dst in range(4):
                for f_i_ctrl in range(3):
                    f_ctrl = pydaw_spinbox_control(None, f_port_num,
                                                   self.plugin_rel_callback,
                                                   self.plugin_val_callback,
                                                   -100, 100, 0,
                                                   kc_none, self.port_dict)
                    f_x = (f_i_dst * 3) + f_i_ctrl
                    self.mod_matrix.setCellWidget(f_i_src, f_x, f_ctrl.control)
                    f_port_num += 1

        self.main_layout.addWidget(self.mod_matrix)
        self.mod_matrix.resizeColumnsToContents()

        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.hlayout2)

        self.adsr_amp =  pydaw_adsr_widget(f_knob_size, True,
                                           pydaw_ports.EUPHORIA_ATTACK,
                                           pydaw_ports.EUPHORIA_DECAY,
                                           pydaw_ports.EUPHORIA_SUSTAIN,
                                           pydaw_ports.EUPHORIA_RELEASE, _("ADSR Amp"),
                                           self.plugin_rel_callback,
                                           self.plugin_val_callback,
                                           self.port_dict,
                                           a_attack_default=0)
        #overriding the default for self, because we want a low minimum default that won't click
        self.adsr_amp.release_knob.control.setMinimum(5)
        self.hlayout2.addWidget(self.adsr_amp.groupbox)
        self.adsr_filter =  pydaw_adsr_widget(f_knob_size, False,
                                              pydaw_ports.EUPHORIA_FILTER_ATTACK,
                                              pydaw_ports.EUPHORIA_FILTER_DECAY,
                                              pydaw_ports.EUPHORIA_FILTER_SUSTAIN,
                                              pydaw_ports.EUPHORIA_FILTER_RELEASE,
                                              _("ADSR 2"), self.plugin_rel_callback,
                                              self.plugin_val_callback, self.port_dict)
        self.hlayout2.addWidget(self.adsr_filter.groupbox)
        self.pitch_env =  pydaw_ramp_env_widget(f_knob_size, self.plugin_rel_callback,
                                                self.plugin_val_callback,
                                                self.port_dict,
                                                pydaw_ports.EUPHORIA_PITCH_ENV_TIME,
                                                None, _("Ramp Env"))

        self.hlayout2.addWidget(self.pitch_env.groupbox)

        self.lfo =  pydaw_lfo_widget(f_knob_size, self.plugin_rel_callback,
                                     self.plugin_val_callback,  self.port_dict,
                                     pydaw_ports.EUPHORIA_LFO_FREQ,
                                     pydaw_ports.EUPHORIA_LFO_TYPE, f_lfo_types, _("LFO"))
        self.hlayout2.addWidget(self.lfo.groupbox)

        self.lfo_pitch =  pydaw_knob_control(f_knob_size, _("Pitch"),
                                             pydaw_ports.EUPHORIA_LFO_PITCH,
                                             self.plugin_rel_callback, self.plugin_val_callback,
                                             -36, 36, 0, kc_integer, self.port_dict)
        self.lfo_pitch.add_to_grid_layout(self.lfo.layout, 2)

        #MonoFX Tab
        self.mono_fx_tab_main_layout =  QtGui.QVBoxLayout(self.mono_fx_tab)
        self.selected_container =  QtGui.QWidget()
        self.mono_fx_tab_selected_hlayout =  QtGui.QHBoxLayout(self.selected_container)
        self.mono_fx_tab_selected_sample =  QtGui.QComboBox(self.mono_fx_tab)
        self.mono_fx_tab_selected_group =  QtGui.QComboBox(self.mono_fx_tab)
        self.mono_fx_tab_selected_sample_label = QtGui.QLabel(_("Selected Sample:"))
        self.mono_fx_tab_selected_group_label =  QtGui.QLabel(_("FX Group:"))
        for f_i in range(1, pydaw_ports.EUPHORIA_MONO_FX_GROUPS_COUNT):
            self.mono_fx_tab_selected_group.addItem(str(f_i))
        for f_i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            self.mono_fx_tab_selected_sample.addItem("")
        self.mono_fx_tab_selected_group.currentIndexChanged.connect(
            self.sample_selected_monofx_groupChanged)
        self.mono_fx_tab_selected_sample.currentIndexChanged.connect(
            self.monoFXSampleSelectedIndexChanged)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_sample_label)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_sample)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_group_label)
        self.mono_fx_tab_selected_hlayout.addWidget(self.mono_fx_tab_selected_group)
        self.hlayout10 = QtGui.QHBoxLayout()
        self.hlayout10.addWidget(self.selected_container)
        self.mono_fx_tab_main_layout.addLayout(self.hlayout10)

        self.hlayout11 = QtGui.QHBoxLayout()
        self.mono_fx_tab_main_layout.addLayout(self.hlayout11)
        self.mono_fx0 =  pydaw_modulex_single(_("FX0"), 0, None, self.monofx0_callback)
        self.hlayout11.addWidget(self.mono_fx0.group_box)
        self.mono_fx1 =  pydaw_modulex_single(_("FX1"), 0, None, self.monofx1_callback)
        self.hlayout11.addWidget(self.mono_fx1.group_box)
        self.hlayout12 = QtGui.QHBoxLayout()
        self.mono_fx_tab_main_layout.addLayout(self.hlayout12)
        self.mono_fx2 =  pydaw_modulex_single(_("FX2"), 0, None, self.monofx2_callback)
        self.hlayout12.addWidget(self.mono_fx2.group_box)
        self.mono_fx3 =  pydaw_modulex_single(_("FX3"), 0, None, self.monofx3_callback)
        self.hlayout12.addWidget(self.mono_fx3.group_box)

        self.monofx_knob_tuple = tuple(self.mono_fx0.knobs + self.mono_fx1.knobs +
                                       self.mono_fx2.knobs + self.mono_fx3.knobs)

        self.last_monofx_group = None
        self.set_monofx_knob_callbacks(0)

        self.master =  pydaw_master_widget(f_knob_size,
                                           self.plugin_rel_callback, self.plugin_val_callback,
                                           pydaw_ports.EUPHORIA_MASTER_VOLUME,
                                           pydaw_ports.EUPHORIA_MASTER_GLIDE,
                                           pydaw_ports.EUPHORIA_MASTER_PITCHBEND_AMT,
                                           self.port_dict, _("Master"))
        self.mono_fx_tab_main_layout.addWidget(self.master.group_box)
        self.master.vol_knob.control.setRange(-24, 24)

        self.open_plugin_file()

    def set_default_size(self):
        self.widget.resize(1100, 720)

    def monofx0_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx0knob0_ctrls, self.monofx0knob1_ctrls,
                                                 self.monofx0knob2_ctrls, self.monofx0comboboxes])

    def monofx1_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx1knob0_ctrls, self.monofx1knob1_ctrls,
                                                 self.monofx1knob2_ctrls, self.monofx1comboboxes])

    def monofx2_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx2knob0_ctrls, self.monofx2knob1_ctrls,
                                                 self.monofx2knob2_ctrls, self.monofx2comboboxes])

    def monofx3_callback(self, a_port, a_val):
        self.monofx_all_callback(a_port, a_val, [self.monofx3knob0_ctrls, self.monofx3knob1_ctrls,
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

    def fade_in_callback(self, a_val):
        f_index = self.selected_sample_index_combobox.currentIndex()
        self.fade_in_ends[f_index].set_value(a_val)
        self.fade_in_ends[f_index].control_value_changed(a_val)

    def fade_out_callback(self, a_val):
        f_index = self.selected_sample_index_combobox.currentIndex()
        self.fade_out_starts[f_index].set_value(a_val)
        self.fade_out_starts[f_index].control_value_changed(a_val)

    def on_loop_tune(self):
        self.find_selected_radio_button()
        if self.sample_table.item(self.selected_row_index, SMP_TB_FILE_PATH_INDEX) is not None:
            f_file_name = str(self.sample_table.item(
                self.selected_row_index, SMP_TB_FILE_PATH_INDEX).text())
            if f_file_name != "":
                f_graph = self.pydaw_project.get_sample_graph_by_name(f_file_name)
                f_note = self.loop_tune_note_selector.get_value()
                f_hz = pydaw_util.pydaw_pitch_to_hz(f_note)
                f_time = 1.0 / f_hz
                f_loop_length = (f_time / f_graph.length_in_seconds) * 1000.0
                f_loop_end_value = \
                    self.loop_starts[self.selected_row_index].get_value() + f_loop_length
                f_loop_end_value = pydaw_util.pydaw_clip_value(
                    f_loop_end_value,
                    self.loop_starts[self.selected_row_index].get_value() + 6.0,
                    1000.0, a_round=True)
                self.loop_ends[self.selected_row_index].set_value(f_loop_end_value)
                self.loop_ends[self.selected_row_index].control_value_changed(f_loop_end_value)
                self.set_sample_graph()
                self.loop_mode_combobox.setCurrentIndex(1)

    def set_sample_graph(self):
        self.find_selected_radio_button()
        if self.sample_table.item(self.selected_row_index, SMP_TB_FILE_PATH_INDEX) is not None:
            f_file_name = str(self.sample_table.item(self.selected_row_index,
                                                     SMP_TB_FILE_PATH_INDEX).text())
            if f_file_name != "":
                f_graph = self.pydaw_project.get_sample_graph_by_name(f_file_name)
                self.sample_graph.draw_item(
                    f_graph.create_sample_graph(True),
                    self.sample_starts[self.selected_row_index].get_value(),
                    self.sample_ends[self.selected_row_index].get_value(),
                    self.loop_starts[self.selected_row_index].get_value(),
                    self.loop_ends[self.selected_row_index].get_value(),
                    self.fade_in_ends[self.selected_row_index].get_value(),
                    self.fade_out_starts[self.selected_row_index].get_value())
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
        self.widget.setWindowTitle("PyDAW Euphoria - {}".format(self.track_name))

    def configure_plugin(self, a_key, a_message):
        self.configure_dict[a_key] = a_message
        self.configure_callback(True, 0, self.track_num, a_key, a_message)

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
            print("Unknown configure message '{}'".format(a_key))

    def set_all_base_pitches(self):
        f_widget = pydaw_note_selector_widget(0, None, None)
        self.set_all_dialog(f_widget, self.sample_base_pitches, _("Set all base pitches"))

    def set_all_low_notes(self):
        f_widget = pydaw_note_selector_widget(0, None, None)
        self.set_all_dialog(f_widget, self.sample_low_notes, _("Set all low notes"))

    def set_all_high_notes(self):
        f_widget = pydaw_note_selector_widget(0, None, None)
        self.set_all_dialog(f_widget, self.sample_high_notes, _("Set all high notes"))

    def set_all_volumes(self):
        f_widget = pydaw_spinbox_control(_("Low Velocity"), 0, None, None, -60, 36, 0)
        self.set_all_dialog(f_widget, self.sample_vols, _("Set all volumes"))

    def set_all_vel_sens(self):
        f_widget = pydaw_spinbox_control(_("Velocity Sensitivity"), 0, None, None, 0, 20, 0)
        self.set_all_dialog(f_widget, self.sample_vel_sens, _("Set all velocity sensitivity"))

    def set_all_low_vels(self):
        f_widget = pydaw_spinbox_control(_("Low Velocity"), 0, None, None, 0, 127, 0)
        self.set_all_dialog(f_widget, self.sample_low_vels, _("Set all low velocities"))

    def set_all_high_vels(self):
        f_widget = pydaw_spinbox_control(_("High Velocity"), 0, None, None, 0, 127, 127)
        self.set_all_dialog(f_widget, self.sample_high_vels, _("Set all high velocities"))

    def set_all_pitches(self):
        f_widget = pydaw_spinbox_control(_("Pitch"), 0, None, None, -36, 36, 0)
        self.set_all_dialog(f_widget, self.sample_pitches, _("Set all sample pitches"))

    def set_all_tunes(self):
        f_widget = pydaw_spinbox_control(_("Tune"), 0, None, None, -100, 100, 0)
        self.set_all_dialog(f_widget, self.sample_tunes, _("Set all sample tunes"))

    def set_all_interpolation_modes(self):
        f_widget = pydaw_combobox_control(120, _("Mode"), 0, None, None,
                                          self.interpolation_modes_list)
        self.set_all_dialog(f_widget, self.sample_modes, _("Set all sample interpolation modes"))

    def set_all_noise_types(self):
        f_widget = pydaw_combobox_control(120, _("Noise Type"), 0, None, None,
                                          self.noise_types_list)
        self.set_all_dialog(f_widget, self.noise_types, _("Set all sample interpolation modes"))

    def set_all_noise_amps(self):
        f_widget = pydaw_spinbox_control(_("Tune"), 0, None, None, -60, 0, -30)
        self.set_all_dialog(f_widget, self.noise_amps, _("Set all noise amps"))

    def set_all_dialog(self, a_widget, a_list, a_title):
        def on_ok(a_val=None):
            f_val = a_widget.get_value()
            for f_item in a_list:
                f_item.set_value(f_val)
                f_item.control_value_changed(f_val)
            f_window.close()

        def on_cancel(a_val=None):
            f_window.close()

        f_window = QtGui.QDialog(self.widget)
        f_window.setMinimumWidth(300)
        f_window.setWindowTitle(a_title)
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_hlayout0 = QtGui.QHBoxLayout()
        f_hlayout0.addWidget(a_widget.widget)
        f_layout.addLayout(f_hlayout0)
        f_hlayout2 = QtGui.QHBoxLayout()
        f_layout.addLayout(f_hlayout2)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_ok_button.pressed.connect(on_ok)
        f_hlayout2.addWidget(f_ok_button)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_cancel_button.pressed.connect(on_cancel)
        f_hlayout2.addWidget(f_cancel_button)
        f_window.exec_()

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
            if f_current_note >= 120:
                break
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
                    if not os.path.isfile(path):
                        QtGui.QMessageBox.warning(self, _("Error"),
                                                  _("File '{}' cannot be read.").format(path))
                        continue
                    self.pydaw_project.get_wav_uid_by_name(path)
                    f_path_sections = path.split(("/"))
                    self.set_selected_sample_combobox_item(f_sample_index_to_load,
                                                           f_path_sections[-1])
                    f_item =  QtGui.QTableWidgetItem()
                    f_item.setText(path)
                    f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
                    self.sample_table.setItem(f_sample_index_to_load,
                                              SMP_TB_FILE_PATH_INDEX, f_item)
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
        path = str(self.file_selector.file_path.text()).strip()
        if path.strip() != "":
            f_uid = self.pydaw_project.get_wav_uid_by_name(path)
            self.pydaw_project.this_pydaw_osc.pydaw_reload_wavpool_item(f_uid)


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
            f_file_path = str(self.sample_table.item(self.selected_row_index,
                                                     SMP_TB_FILE_PATH_INDEX).text())
        self.file_selector.set_file(f_file_path)
        self.view_file_selector.set_file(f_file_path)
        self.set_sample_graph()
        self.loop_mode_combobox.setCurrentIndex(
            self.loop_modes[(self.selected_row_index)].get_value())

    def file_browser_load_button_pressed(self):
        f_result = self.file_browser.files_selected()
        self.load_files(f_result)

    def file_browser_preview_button_pressed(self):
        f_list = self.file_browser.files_listWidget.selectedItems()
        if len(f_list) > 0:
            f_preview_file = "{}/{}".format(
                str(self.file_browser.folder_path_lineedit.text()).strip(),
                str(f_list[0].text()).strip())
            self.pydaw_project.this_pydaw_osc.pydaw_preview_audio(f_preview_file)

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

        self.set_monofx_knob_callbacks(a_value)

    def set_monofx_knob_callbacks(self, a_value):
        for f_knob, f_nc in zip(self.monofx_knob_tuple, self.monofx_null_controls_tuple):
            if self.last_monofx_group is not None:
                f_nc[self.last_monofx_group].set_control_callback()
            f_nc[a_value].set_control_callback(f_knob)

        self.last_monofx_group = a_value


    def setSelectedMonoFX(self):
        self.mono_fx_tab_selected_group.setCurrentIndex(
            self.monofx_groups[self.selected_row_index].get_value())

    def copySamplesToSingleDirectory(self, a_dir):
        f_dir = str(a_dir)
        f_result = ""
        self.find_selected_radio_button()
        for i in range(pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT):
            f_current_file_path = self.sample_table.item(i, SMP_TB_FILE_PATH_INDEX).text()
            if (f_current_file_path is None) or (f_current_file_path == ""):
                continue
            f_file_name = os.path.basename(str(f_current_file_path))
            f_new_file_path = "{}/{}".format(f_dir, f_file_name)
            if f_current_file_path == f_new_file_path:
                print("Source and destination are the same, "
                    "not copying:\n{}\n{}".format(f_current_file_path, f_new_file_path))
            else:
                os.system('cp "{}" "{}"'.format(f_current_file_path, f_new_file_path))
                f_result += "sample|{}|{}\n".format(i, f_file_name)
        return f_result

    def saveToFile(self):
        while True:
            f_selected_path = QtGui.QFileDialog.getSaveFileName(self.widget,
            _("Select a directory to move the samples to..."), pydaw_util.global_home,
            filter=pydaw_util.global_euphoria_file_type_string)
            if f_selected_path is not None:
                f_selected_path = str(f_selected_path)
                if f_selected_path == "":
                    break
                if not f_selected_path.endswith(pydaw_util.global_euphoria_file_type_ext):
                    f_selected_path += pydaw_util.global_euphoria_file_type_ext
                f_dir = os.path.dirname(f_selected_path)
                if len(os.listdir(f_dir)) > 0:
                    f_answer = QtGui.QMessageBox.warning(self.widget, _("Warning"),
                    _("{} is not an empty directory, are you "
                    "sure you want to save here?").format(f_dir),
                    QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, QtGui.QMessageBox.No)
                    if f_answer == QtGui.QMessageBox.No:
                        continue
                f_sample_str = self.copySamplesToSingleDirectory(f_dir)
                f_plugin_file = pydaw_plugin_file.from_dict(self.port_dict, {})
                f_result_str = "{}{}".format(f_sample_str, f_plugin_file)
                pydaw_util.pydaw_write_file_text(f_selected_path, f_result_str)
                break
            else:
                break

    def openFromFile(self):
        f_selected_path = QtGui.QFileDialog.getOpenFileName(self.widget,
        _("Select a directory to move the samples to..."), pydaw_util.global_home,
        filter=pydaw_util.global_euphoria_file_type_string)
        if f_selected_path is not None:
            f_selected_path = str(f_selected_path)
            if f_selected_path == "":
                return
            f_dir = os.path.dirname(f_selected_path)
            f_file_str = pydaw_util.pydaw_read_file_text(f_selected_path)
            self.clearAllSamples()

            for f_line in f_file_str.split("\n"):
                f_line_arr = f_line.split("|", 2)
                if f_line_arr[0] == "\\":
                    break
                if f_line_arr[0] == "sample":
                    f_index = int(f_line_arr[1])
                    f_new_file_path = "{}/{}".format(f_dir, f_line_arr[2])
                    f_item =  QtGui.QTableWidgetItem()
                    f_item.setText(f_new_file_path)
                    f_item.setFlags(QtCore.Qt.ItemIsSelectable|QtCore.Qt.ItemIsEnabled)
                    self.sample_table.setItem(f_index, SMP_TB_FILE_PATH_INDEX, f_item)
                    f_path_sections = f_new_file_path.split(("/"))
                    self.set_selected_sample_combobox_item(f_index, f_path_sections[-1])
                else:
                    f_port = int(f_line_arr[0])
                    f_value = int(f_line_arr[1])
                    self.port_dict[f_port].set_value(f_value)
                    self.port_dict[f_port].control_value_changed(f_value)

            self.generate_files_string()
            self.configure_plugin("load", self.files_string)
            self.sample_table.resizeColumnsToContents()
            self.selectionChanged()

    def sfz_dialog(self):
        def on_file_open(a_val=None):
            f_selected_path = QtGui.QFileDialog.getOpenFileName(self.widget,
            _("Import SFZ instrument..."), pydaw_util.global_home,
            filter="SFZ file (*.sfz)")
            if f_selected_path is not None:
                f_selected_path = str(f_selected_path)
                if f_selected_path == "":
                    return
                try:
                    #Ensuring that it does not raise an exception
                    pydaw_util.sfz_file(f_selected_path)
                    f_file_lineedit.setText(f_selected_path)
                except Exception as ex:
                    QtGui.QMessageBox.warning(self.widget, _("Error"),
                          _("Error importing {}\n{}").format(f_selected_path, ex))
                    return

        def on_ok(a_val=None):
            f_text = str(f_file_lineedit.text())
            if f_text != "":
                for f_path in self.import_sfz(f_text):
                    f_status_label.setText(_("Loading {}").format(f_path))
                    QtGui.qApp.processEvents()
                f_window.close()

        def on_cancel(a_val=None):
            f_window.close()

        f_window = QtGui.QDialog(self.widget)
        f_window.setWindowTitle(_("Import SFZ"))
        f_window.setFixedSize(720, 180)
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_hlayout0 = QtGui.QHBoxLayout()
        f_file_lineedit = QtGui.QLineEdit()
        f_hlayout0.addWidget(f_file_lineedit)
        f_open_file_button = QtGui.QPushButton(_("Open"))
        f_open_file_button.pressed.connect(on_file_open)
        f_hlayout0.addWidget(f_open_file_button)
        f_layout.addLayout(f_hlayout0)
        f_layout.addWidget(QtGui.QLabel(_("Euphoria only supports basic SFZ parameters such as "
            "key and velocity mapping.\nAny effects such as filters, etc... "
            "should be set manually after import.")))
        f_hlayout1 = QtGui.QHBoxLayout()
        f_layout.addItem(QtGui.QSpacerItem(10, 10, vPolicy=QtGui.QSizePolicy.Expanding))
        f_layout.addLayout(f_hlayout1)
        f_status_label = QtGui.QLabel()
        f_hlayout1.addWidget(f_status_label)
        f_hlayout2 = QtGui.QHBoxLayout()
        f_layout.addLayout(f_hlayout2)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_ok_button.pressed.connect(on_ok)
        f_hlayout2.addWidget(f_ok_button)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_cancel_button.pressed.connect(on_cancel)
        f_hlayout2.addWidget(f_cancel_button)
        f_window.exec_()

    def import_sfz(self, a_sfz_path):
        try:
            f_sfz_path = str(a_sfz_path)
            f_sfz = pydaw_util.sfz_file(f_sfz_path)
            f_sfz_dir = os.path.dirname(f_sfz_path)
            self.clearAllSamples()

            for f_index, f_sample in zip(range(len(f_sfz.samples)), f_sfz.samples):
                if f_index >= pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT:
                    QtGui.QMessageBox.warning(self.widget, _("Error"),
                    _("Sample count {} exceeds maximum of {}, not loading all samples").format(
                    len(f_sfz.samples), pydaw_ports.EUPHORIA_MAX_SAMPLE_COUNT))
                    return
                if "sample" in f_sample.dict:
                    f_sample_file = f_sample.dict["sample"].replace("\\", "/")
                    f_new_file_path = "{}/{}".format(f_sfz_dir, f_sample_file)
                    f_new_file_path = pydaw_util.case_insensitive_path(f_new_file_path)
                    f_new_file_path = f_new_file_path.replace("//", "/")

                    yield f_new_file_path

                    f_item =  QtGui.QTableWidgetItem()
                    f_item.setText(f_new_file_path)
                    f_item.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled)
                    self.sample_table.setItem(f_index, SMP_TB_FILE_PATH_INDEX, f_item)
                    f_path_sections = f_new_file_path.split(("/"))
                    self.set_selected_sample_combobox_item(f_index, f_path_sections[-1])

                    f_graph = self.pydaw_project.get_sample_graph_by_name(f_new_file_path)
                    f_frame_count = float(f_graph.frame_count)

                    if "key" in f_sample.dict:
                        f_val = int(float(f_sample.dict["key"]))
                        self.sample_base_pitches[f_index].set_value(f_val)
                        self.sample_base_pitches[f_index].control_value_changed(f_val)
                        self.sample_high_notes[f_index].set_value(f_val)
                        self.sample_high_notes[f_index].control_value_changed(f_val)
                        self.sample_low_notes[f_index].set_value(f_val)
                        self.sample_low_notes[f_index].control_value_changed(f_val)

                    if "pitch_keycenter" in f_sample.dict:
                        f_val = int(float(f_sample.dict["pitch_keycenter"]))
                        self.sample_base_pitches[f_index].set_value(f_val)
                        self.sample_base_pitches[f_index].control_value_changed(f_val)

                    if "lokey" in f_sample.dict:
                        f_val = int(float(f_sample.dict["lokey"]))
                        self.sample_low_notes[f_index].set_value(f_val)
                        self.sample_low_notes[f_index].control_value_changed(f_val)

                    if "hikey" in f_sample.dict:
                        f_val = int(float(f_sample.dict["hikey"]))
                        self.sample_high_notes[f_index].set_value(f_val)
                        self.sample_high_notes[f_index].control_value_changed(f_val)

                    if "offset" in f_sample.dict:
                        f_val = (float(f_sample.dict["offset"]) / f_frame_count) * 1000.0
                        f_val = pydaw_util.pydaw_clip_value(f_val, 0.0, 998.0)
                        self.sample_starts[f_index].set_value(f_val)
                    else:
                        self.sample_starts[f_index].set_value(0.0)

                    self.sample_starts[f_index].control_value_changed(
                        self.sample_starts[f_index].get_value())

                    if "end" in f_sample.dict:
                        f_val = (float(f_sample.dict["end"]) / f_frame_count) * 1000.0
                        f_val = pydaw_util.pydaw_clip_value(f_val, 1.0, 999.0)
                        self.sample_ends[f_index].set_value(f_val)
                    else:
                        self.sample_ends[f_index].set_value(999.0)

                    self.sample_ends[f_index].control_value_changed(
                        self.sample_starts[f_index].get_value())

                    if "loop_mode" in f_sample.dict:
                        if f_sample.dict["loop_mode"].strip() == "loop_continuous":
                            self.loop_modes[f_index].set_value(1)
                        else:
                            self.loop_modes[f_index].set_value(0)
                    else:
                        self.loop_modes[f_index].set_value(0)

                    self.loop_modes[f_index].control_value_changed(
                        self.loop_modes[f_index].get_value())

                    if "loop_start" in f_sample.dict:
                        f_val = (float(f_sample.dict["loop_start"]) / f_frame_count) * 1000.0
                        f_val = pydaw_util.pydaw_clip_value(f_val, 0.0, 1000.0)
                        self.loop_starts[f_index].set_value(f_val)
                    else:
                        self.loop_starts[f_index].set_value(0.0)

                    self.loop_starts[f_index].control_value_changed(
                        self.loop_starts[f_index].get_value())

                    if "loop_end" in f_sample.dict:
                        f_val = (float(f_sample.dict["loop_end"]) / f_frame_count) * 1000.0
                        f_val = pydaw_util.pydaw_clip_value(f_val, 0.0, 1000.0)
                        self.loop_ends[f_index].set_value(f_val)
                    else:
                        self.loop_ends[f_index].set_value(1000.0)

                    self.loop_ends[f_index].control_value_changed(
                        self.loop_ends[f_index].get_value())

                    if "volume" in f_sample.dict:
                        f_val = int(float(f_sample.dict["volume"]))
                        f_val = pydaw_util.pydaw_clip_value(f_val, -40, 12)
                        self.sample_vols[f_index].set_value(f_val)
                        self.sample_vols[f_index].control_value_changed(f_val)

        except Exception as ex:
            QtGui.QMessageBox.warning(self.widget, _("Error"),
            _("Error parsing {}\n{}").format(a_sfz_path, ex))

        self.generate_files_string()
        self.configure_plugin("load", self.files_string)
        self.sample_table.resizeColumnsToContents()
        self.selectionChanged()



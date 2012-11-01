#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
PyDAW is a DAW using Python and Qt for the UI, with a high performance
audio/MIDI back end written in C

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import sys, os
from PyQt4 import QtGui, QtCore
from sys import argv
from os.path import expanduser
from libpydaw import *

class song_editor:
    def open_song(self):
        self.table_widget.clear()
        self.song = this_pydaw_project.get_song()
        for f_pos, f_region in self.song.regions.iteritems():
            self.table_widget.setItem(0, f_pos, QtGui.QTableWidgetItem(f_region))

    def cell_clicked(self, x, y):
        f_cell = self.table_widget.item(x, y)

        if this_edit_mode_selector.add_radiobutton.isChecked():
            if f_cell is None:
                def song_ok_handler():
                    f_new_cell = QtGui.QTableWidgetItem(f_new_lineedit.text())
                    if f_new_radiobutton.isChecked():
                        this_pydaw_project.create_empty_region(f_new_lineedit.text())
                    elif f_copy_radiobutton.isChecked():
                        this_pydaw_project.copy_region(str(f_copy_combobox.currentText()), str(f_new_lineedit.text()))

                    self.table_widget.setItem(x, y, f_new_cell)
                    self.song.add_region_ref(y, str(f_new_lineedit.text()))
                    this_region_editor.open_region(f_new_lineedit.text())
                    this_pydaw_project.save_song(self.song)
                    f_window.close()

                def song_cancel_handler():
                    f_window.close()

                f_window = QtGui.QDialog()
                f_layout = QtGui.QGridLayout()
                f_window.setLayout(f_layout)
                f_new_radiobutton = QtGui.QRadioButton()
                f_new_radiobutton.setChecked(True)
                f_layout.addWidget(f_new_radiobutton, 0, 0)
                f_layout.addWidget(QtGui.QLabel("New:"), 0, 1)
                f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_region_name())
                f_new_lineedit.setMaxLength(24)
                f_layout.addWidget(f_new_lineedit, 0, 2)
                f_copy_radiobutton = QtGui.QRadioButton()
                f_layout.addWidget(f_copy_radiobutton, 1, 0)
                f_copy_combobox = QtGui.QComboBox()
                f_copy_combobox.addItems(this_pydaw_project.get_region_list())
                f_layout.addWidget(QtGui.QLabel("Copy from:"), 1, 1)
                f_layout.addWidget(f_copy_combobox, 1, 2)
                f_ok_button = QtGui.QPushButton("OK")
                f_layout.addWidget(f_ok_button, 5,0)
                f_ok_button.clicked.connect(song_ok_handler)
                f_cancel_button = QtGui.QPushButton("Cancel")
                f_layout.addWidget(f_cancel_button, 5,1)
                f_cancel_button.clicked.connect(song_cancel_handler)
                f_window.exec_()
            else:
                this_region_editor.open_region(str(f_cell.text()))
        elif this_edit_mode_selector.delete_radiobutton.isChecked():
            self.song.remove_region_ref(y)
            this_pydaw_project.save_song(self.song)
            self.table_widget.clear()
            self.open_song()
        elif this_edit_mode_selector.copy_paste_radiobutton.isChecked():
            if f_cell is None:
                if not self.copied_cell is None:
                    f_new_cell = QtGui.QTableWidgetItem(self.copied_cell)
                    self.table_widget.setItem(x, y, f_new_cell)
                    self.song.add_region_ref(y, self.copied_cell)
                    this_pydaw_project.save_song(self.song)
            else:
                self.copied_cell = str(f_cell.text())

    def __init__(self):
        self.copied_cell = None
        self.song = pydaw_song()
        self.group_box = QtGui.QGroupBox()
        self.group_box.setMaximumHeight(200)
        self.main_vlayout = QtGui.QVBoxLayout()
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)
        self.group_box.setLayout(self.main_vlayout)

        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(300)
        self.table_widget.setRowCount(1)
        self.table_widget.cellClicked.connect(self.cell_clicked)
        self.main_vlayout.addWidget(self.table_widget)

class region_list_editor:
    def open_region(self, a_file_name):
        self.enabled = True
        self.table_widget.clear()
        self.region_name_lineedit.setText(a_file_name)
        self.region = this_pydaw_project.get_region(a_file_name)
        for f_item in self.region.items:
            self.table_widget.setItem(f_item.track_num, f_item.bar_num, QtGui.QTableWidgetItem(f_item.item_name))

    def cell_clicked(self, x, y):
        if not self.enabled:
            return
        f_item = self.table_widget.item(x, y)
        if this_edit_mode_selector.add_radiobutton.isChecked() or this_edit_mode_selector.copy_paste_radiobutton.isChecked():
            if f_item is None:
                self.show_cell_dialog(x, y)
            else:
                f_item_name = str(f_item.text())
                if f_item_name != "":
                    this_item_editor.open_item(f_item_name)
                else:
                    self.show_cell_dialog(x, y)
        if this_edit_mode_selector.delete_radiobutton.isChecked():
            self.region.remove_item_ref(x, y)
            this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            self.table_widget.clear()
            self.open_region(str(self.region_name_lineedit.text()))
            

    def show_cell_dialog(self, x, y):
        def note_ok_handler():
            if f_new_radiobutton.isChecked():
                f_cell_text = str(f_new_lineedit.text())
                this_pydaw_project.create_empty_item(f_new_lineedit.text())
                this_pydaw_project.this_dssi_gui.pydaw_save_item(f_new_lineedit.text())
            elif f_copy_radiobutton.isChecked():
                f_cell_text = str(f_copy_combobox.currentText())

            if f_new_radiobutton.isChecked():
                this_item_editor.open_item(f_cell_text)
            f_new_cell = QtGui.QTableWidgetItem(f_cell_text)
            self.region.add_item_ref(x, y, f_cell_text)
            print("self.region.add_item_ref(" + str(x) + ", " + str(y) + ", " + f_cell_text + ")\n")
            self.table_widget.setItem(x, y, f_new_cell)
            this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            f_window.close()

        def note_cancel_handler():
            f_window.close()
            
        def copy_combobox_index_changed(a_index):
            f_copy_radiobutton.setChecked(True)

        f_window = QtGui.QDialog()
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_new_radiobutton = QtGui.QRadioButton()
        f_new_radiobutton.setChecked(True)
        f_layout.addWidget(f_new_radiobutton, 0, 0)
        f_layout.addWidget(QtGui.QLabel("New:"), 0, 1)
        f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_item_name())
        f_new_lineedit.setMaxLength(24)
        f_layout.addWidget(f_new_lineedit, 0, 2)
        f_copy_radiobutton = QtGui.QRadioButton()
        f_layout.addWidget(f_copy_radiobutton, 1, 0)
        f_copy_combobox = QtGui.QComboBox()
        f_copy_combobox.addItems(this_pydaw_project.get_item_list())
        f_copy_combobox.currentIndexChanged.connect(copy_combobox_index_changed)
        f_layout.addWidget(QtGui.QLabel("Existing:"), 1, 1)
        f_layout.addWidget(f_copy_combobox, 1, 2)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_window.exec_()

    def cell_doubleclicked(self, x, y):
        f_clip = QtGui.QApplication.clipboard()
        f_item = QtGui.QTableWidgetItem(f_clip.text())
        self.table_widget.setItem(x, y, f_item)

    def __init__(self):
        self.enabled = False #Prevents user from editing a region before one has been selected
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QVBoxLayout()

        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)
        self.region_num_label = QtGui.QLabel()
        self.region_num_label.setText("Region:")
        self.hlayout0.addWidget(self.region_num_label)
        self.region_name_lineedit = QtGui.QLineEdit("Region0")
        self.region_name_lineedit.setEnabled(False)
        self.hlayout0.addWidget(self.region_name_lineedit)
        self.open_new_items_checkbox = QtGui.QCheckBox("Open New Items In Editor")
        self.open_new_items_checkbox.setChecked(True)
        self.hlayout0.addWidget(self.open_new_items_checkbox)

        self.group_box.setLayout(self.main_vlayout)
        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(8)
        self.table_widget.setRowCount(16)
        self.table_widget.cellClicked.connect(self.cell_clicked)
        self.table_widget.cellDoubleClicked.connect(self.cell_doubleclicked)
        self.main_vlayout.addWidget(self.table_widget)

class item_list_editor:
    #If a_new_file_name is set, a_file_name will be copied into a new file name with the name a_new_file_name
    def __init__(self):
        self.enabled = False
        self.group_box = QtGui.QGroupBox()
        self.main_hlayout = QtGui.QHBoxLayout()
        self.group_box.setLayout(self.main_hlayout)

        self.notes_table_widget = QtGui.QTableWidget()
        self.notes_table_widget.setColumnCount(5)
        self.notes_table_widget.setRowCount(128)
        self.notes_table_widget.cellClicked.connect(self.notes_click_handler)
        self.notes_table_widget.setSortingEnabled(True)

        self.ccs_table_widget = QtGui.QTableWidget()
        self.ccs_table_widget.setColumnCount(3)
        self.ccs_table_widget.setRowCount(128)
        self.ccs_table_widget.cellClicked.connect(self.ccs_click_handler)
        self.ccs_table_widget.setSortingEnabled(True)

        self.main_hlayout.addWidget(self.notes_table_widget)
        self.main_hlayout.addWidget(self.ccs_table_widget)
        self.set_headers()        
        self.default_note_start = 0.0
        self.default_note_length = 1.0
        self.default_note_note = 0
        self.default_note_octave = 3
        self.default_note_velocity = 100
        self.default_cc_num = 0
        self.default_cc_start = 0.0
        self.default_cc_val = 0

    def set_headers(self): #Because clearing the table clears the headers
        self.notes_table_widget.setHorizontalHeaderLabels(['Start', 'Length', 'Note', 'Note#', 'Velocity'])
        self.ccs_table_widget.setHorizontalHeaderLabels(['Start', 'CC', 'Value'])

    def open_item(self, a_item_name):
        self.enabled = True
        self.notes_table_widget.clear()
        self.ccs_table_widget.clear()
        self.set_headers()
        self.item_name = a_item_name
        self.item = this_pydaw_project.get_item(a_item_name)
        self.notes_table_widget.setSortingEnabled(False)
        f_i = 0
        for note in self.item.notes:
            f_note_str = note_num_to_string(note.note_num)
            self.notes_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(note.start)))
            self.notes_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(str(note.length)))
            self.notes_table_widget.setItem(f_i, 2, QtGui.QTableWidgetItem(f_note_str))
            self.notes_table_widget.setItem(f_i, 3, QtGui.QTableWidgetItem(str(note.note_num)))
            self.notes_table_widget.setItem(f_i, 4, QtGui.QTableWidgetItem(str(note.velocity)))
            f_i = f_i + 1
        self.notes_table_widget.setSortingEnabled(True)
        self.ccs_table_widget.setSortingEnabled(False)
        f_i = 0
        for cc in self.item.ccs:
            self.ccs_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(cc.start)))
            self.ccs_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(str(cc.cc_num)))
            self.ccs_table_widget.setItem(f_i, 2, QtGui.QTableWidgetItem(str(cc.cc_val)))
            f_i = f_i + 1
        if this_region_editor.open_new_items_checkbox.isChecked():
            this_main_window.main_tabwidget.setCurrentIndex(1)
        self.ccs_table_widget.setSortingEnabled(True)

    def notes_click_handler(self, x, y):
        if not self.enabled:
            return
        if this_edit_mode_selector.add_radiobutton.isChecked() or this_edit_mode_selector.copy_paste_radiobutton.isChecked():
            self.notes_show_event_dialog(x, y)
        elif this_edit_mode_selector.delete_radiobutton.isChecked():
            self.item.remove_note(pydaw_note(self.notes_table_widget.item(x, 0).text(), self.notes_table_widget.item(x, 1).text(), self.notes_table_widget.item(x, 3).text(), self.notes_table_widget.item(x, 4).text()))
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)

    def ccs_click_handler(self, x, y):
        if not self.enabled:
            return
        if this_edit_mode_selector.add_radiobutton.isChecked() or this_edit_mode_selector.copy_paste_radiobutton.isChecked():
            self.ccs_show_event_dialog(x, y)
        elif this_edit_mode_selector.delete_radiobutton.isChecked():
            self.item.remove_cc(pydaw_cc(self.ccs_table_widget.item(x, 0).text(), self.ccs_table_widget.item(x, 1).text(), self.ccs_table_widget.item(x, 2).text()))
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)

    def notes_show_event_dialog(self, x, y):
        f_cell = self.notes_table_widget.item(x, y)
        if f_cell is not None:
            self.is_existing_note = True
            self.default_note_start = float(self.notes_table_widget.item(x, 0).text())
            self.default_note_length = float(self.notes_table_widget.item(x, 1).text())
            self.default_note_note = int(self.notes_table_widget.item(x, 3).text()) % 12
            self.default_note_octave = (int(self.notes_table_widget.item(x, 3).text()) / 12) - 2
            self.default_note_velocity = int(self.notes_table_widget.item(x, 4).text())
        else:
            self.is_existing_note = False
            
        def note_ok_handler():
            if self.is_existing_note:
                self.item.remove_note(pydaw_note(self.notes_table_widget.item(x, 0).text(), self.notes_table_widget.item(x, 1).text(), self.notes_table_widget.item(x, 3).text(), self.notes_table_widget.item(x, 4).text()))
            f_note_value = (int(f_note.currentIndex()) + (int(f_octave.value()) + 2) * 12)
            f_note_name = str(f_note.currentText()) + str(f_octave.value())
            f_new_note = pydaw_note(f_start.value(), f_length.value(), f_note_value, f_velocity.value())
            if not self.item.add_note(f_new_note):
                QtGui.QMessageBox.warning(f_window, "Error", "Overlapping note events")
                return

            self.default_note_start = f_start.value()
            self.default_note_length = f_length.value()
            self.default_note_note = int(f_note.currentIndex())
            self.default_note_octave = int(f_octave.value())
            self.default_note_velocity = int(f_velocity.value())
            
            self.notes_table_widget.setSortingEnabled(False)
            f_start_item = QtGui.QTableWidgetItem(str(f_start.value()))
            self.notes_table_widget.setItem(x, 0, f_start_item)
            f_length_item = QtGui.QTableWidgetItem(str(f_length.value()))
            self.notes_table_widget.setItem(x, 1, f_length_item)                        
            f_note_name_item = QtGui.QTableWidgetItem(f_note_name)
            self.notes_table_widget.setItem(x, 2, f_note_name_item)
            f_note_num = QtGui.QTableWidgetItem(str(f_note_value))
            self.notes_table_widget.setItem(x, 3, f_note_num)
            f_vel_item = QtGui.QTableWidgetItem(str(f_velocity.value()))
            self.notes_table_widget.setItem(x, 4, f_vel_item)
            self.notes_table_widget.setSortingEnabled(True)

            this_pydaw_project.save_item(self.item_name, self.item)
            f_window.close()

        def note_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog()
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_note_layout = QtGui.QHBoxLayout()
        f_note = QtGui.QComboBox()
        f_note.addItems(['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'])
        f_note.setCurrentIndex(self.default_note_note)
        f_note_layout.addWidget(f_note)
        f_layout.addWidget(QtGui.QLabel("Note"), 1, 0)
        f_octave = QtGui.QSpinBox()
        f_octave.setRange(-2, 8)
        f_octave.setValue(self.default_note_octave)
        f_note_layout.addWidget(f_octave)
        f_layout.addLayout(f_note_layout, 1, 1)
        f_layout.addWidget(QtGui.QLabel("Start(beats)"), 2, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_start.setValue(self.default_note_start)
        f_layout.addWidget(f_start, 2, 1)
        f_layout.addWidget(QtGui.QLabel("Length(beats)"), 3, 0)
        f_length = QtGui.QDoubleSpinBox()
        f_length.setRange(0.1, 16.0)
        f_length.setValue(self.default_note_length)
        f_layout.addWidget(f_length, 3, 1)
        f_velocity = QtGui.QSpinBox()
        f_velocity.setRange(1, 127)
        f_velocity.setValue(self.default_note_velocity)
        f_layout.addWidget(QtGui.QLabel("Velocity"), 4, 0)
        f_layout.addWidget(f_velocity, 4, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_window.exec_()

    def ccs_show_event_dialog(self, x, y):
        f_cell = self.ccs_table_widget.item(x, y)
        if f_cell is not None:
            self.default_cc_start = float(self.ccs_table_widget.item(x, 0).text())
            self.default_cc_num = int(self.ccs_table_widget.item(x, 1).text())
            self.default_cc_val = int(self.ccs_table_widget.item(x, 2).text())

        def cc_ok_handler():
            if not self.item.add_cc(pydaw_cc(f_start.value(), f_cc.value(), f_cc_value.value())):
                QtGui.QMessageBox.warning(f_window, "Error", "Duplicate CC event")
                return

            self.default_cc_start = f_start.value()
            self.default_cc_num = f_cc.value()
            self.default_cc_start = f_start.value()
            
            self.ccs_table_widget.setSortingEnabled(False)
            f_start_item = QtGui.QTableWidgetItem(str(f_start.value()))            
            self.ccs_table_widget.setItem(x, 0, f_start_item)
            f_cc_num_item = QtGui.QTableWidgetItem(str(f_cc.value()))
            self.ccs_table_widget.setItem(x, 1, f_cc_num_item)
            f_cc_val_item = QtGui.QTableWidgetItem(str(f_cc_value.value()))
            self.ccs_table_widget.setItem(x, 2, f_cc_val_item)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.ccs_table_widget.setSortingEnabled(True)
            f_window.close()

        def cc_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog()
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_cc = QtGui.QSpinBox()
        f_cc.setRange(1, 127)
        f_cc.setValue(self.default_cc_num)
        f_layout.addWidget(QtGui.QLabel("CC"), 0, 0)
        f_layout.addWidget(f_cc, 0, 1)
        f_cc_value = QtGui.QSpinBox()
        f_cc_value.setRange(1, 127)
        f_cc_value.setValue(self.default_cc_val)
        f_layout.addWidget(QtGui.QLabel("Value"), 1, 0)
        f_layout.addWidget(f_cc_value, 1, 1)
        f_layout.addWidget(QtGui.QLabel("Position(beats)"), 2, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_start.setValue(self.default_cc_start)
        f_layout.addWidget(f_start, 2, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 4,0)
        f_ok_button.clicked.connect(cc_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 4,1)
        f_cancel_button.clicked.connect(cc_cancel_handler)
        f_window.exec_()

rec_button_group = QtGui.QButtonGroup()

class seq_track:
    def on_vol_change(self, value):
        self.volume_label.setText(str(value) + " dB")
        this_pydaw_project.this_dssi_gui.pydaw_set_vol(self.track_number, self.volume_slider.value())
        this_pydaw_project.save_tracks(this_track_editor.get_tracks())
    def on_pan_change(self, value):
        this_pydaw_project.save_tracks(this_track_editor.get_tracks())
    def on_solo(self, value):
        this_pydaw_project.this_dssi_gui.pydaw_set_solo(self.track_number, self.solo_checkbox.isChecked())
        this_pydaw_project.save_tracks(this_track_editor.get_tracks())
    def on_mute(self, value):
        this_pydaw_project.this_dssi_gui.pydaw_set_mute(self.track_number, self.mute_checkbox.isChecked())
        this_pydaw_project.save_tracks(this_track_editor.get_tracks())
    def on_rec(self, value):
        this_pydaw_project.this_dssi_gui.pydaw_set_track_rec(self.track_number, self.record_radiobutton.isChecked())
        this_pydaw_project.save_tracks(this_track_editor.get_tracks())
    def on_name_changed(self, new_name):
        this_pydaw_project.save_tracks(this_track_editor.get_tracks())
    def on_instrument_change(self, selected_instrument):
        #this_pydaw_project.session_mgr.instrument_index_changed(self.track_number, selected_instrument, str(self.track_name_lineedit.text()))
        if selected_instrument == 0:
            self.track_name_lineedit.setEnabled(True)
        else:
            self.track_name_lineedit.setEnabled(False)
        this_pydaw_project.save_tracks(this_track_editor.get_tracks())
        this_pydaw_project.this_dssi_gui.pydaw_set_instrument_index(self.track_number, selected_instrument)
    def on_show_ui(self):
        if self.instrument_combobox.currentIndex() > 0:
            this_pydaw_project.this_dssi_gui.pydaw_show_ui(self.track_number)

    def __init__(self, a_track_num, a_track_text="track"):
        self.track_number = a_track_num
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.main_vlayout)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout1)
        self.solo_checkbox = QtGui.QCheckBox()
        self.solo_checkbox.setText("Solo")
        self.solo_checkbox.clicked.connect(self.on_solo)
        self.hlayout1.addWidget(self.solo_checkbox)
        self.mute_checkbox = QtGui.QCheckBox()
        self.mute_checkbox.setText("Mute")
        self.mute_checkbox.clicked.connect(self.on_mute)
        self.hlayout1.addWidget(self.mute_checkbox)
        self.record_radiobutton = QtGui.QRadioButton()
        self.record_radiobutton.setText("Rec")
        rec_button_group.addButton(self.record_radiobutton)
        self.record_radiobutton.toggled.connect(self.on_rec)
        self.hlayout1.addWidget(self.record_radiobutton)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout2)
        self.volume_slider = QtGui.QSlider()
        self.volume_slider.setMinimum(-50)
        self.volume_slider.setMaximum(12)
        self.volume_slider.setValue(0)
        self.volume_slider.setOrientation(QtCore.Qt.Horizontal)
        self.volume_slider.valueChanged.connect(self.on_vol_change)
        self.hlayout2.addWidget(self.volume_slider)
        self.volume_label = QtGui.QLabel()
        self.volume_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
        self.volume_label.setMargin(3)
        self.volume_label.setMinimumWidth(51)
        self.volume_label.setText("0 dB")
        self.hlayout2.addWidget(self.volume_label)
        self.hlayout3 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout3)
        self.track_name_lineedit = QtGui.QLineEdit()
        self.track_name_lineedit.setText(a_track_text)
        self.track_name_lineedit.setMaxLength(24)
        self.track_name_lineedit.setMaximumWidth(90)
        self.track_name_lineedit.textChanged.connect(self.on_name_changed)
        self.hlayout3.addWidget(self.track_name_lineedit)
        self.instrument_combobox = QtGui.QComboBox()
        self.instrument_combobox.addItems(["None", "Euphoria", "Ray-V"])
        self.instrument_combobox.currentIndexChanged.connect(self.on_instrument_change)
        self.instrument_combobox.setMinimumWidth(100)
        self.hlayout3.addWidget(self.instrument_combobox)
        self.ui_button = QtGui.QPushButton("UI")
        self.ui_button.pressed.connect(self.on_show_ui)
        self.ui_button.setMaximumWidth(30)
        self.hlayout3.addWidget(self.ui_button)
        
    def open_track(self, a_track):
        self.record_radiobutton.setChecked(a_track.rec)
        self.solo_checkbox.setChecked(a_track.solo)
        self.mute_checkbox.setChecked(a_track.mute)
        self.track_name_lineedit.setText(a_track.name)
        self.volume_slider.setValue(a_track.vol)
        self.instrument_combobox.setCurrentIndex(a_track.inst)

    def get_track(self):
        return pydaw_track(self.solo_checkbox.isChecked(), self.mute_checkbox.isChecked(), self.record_radiobutton.isChecked(),
                           self.volume_slider.value(), str(self.track_name_lineedit.text()), self.instrument_combobox.currentIndex())

class transport_widget:
    def on_play(self):
        self.last_region_num = self.region_spinbox.value()
        self.last_bar = self.bar_spinbox.value()
        this_pydaw_project.this_dssi_gui.pydaw_play(a_region_num=self.region_spinbox.value(), a_bar=self.bar_spinbox.value())
        f_playback_inc = int(((1.0/(float(self.tempo_spinbox.value()) / 60)) * 4000))
        self.beat_timer.start(f_playback_inc)
    def on_stop(self):
        this_pydaw_project.this_dssi_gui.pydaw_stop()
        self.beat_timer.stop()
        self.bar_spinbox.setValue(self.last_bar)
        self.region_spinbox.setValue(self.last_region_num)
    def on_rec(self):
        this_pydaw_project.this_dssi_gui.pydaw_rec()
    def on_tempo_changed(self, a_tempo):
        this_pydaw_project.this_dssi_gui.pydaw_set_tempo(a_tempo)
    def on_loop_mode_changed(self, a_loop_mode):
        this_pydaw_project.this_dssi_gui.pydaw_set_loop_mode(a_loop_mode)
    def on_keybd_combobox_index_changed(self, a_index):
        self.alsa_output_ports.connect_to_pydaw(str(self.keybd_combobox.currentText()))
    def beat_timeout(self):
        if self.loop_mode_combobox.currentIndex() == 1:
            return  #Looping a single bar doesn't require these values to update
        f_new_bar_value = self.bar_spinbox.value() + 1
        self.region_spinbox.value()
        if f_new_bar_value >= 8:
            f_new_bar_value = 0
            if self.loop_mode_combobox.currentIndex() != 2:
                self.region_spinbox.setValue(self.region_spinbox.value() + 1)
        self.bar_spinbox.setValue(f_new_bar_value)

    def __init__(self):
        self.group_box = QtGui.QGroupBox()
        self.grid_layout = QtGui.QGridLayout()
        self.group_box.setLayout(self.grid_layout)
        self.play_button = QtGui.QPushButton()
        self.play_button.setText("Play")
        self.play_button.setMinimumWidth(75)
        self.play_button.pressed.connect(self.on_play)
        self.grid_layout.addWidget(self.play_button, 0, 0)
        self.stop_button = QtGui.QPushButton()
        self.stop_button.setText("Stop")
        self.stop_button.setMinimumWidth(75)
        self.stop_button.pressed.connect(self.on_stop)
        self.grid_layout.addWidget(self.stop_button, 0, 1)
        self.rec_button = QtGui.QPushButton()
        self.rec_button.setText("Rec")
        self.rec_button.setMinimumWidth(75)
        self.rec_button.pressed.connect(self.on_rec)
        self.grid_layout.addWidget(self.rec_button, 0, 2)
        self.grid_layout.addWidget(QtGui.QLabel("BPM:"), 0, 3)
        self.tempo_spinbox = QtGui.QSpinBox()
        self.tempo_spinbox.setRange(50, 200)
        self.tempo_spinbox.valueChanged.connect(self.on_tempo_changed)
        self.grid_layout.addWidget(self.tempo_spinbox, 0, 4)
        self.grid_layout.addWidget(QtGui.QLabel("MIDI Keybd:"), 0, 5)
        self.keybd_combobox = QtGui.QComboBox()
        self.alsa_output_ports = alsa_ports()
        self.keybd_combobox.addItems(self.alsa_output_ports.get_output_fqnames())
        self.keybd_combobox.currentIndexChanged.connect(self.on_keybd_combobox_index_changed)
        self.grid_layout.addWidget(self.keybd_combobox, 0, 6)
        if self.keybd_combobox.count() > 0:
            self.on_keybd_combobox_index_changed(0)
        self.grid_layout.addWidget(QtGui.QLabel("Loop Mode"), 0, 7)
        self.loop_mode_combobox = QtGui.QComboBox()
        self.loop_mode_combobox.addItems(["Off", "Bar", "Region"])
        self.loop_mode_combobox.setMinimumWidth(60)
        self.loop_mode_combobox.currentIndexChanged.connect(self.on_loop_mode_changed)
        self.grid_layout.addWidget(self.loop_mode_combobox, 0, 8)
        self.grid_layout.addWidget(QtGui.QLabel("Region:"), 0, 9)
        self.region_spinbox = QtGui.QSpinBox()
        self.region_spinbox.setRange(0, 300)
        self.grid_layout.addWidget(self.region_spinbox, 0, 10)
        self.grid_layout.addWidget(QtGui.QLabel("Bar:"), 0, 11)
        self.bar_spinbox = QtGui.QSpinBox()
        self.bar_spinbox.setRange(0, 8)
        self.grid_layout.addWidget(self.bar_spinbox, 0, 12)
        #This is an awful way to do this, I'll eventually have IPC that goes both directions...
        self.beat_timer = QtCore.QTimer()
        self.beat_timer.timeout.connect(self.beat_timeout)
        

class edit_mode_selector:
    def __init__(self):
        self.groupbox = QtGui.QGroupBox()
        self.hlayout0 = QtGui.QHBoxLayout(self.groupbox)
        self.hlayout0.addWidget(QtGui.QLabel("Edit Mode:"))
        self.add_radiobutton = QtGui.QRadioButton("Add")
        self.hlayout0.addWidget(self.add_radiobutton)
        self.delete_radiobutton = QtGui.QRadioButton("Delete")
        self.hlayout0.addWidget(self.delete_radiobutton)
        self.copy_paste_radiobutton = QtGui.QRadioButton("Copy/Paste")
        self.hlayout0.addWidget(self.copy_paste_radiobutton)
        self.add_radiobutton.setChecked(True)

class track_editor:
    def open_tracks(self):
        f_tracks = this_pydaw_project.get_tracks()
        for key, f_track in f_tracks.tracks.iteritems():
            self.tracks[key].open_track(f_track)

    def reset(self):
        self.tracks_tablewidget.clear()
        self.tracks_tablewidget.setHorizontalHeaderLabels(['Tracks'])
        self.tracks = []
        for i in range(0, 16):
            track = seq_track(a_track_num=i, a_track_text="track" + str(i + 1))
            self.tracks.append(track)
            self.tracks_tablewidget.insertRow(i)
            self.tracks_tablewidget.setCellWidget(i, 0, track.group_box)
        self.tracks_tablewidget.resizeColumnsToContents()
        self.tracks_tablewidget.resizeRowsToContents()
        self.tracks_tablewidget.setMaximumWidth(300)

    def __init__(self):
        self.tracks_tablewidget = QtGui.QTableWidget()
        self.tracks_tablewidget.setColumnCount(1)
        self.reset()

    def get_tracks(self):
        f_result = pydaw_tracks()
        for f_i in range(0, len(self.tracks)):
            f_result.add_track(f_i, self.tracks[f_i].get_track())
        return f_result

class pydaw_main_window(QtGui.QMainWindow):
    def on_new(self):
        f_file = QtGui.QFileDialog.getSaveFileName(parent=this_main_window ,caption='New Project', directory='.', filter='PyDAW Song (*.pysong)')
        if not f_file is None and not str(f_file) == "":
            f_file = str(f_file)
            if not f_file.endswith(".pysong"):
                f_file += ".pysong"
            global_open_project(str(f_file))
            global_new_project()
        #this_pydaw_project.new_project()
    def on_open(self):
        f_file = QtGui.QFileDialog.getOpenFileName(parent=this_main_window ,caption='Open Project', directory='.', filter='PyDAW Song (*.pysong)')
        if not f_file is None and not str(f_file) == "":
            global_open_project(str(f_file))
    def on_save(self):
        this_pydaw_project.save_project()
    def on_save_as(self):
        f_new_file = QtGui.QFileDialog.getSaveFileName(self, "Save project as...", this_pydaw_project.project_file + ".pysong")
        if f_new_file:        
            this_pydaw_project.save_project_as(f_new_file)

    def __init__(self):
        self.initUI()

    def initUI(self):
        QtGui.QMainWindow.__init__(self)
        stylesheet_file = os.getcwd() + "/pydaw/style.txt"
        pydaw_write_log("Opening CSS stylesheet from:  " + stylesheet_file)
        file_handle = open(stylesheet_file, 'r')
        self.setStyleSheet(file_handle.read())
        file_handle.close()

        self.central_widget = QtGui.QWidget()
        self.setCentralWidget(self.central_widget)

        self.main_layout = QtGui.QVBoxLayout()
        self.central_widget.setLayout(self.main_layout)

        self.menu_bar = self.menuBar()
        self.menu_file = self.menu_bar.addMenu("&File")

        self.new_action = QtGui.QAction("New", self)
        self.menu_file.addAction(self.new_action)
        self.new_action.triggered.connect(self.on_new)

        self.open_action = QtGui.QAction("Open", self)
        self.menu_file.addAction(self.open_action)
        self.open_action.triggered.connect(self.on_open)

        self.save_action = QtGui.QAction("Save", self)
        self.menu_file.addAction(self.save_action)
        self.save_action.triggered.connect(self.on_save)

        self.save_as_action = QtGui.QAction("Save As...", self)
        self.menu_file.addAction(self.save_as_action)
        self.save_as_action.triggered.connect(self.on_save_as)

        self.transport_hlayout = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.transport_hlayout)

        self.transport_hlayout.addWidget(this_transport.group_box, alignment=QtCore.Qt.AlignLeft)
        self.transport_hlayout.addWidget(this_edit_mode_selector.groupbox, alignment=QtCore.Qt.AlignLeft)

        self.main_tabwidget = QtGui.QTabWidget()
        self.song_tab = QtGui.QWidget()
        self.song_tab_hlayout = QtGui.QHBoxLayout(self.song_tab)

        self.main_tabwidget.addTab(self.song_tab, "Song")

        self.main_layout.addWidget(self.main_tabwidget)
        self.song_tab_hlayout.addWidget(this_track_editor.tracks_tablewidget)

        self.song_region_vlayout = QtGui.QVBoxLayout()
        self.song_tab_hlayout.addLayout(self.song_region_vlayout)

        self.song_region_vlayout.addWidget(this_song_editor.group_box)
        self.song_region_vlayout.addWidget(this_region_editor.group_box)

        self.item_tab = QtGui.QWidget()
        self.item_tab_hlayout = QtGui.QHBoxLayout(self.item_tab)
        self.item_tab_hlayout.addWidget(this_item_editor.group_box)

        self.main_tabwidget.addTab(self.item_tab, "Item")

        self.show()

#Opens or creates a new project
def global_open_project(a_project_file):
    #this_pydaw_project.session_mgr.quit_hander()
    global this_pydaw_project
    if(len(argv) >= 2):
        this_pydaw_project = pydaw_project((argv[1]))
    else:
        this_pydaw_project = pydaw_project()
    this_pydaw_project.open_project(a_project_file)
    this_song_editor.open_song()
    this_track_editor.open_tracks()
    this_transport.tempo_spinbox.setValue(140)  #Interim code to make the magic happen, this shouldn't be used once PyDAW goes stable, it should be read from a project file
    #this_main_window.setWindowTitle('PyDAW - ' + self.project_file)

def global_new_project():
    #this_pydaw_project.session_mgr.quit_hander()
    global this_pydaw_project
    if(len(argv) >= 2):
        this_pydaw_project = pydaw_project(a_project_file, (argv[1]))
    else:
        this_pydaw_project = pydaw_project(a_project_file)    
    this_song_editor.table_widget.clear()
    this_region_editor.table_widget.clear()
    this_item_editor.table_widget.clear()
    this_track_editor.reset()
    #this_main_window.setWindowTitle('PyDAW - ' + self.project_file)

def about_to_quit():
    this_pydaw_project.quit_handler()

if __name__ == '__main__':
    pydaw_write_log("\n\n\n***********STARTING***********\n\n")
    for arg in argv:
        pydaw_write_log("arg:  " + arg)

    app = QtGui.QApplication(sys.argv)
    app.setWindowIcon(QtGui.QIcon('pydaw.ico'))
    app.aboutToQuit.connect(about_to_quit)
    this_song_editor = song_editor()
    this_region_editor = region_list_editor()
    this_item_editor = item_list_editor()
    this_transport = transport_widget()
    this_track_editor = track_editor()
    this_edit_mode_selector = edit_mode_selector()

    this_main_window = pydaw_main_window() #You must call this after instantiating the other widgets, as it relies on them existing
    this_main_window.setWindowState(QtCore.Qt.WindowMaximized)

    default_project_file = expanduser("~") + '/dssi/pydaw/default-project/default.pysong'
    global_open_project(default_project_file)

    sys.exit(app.exec_())

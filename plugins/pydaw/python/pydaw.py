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
from os import listdir
from shutil import copyfile
from lms_session import lms_session
from dssi_gui import dssi_gui
from connect import *

class song_editor:
    def cell_clicked(self, x, y):
        f_cell = self.table_widget.item(x, y)
        if f_cell is None:
            def song_ok_handler():
                f_new_cell = QtGui.QTableWidgetItem(f_new_lineedit.text())
                if f_new_radiobutton.isChecked():
                    this_pydaw_project.create_empty_region(f_new_lineedit.text())               
                elif f_copy_radiobutton.isChecked():
                    this_pydaw_project.copy_region(str(f_copy_combobox.currentText()), str(f_new_lineedit.text()))
                    
                self.table_widget.setItem(x, y, f_new_cell)
                this_region_editor.open_region(f_new_lineedit.text())
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
            pass #TODO:  Open in region editor
    
    def __init__(self):
        self.group_box = QtGui.QGroupBox()
        self.group_box.setMaximumHeight(200)
        self.main_vlayout = QtGui.QVBoxLayout()        
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)    
        self.group_box.setLayout(self.main_vlayout)
        
        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(100)
        self.table_widget.setRowCount(1)
        self.table_widget.cellClicked.connect(self.cell_clicked)
        self.main_vlayout.addWidget(self.table_widget)

class region_list_editor:
    def open_region(self, a_file_name):
        self.region_name_lineedit.setText(a_file_name)
        f_region_string = this_pydaw_project.get_region_string(a_file_name)
        f_region_arr = f_region_string.split("\n")
        for f_i in range(0, len(f_region_arr)):
            f_track_arr = f_region_arr[f_i].split("|")
            for f_i2 in range(0, len(f_track_arr)):
                f_new_cell = QtGui.QTableWidgetItem(f_track_arr[f_i2])
                self.table_widget.setItem(f_i, f_i2, f_new_cell)                
        
    def cell_clicked(self, x, y):        
        f_item = self.table_widget.item(x, y)
        if f_item is None:
            self.show_cell_dialog(x, y)
        else:
            f_item_name = str(f_item.text())
            if f_item_name != "":
                this_item_editor.open_item(f_item_name)
            else:
                self.show_cell_dialog(x, y)
    #Maybe use __str__ for this one?                
    def get_region_string(self):
        f_result = ""
        for i in range(0, self.table_widget.rowCount()):
            for i in range(0, self.table_widget.columnCount()):
                f_item = self.table_widget.item(f_i, f_i2)
                if not f_item is None:
                    f_result += f_item.text()
                f_result += "|"
            f_result += "\n"                    
                
    def show_cell_dialog(self, x, y):
        def note_ok_handler():            
            if f_new_radiobutton.isChecked():
                f_cell_text = str(f_new_lineedit.text())                
                this_pydaw_project.create_empty_item(f_new_lineedit.text())                
            elif f_copy_radiobutton.isChecked():
                f_cell_text = str(f_copy_combobox.currentText())
            this_item_editor.open_item(f_cell_text)
            f_new_cell = QtGui.QTableWidgetItem(f_cell_text)
            self.table_widget.setItem(x, y, f_new_cell)
            #this_pydaw_project.save_region()
            f_window.close()
                
        def note_cancel_handler():            
            f_window.close()
            
        f_window = QtGui.QDialog()
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_new_radiobutton = QtGui.QRadioButton()
        f_new_radiobutton.setChecked(True)
        f_layout.addWidget(f_new_radiobutton, 0, 0)
        f_layout.addWidget(QtGui.QLabel("New:"), 0, 1)
        f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_item_name())
        f_layout.addWidget(f_new_lineedit, 0, 2)
        f_copy_radiobutton = QtGui.QRadioButton()
        f_layout.addWidget(f_copy_radiobutton, 1, 0)            
        f_copy_combobox = QtGui.QComboBox()
        f_copy_combobox.addItems(this_pydaw_project.get_item_list())                                    
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
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QVBoxLayout()
        
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)
        self.region_num_label = QtGui.QLabel()
        self.region_num_label.setText("Region:")
        self.hlayout0.addWidget(self.region_num_label)        
        self.region_name_lineedit = QtGui.QLineEdit("Region0")
        self.region_name_lineedit.setEnabled(False)
        #self.region_name_lineedit.textChanged.connect(self.region_name_changed)
        self.hlayout0.addWidget(self.region_name_lineedit)
                
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
        self.events = []                
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.main_vlayout)
        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(2)
        self.table_widget.setHorizontalHeaderLabels(['Notes', 'CCs'])
        self.table_widget.setRowCount(128)
        self.table_widget.cellClicked.connect(self.show_event_dialog)
        self.group_box.setMaximumWidth(270)
        self.main_vlayout.addWidget(self.table_widget)
        
    def open_item(self, a_item_name):
        f_this_item_string = this_pydaw_project.get_item_string(a_item_name)
        #TODO
        
    def show_event_dialog(self, x, y):
        if y == 0:
            f_cell = self.table_widget.item(x, y)
            if f_cell is None:
                f_default_start = 0.0
                f_default_length = 1.0
                f_default_note = 0
                f_default_octave = 3
                f_default_velocity = 100
            else:
                f_cell_arr = f_cell.text().split("|")                
                f_default_start = int(float(f_cell_arr[0]))
                f_default_length = int(float(f_cell_arr[1]))
                f_default_note = int(f_cell_arr[2]) % 12
                f_default_octave = (int(f_cell_arr[2]) / 12) - 2
                f_default_velocity = int(f_cell_arr[3])
            def note_ok_handler():
                f_note_value = (int(f_note.currentIndex()) + (int(f_octave.value()) + 2) * 12)
                f_new_cell = QtGui.QTableWidgetItem(str(f_start.value()) + "|" +
                    str(f_length.value()) + "|" + str(f_note_value) + "|" +
                    str(f_velocity.value()) + "|" + str(f_note.currentText()) + str(f_octave.value())
                    )
                self.table_widget.setItem(x, y, f_new_cell)
                f_window.close()
                
            def note_cancel_handler():            
                f_window.close()
                
            f_window = QtGui.QDialog()
            f_layout = QtGui.QGridLayout()
            f_window.setLayout(f_layout)
            f_note_layout = QtGui.QHBoxLayout()
            f_note = QtGui.QComboBox()
            f_note.addItems(['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'])
            f_note.setCurrentIndex(f_default_note)
            f_note_layout.addWidget(f_note)
            f_layout.addWidget(QtGui.QLabel("Note"), 1, 0)
            f_octave = QtGui.QSpinBox()
            f_octave.setRange(-2, 8)
            f_octave.setValue(f_default_octave)
            f_note_layout.addWidget(f_octave)
            f_layout.addLayout(f_note_layout, 1, 1)
            f_layout.addWidget(QtGui.QLabel("Start(beats)"), 2, 0)
            f_start = QtGui.QDoubleSpinBox()
            f_start.setRange(0.0, 3.99)
            f_start.setValue(f_default_start)
            f_layout.addWidget(f_start, 2, 1)
            f_layout.addWidget(QtGui.QLabel("Length(beats)"), 3, 0)
            f_length = QtGui.QDoubleSpinBox()
            f_length.setRange(0.1, 16.0)
            f_length.setValue(f_default_length)
            f_layout.addWidget(f_length, 3, 1)
            f_velocity = QtGui.QSpinBox()
            f_velocity.setRange(1, 127)
            f_velocity.setValue(f_default_velocity)
            f_layout.addWidget(QtGui.QLabel("Velocity"), 4, 0)
            f_layout.addWidget(f_velocity, 4, 1)
            f_ok_button = QtGui.QPushButton("OK")
            f_layout.addWidget(f_ok_button, 5,0)
            f_ok_button.clicked.connect(note_ok_handler)
            f_cancel_button = QtGui.QPushButton("Cancel")
            f_layout.addWidget(f_cancel_button, 5,1)
            f_cancel_button.clicked.connect(note_cancel_handler)
                    
            f_window.exec_()

        elif y == 1:            
            f_cell = self.table_widget.item(x, y)
            if f_cell is None:
                f_default_start = 0.0
                f_default_cc = 1
                f_default_cc_value = 64
            else:
                f_cell_arr = f_cell.text().split("|")                
                f_default_start = int(float(f_cell_arr[0]))
                f_default_cc = int(f_cell_arr[1])
                f_default_cc_value = int(f_cell_arr[2])
                
            def cc_ok_handler():                
                f_new_cell = QtGui.QTableWidgetItem(str(f_start.value()) + "|" +
                    str(f_cc.value()) + "|" + str(f_cc_value.value())
                    )
                self.table_widget.setItem(x, y, f_new_cell)
                f_window.close()
                
            def cc_cancel_handler():            
                f_window.close()
                
            f_window = QtGui.QDialog()
            f_layout = QtGui.QGridLayout()
            f_window.setLayout(f_layout)
            f_cc = QtGui.QSpinBox()
            f_cc.setRange(1, 127)
            f_cc.setValue(f_default_cc)
            f_layout.addWidget(QtGui.QLabel("CC"), 0, 0)
            f_layout.addWidget(f_cc, 0, 1)
            f_cc_value = QtGui.QSpinBox()
            f_cc_value.setRange(1, 127)
            f_cc_value.setValue(f_default_cc_value)
            f_layout.addWidget(QtGui.QLabel("Value"), 1, 0)
            f_layout.addWidget(f_cc_value, 1, 1)
            f_layout.addWidget(QtGui.QLabel("Position(beats)"), 2, 0)
            f_start = QtGui.QDoubleSpinBox()
            f_start.setRange(0.0, 3.99)
            f_start.setValue(f_default_start)
            f_layout.addWidget(f_start, 2, 1)
            f_ok_button = QtGui.QPushButton("OK")
            f_layout.addWidget(f_ok_button, 4,0)
            f_ok_button.clicked.connect(cc_ok_handler)
            f_cancel_button = QtGui.QPushButton("Cancel")
            f_layout.addWidget(f_cancel_button, 4,1)
            f_cancel_button.clicked.connect(cc_cancel_handler)
                    
            f_window.exec_()

        else:
            print(y)            
            
        
rec_button_group = QtGui.QButtonGroup()
    
class seq_track:
    def on_vol_change(self, value):
        self.volume_label.setText(str(value) + " dB")
    def on_pan_change(self, value):
        pass
    def on_solo(self, value):
        print(value)
    def on_mute(self, value):
        print(value)
    def on_rec(self, value):
        print(value)
    def on_name_changed(self, new_name):
        print(new_name)
    def on_instrument_change(self, selected_instrument):
        this_pydaw_project.session_mgr.instrument_index_changed(self.track_number, selected_instrument, str(self.track_name_lineedit.text()))
        if selected_instrument == 0:
            self.track_name_lineedit.setEnabled(True)
        else:
            self.track_name_lineedit.setEnabled(False)
    
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
        self.hlayout1.addWidget(self.mute_checkbox)
        self.record_radiobutton = QtGui.QRadioButton()
        self.record_radiobutton.setText("Rec")
        rec_button_group.addButton(self.record_radiobutton)
        self.record_radiobutton.clicked.connect(self.on_rec)
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
        self.volume_label.setMinimumWidth(48)
        self.volume_label.setText("0 dB")
        self.hlayout2.addWidget(self.volume_label)
        self.hlayout3 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout3)
        self.track_name_lineedit = QtGui.QLineEdit()
        self.track_name_lineedit.setText(a_track_text)
        self.track_name_lineedit.textChanged.connect(self.on_name_changed)
        self.hlayout3.addWidget(self.track_name_lineedit)
        self.instrument_combobox = QtGui.QComboBox()
        self.instrument_combobox.addItems(["None", "Euphoria", "Ray-V"])
        self.instrument_combobox.currentIndexChanged.connect(self.on_instrument_change)
        self.hlayout3.addWidget(self.instrument_combobox)

class transport_widget:
    def on_play(self):        
        this_dssi_gui.send_configure("play", "testing")
    def on_stop(self):
        this_dssi_gui.send_configure("stop", "testing")
    def on_rec(self):
        this_dssi_gui.send_configure("rec", "testing")
    def on_tempo_changed(self, a_tempo):
        this_dssi_gui.send_configure("tempo", str(a_tempo))
            
    def __init__(self):
        self.group_box = QtGui.QGroupBox()
        self.grid_layout = QtGui.QGridLayout()
        self.group_box.setLayout(self.grid_layout)        
        self.play_button = QtGui.QPushButton()
        self.play_button.setText("Play")
        self.play_button.pressed.connect(self.on_play)
        self.grid_layout.addWidget(self.play_button, 0, 0)
        self.stop_button = QtGui.QPushButton()
        self.stop_button.setText("Stop")
        self.stop_button.pressed.connect(self.on_stop)
        self.grid_layout.addWidget(self.stop_button, 0, 1)    
        self.rec_button = QtGui.QPushButton()
        self.rec_button.setText("Rec")
        self.rec_button.pressed.connect(self.on_rec)
        self.grid_layout.addWidget(self.rec_button, 0, 2)
        self.grid_layout.addWidget(QtGui.QLabel("BPM:"), 0, 3)
        self.tempo_spinbox = QtGui.QSpinBox()
        self.tempo_spinbox.setRange(50, 200)
        self.tempo_spinbox.setValue(140)
        self.tempo_spinbox.valueChanged.connect(self.on_tempo_changed)
        self.grid_layout.addWidget(self.tempo_spinbox, 0, 4)
        self.grid_layout.addWidget(QtGui.QLabel("MIDI Keybd:"), 0, 5)
        self.keybd_combobox = QtGui.QComboBox()
        self.alsa_output_ports = alsa_ports()
        self.keybd_combobox.addItems(self.alsa_output_ports.get_output_fqnames())
        self.grid_layout.addWidget(self.keybd_combobox, 0, 6)
        self.grid_layout.addWidget(QtGui.QLabel("Loop Mode"), 0, 7)
        self.loop_mode_combobox = QtGui.QComboBox()
        self.loop_mode_combobox.addItems(["Off", "Bar", "Region"])
        self.grid_layout.addWidget(self.loop_mode_combobox, 0, 8)
        
class track_editor:
    def __init__(self):
        self.tracks_tablewidget = QtGui.QTableWidget()
        self.tracks_tablewidget.setColumnCount(1)
        self.tracks_tablewidget.setHorizontalHeaderLabels(['Tracks'])                     
        self.tracks = []
        
        for i in range(0, 16):
            track = seq_track(a_track_num=i, a_track_text="track" + str(i))
            self.tracks.append(track)
            self.tracks_tablewidget.insertRow(i)
            self.tracks_tablewidget.setCellWidget(i, 0, track.group_box)

        self.tracks_tablewidget.resizeColumnsToContents()
        self.tracks_tablewidget.resizeRowsToContents()        
        self.tracks_tablewidget.setMaximumWidth(300)

class pydaw_main_window(QtGui.QMainWindow):
    def on_new(self):
        print("Creating new project")
        this_pydaw_project.new_project()
    def on_open(self):
        print("Opening existing project")
        this_pydaw_project.open_project()
    def on_save(self):
        print("Saving project")
        this_pydaw_project.save_project()
        this_dssi_gui.send_configure("save", "testing") #Send a message to the DSSI engine to save it's state.  Currently, this doesn't do anything...
    def on_save_as(self):
        print("Saving project as...")
        this_pydaw_project.save_project_as()
    
    def __init__(self):
        self.initUI()
        
    def initUI(self):               
        QtGui.QMainWindow.__init__(self)
        stylesheet_file = os.getcwd() + "/style.txt"
        print("Opening CSS stylesheet from:  " + stylesheet_file)
        file_handle = open(stylesheet_file, 'r')
        self.setStyleSheet(file_handle.read())
        file_handle.close()        
        
        self.resize(1000, 600)
        self.center()
        
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
        
        self.editor_hlayout = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.editor_hlayout)
        self.editor_hlayout.addWidget(this_track_editor.tracks_tablewidget)
        
        self.song_region_vlayout = QtGui.QVBoxLayout()
        self.editor_hlayout.addLayout(self.song_region_vlayout)

        self.song_region_vlayout.addWidget(this_song_editor.group_box)
        self.song_region_vlayout.addWidget(this_region_editor.group_box)
        self.editor_hlayout.addWidget(this_item_editor.group_box)        
        
        self.show()
        
    def center(self):        
        qr = self.frameGeometry()
        cp = QtGui.QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

#Handles the various file IO for the project.  TODO:  Move to it's own file
class pydaw_project:
    def save_project(self):
        self.session_mgr.save_session_file()
    def save_project_as(self):
        pass
    def open_project(self):
        pass
    def new_project(self, a_project_folder=None, a_project_file=None):
        #TODO:  Show a QFileDialog when either is None
        self.project_folder = a_project_folder
        self.project_file = a_project_file
        self.instrument_folder = self.project_folder + "instruments"
        self.regions_folder = self.project_folder + "regions"
        self.items_folder = self.project_folder + "items"
        
        project_folders = [
            self.project_folder,
            self.instrument_folder,
            self.regions_folder,
            self.items_folder
            ]
                    
        for project_dir in project_folders:
            if not os.path.isdir(project_dir):
                os.mkdir(project_dir)
        
        self.session_mgr = lms_session(self.instrument_folder + '/' + a_project_file + '.pyses')
        this_main_window.setWindowTitle('PyDAW - ' + self.project_file)
        
    def get_region_string(self, a_region_name):
        f_file = open(self.regions_folder + "/" + a_region_name + ".pyreg", "r")        
        f_result = f_file.read()
        f_file.close()
        return f_result
                
    def get_item_string(self, a_item_name):
        f_file = open(self.items_folder + "/" + a_item_name + ".pyitem", "r")        
        f_result = f_file.read()
        f_file.close()
        return f_result
        
    def create_empty_region(self, a_region_name):
        #TODO:  Check for uniqueness, from a pydaw_project.check_for_uniqueness method...
        open(self.regions_folder + "/" + a_region_name + ".pyreg", 'w').close()        
    
    def create_empty_item(self, a_item_name):
        #TODO:  Check for uniqueness, from a pydaw_project.check_for_uniqueness method...
        open(self.items_folder + "/" + a_item_name + ".pyitem", 'w').close()        
        
    def copy_region(self, a_old_region, a_new_region):
        copyfile(self.regions_folder + "/" + a_old_region + ".pyreg", self.regions_folder + "/" + a_new_region + ".pyreg")
    
    def copy_item(self, a_old_item, a_new_item):
        copyfile(self.items_folder + "/" + a_old_item + ".pyitem", self.items_folder + "/" + a_new_item + ".pyitem")

    def save_item(self, a_name, a_string):
        f_file = open(self.items_folder + "/" + a_name + ".pyitem", 'w')
        f_file.write(a_string)
        f_file.close()
    
    def save_region(self, a_name, a_string):
        f_file = open(self.regions_folder + "/" + a_name + ".pyreg", 'w')
        f_file.write(a_string)
        f_file.close()

    def get_next_default_item_name(self):
        for i in range(self.last_item_number, 10000):
            f_result = self.items_folder + "/item-" + str(i) + ".pyitem"
            if not os.path.isfile(f_result):
                self.last_item_number = i
                return "item-" + str(i)

    def get_next_default_region_name(self):
        for i in range(self.last_region_number, 10000):
            f_result = self.regions_folder + "/region-" + str(i) + ".pyreg"
            if not os.path.isfile(f_result):
                self.last_item_number = i
                return "region-" + str(i)
            
    def get_item_list(self):
        f_result = []
        for files in os.listdir(self.items_folder):
            if files.endswith(".pyitem"):
                f_result.append(files.split(".pyitem")[0])
        f_result.sort()
        return f_result
    
    def get_region_list(self):
        f_result = []
        for files in os.listdir(self.regions_folder):
            if files.endswith(".pyreg"):
                f_result.append(files.split(".pyreg")[0])
        f_result.sort()
        return f_result
        
    def __init__(self, a_project_folder=None, a_project_file=None):
        self.last_item_number = 0
        self.last_region_number = 0
        self.new_project(a_project_folder, a_project_file)
    
def about_to_quit():
    this_pydaw_project.session_mgr.quit_hander()
    this_dssi_gui.stop_server()

if __name__ == '__main__':
    for arg in argv:
        print arg
    
    if(len(argv) >= 2):
        this_dssi_gui = dssi_gui(argv[1])
    else:
        this_dssi_gui = dssi_gui()
        
    app = QtGui.QApplication(sys.argv)    
    app.aboutToQuit.connect(about_to_quit)
    this_song_editor = song_editor()
    this_region_editor = region_list_editor()
    this_item_editor = item_list_editor()
    this_transport = transport_widget()
    this_track_editor = track_editor()
    
    this_main_window = pydaw_main_window() #You must call this after instantiating the other widgets, as it relies on them existing
    this_main_window.setWindowState(QtCore.Qt.WindowMaximized)
    
    project_folder = expanduser("~") + '/dssi/pydaw/'    
    this_pydaw_project = pydaw_project(project_folder, "default")        
    
    sys.exit(app.exec_())

#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
PyDAW is a DAW using Python and Qt for the UI, with a high performance,
sample-accurate audio/MIDI back end written in C, and suite of high quality 
built in plugins.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import sys, os, re, operator
from time import sleep, time
from PyQt4 import QtGui, QtCore
from sys import argv
from os.path import expanduser
from libpydaw import *

pydaw_item_gradient = QtGui.QLinearGradient(QtCore.QPointF(0, 0), QtCore.QPointF(100, 100))
pydaw_item_gradient.setColorAt(0, QtGui.QColor(100, 100, 255))
pydaw_item_gradient.setColorAt(1, QtGui.QColor(127, 127, 255))

pydaw_region_gradient = QtGui.QLinearGradient(QtCore.QPointF(0, 0), QtCore.QPointF(80, 80))
pydaw_region_gradient.setColorAt(0, QtGui.QColor(220, 120, 120))
pydaw_region_gradient.setColorAt(1, QtGui.QColor(195, 90, 95))

class song_editor:
    def add_qtablewidgetitem(self, a_name, a_region_num):
        """ Adds a properly formatted item.  This is not for creating empty items... """        
        f_qtw_item = QtGui.QTableWidgetItem(a_name)
        f_qtw_item.setBackground(pydaw_region_gradient)
        f_qtw_item.setTextAlignment(QtCore.Qt.AlignCenter)
        f_qtw_item.setFlags(f_qtw_item.flags() | QtCore.Qt.ItemIsSelectable)
        self.table_widget.setItem(0, a_region_num, f_qtw_item)
    
    def open_song(self):
        """ This method clears the existing song from the editor and opens the one currently in this_pydaw_project """
        self.table_widget.clearContents()
        self.song = this_pydaw_project.get_song()
        for f_pos, f_region in self.song.regions.iteritems():
            self.add_qtablewidgetitem(f_region, f_pos)
        f_headers_arr = []
        for i in range(0, 300):
            f_headers_arr.append(str(i))            
        self.table_widget.setHorizontalHeaderLabels(f_headers_arr)

    def cell_clicked(self, x, y):
        f_is_playing = False
        if (this_transport.is_playing or this_transport.is_recording) and this_transport.follow_checkbox.isChecked():
            f_is_playing = True
            this_transport.follow_checkbox.setChecked(False)
            this_region_editor.table_widget.clearSelection()
        f_cell = self.table_widget.item(x, y)
        
        if f_cell is None:
            def song_ok_handler():
                if f_new_radiobutton.isChecked():
                    this_pydaw_project.create_empty_region(f_new_lineedit.text())
                elif f_copy_radiobutton.isChecked():
                    this_pydaw_project.copy_region(str(f_copy_combobox.currentText()), str(f_new_lineedit.text()))
                self.add_qtablewidgetitem(f_new_lineedit.text(), y)
                self.song.add_region_ref(y, str(f_new_lineedit.text()))
                this_region_editor.open_region(f_new_lineedit.text())
                this_pydaw_project.save_song(self.song)
                f_window.close()

            def song_cancel_handler():
                f_window.close()
                            
            def on_name_changed():
                f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))
                
            def on_current_index_changed(a_index):
                f_copy_radiobutton.setChecked(True)

            f_window = QtGui.QDialog()
            f_window.setWindowTitle("Add region to song...")
            f_layout = QtGui.QGridLayout()
            f_window.setLayout(f_layout)
            f_new_radiobutton = QtGui.QRadioButton()
            f_new_radiobutton.setChecked(True)
            f_layout.addWidget(f_new_radiobutton, 0, 0)
            f_layout.addWidget(QtGui.QLabel("New:"), 0, 1)
            f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_region_name())
            f_new_lineedit.setMaxLength(24)
            f_new_lineedit.editingFinished.connect(on_name_changed)
            f_layout.addWidget(f_new_lineedit, 0, 2)
            f_copy_radiobutton = QtGui.QRadioButton()
            f_layout.addWidget(f_copy_radiobutton, 1, 0)
            f_copy_combobox = QtGui.QComboBox()
            f_copy_combobox.addItems(this_pydaw_project.get_region_list())
            f_copy_combobox.currentIndexChanged.connect(on_current_index_changed)
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
            if not f_is_playing:
                this_transport.region_spinbox.setValue(y)
    
    def __init__(self):
        self.song = pydaw_song()
        self.group_box = QtGui.QGroupBox()
        self.group_box.setMaximumHeight(138)
        self.main_vlayout = QtGui.QVBoxLayout()
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)
        self.group_box.setLayout(self.main_vlayout)
        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(300)        
        self.table_widget.setRowCount(1)
        self.table_widget.setRowHeight(0, 65)
        self.table_widget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.table_widget.verticalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.table_widget.cellClicked.connect(self.cell_clicked)
        self.table_widget.setDragDropOverwriteMode(False)
        self.table_widget.setDragEnabled(True)
        self.table_widget.setDragDropMode(QtGui.QAbstractItemView.InternalMove)
        self.table_widget.dropEvent = self.table_drop_event
        self.table_widget.keyPressEvent = self.table_keyPressEvent
        self.table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.main_vlayout.addWidget(self.table_widget)
            
    def table_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            for f_item in self.table_widget.selectedIndexes():
                f_empty = QtGui.QTableWidgetItem() #Clear the item
                self.table_widget.setItem(f_item.row(), f_item.column(), f_empty)
            self.tablewidget_to_song()
            this_region_editor.clear_items()
            this_region_editor.region_name_lineedit.setText("")
            this_region_editor.enabled = False
        else:
            QtGui.QTableWidget.keyPressEvent(self.table_widget, event)  
    
    def table_drop_event(self, a_event):        
        QtGui.QTableWidget.dropEvent(self.table_widget, a_event)
        a_event.acceptProposedAction()
        self.tablewidget_to_song()
        self.table_widget.clearSelection()
        
    def tablewidget_to_song(self):
        """ Flush the edited content of the QTableWidget back to the native song class... """
        self.song.regions = {}
        for i in range(0, 300):
            f_item = self.table_widget.item(0, i)
            if not f_item is None:
                if f_item.text() != "":
                    self.song.add_region_ref(i, f_item.text())
        this_pydaw_project.save_song(self.song)
        self.open_song()

class region_list_editor:
    def add_qtablewidgetitem(self, a_name, a_track_num, a_bar_num):
        """ Adds a properly formatted item.  This is not for creating empty items... """        
        f_qtw_item = QtGui.QTableWidgetItem(a_name)
        f_qtw_item.setBackground(pydaw_item_gradient)
        f_qtw_item.setTextAlignment(QtCore.Qt.AlignCenter)
        f_qtw_item.setFlags(f_qtw_item.flags() | QtCore.Qt.ItemIsSelectable)
        self.table_widget.setItem(a_track_num, a_bar_num + 1, f_qtw_item)        
    
    def clear_new(self):
        """ Reset the region editor state to empty """
        self.clear_items()
        self.reset_tracks()
        self.enabled = False
        self.region_name_lineedit.setText("")
        self.region = None
    
    def open_tracks(self):
        self.reset_tracks()
        f_tracks = this_pydaw_project.get_tracks()
        for key, f_track in f_tracks.tracks.iteritems():
            self.tracks[key].open_track(f_track)
        
    def reset_tracks(self):
        f_track_ss_handle = open("pydaw/track_style.txt", "r")
        f_track_stylesheet = f_track_ss_handle.read()
        f_track_ss_handle.close()
        self.tracks = []
        for i in range(0, 16):
            track = seq_track(a_track_num=i, a_track_text="track" + str(i + 1))
            self.tracks.append(track)
            track.group_box.setStyleSheet(f_track_stylesheet)
            self.table_widget.setCellWidget(i, 0, track.group_box)  
        self.table_widget.setColumnWidth(0, 390)
        self.set_region_length()
        
    def set_region_length(self, a_length=8):
        self.table_widget.setColumnCount(a_length + 1)
        f_headers = ['Tracks']
        for i in range(0, a_length):
            self.table_widget.setColumnWidth(i + 1, 100)
            f_headers.append(str(i))
        self.table_widget.setHorizontalHeaderLabels(f_headers)
        self.table_widget.resizeRowsToContents()
        self.table_widget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.table_widget.verticalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        self.table_width = 0
        for i in range(0, a_length + 1):
            self.table_width += self.table_widget.columnWidth(i)

    def clear_items(self):
        self.table_widget.setColumnCount(9)
        for i in range(16):
            for i2 in range(1, 9):
                f_empty_item = QtGui.QTableWidgetItem()
                self.table_widget.setItem(i, i2, f_empty_item)
        for i in range(16):
            f_item = QtGui.QTableWidgetItem()
            f_item.setFlags(f_item.flags() & ~QtCore.Qt.ItemIsEditable & ~QtCore.Qt.ItemIsSelectable & ~QtCore.Qt.ItemIsEnabled);
            self.table_widget.setItem(i, 0, f_item)
        self.region_name_lineedit.setText("")
        self.enabled = False
        self.length_alternate_spinbox.setValue(8)
        self.length_default_radiobutton.setChecked(True)

    def get_tracks(self):
        f_result = pydaw_tracks()
        for f_i in range(0, len(self.tracks)):
            f_result.add_track(f_i, self.tracks[f_i].get_track())
        return f_result

    def open_region(self, a_file_name):
        self.enabled = False
        self.clear_items()
        self.region_name_lineedit.setText(a_file_name)
        self.region = this_pydaw_project.get_region(a_file_name)
        if self.region.region_length_bars > 0:
            self.set_region_length(self.region.region_length_bars)
            self.length_alternate_spinbox.setValue(self.region.region_length_bars)
            this_transport.bar_spinbox.setRange(0, (self.region.region_length_bars) - 1)
            self.length_alternate_radiobutton.setChecked(True)
        else:
            self.length_alternate_spinbox.setValue(8)
            this_transport.bar_spinbox.setRange(0, 7)
            self.length_default_radiobutton.setChecked(True)
        self.enabled = True
        for f_item in self.region.items:
            if f_item.bar_num < self.region.region_length_bars or (self.region.region_length_bars == 0 and f_item.bar_num < 8):
                self.add_qtablewidgetitem(f_item.item_name, f_item.track_num, f_item.bar_num)
        
    def cell_clicked(self, x, y):
        if not self.enabled:
            QtGui.QMessageBox.warning(this_main_window, "", "You must create or select a region first by clicking in the song editor above.")
            return
        if (this_transport.is_playing or this_transport.is_recording) and this_transport.follow_checkbox.isChecked():
            this_transport.follow_checkbox.setChecked(False)
        f_item = self.table_widget.item(x, y)
        if f_item is None or f_item.text() == "":
            self.show_cell_dialog(x, y)
    
    def cell_double_clicked(self, x, y):        
        if not self.enabled:
            return
        f_item = self.table_widget.item(x, y)    
        if f_item is None:
            self.show_cell_dialog(x, y)
        else:
            f_item_name = str(f_item.text())
            if f_item_name != "":
                this_item_editor.open_item(f_item_name)
                this_main_window.main_tabwidget.setCurrentIndex(1)
            else:
                self.show_cell_dialog(x, y)
    
    def show_cell_dialog(self, x, y):
        def note_ok_handler():
            if f_new_radiobutton.isChecked() or f_copy_from_radiobutton.isChecked():
                f_cell_text = str(f_new_lineedit.text())
                this_pydaw_project.create_empty_item(f_new_lineedit.text())
                this_pydaw_project.this_dssi_gui.pydaw_save_item(f_new_lineedit.text())
            elif f_copy_radiobutton.isChecked():
                f_cell_text = str(f_copy_combobox.currentText())
                
            if f_copy_from_radiobutton.isChecked():
                this_pydaw_project.copy_item(str(f_copy_combobox.currentText()), str(f_new_lineedit.text()))
                this_pydaw_project.this_dssi_gui.pydaw_save_item(f_new_lineedit.text())

            if f_new_radiobutton.isChecked() or f_copy_from_radiobutton.isChecked():
                this_item_editor.open_item(f_cell_text)
            self.last_item_copied = f_cell_text
            self.add_qtablewidgetitem(f_cell_text, x, y - 1)
            self.region.add_item_ref(x, y - 1, f_cell_text)            
            this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            f_window.close()

        def note_cancel_handler():
            f_window.close()
            
        def copy_combobox_index_changed(a_index):
            f_copy_radiobutton.setChecked(True)
            
        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Add item reference to region...")
        f_layout = QtGui.QGridLayout()
        f_vlayout0 = QtGui.QVBoxLayout()
        f_vlayout1 = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_new_radiobutton = QtGui.QRadioButton()
        f_new_radiobutton.setChecked(True)
        f_layout.addWidget(f_new_radiobutton, 0, 0)
        f_layout.addWidget(QtGui.QLabel("New:"), 0, 1)
        f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_item_name())
        f_new_lineedit.editingFinished.connect(on_name_changed)
        f_new_lineedit.setMaxLength(24)
        f_layout.addWidget(f_new_lineedit, 0, 2)
        f_layout.addLayout(f_vlayout0, 1, 0)        
        f_copy_from_radiobutton = QtGui.QRadioButton()
        f_vlayout0.addWidget(f_copy_from_radiobutton)
        f_copy_radiobutton = QtGui.QRadioButton()
        f_vlayout0.addWidget(f_copy_radiobutton)
        f_copy_combobox = QtGui.QComboBox()
        f_copy_combobox.addItems(this_pydaw_project.get_item_list())        
        if not self.last_item_copied is None:
            f_copy_combobox.setCurrentIndex(f_copy_combobox.findText(self.last_item_copied))
        f_copy_combobox.currentIndexChanged.connect(copy_combobox_index_changed)
        f_layout.addLayout(f_vlayout1, 1, 1)
        f_vlayout1.addWidget(QtGui.QLabel("Copy from:"))
        f_vlayout1.addWidget(QtGui.QLabel("Existing:"))
        f_layout.addWidget(f_copy_combobox, 1, 2)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_window.exec_()

    def update_region_length(self, a_value=None):
        if not self.enabled:
            return
        if self.length_alternate_radiobutton.isChecked():
            self.region.region_length_bars = self.length_alternate_spinbox.value()
            self.set_region_length(self.region.region_length_bars)
        else:
            self.region.region_length_bars = 0
            self.set_region_length()
        this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)        
        self.open_region(self.region_name_lineedit.text())

    def __init__(self):
        self.enabled = False #Prevents user from editing a region before one has been selected
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QVBoxLayout()

        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)
        self.region_num_label = QtGui.QLabel()
        self.region_num_label.setText("Region:")
        self.hlayout0.addWidget(self.region_num_label)
        self.region_name_lineedit = QtGui.QLineEdit()
        self.region_name_lineedit.setEnabled(False)
        self.hlayout0.addWidget(self.region_name_lineedit)
        self.hlayout0.addWidget(QtGui.QLabel("Region Length:"))
        self.length_default_radiobutton = QtGui.QRadioButton("default")
        self.length_default_radiobutton.setChecked(True)
        self.length_default_radiobutton.toggled.connect(self.update_region_length)
        self.hlayout0.addWidget(self.length_default_radiobutton)
        self.length_alternate_radiobutton = QtGui.QRadioButton()
        self.length_alternate_radiobutton.toggled.connect(self.update_region_length)
        self.hlayout0.addWidget(self.length_alternate_radiobutton)
        self.length_alternate_spinbox = QtGui.QSpinBox()
        self.length_alternate_spinbox.setRange(4, 16)
        self.length_alternate_spinbox.setValue(8)
        self.length_alternate_spinbox.valueChanged.connect(self.update_region_length)
        self.hlayout0.addWidget(self.length_alternate_spinbox)
        self.group_box.setLayout(self.main_vlayout)
        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(9)
        self.table_widget.setRowCount(16)
        self.table_widget.cellDoubleClicked.connect(self.cell_double_clicked)
        self.table_widget.cellClicked.connect(self.cell_clicked)
        self.table_widget.setDragDropOverwriteMode(False)
        self.table_widget.setDragEnabled(True)
        self.table_widget.setDragDropMode(QtGui.QAbstractItemView.InternalMove)
        self.table_widget.dropEvent = self.table_drop_event
        self.table_widget.keyPressEvent = self.table_keyPressEvent
        self.table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.table_widget.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)
        self.copy_action = QtGui.QAction("Copy (CTRL+C)", self.table_widget)
        self.copy_action.triggered.connect(self.copy_selected)
        self.table_widget.addAction(self.copy_action)
        self.paste_action = QtGui.QAction("Paste (CTRL+V)", self.table_widget)
        self.paste_action.triggered.connect(self.paste_clipboard)
        self.table_widget.addAction(self.paste_action)
        self.unlink_action = QtGui.QAction("Unlink (CTRL+D)", self.table_widget)
        self.unlink_action.triggered.connect(self.on_unlink_item)
        self.table_widget.addAction(self.unlink_action)
        self.delete_action = QtGui.QAction("Delete (Del)", self.table_widget)
        self.delete_action.triggered.connect(self.delete_selected)
        self.table_widget.addAction(self.delete_action)        
        self.main_vlayout.addWidget(self.table_widget)
        self.last_item_copied = None
        self.reset_tracks()
        self.clipboard = []
    
    def table_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            self.delete_selected()
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.copy_selected()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            self.paste_clipboard()
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.copy_selected()
            self.delete_selected()
        elif event.key() == QtCore.Qt.Key_D and event.modifiers() == QtCore.Qt.ControlModifier:
            self.on_unlink_item()
        else:
            QtGui.QTableWidget.keyPressEvent(self.table_widget, event)

    def on_unlink_item(self):
        """ Rename a single instance of an item and make it into a new item """
        if self.table_widget.currentItem() is None or str(self.table_widget.currentItem().text()) == "":
            return
        x = self.table_widget.currentRow()
        y = self.table_widget.currentColumn()
        
        def note_ok_handler():
            f_cell_text = str(f_new_lineedit.text())
            this_pydaw_project.create_empty_item(f_new_lineedit.text())
            this_pydaw_project.this_dssi_gui.pydaw_save_item(f_new_lineedit.text())
            this_pydaw_project.copy_item(str(self.table_widget.currentItem().text()), str(f_new_lineedit.text()))
            this_pydaw_project.this_dssi_gui.pydaw_save_item(f_new_lineedit.text())
            this_item_editor.open_item(f_cell_text)
            self.last_item_copied = f_cell_text
            self.add_qtablewidgetitem(f_cell_text, x, y - 1)
            self.region.add_item_ref(x, y - 1, f_cell_text)            
            this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            f_window.close()

        def note_cancel_handler():
            f_window.close()
            
        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Copy and unlink item...")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_item_name())
        f_new_lineedit.editingFinished.connect(on_name_changed)
        f_new_lineedit.setMaxLength(24)
        f_layout.addWidget(QtGui.QLabel("New name:"), 0, 0)
        f_layout.addWidget(f_new_lineedit, 0, 1)        
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_window.exec_()

    def paste_clipboard(self):
        f_selected_cells = self.table_widget.selectedIndexes()
        if len(f_selected_cells) == 0:
            return
        f_base_row = f_selected_cells[0].row()
        f_base_column = f_selected_cells[0].column() - 1
        for f_item in self.clipboard:
            f_column = f_item[1] + f_base_column
            if f_column >= 8 or f_column < 0:
                continue                
            f_row = f_item[0] + f_base_row
            if f_row >= 16 or f_row < 0:
                continue
            self.add_qtablewidgetitem(f_item[2], f_row, f_column)
        self.tablewidget_to_region()
        
    def delete_selected(self):
        for f_item in self.table_widget.selectedIndexes():
            f_empty = QtGui.QTableWidgetItem() #Clear the item
            self.table_widget.setItem(f_item.row(), f_item.column(), f_empty)
        self.tablewidget_to_region()
    
    def copy_selected(self):
        self.clipboard = []  #Clear the clipboard
        for f_item in self.table_widget.selectedIndexes():            
            f_cell = self.table_widget.item(f_item.row(), f_item.column())
            if not f_cell is None and not str(f_cell.text()) == "":
                self.clipboard.append([int(f_item.row()), int(f_item.column()) - 1, str(f_cell.text())])
        if len(self.clipboard) > 0:            
            self.clipboard.sort(key=operator.itemgetter(0))
            f_row_offset = self.clipboard[0][0]
            for f_item in self.clipboard:
                f_item[0] -= f_row_offset
            self.clipboard.sort(key=operator.itemgetter(1))
            f_column_offset = self.clipboard[0][1]
            for f_item in self.clipboard:
                f_item[1] -= f_column_offset
        
    def table_drop_event(self, a_event):
        if a_event.pos().x() <= self.table_widget.columnWidth(0) or a_event.pos().x() >= self.table_width:
            print("Drop event out of bounds, ignoring...")
            a_event.ignore()
            return
        QtGui.QTableWidget.dropEvent(self.table_widget, a_event)
        a_event.acceptProposedAction()
        self.tablewidget_to_region()
        self.table_widget.clearSelection()
        
    def tablewidget_to_region(self):
        """ Convert an edited QTableWidget to a native region class """
        self.region.items = []
        for i in range(0, 16):
            for i2 in range(1, 9):
                f_item = self.table_widget.item(i, i2)
                if not f_item is None:
                    if f_item.text() != "":
                        self.region.add_item_ref(i, i2 - 1, f_item.text())
        this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)    

class item_list_editor:
    def clear_notes(self):
        if self.enabled:
            self.item.notes = []
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
    def clear_ccs(self):
        if self.enabled:
            self.item.ccs = []
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
    def clear_pb(self):
        if self.enabled:
            self.item.pitchbends = []
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
    
    def clear_new(self):
        self.enabled = False
        self.item_name_line_edit.setText("")
        self.ccs_table_widget.clearContents()
        self.notes_table_widget.clearContents()
        self.pitchbend_table_widget.clearContents()
    
    def get_notes_table_selected_rows(self):
        f_result = []        
        for i in range(0, self.notes_table_widget.rowCount()):
            f_item = self.notes_table_widget.item(i, 0)
            if not f_item is None and f_item.isSelected():
                f_result.append(pydaw_note(self.notes_table_widget.item(i, 0).text(), self.notes_table_widget.item(i, 1).text(), self.notes_table_widget.item(i, 3).text(), self.notes_table_widget.item(i, 4).text()))        
        return f_result
        
    def get_ccs_table_selected_rows(self):
        f_result = []        
        for i in range(0, self.ccs_table_widget.rowCount()):
            f_item = self.ccs_table_widget.item(i, 0)
            if not f_item is None and f_item.isSelected():
                f_result.append(pydaw_cc(self.ccs_table_widget.item(i, 0).text(), self.ccs_table_widget.item(i, 1).text(), self.ccs_table_widget.item(i, 2).text()))        
        return f_result
        
    def get_pbs_table_selected_rows(self):
        f_result = []        
        for i in range(0, self.pitchbend_table_widget.rowCount()):
            f_item = self.pitchbend_table_widget.item(i, 0)
            if not f_item is None and f_item.isSelected():
                f_result.append(pydaw_pitchbend(self.pitchbend_table_widget.item(i, 0).text(), self.pitchbend_table_widget.item(i, 1).text()))        
        return f_result
        
    def transpose_dialog(self):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
            
        f_multiselect = False
        if self.multiselect_radiobutton.isChecked():
            f_ms_rows = self.get_notes_table_selected_rows()
            if len(f_ms_rows) == 0:
                QtGui.QMessageBox.warning(self.notes_table_widget, "Error", "You have editing in multiselect mode, but you have not selected anything.  All items will be processed")
            else:
                f_multiselect = True
        
        def transpose_ok_handler():
            if f_multiselect:
                self.item.transpose(f_semitone.value(), f_octave.value(), f_ms_rows)
            else:
                self.item.transpose(f_semitone.value(), f_octave.value())
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
            f_window.close()

        def transpose_cancel_handler():
            f_window.close()
            
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Transpose")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        
        f_semitone = QtGui.QSpinBox()
        f_semitone.setRange(-12, 12)
        f_layout.addWidget(QtGui.QLabel("Semitones"), 0, 0)
        f_layout.addWidget(f_semitone, 0, 1)        
        f_octave = QtGui.QSpinBox()
        f_octave.setRange(-5, 5)
        f_layout.addWidget(QtGui.QLabel("Octaves"), 1, 0)
        f_layout.addWidget(f_octave, 1, 1)
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(transpose_ok_handler)
        f_layout.addWidget(f_ok, 2, 0)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(transpose_cancel_handler)
        f_layout.addWidget(f_cancel, 2, 1)
        f_window.exec_()

    def show_not_enabled_warning(self):
        QtGui.QMessageBox.warning(this_main_window, "", "You must open an item first by double-clicking on one in the region editor on the 'Song' tab.")
        
    def time_shift_dialog(self):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
            
        f_multiselect = False
        if self.multiselect_radiobutton.isChecked():
            f_ms_rows = self.get_notes_table_selected_rows()
            if len(f_ms_rows) == 0:
                QtGui.QMessageBox.warning(self.notes_table_widget, "Error", "You have editing in multiselect mode, but you have not selected anything.  All items will be processed")
            else:
                f_multiselect = True
                
        def time_shift_ok_handler():
            self.events_follow_default = f_events_follow_notes.isChecked()
            if f_multiselect:
                self.item.time_shift(f_shift.value(), f_events_follow_notes.isChecked(), f_ms_rows)
            else:
                self.item.time_shift(f_shift.value(), f_events_follow_notes.isChecked())
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
            f_window.close()

        def time_shift_cancel_handler():
            f_window.close()
            
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Time Shift")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        
        f_shift = QtGui.QDoubleSpinBox()
        f_shift.setRange(-4.0, 4.0)
        f_layout.addWidget(QtGui.QLabel("Shift(beats)"), 0, 0)
        f_layout.addWidget(f_shift, 0, 1)
        f_events_follow_notes = QtGui.QCheckBox("CCs and pitchbend follow notes?")
        f_events_follow_notes.setChecked(self.events_follow_default)
        f_layout.addWidget(f_events_follow_notes, 1, 1)
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(time_shift_ok_handler)
        f_layout.addWidget(f_ok, 2, 0)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(time_shift_cancel_handler)
        f_layout.addWidget(f_cancel, 2, 1)
        f_window.exec_()
        
    def length_shift_dialog(self):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
            
        f_multiselect = False
        if self.multiselect_radiobutton.isChecked():
            f_ms_rows = self.get_notes_table_selected_rows()
            if len(f_ms_rows) == 0:
                QtGui.QMessageBox.warning(self.notes_table_widget, "Error", "You have editing in multiselect mode, but you have not selected anything.  All items will be processed")
            else:
                f_multiselect = True
                
        def time_shift_ok_handler():
            if f_multiselect:
                self.item.length_shift(f_shift.value(), f_min_max.value(), f_ms_rows)
            else:
                self.item.length_shift(f_shift.value(), f_min_max.value())
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
            f_window.close()

        def time_shift_cancel_handler():
            f_window.close()
            
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Length Shift")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        
        f_shift = QtGui.QDoubleSpinBox()
        f_shift.setRange(-16.0, 16.0)
        f_layout.addWidget(QtGui.QLabel("Shift(beats)"), 0, 0)
        f_layout.addWidget(f_shift, 0, 1)
        
        f_min_max = QtGui.QDoubleSpinBox()
        f_min_max.setRange(0.01, 16.0)
        f_min_max.setValue(1.0)
        f_layout.addWidget(QtGui.QLabel("Min/Max(beats)"), 1, 0)
        f_layout.addWidget(f_min_max, 1, 1)
        
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(time_shift_ok_handler)
        f_layout.addWidget(f_ok, 2, 0)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(time_shift_cancel_handler)
        f_layout.addWidget(f_cancel, 2, 1)
        f_window.exec_()


    def on_template_open(self):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        f_path= expanduser("~") + "/pydaw/item_templates/" + str(self.template_combobox.currentText()) + ".pyitem"
        if not os.path.isfile(f_path):
            QtGui.QMessageBox.warning(self.notes_table_widget, "Error", "Cannot find specified template")
        else:
            f_item_handle = open(f_path, "r")
            f_item = pydaw_item.from_str(f_item_handle.read())
            f_item_handle.close()
            self.item = f_item
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
    
    def on_template_save_as(self):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        def ok_handler():
            if str(f_name.text()) == "":
                QtGui.QMessageBox.warning(f_window, "Error", "Name cannot be empty")
                return
            f_path= expanduser("~") + "/pydaw/item_templates/" + str(f_name.text()) + ".pyitem"
            f_handle = open(f_path, "w")
            f_handle.write(self.item.__str__())
            f_handle.close()
            self.load_templates()
            f_window.close()

        def cancel_handler():
            f_window.close()
        
        def f_name_text_changed():
            f_name.setText(pydaw_remove_bad_chars(f_name.text()))
            
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Save item as template...")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        
        f_name = QtGui.QLineEdit()
        f_name.textChanged.connect(f_name_text_changed)
        f_layout.addWidget(QtGui.QLabel("Name:"), 0, 0)
        f_layout.addWidget(f_name, 0, 1)        
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(ok_handler)
        f_layout.addWidget(f_ok, 2, 0)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 2, 1)
        f_window.exec_()        
        
    def load_templates(self):
        self.template_combobox.clear()
        f_path= expanduser("~") + "/pydaw/item_templates"
        if not os.path.isdir(f_path):
            os.makedirs(f_path)
        else:
            f_file_list = os.listdir(f_path)
            for f_name in f_file_list:
                if f_name.endswith(".pyitem"):
                    self.template_combobox.addItem(f_name.split(".")[0])
        
    #If a_new_file_name is set, a_file_name will be copied into a new file name with the name a_new_file_name
    def __init__(self):
        self.enabled = False
        self.events_follow_default = True
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QVBoxLayout()
        self.main_hlayout = QtGui.QHBoxLayout()
        self.group_box.setLayout(self.main_vlayout)
        
        self.edit_mode_groupbox = QtGui.QGroupBox()
        self.edit_mode_hlayout0 = QtGui.QHBoxLayout(self.edit_mode_groupbox)        
        self.edit_mode_hlayout0.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        self.edit_mode_hlayout0.addWidget(QtGui.QLabel("Edit Mode:"))
        self.add_radiobutton = QtGui.QRadioButton("Add/Edit")
        self.edit_mode_hlayout0.addWidget(self.add_radiobutton)
        self.multiselect_radiobutton = QtGui.QRadioButton("Multiselect")
        self.edit_mode_hlayout0.addWidget(self.multiselect_radiobutton)
        self.delete_radiobutton = QtGui.QRadioButton("Delete")
        self.edit_mode_hlayout0.addWidget(self.delete_radiobutton)
        self.add_radiobutton.setChecked(True)
        self.editing_hboxlayout = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.editing_hboxlayout)
        self.item_name_line_edit = QtGui.QLineEdit()
        self.item_name_line_edit.setMaximumWidth(200)
        self.item_name_line_edit.setEnabled(False)
        self.editing_hboxlayout.addWidget(QtGui.QLabel("Editing Item:"))
        self.editing_hboxlayout.addWidget(self.item_name_line_edit)
        self.editing_hboxlayout.addWidget(QtGui.QLabel("Templates:"))
        self.template_save_as = QtGui.QPushButton("Save as...")
        self.template_save_as.setMinimumWidth(90)
        self.template_save_as.pressed.connect(self.on_template_save_as)
        self.editing_hboxlayout.addWidget(self.template_save_as)
        self.template_open = QtGui.QPushButton("Open")
        self.template_open.setMinimumWidth(90)
        self.template_open.pressed.connect(self.on_template_open)
        self.editing_hboxlayout.addWidget(self.template_open)
        self.template_combobox = QtGui.QComboBox()
        self.template_combobox.setMinimumWidth(150)
        self.editing_hboxlayout.addWidget(self.template_combobox)
        self.load_templates()
        self.editing_hboxlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        self.editing_hboxlayout.addWidget(self.edit_mode_groupbox, alignment=QtCore.Qt.AlignRight)
        
        self.main_vlayout.addLayout(self.main_hlayout)

        self.notes_groupbox = QtGui.QGroupBox("Notes")
        self.notes_groupbox.setMinimumWidth(573)
        self.notes_vlayout = QtGui.QVBoxLayout(self.notes_groupbox)
        self.notes_gridlayout = QtGui.QGridLayout()
        f_n_spacer_left = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.notes_gridlayout.addItem(f_n_spacer_left, 0, 0, 1, 1)
        self.notes_transpose_button = QtGui.QPushButton("Transpose")
        self.notes_transpose_button.setMinimumWidth(90)
        self.notes_transpose_button.pressed.connect(self.transpose_dialog)
        self.notes_gridlayout.addWidget(self.notes_transpose_button, 0, 1)
        self.notes_shift_button = QtGui.QPushButton("Shift")
        self.notes_shift_button.setMinimumWidth(90)
        self.notes_shift_button.pressed.connect(self.time_shift_dialog)
        self.notes_gridlayout.addWidget(self.notes_shift_button, 0, 2)
        self.notes_length_button = QtGui.QPushButton("Length")
        self.notes_length_button.setMinimumWidth(90)
        self.notes_length_button.pressed.connect(self.length_shift_dialog)
        self.notes_gridlayout.addWidget(self.notes_length_button, 0, 3)
        self.notes_clear_button = QtGui.QPushButton("Clear")
        self.notes_clear_button.setMinimumWidth(90)
        self.notes_clear_button.pressed.connect(self.clear_notes)
        self.notes_gridlayout.addWidget(self.notes_clear_button, 0, 4)        
        self.notes_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 5, 1, 1)
        self.notes_vlayout.addLayout(self.notes_gridlayout) 
        self.notes_table_widget = QtGui.QTableWidget()
        self.notes_table_widget.setColumnCount(5)
        self.notes_table_widget.setRowCount(128)
        self.notes_table_widget.cellClicked.connect(self.notes_click_handler)
        self.notes_table_widget.setSortingEnabled(True)
        self.notes_table_widget.sortItems(0)        
        self.notes_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.notes_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.notes_table_widget.keyPressEvent = self.notes_keyPressEvent
        self.notes_vlayout.addWidget(self.notes_table_widget)

        self.ccs_groupbox = QtGui.QGroupBox("CCs")
        self.ccs_groupbox.setMaximumWidth(370)
        self.ccs_groupbox.setMinimumWidth(370)
        self.ccs_vlayout = QtGui.QVBoxLayout(self.ccs_groupbox)
        self.ccs_gridlayout = QtGui.QGridLayout()
        f_c_spacer_left = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.ccs_gridlayout.addItem(f_c_spacer_left, 0, 0, 1, 1)
        self.ccs_clear_button = QtGui.QPushButton("Clear")
        self.ccs_clear_button.setMinimumWidth(90)
        self.ccs_clear_button.pressed.connect(self.clear_ccs)
        self.ccs_gridlayout.addWidget(self.ccs_clear_button, 0, 1)
        f_c_spacer_right = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.ccs_gridlayout.addItem(f_c_spacer_right, 0, 2, 1, 1)
        self.ccs_vlayout.addLayout(self.ccs_gridlayout)
        self.ccs_table_widget = QtGui.QTableWidget()
        self.ccs_table_widget.setColumnCount(3)
        self.ccs_table_widget.setRowCount(256)
        self.ccs_table_widget.cellClicked.connect(self.ccs_click_handler)
        self.ccs_table_widget.setSortingEnabled(True)
        self.ccs_table_widget.sortItems(0)
        self.ccs_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.ccs_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.ccs_table_widget.keyPressEvent = self.ccs_keyPressEvent
        self.ccs_vlayout.addWidget(self.ccs_table_widget)
        
        self.pb_groupbox = QtGui.QGroupBox("Pitchbend")
        self.pb_groupbox.setMaximumWidth(270)
        self.pb_vlayout = QtGui.QVBoxLayout(self.pb_groupbox)
        self.pb_gridlayout = QtGui.QGridLayout()
        f_p_spacer_left = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.pb_gridlayout.addItem(f_p_spacer_left, 0, 0, 1, 1)
        self.pb_clear_button = QtGui.QPushButton("Clear")
        self.pb_clear_button.setMinimumWidth(90)
        self.pb_clear_button.pressed.connect(self.clear_pb)
        self.pb_gridlayout.addWidget(self.pb_clear_button, 0, 1)
        f_p_spacer_right = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.pb_gridlayout.addItem(f_p_spacer_right, 0, 2, 1, 1)
        self.pb_vlayout.addLayout(self.pb_gridlayout)        
        self.pitchbend_table_widget = QtGui.QTableWidget()
        self.pitchbend_table_widget.setColumnCount(2)
        self.pitchbend_table_widget.setRowCount(128)
        self.pitchbend_table_widget.cellClicked.connect(self.pitchbend_click_handler)
        self.pitchbend_table_widget.setSortingEnabled(True)
        self.pitchbend_table_widget.sortItems(0)
        self.pitchbend_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.pitchbend_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.pitchbend_table_widget.keyPressEvent = self.pbs_keyPressEvent
        self.pb_vlayout.addWidget(self.pitchbend_table_widget)

        self.main_hlayout.addWidget(self.notes_groupbox)
        self.main_hlayout.addWidget(self.ccs_groupbox)
        self.main_hlayout.addWidget(self.pb_groupbox)
        self.set_headers()        
        self.default_note_start = 0.0
        self.default_note_length = 1.0
        self.default_note_note = 0
        self.default_note_octave = 3
        self.default_note_velocity = 100
        self.default_cc_num = 0
        self.default_cc_start = 0.0
        self.default_cc_val = 0
        self.default_quantize = 5
        self.default_pb_start = 0
        self.default_pb_val = 0
        self.default_pb_quantize = 0
        
        self.notes_clipboard = []
        self.ccs_clipboard = []
        self.pbs_clipboard = []

    def set_headers(self): #Because clearing the table clears the headers
        self.notes_table_widget.setHorizontalHeaderLabels(['Start', 'Length', 'Note', 'Note#', 'Velocity'])
        self.ccs_table_widget.setHorizontalHeaderLabels(['Start', 'CC', 'Value'])
        self.pitchbend_table_widget.setHorizontalHeaderLabels(['Start', 'Value'])

    def open_item(self, a_item_name):
        self.enabled = True
        self.item_name_line_edit.setText(a_item_name)
        self.notes_table_widget.clear()
        self.ccs_table_widget.clear()
        self.pitchbend_table_widget.clear()
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
        self.ccs_table_widget.setSortingEnabled(True)
        self.pitchbend_table_widget.setSortingEnabled(False)
        f_i = 0
        for pb in self.item.pitchbends:
            self.pitchbend_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(pb.start)))
            self.pitchbend_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(str(pb.pb_val)))
            f_i = f_i + 1
        self.pitchbend_table_widget.setSortingEnabled(True)            
                
    def notes_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_notes = self.get_notes_table_selected_rows()
                for f_note in f_notes:
                    self.item.remove_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.notes_clipboard = self.get_notes_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_note in self.notes_clipboard:
                self.item.add_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:            
            self.notes_clipboard = self.get_notes_table_selected_rows()
            for f_note in self.notes_clipboard:
                self.item.remove_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        else:
            QtGui.QTableWidget.keyPressEvent(self.notes_table_widget, event)  
    
    def ccs_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_ccs = self.get_ccs_table_selected_rows()
                for f_cc in f_ccs:
                    self.item.remove_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_cc in self.ccs_clipboard:
                self.item.add_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:            
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
            for f_cc in self.ccs_clipboard:
                self.item.remove_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        else:
            QtGui.QTableWidget.keyPressEvent(self.ccs_table_widget, event)  
        
    def pbs_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_pbs = self.get_pbs_table_selected_rows()
                for f_pb in f_pbs:
                    self.item.remove_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.pbs_clipboard = self.get_pbs_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_pb in self.pbs_clipboard:
                self.item.add_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:            
            self.pbs_clipboard = self.get_pbs_table_selected_rows()
            for f_pb in self.pbs_clipboard:
                self.item.remove_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
        else:
            QtGui.QTableWidget.keyPressEvent(self.pitchbend_table_widget, event)        
        
    def notes_click_handler(self, x, y):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        if self.add_radiobutton.isChecked():
            self.notes_show_event_dialog(x, y)
        elif self.delete_radiobutton.isChecked():
            if self.notes_table_widget.item(x, 0) is None or str(self.notes_table_widget.item(x, 0).text()) == "":
                return
            self.item.remove_note(pydaw_note(self.notes_table_widget.item(x, 0).text(), self.notes_table_widget.item(x, 1).text(), self.notes_table_widget.item(x, 3).text(), self.notes_table_widget.item(x, 4).text()))
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)

    def ccs_click_handler(self, x, y):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        if self.add_radiobutton.isChecked():
            self.ccs_show_event_dialog(x, y)
        elif self.delete_radiobutton.isChecked():
            if self.ccs_table_widget.item(x, 0) is None:
                return
            self.item.remove_cc(pydaw_cc(self.ccs_table_widget.item(x, 0).text(), self.ccs_table_widget.item(x, 1).text(), self.ccs_table_widget.item(x, 2).text()))
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)

    def pitchbend_click_handler(self, x, y):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        if self.add_radiobutton.isChecked():
            self.pitchbend_show_event_dialog(x, y)
        elif self.delete_radiobutton.isChecked():
            if self.pitchbend_table_widget.item(x, 0) is None:
                return
            self.item.remove_pb(pydaw_pitchbend(self.pitchbend_table_widget.item(x, 0).text(), self.pitchbend_table_widget.item(x, 1).text()))
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
            f_start_rounded = time_quantize_round(f_start.value())
            f_length_rounded = time_quantize_round(f_length.value())
            f_new_note = pydaw_note(f_start_rounded, f_length_rounded, f_note_value, f_velocity.value())
            if not self.item.add_note(f_new_note):
                QtGui.QMessageBox.warning(f_window, "Error", "Overlapping note events")
                return
            
            self.default_note_start = f_start_rounded
            self.default_note_length = f_length_rounded
            self.default_note_note = int(f_note.currentIndex())
            self.default_note_octave = int(f_octave.value())
            self.default_note_velocity = int(f_velocity.value())
            self.default_quantize = int(f_quantize_combobox.currentIndex())
            this_pydaw_project.save_item(self.item_name, self.item)
            
            self.open_item(self.item_name)            
            
            if self.is_existing_note:
                f_window.close()
            elif not f_add_another.isChecked():
                f_window.close()

        def note_cancel_handler():
            f_window.close()
            
        def quantize_changed(f_quantize_index):
            f_frac = beat_frac_text_to_float(f_quantize_index)
            f_start.setSingleStep(f_frac)
            f_length.setSingleStep(f_frac)
            self.default_quantize = f_quantize_index
        
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Notes")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.currentIndexChanged.connect(quantize_changed)
        f_layout.addWidget(QtGui.QLabel("Quantize(beats)"), 0, 0)
        f_layout.addWidget(f_quantize_combobox, 0, 1)
        f_note_layout = QtGui.QHBoxLayout()
        f_note = QtGui.QComboBox()
        f_note.addItems(int_to_note_array)
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
        if not self.is_existing_note:
            f_add_another = QtGui.QCheckBox("Add another?")
            f_layout.addWidget(f_add_another, 5, 1)
        f_ok_button = QtGui.QPushButton("OK")        
        f_layout.addWidget(f_ok_button, 6,0)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 6,1)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_quantize_combobox.setCurrentIndex(self.default_quantize)
        f_window.exec_()

    def ccs_show_event_dialog(self, x, y):
        f_cell = self.ccs_table_widget.item(x, y)
        if f_cell is not None:
            self.default_cc_start = float(self.ccs_table_widget.item(x, 0).text())
            self.default_cc_num = int(self.ccs_table_widget.item(x, 1).text())
            self.default_cc_val = int(self.ccs_table_widget.item(x, 2).text())

        def cc_ok_handler():
            f_start_rounded = time_quantize_round(f_start.value())
            
            if f_draw_line_checkbox.isChecked():
                self.item.draw_cc_line(f_cc.value(), f_start.value(), f_cc_value.value(), f_end.value(), f_end_value.value())
            else:
                if not self.item.add_cc(pydaw_cc(f_start_rounded, f_cc.value(), f_cc_value.value())):
                    QtGui.QMessageBox.warning(f_window, "Error", "Duplicate CC event")
                    return
            
            self.default_cc_start = f_start.value()
            self.default_cc_num = f_cc.value()
            self.default_cc_start = f_start_rounded            
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
            if not f_add_another.isChecked():
                f_window.close()

        def cc_cancel_handler():
            f_window.close()
        
        def quantize_changed(f_quantize_index):
            f_frac = beat_frac_text_to_float(f_quantize_index)
            f_start.setSingleStep(f_frac)
            self.default_quantize = f_quantize_index

        f_window = QtGui.QDialog()
        f_window.setWindowTitle("CCs")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.currentIndexChanged.connect(quantize_changed)
        f_layout.addWidget(QtGui.QLabel("Quantize(beats)"), 0, 0)
        f_layout.addWidget(f_quantize_combobox, 0, 1)
        f_cc = QtGui.QSpinBox()
        f_cc.setRange(1, 127)
        f_cc.setValue(self.default_cc_num)
        f_layout.addWidget(QtGui.QLabel("CC"), 1, 0)
        f_layout.addWidget(f_cc, 1, 1)
        f_cc_value = QtGui.QSpinBox()
        f_cc_value.setRange(0, 127)
        f_cc_value.setValue(self.default_cc_val)
        f_layout.addWidget(QtGui.QLabel("Value"), 2, 0)
        f_layout.addWidget(f_cc_value, 2, 1)
        f_layout.addWidget(QtGui.QLabel("Start(beats)"), 3, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_start.setValue(self.default_cc_start)
        f_layout.addWidget(f_start, 3, 1)
        f_draw_line_checkbox = QtGui.QCheckBox("Draw line")
        f_layout.addWidget(f_draw_line_checkbox, 4, 1)
        f_layout.addWidget(QtGui.QLabel("End(beats)"), 5, 0)
        f_end = QtGui.QDoubleSpinBox()
        f_end.setRange(0, 3.99)
        f_layout.addWidget(f_end, 5, 1)
        f_layout.addWidget(QtGui.QLabel("End Value"), 6, 0)
        f_end_value = QtGui.QSpinBox()
        f_end_value.setRange(0, 127)
        f_layout.addWidget(f_end_value, 6, 1)
        f_add_another = QtGui.QCheckBox("Add another?")
        f_layout.addWidget(f_add_another, 7, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 8, 0)
        f_ok_button.clicked.connect(cc_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 8, 1)
        f_cancel_button.clicked.connect(cc_cancel_handler)
        f_quantize_combobox.setCurrentIndex(self.default_quantize)
        f_window.exec_()
        
    def pitchbend_show_event_dialog(self, x, y):
        f_cell = self.pitchbend_table_widget.item(x, y)
        if f_cell is not None:
            self.default_pb_start = float(self.pitchbend_table_widget.item(x, 0).text())            
            self.default_pb_val = float(self.pitchbend_table_widget.item(x, 1).text())

        def pb_ok_handler():
            f_start_rounded = time_quantize_round(f_start.value())
            if f_draw_line_checkbox.isChecked():
                self.item.draw_pb_line(f_start.value(), f_pb.value(), f_end.value(), f_end_value.value())
            else:
                if not self.item.add_pb(pydaw_pitchbend(f_start_rounded, f_pb.value())):
                    QtGui.QMessageBox.warning(f_window, "Error", "Duplicate pitchbend event")
                    return
                        
            self.default_pb_start = f_start_rounded
            self.default_pb_val = f_pb.value()
                        
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item(self.item_name)
            if not f_add_another.isChecked():
                f_window.close()

        def pb_cancel_handler():
            f_window.close()
        
        def quantize_changed(f_quantize_index):
            f_frac = beat_frac_text_to_float(f_quantize_index)
            f_start.setSingleStep(f_frac)
            self.default_pb_quantize = f_quantize_index

        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Pitchbend")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.currentIndexChanged.connect(quantize_changed)
        f_layout.addWidget(QtGui.QLabel("Quantize(beats)"), 0, 0)
        f_layout.addWidget(f_quantize_combobox, 0, 1)
        f_pb = QtGui.QDoubleSpinBox()
        f_pb.setSingleStep(0.01)
        f_pb.setRange(-1, 1)
        f_pb.setValue(self.default_pb_val)
        f_layout.addWidget(QtGui.QLabel("Value"), 2, 0)
        f_layout.addWidget(f_pb, 2, 1)
        f_layout.addWidget(QtGui.QLabel("Position(beats)"), 3, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_start.setValue(self.default_pb_start)
        f_layout.addWidget(f_start, 3, 1)
        f_draw_line_checkbox = QtGui.QCheckBox("Draw line")
        f_layout.addWidget(f_draw_line_checkbox, 4, 1)
        f_layout.addWidget(QtGui.QLabel("End(beats)"), 5, 0)
        f_end = QtGui.QDoubleSpinBox()
        f_end.setRange(0, 3.99)
        f_layout.addWidget(f_end, 5, 1)
        f_layout.addWidget(QtGui.QLabel("End Value"), 6, 0)
        f_end_value = QtGui.QDoubleSpinBox()
        f_end_value.setRange(-1, 1)
        f_end_value.setSingleStep(0.01)
        f_layout.addWidget(f_end_value, 6, 1)
        f_add_another = QtGui.QCheckBox("Add another?")
        f_layout.addWidget(f_add_another, 7, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 8,0)
        f_ok_button.clicked.connect(pb_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 8,1)
        f_cancel_button.clicked.connect(pb_cancel_handler)
        f_quantize_combobox.setCurrentIndex(self.default_pb_quantize)
        f_window.exec_()

rec_button_group = QtGui.QButtonGroup()

class seq_track:
    def on_vol_change(self, value):
        self.volume_label.setText(str(value) + " dB")
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_vol(self.track_number, self.volume_slider.value())        
    def on_vol_released(self):
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    def on_pan_change(self, value):
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    def on_solo(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_solo(self.track_number, self.solo_checkbox.isChecked())
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    def on_mute(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_mute(self.track_number, self.mute_checkbox.isChecked())
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    def on_rec(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_track_rec(self.track_number, self.record_radiobutton.isChecked())
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    def on_name_changed(self):
        self.track_name_lineedit.setText(pydaw_remove_bad_chars(self.track_name_lineedit.text()))
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
        this_pydaw_project.this_dssi_gui.pydaw_save_track_name(self.track_number, self.track_name_lineedit.text())
    def on_instrument_change(self, selected_instrument):
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_instrument_index(self.track_number, selected_instrument)
    def on_show_ui(self):
        if self.instrument_combobox.currentIndex() > 0:
            this_pydaw_project.this_dssi_gui.pydaw_show_ui(self.track_number)            
    def on_show_fx(self):
        if self.instrument_combobox.currentIndex() > 0:
            this_pydaw_project.this_dssi_gui.pydaw_show_fx(self.track_number)    

    def __init__(self, a_track_num, a_track_text="track"):
        self.suppress_osc = True
        self.track_number = a_track_num
        self.group_box = QtGui.QGroupBox()        
        self.main_vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.main_vlayout)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout2)
        self.volume_slider = QtGui.QSlider()
        self.volume_slider.setMinimum(-50)
        self.volume_slider.setMaximum(12)
        self.volume_slider.setValue(0)
        self.volume_slider.setOrientation(QtCore.Qt.Horizontal)
        self.volume_slider.valueChanged.connect(self.on_vol_change)
        self.volume_slider.sliderReleased.connect(self.on_vol_released)
        self.hlayout2.addWidget(self.volume_slider)
        self.volume_label = QtGui.QLabel()
        self.volume_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
        self.volume_label.setMargin(3)
        self.volume_label.setMinimumWidth(54)
        self.volume_label.setText("0 dB")
        self.hlayout2.addWidget(self.volume_label)
        self.hlayout3 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout3)
        self.track_name_lineedit = QtGui.QLineEdit()
        self.track_name_lineedit.setText(a_track_text)
        self.track_name_lineedit.setMaxLength(24)
        self.track_name_lineedit.setMaximumWidth(90)
        self.track_name_lineedit.setMinimumWidth(90)
        self.track_name_lineedit.editingFinished.connect(self.on_name_changed)
        self.hlayout3.addWidget(self.track_name_lineedit)
        self.instrument_combobox = QtGui.QComboBox()
        self.instrument_combobox.addItems(["None", "Euphoria", "Ray-V"])
        self.instrument_combobox.currentIndexChanged.connect(self.on_instrument_change)
        self.instrument_combobox.setMinimumWidth(100)
        self.hlayout3.addWidget(self.instrument_combobox)
        f_button_style = "QPushButton { background-color: black; border-style: outset; border-width: 2px;	border-radius: 5px; border-color: white; font: bold 12px; padding: 2px;	color:white;}"
        self.ui_button = QtGui.QPushButton("UI")
        self.ui_button.pressed.connect(self.on_show_ui)
        self.ui_button.setMinimumWidth(30)
        self.ui_button.setMaximumWidth(30)
        self.ui_button.setStyleSheet(f_button_style)
        self.hlayout3.addWidget(self.ui_button)
        self.fx_button = QtGui.QPushButton("FX")
        self.fx_button.pressed.connect(self.on_show_fx)
        self.fx_button.setMinimumWidth(30)
        self.fx_button.setMaximumWidth(30)
        self.fx_button.setStyleSheet(f_button_style)
        self.hlayout3.addWidget(self.fx_button)
        self.solo_checkbox = QtGui.QCheckBox()        
        self.solo_checkbox.clicked.connect(self.on_solo)
        self.solo_checkbox.setStyleSheet("QCheckBox{ padding: 0px; } QCheckBox::indicator::unchecked{ image: url(pydaw/solo-off.png);}QCheckBox::indicator::checked{image: url(pydaw/solo-on.png);}")
        self.hlayout3.addWidget(self.solo_checkbox)
        self.mute_checkbox = QtGui.QCheckBox()        
        self.mute_checkbox.clicked.connect(self.on_mute)
        self.mute_checkbox.setStyleSheet("QCheckBox{ padding: 0px; } QCheckBox::indicator::unchecked{ image: url(pydaw/mute-off.png);}QCheckBox::indicator::checked{image: url(pydaw/mute-on.png);}")
        self.hlayout3.addWidget(self.mute_checkbox)
        self.record_radiobutton = QtGui.QRadioButton()        
        rec_button_group.addButton(self.record_radiobutton)
        self.record_radiobutton.toggled.connect(self.on_rec)
        self.record_radiobutton.setStyleSheet("QRadioButton{ padding: 0px; } QRadioButton::indicator::unchecked{image: url(pydaw/record-off.png);}QRadioButton::indicator::checked{image: url(pydaw/record-on.png);}")
        self.hlayout3.addWidget(self.record_radiobutton)
        self.suppress_osc = False
        
    def open_track(self, a_track, a_notify_osc=False):
        if not a_notify_osc:
            self.suppress_osc = True
        self.record_radiobutton.setChecked(a_track.rec)
        self.solo_checkbox.setChecked(a_track.solo)
        self.mute_checkbox.setChecked(a_track.mute)
        self.track_name_lineedit.setText(a_track.name)
        self.volume_slider.setValue(a_track.vol)
        self.instrument_combobox.setCurrentIndex(a_track.inst)
        self.suppress_osc = False

    def get_track(self):
        return pydaw_track(self.solo_checkbox.isChecked(), self.mute_checkbox.isChecked(), self.record_radiobutton.isChecked(),
                           self.volume_slider.value(), str(self.track_name_lineedit.text()), self.instrument_combobox.currentIndex())

class transport_widget:
    def init_playback_cursor(self, a_bar=True):
        if not self.follow_checkbox.isChecked():
            return
        if this_song_editor.table_widget.item(0, self.region_spinbox.value()) is not None:
            this_region_editor.open_region(this_song_editor.table_widget.item(0, self.region_spinbox.value()).text())
        else:
            this_region_editor.clear_items()
        if a_bar:        
            this_region_editor.table_widget.selectColumn(self.bar_spinbox.value() + 1)
        else:
            this_region_editor.table_widget.clearSelection()
        this_song_editor.table_widget.selectColumn(self.region_spinbox.value())
    def on_play(self):
        if self.is_recording:
            self.rec_button.setChecked(True)
            return
        if self.is_playing:
            self.region_spinbox.setValue(self.last_region_num)
            self.bar_spinbox.setValue(self.last_bar)            
        self.is_playing = True
        self.init_playback_cursor()
        self.last_region_num = self.region_spinbox.value()
        self.last_bar = self.bar_spinbox.value()
        this_pydaw_project.this_dssi_gui.pydaw_play(a_region_num=self.region_spinbox.value(), a_bar=self.bar_spinbox.value())
        f_playback_inc = int(((1.0/(float(self.tempo_spinbox.value()) / 60)) * 4000))        
        self.beat_timer.stop()
        self.beat_timer.start(f_playback_inc)
    def on_stop(self):
        if not self.is_playing and not self.is_recording:
            return
        this_pydaw_project.this_dssi_gui.pydaw_stop()
        self.beat_timer.stop()
        self.bar_spinbox.setValue(self.last_bar)
        self.region_spinbox.setValue(self.last_region_num)
        self.init_playback_cursor(a_bar=False)
        if self.is_recording:
            self.is_recording = False
            sleep(2)  #Give it some time to flush the recorded items to disk...                
            if(this_region_editor.enabled):
                this_region_editor.open_region(this_region_editor.region.name)
            this_song_editor.open_song()
            this_pydaw_project.record_stop_git_commit()
        self.is_playing = False
        if not this_song_editor.table_widget.item(0, self.region_spinbox.value()) is None:
            this_region_editor.open_region(this_song_editor.table_widget.item(0, self.region_spinbox.value()).text())        
    def on_rec(self):
        if self.is_playing:
            self.play_button.setChecked(True)
            return
        self.is_recording = True
        self.init_playback_cursor()        
        self.last_region_num = self.region_spinbox.value()
        self.last_bar = self.bar_spinbox.value()
        this_pydaw_project.this_dssi_gui.pydaw_rec(a_region_num=self.region_spinbox.value(), a_bar=self.bar_spinbox.value())
        f_playback_inc = int(((1.0/(float(self.tempo_spinbox.value()) / 60)) * 4000))
        self.beat_timer.start(f_playback_inc)
    def on_tempo_changed(self, a_tempo):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_tempo(a_tempo)
        self.transport.bpm = a_tempo
        this_pydaw_project.save_transport(self.transport)
    def on_loop_mode_changed(self, a_loop_mode):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_loop_mode(a_loop_mode)
        self.transport.loop_mode = a_loop_mode
        this_pydaw_project.save_transport(self.transport)
    def on_keybd_combobox_index_changed(self, a_index):
        self.alsa_output_ports.connect_to_pydaw(str(self.keybd_combobox.currentText()))
        self.transport.midi_keybd = str(self.keybd_combobox.currentText())
        this_pydaw_project.save_transport(self.transport)
    def on_bar_changed(self, a_bar):
        self.transport.bar = a_bar
        if not self.is_playing and not self.is_recording:
            this_pydaw_project.save_transport(self.transport)
    def on_region_changed(self, a_region):
        self.transport.region = a_region
        if not self.is_playing and not self.is_recording:
            this_pydaw_project.save_transport(self.transport)
    def on_follow_cursor_check_changed(self):
        if self.follow_checkbox.isChecked():
            f_item = this_song_editor.table_widget.item(0, self.region_spinbox.value())
            if not f_item is None and f_item.text() != "":
                this_region_editor.open_region(f_item.text())
            else:
                this_region_editor.clear_items()
            this_song_editor.table_widget.selectColumn(self.region_spinbox.value())
            this_region_editor.table_widget.selectColumn(self.bar_spinbox.value())
        else:
            this_region_editor.table_widget.clearSelection()
    def beat_timeout(self):
        if self.loop_mode_combobox.currentIndex() == 1:
            return  #Looping a single bar doesn't require these values to update
        f_new_bar_value = self.bar_spinbox.value() + 1
        #self.region_spinbox.value()
        f_region_length = 8
        if this_region_editor.region is not None and this_region_editor.region.region_length_bars > 0:
            f_region_length = this_region_editor.region.region_length_bars
        if f_new_bar_value >= f_region_length:
            f_new_bar_value = 0
            if self.loop_mode_combobox.currentIndex() != 2:
                self.region_spinbox.setValue(self.region_spinbox.value() + 1)                
                if self.follow_checkbox.isChecked():
                    f_item = this_song_editor.table_widget.item(0, self.region_spinbox.value())
                    if not f_item is None and f_item.text() != "":
                        this_region_editor.open_region(f_item.text())
                    else:
                        this_region_editor.clear_items()
        self.bar_spinbox.setValue(f_new_bar_value)
        if self.follow_checkbox.isChecked():
            this_song_editor.table_widget.selectColumn(self.region_spinbox.value())
            this_region_editor.table_widget.selectColumn(f_new_bar_value + 1)  

    def open_transport(self, a_notify_osc=False):
        if not a_notify_osc:
            self.suppress_osc = True
        self.transport = this_pydaw_project.get_transport()    
        self.tempo_spinbox.setValue(int(self.transport.bpm))
        self.region_spinbox.setValue(int(self.transport.region))
        self.bar_spinbox.setValue(int(self.transport.bar))
        self.last_bar = int(self.transport.bar)
        self.loop_mode_combobox.setCurrentIndex(int(self.transport.loop_mode))
        for i in range(self.keybd_combobox.count()):
            if str(self.keybd_combobox.itemText(i)) == self.transport.midi_keybd:
                self.keybd_combobox.setCurrentIndex(i)
                break
        self.suppress_osc = False

    def __init__(self):
        self.suppress_osc = True
        self.is_recording = False
        self.is_playing = False
        self.last_bar = 0
        self.transport = pydaw_transport()
        self.group_box = QtGui.QGroupBox()
        self.vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.vlayout)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.hlayout2 = QtGui.QHBoxLayout()
        self.vlayout.addLayout(self.hlayout1)
        self.vlayout.addLayout(self.hlayout2)
        self.play_button = QtGui.QRadioButton()        
        self.play_button.setStyleSheet("QRadioButton::indicator::unchecked{image: url(pydaw/play-off.png);}QRadioButton::indicator::checked{image: url(pydaw/play-on.png);}")
        self.play_button.clicked.connect(self.on_play)
        self.hlayout1.addWidget(self.play_button)
        self.stop_button = QtGui.QRadioButton()
        self.stop_button.setStyleSheet("QRadioButton::indicator::unchecked{image: url(pydaw/stop-off.png);}QRadioButton::indicator::checked{image: url(pydaw/stop-off.png);}")
        self.stop_button.clicked.connect(self.on_stop)
        self.hlayout1.addWidget(self.stop_button)
        self.rec_button = QtGui.QRadioButton()
        self.rec_button.setStyleSheet("QRadioButton::indicator::unchecked{image: url(pydaw/rec-off.png);}QRadioButton::indicator::checked{image: url(pydaw/rec-on.png);}")
        self.rec_button.clicked.connect(self.on_rec)        
        self.hlayout1.addWidget(self.rec_button)
        self.hlayout1.addWidget(QtGui.QLabel("BPM:"))
        self.tempo_spinbox = QtGui.QSpinBox()
        self.tempo_spinbox.setRange(50, 200)
        self.tempo_spinbox.valueChanged.connect(self.on_tempo_changed)
        self.hlayout1.addWidget(self.tempo_spinbox)
        self.hlayout1.addWidget(QtGui.QLabel("Region:"))
        self.region_spinbox = QtGui.QSpinBox()
        self.region_spinbox.setRange(0, 300)
        self.region_spinbox.valueChanged.connect(self.on_region_changed)
        self.hlayout1.addWidget(self.region_spinbox)
        self.hlayout1.addWidget(QtGui.QLabel("Bar:"))
        self.bar_spinbox = QtGui.QSpinBox()
        self.bar_spinbox.setRange(0, 7)
        self.bar_spinbox.valueChanged.connect(self.on_bar_changed)
        self.hlayout1.addWidget(self.bar_spinbox)
        f_loop_midi_gridlayout = QtGui.QGridLayout()
        f_loop_midi_gridlayout.addWidget(QtGui.QLabel("MIDI In:"), 0, 0)
        self.keybd_combobox = QtGui.QComboBox()
        self.alsa_output_ports = alsa_ports()
        self.keybd_combobox.addItems(self.alsa_output_ports.get_output_fqnames())
        self.keybd_combobox.currentIndexChanged.connect(self.on_keybd_combobox_index_changed)
        f_loop_midi_gridlayout.addWidget(self.keybd_combobox, 0, 1)
        f_loop_midi_gridlayout.addWidget(QtGui.QLabel("Loop Mode:"), 1, 0)
        f_lower_ctrl_layout = QtGui.QHBoxLayout()
        self.loop_mode_combobox = QtGui.QComboBox()
        self.loop_mode_combobox.addItems(["Off", "Bar", "Region"])
        self.loop_mode_combobox.setMinimumWidth(90)
        self.loop_mode_combobox.currentIndexChanged.connect(self.on_loop_mode_changed)
        f_lower_ctrl_layout.addWidget(self.loop_mode_combobox)
        self.follow_checkbox = QtGui.QCheckBox("Follow cursor?")
        self.follow_checkbox.setChecked(True)
        self.follow_checkbox.clicked.connect(self.on_follow_cursor_check_changed)
        f_lower_ctrl_layout.addWidget(self.follow_checkbox)
        self.scope_button = QtGui.QPushButton("Scope")
        self.scope_button.pressed.connect(launch_jack_oscrolloscope)
        f_button_style = "QPushButton { background-color: black; border-style: outset; border-width: 2px;	border-radius: 5px; border-color: white; font: bold 12px; padding: 2px;	color:white;}"
        self.scope_button.setStyleSheet(f_button_style)
        f_lower_ctrl_layout.addWidget(self.scope_button)
        f_loop_midi_gridlayout.addLayout(f_lower_ctrl_layout, 1, 1)
        self.hlayout1.addLayout(f_loop_midi_gridlayout)
        #This is an awful way to do this, I'll eventually have IPC that goes both directions...
        self.beat_timer = QtCore.QTimer()
        self.beat_timer.timeout.connect(self.beat_timeout)
        self.suppress_osc = False
        
class pydaw_main_window(QtGui.QMainWindow):
    def on_new(self):
        f_file = QtGui.QFileDialog.getSaveFileName(parent=this_main_window ,caption='New Project', directory='.', filter='PyDAW Project (*.pydaw)')
        if not f_file is None and not str(f_file) == "":
            f_file = str(f_file)
            if not f_file.endswith(".pydaw"):
                f_file += ".pydaw"
            global_new_project(f_file)
    def on_open(self):
        f_file = QtGui.QFileDialog.getOpenFileName(parent=this_main_window ,caption='Open Project', directory='.', filter='PyDAW Project (*.pydaw)')
        if not f_file is None and not str(f_file) == "":
            global_open_project(str(f_file))
    def on_save(self):
        this_pydaw_project.save_project()
    def on_save_as(self):
        f_new_file = QtGui.QFileDialog.getSaveFileName(self, "Save project as...", this_pydaw_project.project_file + ".pydaw")
        if f_new_file:        
            this_pydaw_project.save_project_as(f_new_file)
            set_window_title()
            set_default_project(f_new_file)

    def show_offline_rendering_wait_window(self, a_file_name):
        f_file_name = str(a_file_name)
        def ok_handler():
            f_window.close()

        def cancel_handler():
            f_window.close()
        
        def timeout_handler():
            if os.path.isfile(f_file_name):
                f_ok.setEnabled(True)
                f_timer.stop()
                f_time_label.setText("Finished in " + str(f_time_label.text()))
            else:
                f_elapsed_time = time() - f_start_time
                f_time_label.setText(str(round(f_elapsed_time, 1)))
            
        f_start_time = time()
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Rendering to .wav, please wait")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_time_label = QtGui.QLabel("")
        f_time_label.setMinimumWidth(360)
        f_layout.addWidget(f_time_label, 1, 1)
        f_timer = QtCore.QTimer()
        f_timer.timeout.connect(timeout_handler)
        
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(ok_handler)
        f_ok.setEnabled(False)
        f_layout.addWidget(f_ok)
        f_layout.addWidget(f_ok, 2, 2)
        #f_cancel = QtGui.QPushButton("Cancel")
        #f_cancel.pressed.connect(cancel_handler)
        #f_layout.addWidget(f_cancel, 9, 2)
        #TODO:  Send a 'cancel_offline_render' message to the engine...
        f_timer.start(200)
        f_window.exec_()

    def on_offline_render(self):
        def ok_handler():
            if str(f_name.text()) == "":
                QtGui.QMessageBox.warning(f_window, "Error", "Name cannot be empty")
                return
            if (f_end_region.value() < f_start_region.value()) or \
            ((f_end_region.value() == f_start_region.value()) and (f_start_bar.value() <= f_end_region.value())):
                QtGui.QMessageBox.warning(f_window, "Error", "End point is before start point.")
                return
            #TODO:  Check that the end is actually after the start....
            this_pydaw_project.this_dssi_gui.pydaw_offline_render(f_start_region.value(), f_start_bar.value(), f_end_region.value(), f_end_bar.value(), f_name.text())
            f_window.close()
            self.show_offline_rendering_wait_window(f_name.text())

        def cancel_handler():
            f_window.close()
        
        def file_name_select():
            f_file_name = str(QtGui.QFileDialog.getSaveFileName(f_window, "Select a file name to save to..."))
            if not f_file_name.endswith(".wav"):
                f_file_name += ".wav"
            if not f_file_name is None and not str(f_file_name) == "":
                f_name.setText(f_file_name)
            
        f_start_reg = 0
        f_end_reg = 0
                    
        for i in range(300):
            f_item = this_song_editor.table_widget.item(0, i)
            if not f_item is None or f_item.text() != "":
                f_start_reg = i
                break
        
        for i in range(f_start_reg + 1, 300):
            f_item = this_song_editor.table_widget.item(0, i)
            if f_item is None or f_item.text() == "":
                f_end_reg = i
                break
            
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Offline Render")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        
        f_name = QtGui.QLineEdit()
        f_name.setReadOnly(True)
        f_name.setMinimumWidth(360)
        f_layout.addWidget(QtGui.QLabel("File Name:"), 0, 0)
        f_layout.addWidget(f_name, 0, 1)
        f_select_file = QtGui.QPushButton("Select")
        f_select_file.pressed.connect(file_name_select)
        f_layout.addWidget(f_select_file, 0, 2)
        
        f_layout.addWidget(QtGui.QLabel("Start:"), 1, 0)   
        f_start_hlayout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_start_hlayout, 1, 1)
        f_start_hlayout.addWidget(QtGui.QLabel("Region:"))
        f_start_region = QtGui.QSpinBox()
        f_start_region.setRange(0, 298)
        f_start_region.setValue(f_start_reg)
        f_start_hlayout.addWidget(f_start_region)
        f_start_hlayout.addWidget(QtGui.QLabel("Bar:"))
        f_start_bar = QtGui.QSpinBox()
        f_start_bar.setRange(0, 8)
        f_start_hlayout.addWidget(f_start_bar)
        
        f_layout.addWidget(QtGui.QLabel("End:"), 2, 0) 
        f_end_hlayout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_end_hlayout, 2, 1)
        f_end_hlayout.addWidget(QtGui.QLabel("Region:"))
        f_end_region = QtGui.QSpinBox()
        f_end_region.setRange(0, 298)
        f_end_region.setValue(f_end_reg)
        f_end_hlayout.addWidget(f_end_region)
        f_end_hlayout.addWidget(QtGui.QLabel("Bar:"))
        f_end_bar = QtGui.QSpinBox()
        f_end_bar.setRange(0, 8)
        f_end_bar.setValue(1)
        f_end_hlayout.addWidget(f_end_bar)
        f_layout.addWidget(QtGui.QLabel("File is exported to 32 bit .wav at the sample rate Jack is running at.\nYou can convert the format using other programs such as Audacity"), 3, 1)
        f_ok_layout = QtGui.QHBoxLayout()
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 9, 1)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 9, 2)
        f_window.exec_()        
    
    def on_undo_history(self):
        f_window = QtGui.QDialog()
        f_window.setWindowTitle("Undo history")
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_widget = pydaw_git_log_widget(this_pydaw_project.git_repo, global_ui_refresh_callback)
        f_widget.populate_table()
        f_layout.addWidget(f_widget)
        f_window.setGeometry(QtCore.QRect(f_window.x(), f_window.y(), 900, 720))
        f_window.exec_()        
    
    def on_user_manual(self):
        self.show_help_file("pydaw/manual.txt")
    
    def on_about(self):
        self.show_help_file("pydaw/about.txt")
            
    def show_help_file(self, a_file):
        f_window = QtGui.QDialog()
        f_window.setWindowTitle(a_file)
        f_window.setMinimumHeight(600)
        f_window.setMinimumWidth(600)
        f_text_edit = QtGui.QTextEdit()
        f_text_edit.setAutoFormatting(QtGui.QTextEdit.AutoAll)
        f_text_edit.setText(open(a_file).read())
        f_text_edit.setReadOnly(True)        
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_layout.addWidget(f_text_edit)        
        f_window.exec_()

    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        stylesheet_file = os.getcwd() + "/pydaw/style.txt"
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

        self.offline_render_action = QtGui.QAction("Offline Render...", self)
        self.menu_file.addAction(self.offline_render_action)
        self.offline_render_action.triggered.connect(self.on_offline_render)
        
        self.menu_edit = self.menu_bar.addMenu("&Edit")
        
        self.undo_history_action = QtGui.QAction("Undo History...", self)
        self.menu_edit.addAction(self.undo_history_action)
        self.undo_history_action.triggered.connect(self.on_undo_history)        
        
        self.menu_help = self.menu_bar.addMenu("&Help")
        
        self.manual_action = QtGui.QAction("User Manual...", self)
        self.menu_help.addAction(self.manual_action)
        self.manual_action.triggered.connect(self.on_user_manual)
        
        self.about_action = QtGui.QAction("About...", self)
        self.menu_help.addAction(self.about_action)
        self.about_action.triggered.connect(self.on_about)

        self.transport_hlayout = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.transport_hlayout)

        self.transport_hlayout.addWidget(this_transport.group_box, alignment=QtCore.Qt.AlignLeft)
        
        self.main_tabwidget = QtGui.QTabWidget()
        self.song_tab = QtGui.QWidget()
        self.song_tab_hlayout = QtGui.QHBoxLayout(self.song_tab)

        self.main_tabwidget.addTab(self.song_tab, "Song")

        self.main_layout.addWidget(self.main_tabwidget)

        self.song_region_vlayout = QtGui.QVBoxLayout()
        self.song_tab_hlayout.addLayout(self.song_region_vlayout)

        self.song_region_vlayout.addWidget(this_song_editor.group_box)
        self.song_region_vlayout.addWidget(this_region_editor.group_box)

        self.item_tab = QtGui.QWidget()
        self.item_tab_hlayout = QtGui.QHBoxLayout(self.item_tab)
        self.item_tab_hlayout.addWidget(this_item_editor.group_box)        
        self.item_tab_hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))

        self.main_tabwidget.addTab(self.item_tab, "Item")
        #Begin CC Map tab
        self.cc_map_tab = QtGui.QWidget()
        f_cc_map_main_vlayout = QtGui.QVBoxLayout(self.cc_map_tab)
        f_cc_map_label = QtGui.QLabel("Below you can edit the MIDI CC maps for PyDAW's plugins. All CCs are sent to both Ray-V/Euphoria and Modulex, \nso the first 2 can overlap each other, but neither should overlap with Modulex.  \nYou must restart PyDAW for changes to the CC maps to take effect.  Maps will not populate until you've started\nthe plugin for the first time, and then restarted PyDAW..")
        f_cc_map_label.setMaximumWidth(1200)
        f_cc_map_main_vlayout.addWidget(f_cc_map_label)
        f_cc_map_hlayout = QtGui.QHBoxLayout()
        f_cc_map_main_vlayout.addLayout(f_cc_map_hlayout)
        self.cc_map_rayv = pydaw_cc_map_editor(2)
        f_cc_map_hlayout.addWidget(self.cc_map_rayv.groupbox)
        self.cc_map_euphoria = pydaw_cc_map_editor(1)
        f_cc_map_hlayout.addWidget(self.cc_map_euphoria.groupbox)
        self.cc_map_modulex = pydaw_cc_map_editor(-1)
        f_cc_map_hlayout.addWidget(self.cc_map_modulex.groupbox)
        f_ccs_spacer = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        f_cc_map_hlayout.addItem(f_ccs_spacer)
        self.main_tabwidget.addTab(self.cc_map_tab, "CC Maps")        
        self.show()
    
    def closeEvent(self, event):
        f_reply = QtGui.QMessageBox.question(self, 'Message',
            "Save project before quitting?", QtGui.QMessageBox.Yes | 
            QtGui.QMessageBox.No | QtGui.QMessageBox.Cancel, QtGui.QMessageBox.No)

        if f_reply == QtGui.QMessageBox.Yes:
            this_pydaw_project.save_project()            
            event.accept()
        elif f_reply == QtGui.QMessageBox.Cancel:
            event.ignore()
        else:
            event.accept()

class pydaw_cc_map_editor:
    def on_save(self):
        f_result_dict = {}
        for i in range(0, 128):
            f_cc_num_str = str(i)
            for i in range(len(f_cc_num_str), 3):
                f_cc_num_str = "0" + f_cc_num_str
            f_result_dict[f_cc_num_str] = ["000", "empty"]
        for i in range(0, 128):
            if not self.cc_table.item(i, 0) is None:
                f_cc_num_str = str(self.cc_table.item(i, 0).text())
                for i in range(len(f_cc_num_str), 3):
                    f_cc_num_str = "0" + f_cc_num_str
                f_result_dict[f_cc_num_str] = [str(self.cc_table.item(i, 2).text()), str(self.cc_table.item(i, 1).text())]        
        f_result = '''"This is a MIDI CC mapping file.  The first 3 digits are the MIDI CC number,  do not edit them.  
The 2nd 3 digits are the LADSPA port number, you may change these from any value from 001 to 999.  
Any additional text must be enclosed in quotation marks."

'''
        for k, v in sorted(f_result_dict.iteritems()):
            f_result += str(k) + "-" + str(v[0]) + ' "' + str(v[1]) + '"' + "\n"
        f_handle = open(self.file_name, "w")
        f_handle.write(f_result)
        f_handle.close()        
    
    def on_click(self, x, y):
        f_cell = self.cc_table.item(x, y)
        if f_cell is None or str(f_cell.text()) == "":
            return
        
        def cc_ok_handler():
            f_cc_num_str = str(f_cc.value())
            for i in range(len(f_cc_num_str), 3):
                f_cc_num_str = "0" + f_cc_num_str
            self.cc_table.setItem(x, 0, QtGui.QTableWidgetItem(f_cc_num_str))
            f_window.close()

        def cc_cancel_handler():
            f_window.close()
        
        def quantize_changed(f_quantize_index):
            f_frac = beat_frac_text_to_float(f_quantize_index)
            f_start.setSingleStep(f_frac)
            self.default_quantize = f_quantize_index

        f_default_cc_num = int(self.cc_table.item(x, 0).text())            
        
        f_window = QtGui.QDialog()
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_cc = QtGui.QSpinBox()
        f_cc.setRange(1, 127)
        f_cc.setValue(f_default_cc_num)
        f_layout.addWidget(QtGui.QLabel("CC"), 1, 0)
        f_layout.addWidget(f_cc, 1, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 7,0)
        f_ok_button.clicked.connect(cc_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 7,1)
        f_cancel_button.clicked.connect(cc_cancel_handler)
        f_window.exec_()
        
    
    def __init__(self, a_index):
        if a_index == -1:
            f_name = "Modulex"
            self.file_name = expanduser("~") + "/pydaw/lms_modulex-cc_map.txt"
        elif a_index == 1:
            f_name = "Euphoria"
            self.file_name = expanduser("~") + "/pydaw/euphoria-cc_map.txt"
        elif a_index == 2:
            f_name = "Ray-V"
            self.file_name = expanduser("~") + "/pydaw/ray_v-cc_map.txt"
        else:
            assert(0)
        self.groupbox = QtGui.QGroupBox(f_name)
        self.groupbox.setMaximumWidth(420)
        f_vlayout = QtGui.QVBoxLayout(self.groupbox)
        self.cc_table = QtGui.QTableWidget(127, 3)
        self.cc_table.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.cc_table.setHorizontalHeaderLabels(["CC", "Description", "LADSPA Port"])
        self.cc_table.cellClicked.connect(self.on_click)
        f_vlayout.addWidget(self.cc_table)
        f_button_layout = QtGui.QHBoxLayout()
        f_vlayout.addLayout(f_button_layout)
        f_button_spacer = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        f_button_layout.addItem(f_button_spacer)
        f_save_button = QtGui.QPushButton("Save")
        f_save_button.pressed.connect(self.on_save)
        f_button_layout.addWidget(f_save_button)
        try:
            f_cc_map_text = open(self.file_name, "r").read()
        except:
            return  #If we can't open the file, then it likely doesn't exist, and we won't bother trying to edit it
        
        f_cc_map_arr = f_cc_map_text.split("\n")
        f_row_index = 0
        for f_line in f_cc_map_arr:
            if not re.match(r'[0-1][0-9][0-9]-[0-9][0-9][0-9] "*"', f_line) is None:
                f_line_arr = f_line.split("-")
                f_cc_num = f_line_arr[0]
                f_line_arr2 = f_line_arr[1].split(" ", 1)                
                f_ladspa_port = f_line_arr2[0]
                if f_ladspa_port != "000":
                    f_desc = f_line_arr2[1].strip('"')
                    self.cc_table.setItem(f_row_index, 0, QtGui.QTableWidgetItem(f_cc_num))
                    self.cc_table.setItem(f_row_index, 1, QtGui.QTableWidgetItem(f_desc))
                    self.cc_table.setItem(f_row_index, 2, QtGui.QTableWidgetItem(f_ladspa_port))
                    f_row_index += 1

def set_default_project(a_project_path):
    f_def_file = expanduser("~") + '/pydaw/last-project.txt'
    f_handle = open(f_def_file, 'w')
    f_handle.write(str(a_project_path))
    f_handle.close()

def global_close_all():    
    this_region_editor.clear_new()
    this_item_editor.clear_new()
    
def global_ui_refresh_callback():
    """ Use this to re-open all existing items/regions/song in their editors when the files have been changed externally"""
    if this_item_editor.enabled:    
        this_item_editor.open_item(this_item_editor.item_name)
    if this_region_editor.enabled:
        this_region_editor.open_region(this_region_editor.region_name_lineedit.text())
    this_region_editor.open_tracks()
    this_song_editor.open_song()
    this_pydaw_project.this_dssi_gui.pydaw_open_song(this_pydaw_project.project_folder)  #Re-open the project so that any changes can be caught by the back-end
    
def set_window_title():
    this_main_window.setWindowTitle('PyDAW - ' + this_pydaw_project.project_folder + "/" + this_pydaw_project.project_file + ".pydaw")
#Opens or creates a new project
def global_open_project(a_project_file, a_notify_osc=True):
    global_close_all()
    global this_pydaw_project
    if(len(argv) >= 2):
        this_pydaw_project = pydaw_project((argv[1]))
    else:
        this_pydaw_project = pydaw_project()
    this_pydaw_project.suppress_updates = True
    this_pydaw_project.open_project(a_project_file, a_notify_osc)
    this_song_editor.open_song()    
    this_region_editor.open_tracks()
    this_transport.open_transport()
    set_default_project(a_project_file)
    set_window_title()
    this_pydaw_project.suppress_updates = False

def global_new_project(a_project_file):
    global_close_all()
    global this_pydaw_project
    if(len(argv) >= 2):
        this_pydaw_project = pydaw_project(argv[1])
    else:
        this_pydaw_project = pydaw_project()
    this_pydaw_project.new_project(a_project_file)
    this_pydaw_project.save_transport(this_transport.transport)
    this_pydaw_project.save_project()
    this_song_editor.open_song()
    this_pydaw_project.save_song(this_song_editor.song)
    this_transport.open_transport()
    set_default_project(a_project_file)
    set_window_title()

def about_to_quit():
    this_pydaw_project.quit_handler()

app = QtGui.QApplication(sys.argv)
app.setWindowIcon(QtGui.QIcon('pydaw.ico'))
app.aboutToQuit.connect(about_to_quit)
this_song_editor = song_editor()
this_region_editor = region_list_editor()
this_item_editor = item_list_editor()
this_transport = transport_widget()

this_main_window = pydaw_main_window() #You must call this after instantiating the other widgets, as it relies on them existing
this_main_window.setWindowState(QtCore.Qt.WindowMaximized)

f_def_file = expanduser("~") + '/pydaw/last-project.txt'
if os.path.exists(f_def_file):
    f_handle = open(f_def_file, 'r')
    default_project_file = f_handle.read()
    f_handle.close()
    if not os.path.exists(default_project_file):
        default_project_file = expanduser("~") + '/pydaw/default-project/default.pydaw'
else:
    default_project_file = expanduser("~") + '/pydaw/default-project/default.pydaw'
if os.path.exists(default_project_file):
    global_open_project(default_project_file)
else:
    global_new_project(default_project_file)

sys.exit(app.exec_())

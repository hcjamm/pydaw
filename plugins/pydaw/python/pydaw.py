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

import sip

global_pydaw_version_string = "pydaw2"
global_pydaw_file_type_string = 'PyDAW2 Project (*.pydaw2)'

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
        for f_i in range(0, 300):
            f_headers_arr.append(str(f_i))
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
                    this_pydaw_project.git_repo.git_commit("-a", "Create empty region '" + str(f_new_lineedit.text()) + "' at " + str(y))
                elif f_copy_radiobutton.isChecked():
                    this_pydaw_project.copy_region(str(f_copy_combobox.currentText()), str(f_new_lineedit.text()))
                    this_pydaw_project.git_repo.git_commit("-a", "Create new region '" + str(f_new_lineedit.text()) + "' at " + str(y) + " copying from " + str(f_copy_combobox.currentText()))
                self.add_qtablewidgetitem(f_new_lineedit.text(), y)
                self.song.add_region_ref(y, str(f_new_lineedit.text()))
                this_region_editor.open_region(f_new_lineedit.text())
                this_pydaw_project.save_song(self.song)
                if not f_is_playing:
                    this_transport.region_spinbox.setValue(y)
                f_window.close()

            def song_cancel_handler():
                f_window.close()

            def on_name_changed():
                f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

            def on_current_index_changed(a_index):
                f_copy_radiobutton.setChecked(True)

            f_window = QtGui.QDialog(this_main_window)
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
        self.main_vlayout = QtGui.QVBoxLayout()
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)
        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(300)
        self.table_widget.setRowCount(1)
        self.table_widget.setMinimumHeight(102)
        self.table_widget.setMaximumHeight(102)
        self.table_widget.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.table_widget.verticalHeader().setVisible(False)
        self.table_widget.setAutoScroll(True)
        self.table_widget.setAutoScrollMargin(1)
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
            f_commit_msg = "Deleted region references at "
            for f_item in self.table_widget.selectedIndexes():
                f_commit_msg += str(f_item.column())
                f_empty = QtGui.QTableWidgetItem() #Clear the item
                self.table_widget.setItem(f_item.row(), f_item.column(), f_empty)
            self.tablewidget_to_song()
            this_region_editor.clear_items()
            this_region_editor.region_name_lineedit.setText("")
            this_region_editor.enabled = False
            this_pydaw_project.git_repo.git_commit("-a", f_commit_msg)
        else:
            QtGui.QTableWidget.keyPressEvent(self.table_widget, event)

    def table_drop_event(self, a_event):
        QtGui.QTableWidget.dropEvent(self.table_widget, a_event)
        a_event.acceptProposedAction()
        self.tablewidget_to_song()
        self.table_widget.clearSelection()
        this_pydaw_project.git_repo.git_commit("-a", "Drag-n-drop song item(s)")

    def tablewidget_to_song(self):
        """ Flush the edited content of the QTableWidget back to the native song class... """
        self.song.regions = {}
        for f_i in range(0, 300):
            f_item = self.table_widget.item(0, f_i)
            if not f_item is None:
                if f_item.text() != "":
                    self.song.add_region_ref(f_i, f_item.text())
        this_pydaw_project.save_song(self.song)
        self.open_song()

class region_list_editor:
    def add_qtablewidgetitem(self, a_name, a_track_num, a_bar_num):
        """ Adds a properly formatted item.  This is not for creating empty items... """
        f_qtw_item = QtGui.QTableWidgetItem(a_name)
        #f_qtw_item.setBackground(pydaw_item_gradient)
        f_qtw_item.setBackground(pydaw_track_gradients[a_track_num])
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
        self.tracks = []
        for i in range(0, pydaw_midi_track_count):
            track = seq_track(a_track_num=i, a_track_text="track" + str(i + 1))
            self.tracks.append(track)
            self.table_widget.setCellWidget(i, 0, track.group_box)
        self.table_widget.setColumnWidth(0, 390)
        self.set_region_length()

    def set_region_length(self, a_length=8):
        self.region_length = a_length
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
        for i in range(self.table_widget.rowCount()):
            for i2 in range(1, self.table_widget.columnCount()):
                f_empty_item = QtGui.QTableWidgetItem()
                self.table_widget.setItem(i, i2, f_empty_item)
        for i in range(self.table_widget.rowCount()):
            f_item = QtGui.QTableWidgetItem()
            f_item.setFlags(f_item.flags() & ~QtCore.Qt.ItemIsEditable & ~QtCore.Qt.ItemIsSelectable & ~QtCore.Qt.ItemIsEnabled)
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
            self.set_region_length()
            self.length_alternate_spinbox.setValue(8)
            this_transport.bar_spinbox.setRange(0, 7)
            self.length_default_radiobutton.setChecked(True)
        self.enabled = True
        for f_item in self.region.items:
            if f_item.bar_num < self.region.region_length_bars or (self.region.region_length_bars == 0 and f_item.bar_num < 8):
                self.add_qtablewidgetitem(f_item.item_name, f_item.track_num, f_item.bar_num)

    def warn_no_region_selected(self):
        QtGui.QMessageBox.warning(this_main_window, "", "You must create or select a region first by clicking in the song editor above.")

    def cell_clicked(self, x, y):
        if y <= 0 or x < 0:
            return
        if not self.enabled:
            self.warn_no_region_selected()
            return
        if (this_transport.is_playing or this_transport.is_recording) and this_transport.follow_checkbox.isChecked():
            this_transport.follow_checkbox.setChecked(False)
        f_item = self.table_widget.item(x, y)
        if f_item is None or f_item.text() == "":
            self.show_cell_dialog(x, y)

    def cell_double_clicked(self, x, y):
        if not self.enabled:
            self.warn_no_region_selected()
            return
        f_item = self.table_widget.item(x, y)
        if f_item is None:
            self.show_cell_dialog(x, y)
        else:
            f_item_name = str(f_item.text())
            if f_item_name != "":
                this_item_editor.open_item([f_item_name])
                this_main_window.main_tabwidget.setCurrentIndex(1)
            else:
                self.show_cell_dialog(x, y)

    def show_cell_dialog(self, x, y):
        def note_ok_handler():
            if (f_new_radiobutton.isChecked() and f_item_count.value() == 1):
                f_cell_text = str(f_new_lineedit.text())
                this_pydaw_project.create_empty_item(f_cell_text)
                this_item_editor.open_item([f_cell_text])
                self.add_qtablewidgetitem(f_cell_text, x, y - 1)
                self.region.add_item_ref(x, y - 1, f_cell_text)
                this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            elif f_new_radiobutton.isChecked() and f_item_count.value() > 1:
                f_name_suffix = 1
                f_cell_text = str(f_new_lineedit.text())
                f_item_list = []
                for i in range(f_item_count.value()):
                    while this_pydaw_project.item_exists(f_cell_text + "-" + str(f_name_suffix)):
                        f_name_suffix += 1
                    f_item_name = f_cell_text + "-" + str(f_name_suffix)
                    f_item_list.append(f_item_name)
                    this_pydaw_project.create_empty_item(f_item_name)
                    self.add_qtablewidgetitem(f_item_name, x, y - 1 + i)
                    self.region.add_item_ref(x, y - 1 + i, f_item_name)
                this_item_editor.open_item(f_item_list)
                this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            elif f_copy_radiobutton.isChecked():
                f_cell_text = str(f_copy_combobox.currentText())
                self.add_qtablewidgetitem(f_cell_text, x, y - 1)
                self.region.add_item_ref(x, y - 1, f_cell_text)
                this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            elif f_copy_from_radiobutton.isChecked():
                f_cell_text = str(f_new_lineedit.text())
                this_pydaw_project.copy_item(str(f_copy_combobox.currentText()), f_cell_text)
                self.add_qtablewidgetitem(f_cell_text, x, y - 1)
                self.region.add_item_ref(x, y - 1, f_cell_text)
                this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)

            this_pydaw_project.git_repo.git_commit("-a", "Add reference(s) to item (group) '" + f_cell_text + "' in region '" + str(self.region_name_lineedit.text()))
            self.last_item_copied = f_cell_text

            f_window.close()

        def note_cancel_handler():
            f_window.close()

        def copy_combobox_index_changed(a_index):
            f_copy_radiobutton.setChecked(True)

        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog(this_main_window)
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
        f_layout.addWidget(QtGui.QLabel("Item Count:"), 2, 1)
        f_item_count = QtGui.QSpinBox()
        f_item_count.setRange(1, self.region_length - y + 1)
        f_item_count.setToolTip("Only used for 'New'")
        f_layout.addWidget(f_item_count, 2, 2)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_ok_cancel_layout, 5, 2)
        f_ok_button = QtGui.QPushButton("OK")
        f_ok_cancel_layout.addWidget(f_ok_button)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_ok_cancel_layout.addWidget(f_cancel_button)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_window.exec_()

    def update_region_length(self, a_value=None):
        if not self.enabled:
            return
        if self.length_alternate_radiobutton.isChecked():
            self.region.region_length_bars = self.length_alternate_spinbox.value()
            self.set_region_length(self.region.region_length_bars)
            this_pydaw_project.git_repo.git_commit("-a", "Set region '" + str(self.region_name_lineedit.text()) + "' length to " + str(self.length_alternate_spinbox.value()))
        else:
            self.region.region_length_bars = 0
            self.set_region_length()
            this_pydaw_project.git_repo.git_commit("-a", "Set region '" + str(self.region_name_lineedit.text()) + "' length to default value")
        this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
        self.open_region(self.region_name_lineedit.text())

    def column_clicked(self, a_val):
        if a_val > 0:
            this_transport.bar_spinbox.setValue(a_val - 1)

    def __init__(self):
        self.enabled = False #Prevents user from editing a region before one has been selected
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QGridLayout()

        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0, 1, 0)
        self.region_num_label = QtGui.QLabel()
        self.region_num_label.setText("Region:")
        self.hlayout0.addWidget(self.region_num_label)
        self.region_name_lineedit = QtGui.QLineEdit()
        self.region_name_lineedit.setEnabled(False)
        self.region_name_lineedit.setMaximumWidth(330)
        self.hlayout0.addWidget(self.region_name_lineedit)
        self.hlayout0.addItem(QtGui.QSpacerItem(10,10, QtGui.QSizePolicy.Expanding))
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
        self.table_widget.verticalHeader().setVisible(False)
        self.table_widget.horizontalHeader().sectionClicked.connect(self.column_clicked)
        self.table_widget.setMinimumHeight(360)
        self.table_widget.setAutoScroll(True)
        self.table_widget.setAutoScrollMargin(1)
        self.table_widget.setColumnCount(9)
        self.table_widget.setRowCount(pydaw_midi_track_count)
        self.table_widget.cellDoubleClicked.connect(self.cell_double_clicked)
        self.table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.table_widget.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.table_widget.cellClicked.connect(self.cell_clicked)
        self.table_widget.setDragDropOverwriteMode(False)
        self.table_widget.setDragEnabled(True)
        self.table_widget.setDragDropMode(QtGui.QAbstractItemView.InternalMove)
        self.table_widget.dropEvent = self.table_drop_event
        self.table_widget.keyPressEvent = self.table_keyPressEvent
        self.table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.table_widget.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)

        self.edit_group_action = QtGui.QAction("Edit Selected Items as Group", self.table_widget)
        self.edit_group_action.triggered.connect(self.edit_group)
        self.table_widget.addAction(self.edit_group_action)

        self.copy_action = QtGui.QAction("Copy (CTRL+C)", self.table_widget)
        self.copy_action.triggered.connect(self.copy_selected)
        self.table_widget.addAction(self.copy_action)
        self.paste_action = QtGui.QAction("Paste (CTRL+V)", self.table_widget)
        self.paste_action.triggered.connect(self.paste_clipboard)
        self.table_widget.addAction(self.paste_action)
        self.unlink_action = QtGui.QAction("Unlink Single Item(CTRL+D)", self.table_widget)
        self.unlink_action.triggered.connect(self.on_unlink_item)
        self.table_widget.addAction(self.unlink_action)
        self.unlink_selected_action = QtGui.QAction("Auto-Unlink Selected Items", self.table_widget)
        self.unlink_selected_action.triggered.connect(self.on_auto_unlink_selected)
        self.table_widget.addAction(self.unlink_selected_action)
        self.delete_action = QtGui.QAction("Delete (Del)", self.table_widget)
        self.delete_action.triggered.connect(self.delete_selected)
        self.table_widget.addAction(self.delete_action)
        self.draw_ccs_action = QtGui.QAction("Draw CC Automation (CTRL+F)", self.table_widget)
        self.draw_ccs_action.triggered.connect(self.on_draw_ccs)
        self.table_widget.addAction(self.draw_ccs_action)
        self.main_vlayout.addWidget(self.table_widget, 2, 0)
        self.last_item_copied = None
        self.reset_tracks()
        self.clipboard = []
        self.last_cc_line_num = 1

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

    def edit_group(self):
        f_result = []
        for i in range(pydaw_midi_track_count):
            for i2 in range(1, self.region_length + 1):
                f_item = self.table_widget.item(i, i2)
                if not f_item is None and not str(f_item.text()) == "" and f_item.isSelected():
                    f_result.append(str(f_item.text()))
        this_item_editor.open_item(f_result)
        this_main_window.main_tabwidget.setCurrentIndex(1)

    def on_draw_ccs(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return

        def ccs_ok_handler():
            self.last_cc_line_num = f_cc_num.value()
            f_unlink_base_name = pydaw_remove_bad_chars(f_new_lineedit.text())
            if f_unlink_base_name == "":
                QtGui.QMessageBox.warning(self.table_widget, "Error", "New/Unlink Prefix cannot be empty.")
                return
            if f_end_val.value() == f_start_val.value():
                QtGui.QMessageBox.warning(self.table_widget, "Error", "End value is the same as start value.")
                return
            f_item_list = []
            f_item_names = []  #doing this as 2 separate lists because AFAIK ordering isn't guaranteed in a dict?

            f_track_num = f_track_combobox.currentIndex()
            for i in range(f_start_bar.value() + 1, f_end_bar.value() + 1):
                f_item = self.table_widget.item(f_track_num, i)
                if f_item is None or str(f_item.text()) == "":  #Create the item
                    f_new_item_name = this_pydaw_project.get_next_default_item_name(f_unlink_base_name)
                    print(f_new_item_name)
                    this_pydaw_project.create_empty_item(f_new_item_name)
                    f_item_list.append(this_pydaw_project.get_item(f_new_item_name))
                    f_item_names.append(f_new_item_name)
                    self.region.add_item_ref(f_track_num, i - 1, f_new_item_name)
                elif f_unlink_items.isChecked():  #item exists and we are unlinking it
                    f_new_item_name = this_pydaw_project.get_next_default_item_name(f_unlink_base_name)
                    print(f_new_item_name)
                    f_item_text = str(f_item.text())
                    this_pydaw_project.copy_item(f_item_text, f_new_item_name)
                    f_item_list.append(this_pydaw_project.get_item(f_new_item_name))
                    f_item_names.append(f_new_item_name)
                    self.region.add_item_ref(f_track_num, i - 1, f_new_item_name)
                else:
                    f_item_text = str(f_item.text())
                    f_item_list.append(this_pydaw_project.get_item(f_item_text))
                    f_item_names.append(f_item_text)

            pydaw_draw_multi_item_cc_line(f_cc_num.value(), f_start_val.value(), f_end_val.value(), f_item_list)

            for i in range(len(f_item_list)):
                this_pydaw_project.save_item(f_item_names[i], f_item_list[i])

            this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            self.open_region(self.region_name_lineedit.text())
            this_pydaw_project.git_repo.git_commit("-a", "Draw CC line in region " + str(self.region_name_lineedit.text()))
            f_window.close()

        def ccs_cancel_handler():
            f_window.close()

        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        def on_start_end_change(f_value=None):
            if f_end_bar.value() <= f_start_bar.value():
                f_end_bar.setValue(f_start_bar.value() + 1)


        def plugin_changed(a_val=None):
            f_control_combobox.clear()
            f_control_combobox.addItems(global_cc_maps[str(f_plugin_combobox.currentText())].keys())

        def control_changed(a_val=None):
            f_plugin_str = str(f_plugin_combobox.currentText())
            f_control_str = str(f_control_combobox.currentText())
            if f_plugin_str != '' and f_control_str != '':
                f_value = int(global_cc_maps[f_plugin_str][f_control_str])
                f_cc_num.setValue(f_value)

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Draw multi-item CCs...")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_plugin_combobox = QtGui.QComboBox()
        f_plugin_combobox.addItems(global_cc_maps.keys())
        f_layout.addWidget(QtGui.QLabel("Plugin"), 1, 0)
        f_layout.addWidget(f_plugin_combobox, 1, 1)

        f_control_combobox = QtGui.QComboBox()
        f_layout.addWidget(QtGui.QLabel("Control"), 2, 0)
        f_layout.addWidget(f_control_combobox, 2, 1)

        plugin_changed()
        f_plugin_combobox.currentIndexChanged.connect(plugin_changed)
        f_control_combobox.currentIndexChanged.connect(control_changed)

        f_cc_num = QtGui.QSpinBox()
        f_cc_num.setRange(1, 127)
        f_cc_num.setValue(self.last_cc_line_num)
        f_layout.addWidget(QtGui.QLabel("CC Num:"), 3, 0)
        f_layout.addWidget(f_cc_num, 3, 1)

        f_track_combobox = QtGui.QComboBox()
        for f_track in this_region_editor.tracks:
            f_track_combobox.addItem(f_track.track_name_lineedit.text())
        f_track_combobox.setCurrentIndex(self.table_widget.currentRow())
        f_layout.addWidget(QtGui.QLabel("Track:"), 6, 0)
        f_layout.addWidget(f_track_combobox, 6, 1)
        f_start_bar = QtGui.QSpinBox()
        f_start_bar.setRange(0, 7)  #TODO:  Make 7 into region length - 1...
        f_start_bar.valueChanged.connect(on_start_end_change)
        f_layout.addWidget(QtGui.QLabel("Start"), 7, 0)
        f_layout.addWidget(f_start_bar, 7, 1)
        f_end_bar = QtGui.QSpinBox()
        f_end_bar.setRange(1, 8)  #TODO:  Make 8 into region length...
        f_end_bar.valueChanged.connect(on_start_end_change)
        f_start_bar.setValue(self.table_widget.currentColumn() - 1)
        f_layout.addWidget(QtGui.QLabel("End"), 9, 0)
        f_layout.addWidget(f_end_bar, 9, 1)
        f_start_val = QtGui.QSpinBox()
        f_start_val.setRange(0, 127)
        f_layout.addWidget(QtGui.QLabel("Start Value"), 14, 0)
        f_layout.addWidget(f_start_val, 14, 1)
        f_end_val = QtGui.QSpinBox()
        f_end_val.setRange(0, 127)
        f_layout.addWidget(QtGui.QLabel("End Value"), 18, 0)
        f_layout.addWidget(f_end_val, 18, 1)
        f_unlink_items = QtGui.QCheckBox("Unlink items first?")
        f_layout.addWidget(f_unlink_items, 21, 1)
        f_new_lineedit = QtGui.QLineEdit()
        f_new_lineedit.editingFinished.connect(on_name_changed)
        f_new_lineedit.setMaxLength(24)
        f_new_lineedit.setText("unlinked")
        f_layout.addWidget(QtGui.QLabel("New/Unlink Prefix"), 24, 0)
        f_layout.addWidget(f_new_lineedit, 24, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 27, 0)
        f_ok_button.clicked.connect(ccs_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 27, 1)
        f_cancel_button.clicked.connect(ccs_cancel_handler)
        f_window.exec_()

    def on_unlink_item(self):
        """ Rename a single instance of an item and make it into a new item """
        if not self.enabled:
            self.warn_no_region_selected()
            return

        f_current_item = self.table_widget.currentItem()
        x = self.table_widget.currentRow()
        y = self.table_widget.currentColumn()

        if f_current_item is None or str(f_current_item.text()) == "" or x < 0 or y < 1:
            return

        f_current_item_text = str(f_current_item.text())
        x = self.table_widget.currentRow()
        y = self.table_widget.currentColumn()

        def note_ok_handler():
            f_cell_text = str(f_new_lineedit.text())
            #this_pydaw_project.create_empty_item(f_new_lineedit.text())
            this_pydaw_project.copy_item(str(f_current_item.text()), str(f_new_lineedit.text()))
            this_item_editor.open_item([f_cell_text])
            self.last_item_copied = f_cell_text
            self.add_qtablewidgetitem(f_cell_text, x, y - 1)
            self.region.add_item_ref(x, y - 1, f_cell_text)
            this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
            this_pydaw_project.git_repo.git_commit("-a", "Unlink item '" +  f_current_item_text + "' as '" + f_cell_text + "'")
            f_window.close()

        def note_cancel_handler():
            f_window.close()

        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog(this_main_window)
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

    def on_auto_unlink_selected(self):
        """ Currently adds an automatic -N suffix, but this behavior may be changed later"""
        for i in range(pydaw_midi_track_count):
            for i2 in range(1, self.region_length + 1):
                f_item = self.table_widget.item(i, i2)
                if not f_item is None and not str(f_item.text()) == "" and f_item.isSelected():
                    f_item_name = str(f_item.text())
                    f_name_suffix = 1
                    while this_pydaw_project.item_exists(f_item_name + "-" + str(f_name_suffix)):
                        f_name_suffix += 1
                    f_cell_text = f_item_name + "-" + str(f_name_suffix)
                    this_pydaw_project.copy_item(f_item_name, f_cell_text)
                    self.add_qtablewidgetitem(f_cell_text, i, i2 - 1)
                    self.region.add_item_ref(i, i2 - 1, f_cell_text)
        this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)
        this_pydaw_project.git_repo.git_commit("-a", "Auto-Unlink items")

    def paste_clipboard(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return
        f_selected_cells = self.table_widget.selectedIndexes()
        if len(f_selected_cells) == 0:
            return
        f_base_row = f_selected_cells[0].row()
        f_base_column = f_selected_cells[0].column() - 1
        for f_item in self.clipboard:
            f_column = f_item[1] + f_base_column
            f_region_length = 8
            if self.region.region_length_bars > 0:
                f_region_length = self.region.region_length_bars
            if f_column >= f_region_length or f_column < 0:
                continue
            f_row = f_item[0] + f_base_row
            if f_row >= pydaw_midi_track_count or f_row < 0:
                continue
            self.add_qtablewidgetitem(f_item[2], f_row, f_column)
        self.tablewidget_to_region()

    def delete_selected(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return

        for f_item in self.table_widget.selectedIndexes():
            f_empty = QtGui.QTableWidgetItem() #Clear the item
            self.table_widget.setItem(f_item.row(), f_item.column(), f_empty)
        self.tablewidget_to_region()

    def copy_selected(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return

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
        for i in range(0, pydaw_midi_track_count):
            for i2 in range(1, self.table_widget.columnCount()):
                f_item = self.table_widget.item(i, i2)
                if not f_item is None:
                    if f_item.text() != "":
                        self.region.add_item_ref(i, i2 - 1, f_item.text())
        this_pydaw_project.save_region(str(self.region_name_lineedit.text()), self.region)

def global_update_audio_track_comboboxes(a_index=None, a_value=None):
    if not a_index is None and not a_value is None:
        global_audio_track_names[int(a_index)] = str(a_value)
    global global_suppress_audio_track_combobox_changes
    global_suppress_audio_track_combobox_changes = True
    if this_audio_editor.track_type_combobox.currentIndex() == 0:
        this_audio_editor.enabled = False
        f_tmp_index = this_audio_editor.track_select_combobox.currentIndex()
        this_audio_editor.track_select_combobox.clear()
        this_audio_editor.track_select_combobox.addItems(['test', 'test2'])  #This is to ensure that the text clears, which apparently won't happen automatically
        this_audio_editor.track_select_combobox.setCurrentIndex(1)
        this_audio_editor.track_select_combobox.clear()
        this_audio_editor.track_select_combobox.addItems(global_audio_track_names.values())
        this_audio_editor.track_select_combobox.setCurrentIndex(f_tmp_index)
        this_audio_editor.enabled = True
    for f_cbox in global_audio_track_comboboxes:
        f_current_index = f_cbox.currentIndex()
        f_cbox.clear()
        f_cbox.addItems(['test', 'test2'])  #This is to ensure that the text clears, which apparently won't happen automatically
        f_cbox.setCurrentIndex(1)
        f_cbox.clear()
        f_cbox.addItems(global_audio_track_names.values())
        f_cbox.setCurrentIndex(f_current_index)

    global_suppress_audio_track_combobox_changes = False

global_bus_track_names = ['Master', 'Bus1', 'Bus2', 'Bus3', 'Bus4']




class audio_viewer_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_length, a_height, a_name, a_track_num, a_y_pos, a_audio_item):
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, a_length, a_height)
        f_name_arr = a_name.split("/")
        f_name = f_name_arr[len(f_name_arr) - 1]
        self.label = QtGui.QGraphicsSimpleTextItem(f_name, parent=self)
        self.label.setPos(10, 5)
        self.label.setBrush(QtCore.Qt.white)
        self.label.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        #self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)  #This caused problems with multiselect + moving items
        self.track_num = a_track_num
        self.mouse_y_pos = a_y_pos
        self.audio_item = a_audio_item
        self.setToolTip("Double click to open editor dialog, or click and drag to move")

    def pos_to_musical_time(self, a_pos):
        f_pos_raw = a_pos * 0.01
        f_pos_region = int(f_pos_raw)
        f_pos_bars_raw = (f_pos_raw - float(f_pos_region)) * 8.0
        f_pos_bars = int(f_pos_bars_raw)
        f_pos_beats = f_pos_bars_raw - float(f_pos_bars)
        return(f_pos_region, f_pos_bars, f_pos_beats)

    def mouseDoubleClickEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseDoubleClickEvent(self, a_event)
        this_audio_editor.show_cell_dialog(self.track_num, 0, self.audio_item)
        a_event.accept()

    def mousePressEvent(self, a_event):
        QtGui.QGraphicsRectItem.mousePressEvent(self, a_event)
        self.setGraphicsEffect(QtGui.QGraphicsOpacityEffect())

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        f_pos = self.pos().x()
        if f_pos < 0:
            f_pos = 0
        self.setPos(f_pos, self.mouse_y_pos)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        self.setGraphicsEffect(None)
        f_pos_x = self.pos().x()
        if this_audio_items_viewer.snap_enabled:
            f_pos_x *= this_audio_items_viewer.snap_divisor_recip
            f_pos_x = round(f_pos_x, 0)
            f_pos_x *= this_audio_items_viewer.snap_multiplier
            print("f_pos_x:" + str(f_pos_x))
        self.setPos(f_pos_x, self.mouse_y_pos)
        f_audio_items = this_pydaw_project.get_audio_items()
        f_item = f_audio_items.items[self.track_num]
        f_start_result = self.pos_to_musical_time(f_pos_x)
        f_item.start_region = f_start_result[0]
        f_item.start_bar = f_start_result[1]
        f_item.start_beat = f_start_result[2]
        if f_item.end_mode == 1:
            f_end_result = self.pos_to_musical_time(f_pos_x + self.rect().width())
            f_item.end_region = f_end_result[0]
            f_item.end_bar = f_end_result[1]
            f_item.end_beat = f_end_result[2]
        this_pydaw_project.save_audio_items(f_audio_items)
        this_pydaw_project.this_dssi_gui.pydaw_update_single_audio_item(self.track_num, f_item)
        self.audio_item = f_item
        this_audio_editor.open_items(False)

class audio_items_viewer(QtGui.QGraphicsView):
    def __init__(self, a_item_length=4, a_region_length=8, a_bpm=140.0):
        self.item_length = float(a_item_length)
        self.region_length = float(a_region_length)
        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(90,90,90))
        self.setScene(self.scene)
        self.audio_items = []
        self.track = 0
        self.gradient_index = 0
        self.set_bpm(a_bpm)
        self.ruler_height = 20
        self.px_per_region = 100
        self.draw_headers()
        self.setAlignment(QtCore.Qt.AlignTop)
        self.snap_enabled = False
        self.snap_multiplier = 1.0
        self.snap_divisor_recip = 1.0

    def set_snap(self, a_index):
        print("set_snap: " + str(a_index))
        if a_index == 0:
            self.snap_enabled = False
        elif a_index == 1:
            self.snap_enabled = True
            self.snap_multiplier = 100
            self.snap_divisor_recip = 0.01
        elif a_index == 2:
            self.snap_enabled = True
            self.snap_multiplier = 12.5
            self.snap_divisor_recip = 0.08

    def set_zoom(self, a_scale):
        """ a_scale == number from 1.0 to 6.0 """
        self.scale(a_scale, 1.0)

    def set_bpm(self, a_bpm):
        self.bps = a_bpm / 60.0
        self.beats_per_region = self.item_length * self.region_length
        self.regions_per_second = self.bps / self.beats_per_region

    def f_seconds_to_regions(self, a_track_seconds):
        '''converts seconds to regions'''
        return a_track_seconds * self.regions_per_second

    def draw_headers(self):
        f_total_regions = 300
        f_size = self.px_per_region * f_total_regions
        f_ruler = QtGui.QGraphicsRectItem(0, 0, f_size, self.ruler_height)
        self.scene.addItem(f_ruler)
        for i in range(0, f_total_regions):
            #f_tick = QtGui.QGraphicsLineItem(self.px_per_region*i, 0, self.px_per_region*i, self.ruler_height, f_ruler)
            f_number = QtGui.QGraphicsSimpleTextItem("%d" % i, f_ruler)
            f_number.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
            f_number.setPos(self.px_per_region*(i), 2)
            f_number.setBrush(QtCore.Qt.white)

    def draw_item_seconds(self, a_start_region, a_start_bar, a_start_beat, a_seconds, a_name, a_track_num, a_audio_item):
        f_start = (a_start_region + (((a_start_bar * self.item_length) + a_start_beat) / self.beats_per_region)) * self.px_per_region
        f_length = self.f_seconds_to_regions(a_seconds) * self.px_per_region
        self.draw_item(f_start, f_length, a_name, a_track_num, a_audio_item)

    def draw_item_musical_time(self, a_start_region, a_start_bar, a_start_beat, a_end_region, a_end_bar, a_end_beat, a_seconds, a_name, a_track_num, a_audio_item):
        f_start = (a_start_region + (((a_start_bar * self.item_length) + a_start_beat) / self.beats_per_region)) * self.px_per_region
        f_length = ((a_end_region + (((a_end_bar * self.item_length) + a_end_beat) / self.beats_per_region))  * self.px_per_region) - f_start
        f_length_seconds = self.f_seconds_to_regions(a_seconds) * self.px_per_region
        if f_length_seconds < f_length:
            f_length = f_length_seconds
        self.draw_item(f_start, f_length, a_name, a_track_num, a_audio_item)

    def clear_drawn_items(self):
        self.track = 0
        self.gradient_index = 0
        self.audio_items = []
        self.scene.clear()
        self.draw_headers()

    def draw_item(self, a_start, a_length, a_name, a_track_num, a_audio_item):
        '''a_start in seconds, a_length in seconds'''
        f_height = 65
        f_padding = 2
        f_track_num = self.ruler_height + f_padding + (f_height + f_padding) * self.track
        f_audio_item = audio_viewer_item(a_length, f_height, a_name, a_track_num, f_track_num, a_audio_item)
        self.audio_items.append(f_audio_item)
        f_audio_item.setPos(a_start, f_track_num)
        f_audio_item.setBrush(pydaw_track_gradients[self.gradient_index])
        self.gradient_index += 1
        if self.gradient_index >=  len(pydaw_track_gradients):
            self.gradient_index = 0
        self.scene.addItem(f_audio_item)
        self.track += 1

class audio_items_viewer_widget():
    def __init__(self):
        self.widget = QtGui.QWidget()
        self.vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.vlayout)
        self.controls_grid_layout = QtGui.QGridLayout()
        self.controls_grid_layout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding), 0, 30)
        self.vlayout.addLayout(self.controls_grid_layout)
        self.vlayout.addWidget(this_audio_items_viewer)
        self.snap_combobox = QtGui.QComboBox()
        self.snap_combobox.setMinimumWidth(150)
        self.snap_combobox.addItems(["None", "Region", "Bar"])
        self.controls_grid_layout.addWidget(QtGui.QLabel("Snap:"), 0, 0)
        self.controls_grid_layout.addWidget(self.snap_combobox, 0, 1)
        self.snap_combobox.currentIndexChanged.connect(self.set_snap)
        self.add_item_button = QtGui.QPushButton("Add Item")
        self.controls_grid_layout.addWidget(self.add_item_button, 0, 2)
        self.add_item_button.pressed.connect(self.add_item)
        self.h_zoom_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
        self.h_zoom_slider.setRange(0, 100)
        self.h_zoom_slider.setMaximumWidth(600)
        self.h_zoom_slider.setValue(0)
        self.last_scale_value = 0
        self.h_zoom_slider.valueChanged.connect(self.set_zoom)
        self.controls_grid_layout.addWidget(QtGui.QLabel("Zoom:"), 0, 49)
        self.controls_grid_layout.addWidget(self.h_zoom_slider, 0, 50)

    def add_item(self):
        f_audio_items = this_pydaw_project.get_audio_items()
        for i in range(pydaw_max_audio_item_count):
            if not f_audio_items.items.has_key(i):
                this_audio_editor.show_cell_dialog(i, 0)
                break
        #TODO:  QMessageBox if no available slots

    def set_snap(self, a_val=None):
        this_audio_items_viewer.set_snap(self.snap_combobox.currentIndex())

    def set_zoom(self, a_val=None):
        """ This is a ridiculously convoluted way to do this, but I see no other way in the Qt docs.  When
        Scaling, 1.0 does not return to it's original scale, and QSlider skips values when moved quickly, making
        it necessary to interpolate the inbetween values"""
        if self.last_scale_value > self.h_zoom_slider.value():
            for i in range(self.h_zoom_slider.value(), self.last_scale_value):
                this_audio_items_viewer.set_zoom(0.97)
        else:
            for i in range(self.last_scale_value, self.h_zoom_slider.value()):
                this_audio_items_viewer.set_zoom(1.03)
        self.last_scale_value = self.h_zoom_slider.value()


class audio_list_editor:
    def open_tracks(self):
        f_busses = this_pydaw_project.get_bus_tracks()
        for key, f_track in f_busses.busses.iteritems():
            self.busses[key].open_track(f_track)
        f_tracks = this_pydaw_project.get_audio_tracks()
        for key, f_track in f_tracks.tracks.iteritems():
            self.tracks[key].open_track(f_track)
        f_inputs = this_pydaw_project.get_audio_input_tracks()
        for key, f_track in f_inputs.tracks.iteritems():
            self.inputs[key].open_track(f_track)

    def open_items(self, a_update_viewer=True):
        self.audio_items = this_pydaw_project.get_audio_items()
        self.audio_items_table_widget.clearContents()

        if a_update_viewer:
            this_audio_items_viewer.clear_drawn_items()

        f_samplegraphs = this_pydaw_project.get_samplegraphs()

        for k, v in self.audio_items.items.iteritems():
            self.audio_items_table_widget.setItem(k, 0, QtGui.QTableWidgetItem(str(v.file)))
            self.audio_items_table_widget.setItem(k, 1, QtGui.QTableWidgetItem(str(float(v.sample_start) * 0.1)))
            self.audio_items_table_widget.setItem(k, 2, QtGui.QTableWidgetItem(str(float(v.sample_end) * 0.1)))
            self.audio_items_table_widget.setItem(k, 3, QtGui.QTableWidgetItem(str(v.start_region)))
            self.audio_items_table_widget.setItem(k, 4, QtGui.QTableWidgetItem(str(v.start_bar)))
            self.audio_items_table_widget.setItem(k, 5, QtGui.QTableWidgetItem(str(v.start_beat)))
            self.audio_items_table_widget.setItem(k, 6, QtGui.QTableWidgetItem(str(v.end_mode)))
            self.audio_items_table_widget.setItem(k, 7, QtGui.QTableWidgetItem(str(v.end_region)))
            self.audio_items_table_widget.setItem(k, 8, QtGui.QTableWidgetItem(str(v.end_bar)))
            self.audio_items_table_widget.setItem(k, 9, QtGui.QTableWidgetItem(str(v.end_beat)))
            self.audio_items_table_widget.setItem(k, 10, QtGui.QTableWidgetItem(str(global_timestretch_modes[v.time_stretch_mode])))
            self.audio_items_table_widget.setItem(k, 11, QtGui.QTableWidgetItem(str(v.pitch_shift)))
            self.audio_items_table_widget.setItem(k, 13, QtGui.QTableWidgetItem(str(v.output_track)))
            self.audio_items_table_widget.setItem(k, 14, QtGui.QTableWidgetItem(str(v.vol)))
            self.audio_items_table_widget.setItem(k, 12, QtGui.QTableWidgetItem(str(v.timestretch_amt)))
            if a_update_viewer:
                f_temp_seconds = f_samplegraphs.get_sample_graph(v.file).length_in_seconds
                if v.time_stretch_mode == 1:
                    f_temp_seconds /= pydaw_pitch_to_ratio(v.pitch_shift)
                elif v.time_stretch_mode == 2:
                    f_temp_seconds /= v.timestretch_amt
                if v.end_mode == 0:
                    this_audio_items_viewer.draw_item_seconds(v.start_region, v.start_bar, v.start_beat, f_temp_seconds, v.file, k, v)
                elif v.end_mode == 1:
                    this_audio_items_viewer.draw_item_musical_time(v.start_region, v.start_bar, v.start_beat, v.end_region, v.end_bar, \
                    v.end_beat, f_temp_seconds, v.file, k, v)
                else:
                    print("Invalid end mode, not drawing audio item")
        self.audio_items_table_widget.resizeColumnsToContents()

    def reset_tracks(self):
        self.tracks = []
        self.inputs = []
        self.busses = []

        for i in range(pydaw_audio_track_count):
            track = audio_track(a_track_num=i, a_track_text="track" + str(i + 1))
            self.tracks.append(track)
            self.audio_tracks_table_widget.setCellWidget(i, 0, track.group_box)
        for i in range(pydaw_audio_input_count):
            f_input = audio_input_track(i)
            self.inputs.append(f_input)
            self.audio_tracks_table_widget.setCellWidget(i, 1, f_input.group_box)
        for i in range(pydaw_bus_count):
            track = seq_track(a_track_num=i, a_track_text="Bus" + str(i), a_instrument=False)
            self.busses.append(track)
            self.audio_tracks_table_widget.setCellWidget(i, 2, track.group_box)
        self.busses[0].track_name_lineedit.setText("Master")
        self.audio_tracks_table_widget.setColumnWidth(0, 390)
        self.audio_tracks_table_widget.setColumnWidth(1, 390)
        self.audio_tracks_table_widget.setColumnWidth(2, 390)
        self.audio_tracks_table_widget.resizeRowsToContents()

    def cell_clicked(self, x, y):
        f_item = self.audio_items_table_widget.item(x, 0)
        if f_item is None or f_item.text() == "":
            self.show_cell_dialog(x, y, None)
        else:
            self.show_cell_dialog(x, y, pydaw_audio_item(self.audio_items_table_widget.item(x, 0).text(),
                int(float(self.audio_items_table_widget.item(x, 1).text()) * 10.0),
                int(float(self.audio_items_table_widget.item(x, 2).text()) * 10.0),
                self.audio_items_table_widget.item(x, 3).text(), self.audio_items_table_widget.item(x, 4).text(),
                self.audio_items_table_widget.item(x, 5).text(), self.audio_items_table_widget.item(x, 6).text(),
                self.audio_items_table_widget.item(x, 7).text(), self.audio_items_table_widget.item(x, 8).text(),
                self.audio_items_table_widget.item(x, 9).text(),
                global_timestretch_modes.index(str(self.audio_items_table_widget.item(x, 10).text())),
                self.audio_items_table_widget.item(x, 11).text(), self.audio_items_table_widget.item(x, 13).text(),
                self.audio_items_table_widget.item(x, 14).text(), self.audio_items_table_widget.item(x, 12).text()
                ))

    def show_cell_dialog(self, x, y, a_item=None):
        def ok_handler():
            if str(f_name.text()) == "":
                QtGui.QMessageBox.warning(f_window, "Error", "Name cannot be empty")
                return
            if f_end_musical_time.isChecked():
                f_start_beat_total = float((f_start_region.value() * 8 * 4) + (f_start_bar.value() * 4)) + f_start_beat.value()
                f_end_beat_total = float((f_end_region.value() * 8 * 4) + (f_end_bar.value() * 4)) + f_end_beat.value()
                if f_start_beat_total >= f_end_beat_total:
                    QtGui.QMessageBox.warning(f_window, "Error", "End point is less than or equal to start point.")
                    print("audio items:  start==" + str(f_start_beat_total) + "|" + "end==" + str(f_end_beat_total))
                    return

            if f_end_sample_length.isChecked(): f_end_mode = 0
            else: f_end_mode = 1

            f_new_item = pydaw_audio_item(f_name.text(), f_sample_start.value(), f_sample_end.value(), f_start_region.value(),
                    f_start_bar.value(), f_start_beat.value(), f_end_mode, f_end_region.value(), f_end_bar.value(), f_end_beat.value(),
                    f_timestretch_mode.currentIndex(), f_pitch_shift.value(), f_output_combobox.currentIndex(), f_sample_vol_slider.value(),
                    f_timestretch_amt.value())

            this_pydaw_project.this_dssi_gui.pydaw_load_single_audio_item(x, f_new_item)
            self.audio_items.add_item(x, f_new_item)
            this_pydaw_project.save_audio_items(self.audio_items)
            self.open_items()
            this_pydaw_project.git_repo.git_commit("-a", "Update audio items")
            f_window.close()

        def cancel_handler():
            f_window.close()

        def wait_for_samplegraph():
            global f_sg_wait_uid
            f_file_name = this_pydaw_project.samplegraph_folder + "/" + str(f_sg_wait_uid) + ".pygraph"
            if os.path.isfile(f_file_name):
                global f_sg_timer
                f_sg_timer.stop()
                global f_ai_sample_graph
                global f_sg_wait_file_name
                f_graph = f_samplegraphs.get_sample_graph(f_sg_wait_file_name)
                f_sg_wait_file_name = None
                f_sample_start_end_vlayout.removeWidget(f_ai_sample_graph)
                f_ai_sample_graph.setParent(None)
                f_ai_sample_graph.deleteLater()
                f_ai_sample_graph = None
                f_ai_sample_graph = f_graph.create_sample_graph()
                f_sample_start_end_vlayout.addWidget(f_ai_sample_graph)

        def create_sample_graph(a_file_name):
            f_graph = f_samplegraphs.get_sample_graph(a_file_name)
            global f_ai_sample_graph
            if f_graph is None:  #We must generate one and wait
                f_sample_start_end_vlayout.removeWidget(f_ai_sample_graph)
                f_ai_sample_graph.setParent(None)
                sip.delete(f_ai_sample_graph)
                #f_ai_sample_graph.deleteLater()
                f_ai_sample_graph = None
                f_ai_sample_graph = QtGui.QLabel("Generating preview...")
                f_ai_sample_graph.setMinimumHeight(300)
                f_sample_start_end_vlayout.addWidget(f_ai_sample_graph)
                global f_sg_wait_uid
                global f_sg_wait_file_name
                f_sg_wait_file_name = a_file_name
                f_sg_wait_uid = pydaw_gen_uid()
                this_pydaw_project.this_dssi_gui.pydaw_generate_sample_graph(a_file_name, f_sg_wait_uid)
                f_samplegraphs.add_ref(a_file_name, f_sg_wait_uid)
                this_pydaw_project.save_samplegraphs(f_samplegraphs)
                global f_sg_timer
                f_sg_timer.start(200)
            else:
                try:
                    f_ai_sample_graph.setParent(None)
                    sip.delete(f_ai_sample_graph)
                    f_ai_sample_graph = None
                except:
                    print("Failed:  f_sample_start_end_vlayout.removeWidget(f_ai_sample_graph)")
                f_ai_sample_graph = f_graph.create_sample_graph()
                f_sample_start_end_vlayout.addWidget(f_ai_sample_graph)

        def file_name_select():
            f_file_name = str(QtGui.QFileDialog.getOpenFileName(f_window, "Select a .wav file to open...", self.last_open_dir, filter=".wav files(*.wav)"))
            if not f_file_name is None and not f_file_name == "":
                f_name.setText(f_file_name)
                self.last_open_dir = os.path.dirname(f_file_name)
                create_sample_graph(f_file_name)

        def clear_handler():
            this_pydaw_project.this_dssi_gui.pydaw_clear_single_audio_item(x)
            self.audio_items.remove_item(x)
            this_pydaw_project.save_audio_items(self.audio_items)
            self.open_items()
            f_window.close()

        def sample_start_changed(a_val=None):
            if f_sample_end.value() < f_sample_start.value() + 10:
                f_sample_end.setValue(f_sample_start.value() + 10)

        def sample_end_changed(a_val=None):
            if f_sample_start.value() > f_sample_end.value() - 10:
                f_sample_start.setValue(f_sample_end.value() - 10)

        def sample_vol_changed(a_val=None):
            f_sample_vol_label.setText(str(f_sample_vol_slider.value()) + "dB")

        def f_quitting(a_val=None):
            try:
                global f_sg_timer
                f_sg_timer.stop()
            except:
                pass

        f_samplegraphs = this_pydaw_project.get_samplegraphs()

        global f_sg_timer
        f_sg_timer = QtCore.QTimer()
        f_sg_timer.timeout.connect(wait_for_samplegraph)

        global f_sg_wait_uid
        f_sg_wait_uid = None

        global f_sg_wait_file_name
        f_sg_wait_file_name = None

        f_window = QtGui.QDialog(this_main_window)
        f_window.finished.connect(f_quitting)
        f_window.setMinimumWidth(800)
        f_window.setWindowTitle("Add/edit an audio item..")
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

        f_sample_start_end_vlayout = QtGui.QVBoxLayout()
        f_layout.addWidget(QtGui.QLabel("Start/End:"), 1, 0)
        f_layout.addLayout(f_sample_start_end_vlayout, 1, 1)
        f_sample_start = QtGui.QSlider(QtCore.Qt.Horizontal)
        f_sample_start.setObjectName("wavleft")
        f_sample_start.valueChanged.connect(sample_start_changed)
        f_sample_start.setRange(0, 990)
        f_sample_start_end_vlayout.addWidget(f_sample_start)
        f_sample_end = QtGui.QSlider(QtCore.Qt.Horizontal)
        f_sample_end.setObjectName("wavright")
        f_sample_end.valueChanged.connect(sample_end_changed)
        f_sample_end.setRange(10, 1000)
        f_sample_end.setValue(1000)
        f_sample_start_end_vlayout.addWidget(f_sample_end)
        global f_ai_sample_graph
        if not a_item is None:
            print("Loading a_item.file : " + a_item.file)
            try:
                create_sample_graph(a_item.file)
            except:
                pass
        else:
            f_ai_sample_graph = QtGui.QLabel()
            f_ai_sample_graph.setMinimumHeight(300)
        try:
            f_sample_start_end_vlayout.addWidget(f_ai_sample_graph)
        except:
            print("Error creating sample_graph, if you are running in GUI debug-only mode this was to be expected")

        f_sample_vol_layout = QtGui.QVBoxLayout()
        f_sample_vol_slider = QtGui.QSlider(QtCore.Qt.Vertical)
        f_sample_vol_slider.setRange(-24, 24)
        f_sample_vol_slider.setValue(0)
        f_sample_vol_slider.valueChanged.connect(sample_vol_changed)
        f_sample_vol_layout.addWidget(f_sample_vol_slider)
        f_sample_vol_label = QtGui.QLabel("0db")
        f_sample_vol_layout.addWidget(f_sample_vol_label)
        f_layout.addLayout(f_sample_vol_layout, 1, 2)

        f_layout.addWidget(QtGui.QLabel("Start:"), 3, 0)
        f_start_hlayout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_start_hlayout, 3, 1)
        f_start_hlayout.addWidget(QtGui.QLabel("Region:"))
        f_start_region = QtGui.QSpinBox()
        f_start_region.setRange(0, 298)
        f_start_hlayout.addWidget(f_start_region)
        f_start_hlayout.addWidget(QtGui.QLabel("Bar:"))
        f_start_bar = QtGui.QSpinBox()
        f_start_bar.setRange(0, 7)
        f_start_hlayout.addWidget(f_start_bar)
        f_start_hlayout.addWidget(QtGui.QLabel("Beat:"))
        f_start_beat = QtGui.QDoubleSpinBox()
        f_start_beat.setRange(0, 3.99)
        f_start_hlayout.addWidget(f_start_beat)

        f_layout.addWidget(QtGui.QLabel("End:"), 5, 0)
        f_end_hlayout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_end_hlayout, 5, 1)
        f_end_sample_length = QtGui.QRadioButton("Sample Length")
        f_end_sample_length.setChecked(True)
        f_end_hlayout.addWidget(f_end_sample_length)
        f_end_musical_time = QtGui.QRadioButton("At:")
        f_end_hlayout.addWidget(f_end_musical_time)
        f_end_hlayout.addWidget(QtGui.QLabel("Region:"))
        f_end_region = QtGui.QSpinBox()
        f_end_region.setRange(0, 298)
        f_end_hlayout.addWidget(f_end_region)
        f_end_hlayout.addWidget(QtGui.QLabel("Bar:"))
        f_end_bar = QtGui.QSpinBox()
        f_end_bar.setRange(0, 7)
        f_end_bar.setValue(1)
        f_end_hlayout.addWidget(f_end_bar)
        f_end_hlayout.addWidget(QtGui.QLabel("Beats:"))
        f_end_beat = QtGui.QDoubleSpinBox()
        f_end_beat.setRange(0, 3.99)
        f_end_hlayout.addWidget(f_end_beat)

        f_layout.addWidget(QtGui.QLabel("Time Stretching:"), 7, 0)
        f_timestretch_hlayout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_timestretch_hlayout, 7, 1)
        f_timestretch_hlayout.addWidget(QtGui.QLabel("Mode:"))
        f_timestretch_mode = QtGui.QComboBox()
        f_timestretch_mode.setMinimumWidth(190)
        f_timestretch_hlayout.addWidget(f_timestretch_mode)
        f_timestretch_mode.addItems(global_timestretch_modes)
        f_timestretch_hlayout.addWidget(QtGui.QLabel("Pitch Shift:"))
        f_pitch_shift = QtGui.QDoubleSpinBox()
        f_pitch_shift.setRange(-36, 36)
        f_timestretch_hlayout.addWidget(f_pitch_shift)

        f_timestretch_hlayout.addWidget(QtGui.QLabel("Time Stretch:"))
        f_timestretch_amt = QtGui.QDoubleSpinBox()
        f_timestretch_amt.setRange(0.2, 4.0)
        f_timestretch_amt.setSingleStep(0.1)
        f_timestretch_amt.setValue(1.0)
        f_timestretch_hlayout.addWidget(f_timestretch_amt)

        f_timestretch_hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))

        f_output_hlayout = QtGui.QHBoxLayout()
        f_output_hlayout.addWidget(QtGui.QLabel("Audio Track:"))
        f_output_combobox = QtGui.QComboBox()
        f_output_combobox.setMinimumWidth(360)
        f_output_combobox.addItems(global_audio_track_names.values())
        f_output_hlayout.addWidget(f_output_combobox)
        f_output_hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))
        f_layout.addWidget(QtGui.QLabel("Output:"), 9, 0)
        f_layout.addLayout(f_output_hlayout, 9, 1)

        f_ok_layout = QtGui.QHBoxLayout()
        if not a_item is None:
            f_clear_button = QtGui.QPushButton("Clear Item")
            f_clear_button.pressed.connect(clear_handler)
            f_ok_layout.addWidget(f_clear_button)
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 11, 1)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 11, 2)

        if not a_item is None:
            f_name.setText(a_item.file)
            f_sample_start.setValue(a_item.sample_start)
            f_sample_end.setValue(a_item.sample_end)
            f_start_region.setValue(a_item.start_region)
            f_start_bar.setValue(a_item.start_bar)
            f_start_beat.setValue(a_item.start_beat)
            if a_item.end_mode == 1:
                f_end_musical_time.setChecked(True)
            f_end_region.setValue(a_item.end_region)
            f_end_bar.setValue(a_item.end_bar)
            f_end_beat.setValue(a_item.end_beat)
            f_timestretch_mode.setCurrentIndex(a_item.time_stretch_mode)
            f_pitch_shift.setValue(a_item.pitch_shift)
            f_timestretch_amt.setValue(a_item.timestretch_amt)
            f_output_combobox.setCurrentIndex(a_item.output_track)
            f_sample_vol_slider.setValue(a_item.vol)

        f_window.exec_()

    def __init__(self):
        self.enabled = False #Prevents user from editing a region before one has been selected
        self.last_open_dir = expanduser("~")
        self.group_box = QtGui.QGroupBox()
        self.main_vlayout = QtGui.QVBoxLayout()

        self.group_box.setLayout(self.main_vlayout)
        self.audio_tracks_table_widget = QtGui.QTableWidget()
        self.main_vlayout.addWidget(self.audio_tracks_table_widget)
        self.audio_tracks_table_widget.setColumnCount(3)
        self.audio_tracks_table_widget.setHorizontalHeaderLabels(["Audio Tracks", "Audio Inputs", "Track Busses"])
        self.audio_tracks_table_widget.verticalHeader().setVisible(False)
        self.audio_tracks_table_widget.setRowCount(pydaw_audio_track_count)
        self.audio_tracks_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.audio_tracks_table_widget.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.audio_tracks_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.audio_tracks_table_widget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Fixed)
        #self.table_widget.verticalHeader().setResizeMode(QtGui.QHeaderView.Fixed)

        self.items_groupbox = QtGui.QGroupBox()
        self.items_vlayout = QtGui.QVBoxLayout()
        self.items_groupbox.setLayout(self.items_vlayout)

        self.audio_items_table_widget = QtGui.QTableWidget()
        self.audio_items_table_widget.setColumnCount(15)
        self.audio_items_table_widget.setHorizontalHeaderLabels(["Path", "Sample Start", "Sample End", "Start Region", "Start Bar", "Start Beat", \
        "End Mode", "End Region", "End Bar", "End Beat", "Timestretch Mode", "Pitch", "Timestretch", "Audio Track", "Volume"])
        self.audio_items_table_widget.setRowCount(32)
        self.audio_items_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.audio_items_table_widget.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)

        self.audio_items_table_widget.cellClicked.connect(self.cell_clicked)
        self.items_vlayout.addWidget(self.audio_items_table_widget)

        self.ccs_tab = QtGui.QGroupBox()
        self.ccs_hlayout = QtGui.QHBoxLayout()
        self.ccs_tab.setLayout(self.ccs_hlayout)
        self.ccs_vlayout = QtGui.QVBoxLayout()
        self.ccs_hlayout.addLayout(self.ccs_vlayout)
        self.ccs_hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding))

        self.ccs_groupbox = QtGui.QGroupBox("CCs")
        self.ccs_groupbox.setMaximumWidth(510)
        self.ccs_groupbox.setMinimumWidth(510)
        self.ccs_gridlayout = QtGui.QGridLayout()

        self.track_type_combobox = QtGui.QComboBox()
        self.track_type_combobox.setMinimumWidth(110)
        self.track_type_combobox.currentIndexChanged.connect(self.automation_track_type_changed)
        self.track_type_combobox.addItems(["Audio", "Bus"])
        self.ccs_gridlayout.addWidget(QtGui.QLabel("Track Type:"), 0, 2)
        self.ccs_gridlayout.addWidget(self.track_type_combobox, 0, 3)

        self.track_select_combobox = QtGui.QComboBox()
        self.track_select_combobox.setMinimumWidth(240)
        self.track_select_combobox.currentIndexChanged.connect(self.automation_track_changed)
        #self.track_select_combobox.addItems(global_bus_track_names)
        self.ccs_gridlayout.addWidget(QtGui.QLabel("Track:"), 0, 4)
        self.ccs_gridlayout.addWidget(self.track_select_combobox, 0, 5)

        self.ccs_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 7, 1, 1)

        self.ccs_clear_button = QtGui.QPushButton("Clear")
        self.ccs_clear_button.setMinimumWidth(90)
        self.ccs_clear_button.pressed.connect(self.clear_ccs)
        self.ccs_gridlayout.addWidget(self.ccs_clear_button, 0, 9)

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

        self.ccs_gridlayout.addWidget(self.edit_mode_groupbox, 0, 10)

        self.ccs_vlayout.addLayout(self.ccs_gridlayout)
        self.ccs_table_widget = QtGui.QTableWidget()
        self.ccs_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.ccs_table_widget.setColumnCount(5)
        self.ccs_table_widget.setRowCount(2048)
        self.ccs_table_widget.setHorizontalHeaderLabels(["Region", "Bar", "Beat", "CC", "Value"])
        self.ccs_table_widget.cellClicked.connect(self.ccs_click_handler)
        #self.ccs_table_widget.setSortingEnabled(True)
        #self.ccs_table_widget.sortItems(0)
        self.ccs_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.ccs_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.ccs_table_widget.keyPressEvent = self.ccs_keyPressEvent
        self.ccs_vlayout.addWidget(self.ccs_table_widget)

        self.reset_tracks()

        self.default_cc_start_region = 0
        self.default_cc_start_bar = 0
        self.default_cc_start = 0.0
        self.default_cc_num = 0
        self.default_cc_value = 0
        self.default_quantize = 5

        self.ccs_clipboard = []

        self.enabled = True

    def automation_track_type_changed(self, a_val=None):
        if self.enabled:
            self.enabled = False
            self.track_select_combobox.clear()
            if self.track_type_combobox.currentIndex() == 0:
                self.track_select_combobox.addItems(global_audio_track_names.values())  #TODO:  This won't refresh with the name changes...
            elif self.track_type_combobox.currentIndex() == 1:
                self.track_select_combobox.addItems(global_bus_track_names)
            self.track_select_combobox.setCurrentIndex(0)
            self.enabled = True
            self.automation_track_changed()

    def automation_track_changed(self, a_val=None):
        if self.enabled:
            self.ccs_table_widget.clearContents()
            if self.track_type_combobox.currentIndex() == 0:
                self.item = this_pydaw_project.get_audio_automation(self.track_select_combobox.currentIndex())
            elif self.track_type_combobox.currentIndex() == 1:
                self.item = this_pydaw_project.get_bus_automation(self.track_select_combobox.currentIndex())
            else:
                print("automation_track_changed(): Invalid index")
                return
            self.ccs_table_widget.setSortingEnabled(False)
            for i in range(len(self.item.items)):
                self.ccs_table_widget.setItem(i, 0, QtGui.QTableWidgetItem(str(self.item.items[i].region)))
                self.ccs_table_widget.setItem(i, 1, QtGui.QTableWidgetItem(str(self.item.items[i].bar)))
                self.ccs_table_widget.setItem(i, 2, QtGui.QTableWidgetItem(str(self.item.items[i].beat)))
                self.ccs_table_widget.setItem(i, 3, QtGui.QTableWidgetItem(str(self.item.items[i].cc)))
                self.ccs_table_widget.setItem(i, 4, QtGui.QTableWidgetItem(str(self.item.items[i].value)))
            self.ccs_table_widget.setSortingEnabled(True)
            self.ccs_table_widget.sortItems(2)  #This creates a proper ordering by time, since Qt uses a "stable sort"
            self.ccs_table_widget.sortItems(1)
            self.ccs_table_widget.sortItems(0)

    def ccs_show_event_dialog(self, x, y):
        f_cell = self.ccs_table_widget.item(x, y)
        if f_cell is not None:
            self.default_cc_start_region = int(self.ccs_table_widget.item(x, 0).text())
            self.default_cc_start_bar = int(self.ccs_table_widget.item(x, 1).text())
            self.default_cc_start = float(self.ccs_table_widget.item(x, 2).text())
            self.default_cc_num = int(self.ccs_table_widget.item(x, 3).text())
            self.default_cc_val = int(self.ccs_table_widget.item(x, 4).text())

        def cc_ok_handler():
            f_start_rounded = time_quantize_round(f_start.value())

            if f_draw_line_checkbox.isChecked() and f_cc_value.value() != f_end_value.value():
                self.item.draw_cc_line(f_cc.value(), f_cc_value.value(), f_start_region.value(), f_start_bar.value(), \
                f_start.value(), f_end_value.value(), f_end_region.value(), f_end_bar.value(), f_end.value())
            else:
                if not self.item.add_cc(pydaw_song_level_cc(f_start_region.value(), f_start_bar.value(), f_start_rounded, f_cc.value(), f_cc_value.value())):
                    QtGui.QMessageBox.warning(f_window, "Error", "Duplicate CC event")
                    return

            self.default_cc_num = f_cc.value()
            self.default_cc_start_region = f_start_region.value()
            self.default_cc_start_bar = f_start_bar.value()
            self.default_cc_start = f_start_rounded
            self.default_cc_value = f_cc_value.value()

            if self.track_type_combobox.currentIndex() == 0:
                this_pydaw_project.save_audio_automation(self.track_select_combobox.currentIndex(), self.item)
            elif self.track_type_combobox.currentIndex() == 1:
                this_pydaw_project.save_bus_automation(self.track_select_combobox.currentIndex(), self.item)

            self.automation_track_changed() #which is essentially 'open_item' for this class...

            this_pydaw_project.git_repo.git_commit("-a", "Update song-level CCs for " + str(self.track_type_combobox.currentIndex()) + "|" + \
            str(self.track_select_combobox.currentIndex()) + "'")

            this_pydaw_project.this_dssi_gui.pydaw_reload_song_level_automation(self.get_track_type(), \
            self.track_select_combobox.currentIndex())

            if not f_add_another.isChecked():
                f_window.close()

        def cc_cancel_handler():
            f_window.close()

        def quantize_changed(f_quantize_index):
            f_frac = beat_frac_text_to_float(f_quantize_index)
            f_start.setSingleStep(f_frac)
            self.default_quantize = f_quantize_index


        def add_another_clicked(a_checked):
            if a_checked:
                f_cancel_button.setText("Close")
            else:
                f_cancel_button.setText("Cancel")

        def control_changed(a_val=None):
            f_control_str = str(f_control_combobox.currentText())
            if f_control_str != '':
                f_value = int(global_cc_maps["Modulex"][f_control_str])
                f_cc.setValue(f_value)

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("CCs")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.currentIndexChanged.connect(quantize_changed)
        f_layout.addWidget(QtGui.QLabel("Quantize(beats)"), 0, 0)
        f_layout.addWidget(f_quantize_combobox, 0, 1)

        f_control_combobox = QtGui.QComboBox()
        f_layout.addWidget(QtGui.QLabel("Control"), 3, 0)
        f_layout.addWidget(f_control_combobox, 3, 1)

        f_control_combobox.addItems(global_cc_maps["Modulex"].keys())
        f_control_combobox.currentIndexChanged.connect(control_changed)

        f_cc = QtGui.QSpinBox()
        f_cc.setRange(1, 127)
        f_cc.setValue(self.default_cc_num)
        f_layout.addWidget(QtGui.QLabel("CC"), 4, 0)
        f_layout.addWidget(f_cc, 4, 1)
        f_cc_value = QtGui.QSpinBox()
        f_cc_value.setRange(0, 127)
        f_cc_value.setValue(self.default_cc_value)
        f_layout.addWidget(QtGui.QLabel("Value"), 5, 0)
        f_layout.addWidget(f_cc_value, 5, 1)

        f_layout.addWidget(QtGui.QLabel("Start(Region)"), 6, 0)
        f_start_region = QtGui.QSpinBox()
        f_start_region.setRange(0, 299)
        f_layout.addWidget(f_start_region, 6, 1)
        f_start_region.setValue(self.default_cc_start_region)

        f_layout.addWidget(QtGui.QLabel("Start(bar)"), 7, 0)
        f_start_bar = QtGui.QSpinBox()
        f_start_bar.setRange(0, 15)
        f_layout.addWidget(f_start_bar, 7, 1)
        f_start_bar.setValue(self.default_cc_start_bar)

        f_layout.addWidget(QtGui.QLabel("Start(beats)"), 8, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_layout.addWidget(f_start, 8, 1)
        f_start.setValue(self.default_cc_start)

        f_draw_line_checkbox = QtGui.QCheckBox("Draw line")
        f_layout.addWidget(f_draw_line_checkbox, 10, 1)
        f_layout.addWidget(QtGui.QLabel("End Value"), 11, 0)
        f_end_value = QtGui.QSpinBox()
        f_end_value.setRange(0, 127)
        f_layout.addWidget(f_end_value, 11, 1)

        f_layout.addWidget(QtGui.QLabel("End(region)"), 12, 0)
        f_end_region = QtGui.QSpinBox()
        f_end_region.setRange(0, 299)
        f_layout.addWidget(f_end_region, 12, 1)

        f_layout.addWidget(QtGui.QLabel("End(bar)"), 13, 0)
        f_end_bar = QtGui.QSpinBox()
        f_end_bar.setRange(0, 15)
        f_layout.addWidget(f_end_bar, 13, 1)

        f_layout.addWidget(QtGui.QLabel("End(beats)"), 15, 0)
        f_end = QtGui.QDoubleSpinBox()
        f_end.setRange(0, 3.99)
        f_layout.addWidget(f_end, 15, 1)

        f_add_another = QtGui.QCheckBox("Add another?")
        f_add_another.toggled.connect(add_another_clicked)
        f_layout.addWidget(f_add_another, 18, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 19, 0)
        f_ok_button.clicked.connect(cc_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 19, 1)
        f_cancel_button.clicked.connect(cc_cancel_handler)
        f_quantize_combobox.setCurrentIndex(self.default_quantize)
        f_window.exec_()

    def get_ccs_table_selected_rows(self):
        f_result = []
        for i in range(0, self.ccs_table_widget.rowCount()):
            f_item = self.ccs_table_widget.item(i, 0)
            if not f_item is None and f_item.isSelected():
                f_result.append(pydaw_song_level_cc(self.ccs_table_widget.item(i, 0).text(), self.ccs_table_widget.item(i, 1).text(), \
                self.ccs_table_widget.item(i, 2).text(), self.ccs_table_widget.item(i, 3).text(), self.ccs_table_widget.item(i, 4).text()))
        return f_result

    def ccs_click_handler(self, x, y):
        if not self.enabled:
            return
        if self.add_radiobutton.isChecked():
            self.ccs_show_event_dialog(x, y)
        elif self.delete_radiobutton.isChecked():
            if self.ccs_table_widget.item(x, 0) is None:
                return
            self.item.remove_cc(pydaw_song_level_cc(self.ccs_table_widget.item(x, 0).text(), self.ccs_table_widget.item(x, 1).text(), \
            self.ccs_table_widget.item(x, 2).text(), self.ccs_table_widget.item(x, 3).text(), self.ccs_table_widget.item(x, 4).text()))
            self.save_and_load("Deleted CC for " + str(self.track_type_combobox.currentIndex()) + "|" + \
            str(self.track_select_combobox.currentIndex()) + "'")

    def get_track_type(self):
        """ Return the PyDAW track type int value from the track type combobox index """
        f_index = self.track_type_combobox.currentIndex()
        if f_index == 0:
            return 2
        elif f_index == 1:
            return 1
        else:
            assert(False)

    def save_and_load(self, a_message):
        """In the interest of DRY principles, consolidate saving and loading to a function, since this class has so many steps """
        if self.track_type_combobox.currentIndex() == 0:
            this_pydaw_project.save_audio_automation(self.track_select_combobox.currentIndex(), self.item)
        elif self.track_type_combobox.currentIndex() == 1:
            this_pydaw_project.save_bus_automation(self.track_select_combobox.currentIndex(), self.item)
        self.automation_track_changed() #which is essentially 'open_item' for this class...
        this_pydaw_project.git_repo.git_commit("-a", a_message)
        this_pydaw_project.this_dssi_gui.pydaw_reload_song_level_automation(self.get_track_type(), \
        self.track_select_combobox.currentIndex())

    def ccs_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_ccs = self.get_ccs_table_selected_rows()
                for f_cc in f_ccs:
                    self.item.remove_cc(f_cc)
            self.save_and_load("Deleted CC for " + str(self.track_type_combobox.currentIndex()) + "|" + \
            str(self.track_select_combobox.currentIndex()) + "'")
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_cc in self.ccs_clipboard:
                self.item.add_cc(f_cc)
            self.save_and_load("Pasted CCs into " + str(self.track_type_combobox.currentIndex()) + "|" + \
            str(self.track_select_combobox.currentIndex()) + "'")
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
            for f_cc in self.ccs_clipboard:
                self.item.remove_cc(f_cc)
            self.save_and_load("Cut CCs from " + str(self.track_type_combobox.currentIndex()) + "|" + \
            str(self.track_select_combobox.currentIndex()) + "'")
        else:
            QtGui.QTableWidget.keyPressEvent(self.ccs_table_widget, event)

    def clear_ccs(self):
        if self.enabled:
            self.item.items = []
            self.save_and_load("Deleted CC for " + str(self.track_type_combobox.currentIndex()) + "|" + \
            str(self.track_select_combobox.currentIndex()) + "'")

class audio_track:
    def on_vol_change(self, value):
        self.volume_label.setText(str(value) + " dB")
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_vol(self.track_number, self.volume_slider.value(), 2)
    def on_vol_released(self):
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].vol = self.volume_slider.value()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.git_repo.git_commit("-a", "Set audio track " + str(self.track_number) + " to " + str(self.volume_slider.value()))
    def on_solo(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_solo(self.track_number, self.solo_checkbox.isChecked(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].solo = self.solo_checkbox.isChecked()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.git_repo.git_commit("-a", "Set audio track " + str(self.track_number) + " soloed to " + str(self.solo_checkbox.isChecked()))
    def on_mute(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_mute(self.track_number, self.mute_checkbox.isChecked(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].mute = self.mute_checkbox.isChecked()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.git_repo.git_commit("-a", "Set audio track " + str(self.track_number) + " muted to " + str(self.mute_checkbox.isChecked()))
    def on_name_changed(self):
        self.track_name_lineedit.setText(pydaw_remove_bad_chars(self.track_name_lineedit.text()))
        this_pydaw_project.this_dssi_gui.pydaw_save_track_name(self.track_number, self.track_name_lineedit.text(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].name = str(self.track_name_lineedit.text())
        this_pydaw_project.save_audio_tracks(f_tracks)
        global_update_audio_track_comboboxes(self.track_number, self.track_name_lineedit.text())
        this_pydaw_project.git_repo.git_commit("-a", "Set audio track " + str(self.track_number) + " name to " + str(self.track_name_lineedit.text()))
    def on_show_fx(self):
        this_pydaw_project.this_dssi_gui.pydaw_show_fx(self.track_number, 2)
    def on_bus_changed(self, a_value=0):
        this_pydaw_project.this_dssi_gui.pydaw_set_bus(self.track_number, self.bus_combobox.currentIndex(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].bus_num = self.bus_combobox.currentIndex()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.git_repo.git_commit("-a", "Set audio track " + str(self.track_number) + " bus to " + str(self.track_name_lineedit.text()))

    def __init__(self, a_track_num, a_track_text="track"):
        self.suppress_osc = True
        self.track_number = a_track_num
        self.group_box = QtGui.QWidget()
        self.group_box.setAutoFillBackground(True)
        self.group_box.setPalette(QtGui.QPalette(QtCore.Qt.black))
        self.group_box.setMinimumWidth(330)
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
        self.hlayout3.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        self.fx_button = QtGui.QPushButton("FX")
        self.fx_button.pressed.connect(self.on_show_fx)
        self.fx_button.setObjectName("fxbutton")
        self.fx_button.setMinimumWidth(24)
        self.fx_button.setMaximumWidth(24)
        self.bus_combobox = QtGui.QComboBox()
        self.bus_combobox.addItems(['M', '1','2','3','4'])
        self.bus_combobox.setMinimumWidth(54)
        self.bus_combobox.currentIndexChanged.connect(self.on_bus_changed)
        self.hlayout3.addWidget(QtGui.QLabel("Bus:"))
        self.hlayout3.addWidget(self.bus_combobox)
        self.hlayout3.addWidget(self.fx_button)
        self.solo_checkbox = QtGui.QCheckBox()
        self.solo_checkbox.clicked.connect(self.on_solo)
        self.solo_checkbox.setObjectName("solo_checkbox")
        self.hlayout3.addWidget(self.solo_checkbox)
        self.mute_checkbox = QtGui.QCheckBox()
        self.mute_checkbox.clicked.connect(self.on_mute)
        self.mute_checkbox.setObjectName("mute_checkbox")
        self.hlayout3.addWidget(self.mute_checkbox)
        self.hlayout3.addWidget(self.fx_button)
        self.suppress_osc = False

    def open_track(self, a_track, a_notify_osc=False):
        if not a_notify_osc:
            self.suppress_osc = True
        self.track_name_lineedit.setText(a_track.name)
        global_update_audio_track_comboboxes(self.track_number, a_track.name)
        self.volume_slider.setValue(a_track.vol)
        self.solo_checkbox.setChecked(a_track.solo)
        self.mute_checkbox.setChecked(a_track.mute)
        self.bus_combobox.setCurrentIndex(a_track.bus_num)
        self.suppress_osc = False

    def get_track(self):
        return pydaw_audio_track(self.solo_checkbox.isChecked(), self.mute_checkbox.isChecked(), self.volume_slider.value(), str(self.track_name_lineedit.text()), self.bus_combobox.currentIndex())



class audio_input_track:
    def on_vol_change(self, value):
        self.volume_label.setText(str(value) + " dB")
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_vol(self.track_number, self.volume_slider.value(), 3)
    def on_vol_released(self):
        f_tracks = this_pydaw_project.get_audio_input_tracks()
        f_tracks.tracks[self.track_number].vol = self.volume_slider.value()
        this_pydaw_project.save_audio_inputs(f_tracks)
        this_pydaw_project.git_repo.git_commit("-a", "Set audio input " + str(self.track_number) + " volume to " + str(self.volume_slider.value()))
    def on_rec(self, value):
        if not self.suppress_osc:
            f_tracks = this_pydaw_project.get_audio_input_tracks()
            f_tracks.tracks[self.track_number].rec = self.rec_checkbox.isChecked()
            this_pydaw_project.save_audio_inputs(f_tracks)
            this_pydaw_project.this_dssi_gui.pydaw_update_audio_inputs()
            this_pydaw_project.git_repo.git_commit("-a", "Set audio input " + str(self.track_number) + " record to " + str(self.rec_checkbox.isChecked()))
    def on_output_changed(self, a_value=0):
        if not global_suppress_audio_track_combobox_changes and not self.suppress_osc:
            f_tracks = this_pydaw_project.get_audio_input_tracks()
            f_tracks.tracks[self.track_number].output = self.output_combobox.currentIndex()
            this_pydaw_project.save_audio_inputs(f_tracks)
            this_pydaw_project.this_dssi_gui.pydaw_update_audio_inputs()
            this_pydaw_project.git_repo.git_commit("-a", "Set audio input " + str(self.track_number) + " output to " + str(self.output_combobox.currentIndex()))

    def __init__(self, a_track_num):
        self.suppress_osc = True
        self.track_number = a_track_num
        self.group_box = QtGui.QWidget()
        self.group_box.setAutoFillBackground(True)
        self.group_box.setPalette(QtGui.QPalette(QtCore.Qt.black))
        #self.group_box.setMinimumHeight(90)
        self.group_box.setMinimumWidth(330)
        #self.group_box.setObjectName("seqtrack")
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
        #self.hlayout3.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        self.output_combobox = QtGui.QComboBox()
        self.output_combobox.addItems(global_audio_track_names.values())
        self.output_combobox.setMinimumWidth(150)
        self.output_combobox.currentIndexChanged.connect(self.on_output_changed)
        self.hlayout3.addWidget(QtGui.QLabel("Out:"))
        self.hlayout3.addWidget(self.output_combobox)

        global_audio_track_comboboxes.append(self.output_combobox)

        self.hlayout3.addItem(QtGui.QSpacerItem(10,10,QtGui.QSizePolicy.Expanding))
        self.rec_checkbox = QtGui.QCheckBox()
        self.rec_checkbox.clicked.connect(self.on_rec)
        self.rec_checkbox.setObjectName("rec_arm_checkbox")
        self.hlayout3.addWidget(self.rec_checkbox)
        self.suppress_osc = False

    def open_track(self, a_track, a_notify_osc=False):
        if not a_notify_osc:
            self.suppress_osc = True
        self.volume_slider.setValue(a_track.vol)
        self.rec_checkbox.setChecked(a_track.rec)
        self.output_combobox.setCurrentIndex(a_track.output)
        self.suppress_osc = False


global_piano_roll_snap = False
global_piano_roll_grid_width = 1000.0
global_piano_keys_width = 34  #Width of the piano keys in px
global_piano_roll_grid_max_start_time = 999.0 + global_piano_keys_width
global_piano_roll_note_height = 15
global_piano_roll_snap_divisor = 16.0
global_piano_roll_snap_value = global_piano_roll_grid_width / global_piano_roll_snap_divisor
global_piano_roll_snap_divisor_beats = global_piano_roll_snap_divisor / 4.0
global_piano_roll_note_count = 120
global_piano_roll_header_height = 20
global_piano_roll_total_height = 1000  #gets updated by the piano roll to it's real value

pydaw_note_gradient = QtGui.QLinearGradient(QtCore.QPointF(0, 0), QtCore.QPointF(0, 12))
pydaw_note_gradient.setColorAt(0, QtGui.QColor(163, 136, 30))
pydaw_note_gradient.setColorAt(1, QtGui.QColor(230, 221, 45))

global_selected_piano_note = None   #Used for mouse click hackery
global_selected_piano_note_pos = None

def pydaw_set_piano_roll_quantize(a_index):
    global global_piano_roll_snap
    global global_piano_roll_snap_value
    global global_piano_roll_snap_divisor
    global global_piano_roll_snap_divisor_beats
    if a_index == 0:
        global_piano_roll_snap = False
    elif a_index == 1:
        global_piano_roll_snap_divisor = 16.0
        global_piano_roll_snap = True
    elif a_index == 2:
        global_piano_roll_snap_divisor =  12.0
        global_piano_roll_snap = True
    elif a_index == 3:
        global_piano_roll_snap_divisor =  8.0
        global_piano_roll_snap = True
    elif a_index == 4:
        global_piano_roll_snap_divisor =  4.0
        global_piano_roll_snap = True
    global_piano_roll_snap_value = (global_piano_roll_grid_width * this_piano_roll_editor.item_count) / global_piano_roll_snap_divisor
    global_piano_roll_snap_divisor_beats = global_piano_roll_snap_divisor / (4.0 * this_piano_roll_editor.item_count)

class piano_roll_note_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_length, a_note_height, a_note, a_note_item, a_item_index):
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, a_length, a_note_height)
        self.item_index = a_item_index
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setBrush(pydaw_note_gradient)
        self.note_height = a_note_height
        self.setToolTip("Double-click to edit note properties, click and drag to move,\nclick and drag the end to change length, and Shift+click to delete.\nYou can marquee-select multiple items by holding down CTRL, then clicking and dragging.")
        self.note_item = a_note_item
        self.setAcceptHoverEvents(True)
        self.resize_start_pos = 0.0
        if global_selected_piano_note is not None and a_note_item == global_selected_piano_note:
            self.is_resizing = True
            self.resize_last_mouse_pos = global_selected_piano_note_pos.x()
            self.resize_pos = global_selected_piano_note_pos
            self.resize_start_pos = self.note_item.start
        else:
            self.is_resizing = False
        self.resize_rect = self.rect()
        self.setPen(QtGui.QPen(pydaw_track_gradients[3], 2))
        self.mouse_y_pos = QtGui.QCursor.pos().y()

    def mouse_is_at_end(self, a_pos):
        return (a_pos.x() > (self.rect().width() * 0.8))

    def hoverMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverMoveEvent(self, a_event)
        if not self.is_resizing:
            this_piano_roll_editor.click_enabled = False
            if self.mouse_is_at_end(a_event.pos()):
                QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.SizeHorCursor))
            else:
                QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.ArrowCursor))

    def hoverEnterEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverEnterEvent(self, a_event)
        this_piano_roll_editor.click_enabled = False

    def hoverLeaveEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverLeaveEvent(self, a_event)
        this_piano_roll_editor.click_enabled = True
        QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.ArrowCursor))

    def mouseDoubleClickEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseDoubleClickEvent(self, a_event)
        this_item_editor.notes_show_event_dialog(None, None, self.note_item)

    def mousePressEvent(self, a_event):
        QtGui.QGraphicsRectItem.mousePressEvent(self, a_event)
        if a_event.modifiers() == QtCore.Qt.ShiftModifier:
            this_item_editor.item.remove_note(self.note_item)
            this_item_editor.save_and_reload()
            QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.ArrowCursor))
        else:
            self.o_brush = self.brush()
            self.setBrush(QtGui.QColor(255,200,100))
            self.o_pos = self.pos()
            if self.mouse_is_at_end(a_event.pos()):
                self.is_resizing = True
                self.mouse_y_pos = QtGui.QCursor.pos().y()
                self.resize_last_mouse_pos = a_event.pos().x()
                for f_item in this_piano_roll_editor.note_items:
                    if f_item.isSelected():
                        f_item.resize_start_pos = f_item.note_item.start + (4.0 * f_item.item_index)
                        f_item.resize_pos = f_item.pos()
                        f_item.resize_rect = f_item.rect()
        this_piano_roll_editor.click_enabled = True

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        if self.is_resizing:
            f_adjusted_width_diff = (a_event.pos().x() - self.resize_last_mouse_pos)
            self.resize_last_mouse_pos = a_event.pos().x()
        for f_item in this_piano_roll_editor.note_items:
            if f_item.isSelected():
                f_pos_x = f_item.pos().x()
                f_pos_y = f_item.pos().y()
                if self.is_resizing:
                    f_adjusted_width = f_item.resize_rect.width() + f_adjusted_width_diff
                    if f_adjusted_width < 12.0:
                        f_adjusted_width = 12.0
                    f_item.resize_rect.setWidth(f_adjusted_width)
                    f_item.setRect(f_item.resize_rect)
                    f_pos_y = (int((f_pos_y - global_piano_roll_header_height)/self.note_height) * self.note_height) + global_piano_roll_header_height
                    f_item.setPos(f_item.resize_pos.x(), f_pos_y)
                    QtGui.QCursor.setPos(QtGui.QCursor.pos().x(), self.mouse_y_pos)
                else:
                    if f_pos_x < global_piano_keys_width:
                        f_pos_x = global_piano_keys_width
                    elif f_pos_x > global_piano_roll_grid_max_start_time:
                        f_pos_x = global_piano_roll_grid_max_start_time
                    if f_pos_y < global_piano_roll_header_height:
                        f_pos_y = global_piano_roll_header_height
                    elif f_pos_y > global_piano_roll_total_height:
                        f_pos_y = global_piano_roll_total_height
                    f_pos_y = (int((f_pos_y - global_piano_roll_header_height)/self.note_height) * self.note_height) + global_piano_roll_header_height
                    if global_piano_roll_snap:
                        f_pos_x = (int((f_pos_x - global_piano_keys_width)/global_piano_roll_snap_value) * global_piano_roll_snap_value) + global_piano_keys_width
                    f_item.setPos(f_pos_x, f_pos_y)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        for f_item in this_piano_roll_editor.note_items:
            if f_item.isSelected():
                f_item.setBrush(self.o_brush)
                f_pos_x = f_item.pos().x()
                f_pos_y = f_item.pos().y()
                if self.is_resizing:
                    if global_piano_roll_snap:
                        f_adjusted_width = (round(f_item.resize_rect.width()/global_piano_roll_snap_value) * global_piano_roll_snap_value)
                        if f_adjusted_width == 0.0:
                            f_adjusted_width = global_piano_roll_snap_value
                        f_item.resize_rect.setWidth(f_adjusted_width)
                        f_item.setRect(f_item.resize_rect)
                    f_new_note_length = ((f_pos_x + f_item.rect().width() - global_piano_keys_width) * 0.001 * 4.0) - f_item.resize_start_pos #float(this_piano_roll_editor.item_count)
                    f_new_note_length = round(f_new_note_length * global_piano_roll_snap_divisor_beats) / global_piano_roll_snap_divisor_beats
                    if f_new_note_length < pydaw_min_note_length:
                        f_new_note_length = pydaw_min_note_length
                    f_item.note_item.set_length(f_new_note_length)
                else:
                    this_item_editor.items[self.item_index].notes.remove(self.note_item)
                    f_new_note_start = (f_pos_x - global_piano_keys_width) * 4.0 * 0.001 #* float(this_piano_roll_editor.item_count)
                    print(str(f_new_note_start))
                    self.item_index, f_new_note_start = pydaw_beats_to_index(f_new_note_start)
                    f_new_note_num = int(global_piano_roll_note_count - ((f_pos_y - global_piano_roll_header_height) / global_piano_roll_note_height))
                    f_item.note_item.set_start(f_new_note_start)
                    f_item.note_item.note_num = f_new_note_num
                    this_item_editor.items[self.item_index].notes.append(self.note_item)
                    this_item_editor.items[self.item_index].notes.sort()
                    print(str(self.item_index))
        this_item_editor.items[self.item_index].fix_overlaps()
        this_item_editor.save_and_reload()
        self.is_resizing = False
        QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.ArrowCursor))
        this_piano_roll_editor.click_enabled = True

class piano_key_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_piano_width, a_note_height, a_parent):
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, a_piano_width, a_note_height, a_parent)
        self.setAcceptHoverEvents(True)
        self.hover_brush = QtGui.QColor(200,200,200)

    def hoverEnterEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverEnterEvent(self, a_event)
        self.o_brush = self.brush()
        self.setBrush(self.hover_brush)

    def hoverLeaveEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverLeaveEvent(self, a_event)
        self.setBrush(self.o_brush)

class piano_roll_editor(QtGui.QGraphicsView):
    def __init__(self, a_item_length=4, a_grid_div=16):
        self.item_length = float(a_item_length)
        self.viewer_width = 1000
        self.item_count = 1
        self.grid_div = a_grid_div

        self.end_octave = 8
        self.start_octave = -2
        self.notes_in_octave = 12
        self.total_notes = global_piano_roll_note_count #(self.end_octave - self.start_octave) * self.notes_in_octave + 1 #for C8
        self.note_height = global_piano_roll_note_height
        self.octave_height = self.notes_in_octave * self.note_height

        self.header_height = global_piano_roll_header_height

        self.piano_height = self.note_height*self.total_notes
        self.piano_width = 32
        self.padding = 2
        self.piano_height = self.note_height * self.total_notes
        global global_piano_roll_total_height
        global_piano_roll_total_height = self.piano_height + global_piano_roll_header_height

        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(100,100,100))
        self.scene.mousePressEvent = self.sceneMousePressEvent
        self.setAlignment(QtCore.Qt.AlignLeft)
        self.setScene(self.scene)
        self.draw_header()
        self.draw_piano()
        self.draw_grid()

        self.setDragMode(QtGui.QGraphicsView.RubberBandDrag)
        self.note_items = []

        self.right_click = False
        self.left_click = False
        self.click_enabled = True

    def keyPressEvent(self, a_event):
        QtGui.QGraphicsView.keyPressEvent(self, a_event)
        if a_event.key() == QtCore.Qt.Key_Delete:
            for f_item in self.note_items:
                if f_item.isSelected():
                    this_item_editor.items[f_item.item_index].remove_note(f_item.note_item)
        this_item_editor.save_and_reload()


    def sceneMousePressEvent(self, a_event):
        if not this_item_editor.enabled:
            this_item_editor.show_not_enabled_warning()
            QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)
            return
        if a_event.modifiers() == QtCore.Qt.ControlModifier:
            a_event.setAccepted(True)
            QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)
        elif self.click_enabled and this_item_editor.enabled:
            f_pos_x = a_event.scenePos().x()
            f_pos_y = a_event.scenePos().y()
            if f_pos_x > global_piano_keys_width and f_pos_x < global_piano_roll_grid_max_start_time and \
            f_pos_y > global_piano_roll_header_height and f_pos_y < global_piano_roll_total_height:
                f_note = int(self.total_notes - ((f_pos_y - global_piano_roll_header_height) / self.note_height)) + 1
                if global_piano_roll_snap:
                    f_beat = (int((f_pos_x - global_piano_keys_width)/global_piano_roll_snap_value) * global_piano_roll_snap_value) * 0.001 * 4.0
                else:
                    f_beat = (f_pos_x - global_piano_keys_width) * 0.001 * 4.0
                f_note_item = pydaw_note(f_beat, 0.25, f_note, 100)
                this_item_editor.add_note(f_note_item)
                global global_selected_piano_note
                global_selected_piano_note = f_note_item
                global global_selected_piano_note_pos
                global_selected_piano_note_pos = a_event.scenePos()
                this_item_editor.save_and_reload()

            QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)
        else:
            QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)

    def draw_header(self):
        self.header = QtGui.QGraphicsRectItem(0, 0, self.viewer_width, self.header_height)
        self.header.setPos(self.piano_width + self.padding, 0)
        self.scene.addItem(self.header)
        self.beat_width = self.viewer_width / self.item_length
        self.value_width = self.beat_width / self.grid_div

    def draw_piano(self):
        f_labels = ['B', 'Bb', 'A', 'Ab', 'G', 'Gb', 'F', 'E', 'Eb', 'D', 'Db', 'C']
        f_black_notes = [2, 4, 6, 9, 11]
        f_piano_label = QtGui.QFont()
        f_piano_label.setPointSize(8)
        self.piano = QtGui.QGraphicsRectItem(0, 0, self.piano_width, self.piano_height)
        self.piano.setPos(0, self.header_height)
        self.scene.addItem(self.piano)
        f_key = piano_key_item(self.piano_width, self.note_height, self.piano)
        f_label = QtGui.QGraphicsSimpleTextItem("C8", f_key)
        f_label.setPos(4, 0)
        f_label.setFont(f_piano_label)
        f_key.setBrush(QtGui.QColor(255,255,255))
        for i in range(self.end_octave-self.start_octave, self.start_octave-self.start_octave, -1):
            for j in range(self.notes_in_octave, 0, -1):
                f_key = piano_key_item(self.piano_width, self.note_height, self.piano)
                f_key.setPos(0, self.note_height * (j) + self.octave_height*(i-1))
                if j == 12:
                    f_label = QtGui.QGraphicsSimpleTextItem("%s%d" % (f_labels[j-1], self.end_octave-i), f_key)
                    f_label.setPos(4, 0)
                    f_label.setFont(f_piano_label)
                if j in f_black_notes:
                    f_key.setBrush(QtGui.QColor(0,0,0))
                else:
                    f_key.setBrush(QtGui.QColor(255,255,255))

    def draw_grid(self):
        for i in range(self.end_octave-self.start_octave, self.start_octave-self.start_octave, -1):
            for j in range(self.notes_in_octave, 0, -1):
                f_note_bar = QtGui.QGraphicsRectItem(0, 0, self.viewer_width, self.note_height, self.piano)
                f_note_bar.setPos(self.piano_width + self.padding,  self.note_height*(j) + self.octave_height*(i-1))
                #basic implementation of zth's "show scales", there are a few transparency issues
                #if j not in f_black_notes:
                #    f_note_bar.setBrush(QtGui.QColor(230,230,230,100))
        f_beat_pen = QtGui.QPen()
        f_beat_pen.setWidth(2)
        f_line_pen = QtGui.QPen()
        f_line_pen.setColor(QtGui.QColor(0,0,0,40))
        for i in range(0, int(self.item_length) + 1):
            f_beat = QtGui.QGraphicsLineItem(0, 0, 0, self.piano_height+self.header_height-f_beat_pen.width(), self.header)
            f_beat.setPos(self.beat_width * i, 0.5 * f_beat_pen.width())
            f_beat.setPen(f_beat_pen)
            if i < self.item_length:
                f_number = QtGui.QGraphicsSimpleTextItem(str(i), self.header)
                f_number.setPos(self.beat_width * i + 5, 2)
                f_number.setBrush(QtCore.Qt.white)
                for j in range(0, self.grid_div):
                    f_line = QtGui.QGraphicsLineItem(0, 0, 0, self.piano_height, self.header)
                    if float(j) == self.grid_div / 2.0:
                        f_line.setLine(0, 0, 0, self.piano_height)
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.header_height)
                    else:
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.header_height)
                        f_line.setPen(f_line_pen)

    def set_zoom(self, a_scale):
        self.scale(a_scale, 1.0)

    def clear_drawn_items(self):
        self.note_items = []
        self.scene.clear()
        self.draw_header()
        self.draw_piano()
        self.draw_grid()

    def draw_item(self):
        """ Draw all notes in an instance of the pydaw_item class"""
        self.item_count = len(this_item_editor.items)
        self.viewer_width = 1000 * self.item_count
        self.item_length = float(4 * self.item_count)
        pydaw_set_piano_roll_quantize(this_piano_roll_editor_widget.snap_combobox.currentIndex())
        global global_piano_roll_grid_max_start_time
        global_piano_roll_grid_max_start_time = (999.0 * self.item_count) + global_piano_keys_width
        self.clear_drawn_items()
        if not this_item_editor.enabled:
            return
        f_beat_offset = 0
        for f_item in this_item_editor.items:
            for f_note in f_item.notes:
                self.draw_note(f_note, f_beat_offset)
            f_beat_offset += 1

    def draw_note(self, a_note, a_item_index):
        """ a_note is an instance of the pydaw_note class"""
        f_start = self.piano_width + self.padding + self.beat_width * (a_note.start + (float(a_item_index) * 4.0))
        f_length = self.beat_width * a_note.length
        f_note = self.header_height + self.note_height * (self.total_notes - a_note.note_num)
        f_note_item = piano_roll_note_item(f_length, self.note_height, a_note.note_num, a_note, a_item_index)
        f_note_item.setPos(f_start, f_note)
        f_vel_opacity = QtGui.QGraphicsOpacityEffect()
        f_vel_opacity.setOpacity((a_note.velocity * 0.007874016 * 0.6) + 0.4)
        f_note_item.setGraphicsEffect(f_vel_opacity)
        self.scene.addItem(f_note_item)
        self.note_items.append(f_note_item)

class piano_roll_editor_widget():
    def quantize_dialog(self):
        this_item_editor.quantize_dialog()
    def transpose_dialog(self):
        this_item_editor.transpose_dialog()
    def time_shift_dialog(self):
        this_item_editor.time_shift_dialog()
    def length_shift_dialog(self):
        this_item_editor.length_shift_dialog()
    def velocity_dialog(self):
        this_item_editor.velocity_dialog()
    def clear_notes(self):
        this_item_editor.clear_notes()

    def __init__(self):
        self.widget = QtGui.QWidget()
        self.vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.vlayout)

        self.controls_grid_layout = QtGui.QGridLayout()
        self.controls_grid_layout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding), 0, 30)

        f_button_width = 82
        self.notes_quantize_button = QtGui.QPushButton("Quantize")
        self.notes_quantize_button.setMinimumWidth(f_button_width)
        self.notes_quantize_button.pressed.connect(self.quantize_dialog)
        self.controls_grid_layout.addWidget(self.notes_quantize_button, 0, 10)
        self.notes_transpose_button = QtGui.QPushButton("Transpose")
        self.notes_transpose_button.setMinimumWidth(f_button_width)
        self.notes_transpose_button.pressed.connect(self.transpose_dialog)
        self.controls_grid_layout.addWidget(self.notes_transpose_button, 0, 12)
        self.notes_shift_button = QtGui.QPushButton("Shift")
        self.notes_shift_button.setMinimumWidth(f_button_width)
        self.notes_shift_button.pressed.connect(self.time_shift_dialog)
        self.controls_grid_layout.addWidget(self.notes_shift_button, 0, 13)
        self.notes_length_button = QtGui.QPushButton("Length")
        self.notes_length_button.setMinimumWidth(f_button_width)
        self.notes_length_button.pressed.connect(self.length_shift_dialog)
        self.controls_grid_layout.addWidget(self.notes_length_button, 0, 14)

        self.notes_velocity_button = QtGui.QPushButton("Velocity")
        self.notes_velocity_button.setMinimumWidth(f_button_width)
        self.notes_velocity_button.pressed.connect(self.velocity_dialog)
        self.controls_grid_layout.addWidget(self.notes_velocity_button, 0, 15)
        self.controls_grid_layout.addItem(QtGui.QSpacerItem(200, 10, QtGui.QSizePolicy.Minimum), 0, 16)
        self.notes_clear_button = QtGui.QPushButton("Clear")
        self.notes_clear_button.setMinimumWidth(f_button_width)
        self.notes_clear_button.pressed.connect(self.clear_notes)
        self.controls_grid_layout.addWidget(self.notes_clear_button, 0, 17)
        self.controls_grid_layout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding), 0, 18)

        self.vlayout.addLayout(self.controls_grid_layout)
        self.vlayout.addWidget(this_piano_roll_editor)
        self.snap_combobox = QtGui.QComboBox()
        self.snap_combobox.setMinimumWidth(150)
        self.snap_combobox.addItems(["None", "1/4", "1/3", "1/2", "1"])
        self.controls_grid_layout.addWidget(QtGui.QLabel("Snap (beats):"), 0, 0)
        self.controls_grid_layout.addWidget(self.snap_combobox, 0, 1)
        self.snap_combobox.currentIndexChanged.connect(self.set_snap)
        self.snap_combobox.setCurrentIndex(1)
        self.add_item_button = QtGui.QPushButton("Add Note")
        self.controls_grid_layout.addWidget(self.add_item_button, 0, 2)
        self.add_item_button.pressed.connect(self.add_note)
        #self.h_zoom_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
        #self.h_zoom_slider.setRange(0, 100)
        #self.h_zoom_slider.setMaximumWidth(600)
        #self.h_zoom_slider.setValue(0)
        #self.last_scale_value = 0
        #self.h_zoom_slider.valueChanged.connect(self.set_zoom)
        #self.controls_grid_layout.addWidget(QtGui.QLabel("Zoom:"), 0, 49)
        #self.controls_grid_layout.addWidget(self.h_zoom_slider, 0, 50)

    def add_note(self):
        if not this_item_editor.enabled:
            this_item_editor.show_not_enabled_warning()
            return
        this_item_editor.notes_show_event_dialog(len(this_item_editor.item.notes), 0)

    def set_snap(self, a_val=None):
        pydaw_set_piano_roll_quantize(self.snap_combobox.currentIndex())

    def set_zoom(self, a_val=None):
        """ This is a ridiculously convoluted way to do this, but I see no other way in the Qt docs.  When
        Scaling, 1.0 does not return to it's original scale, and QSlider skips values when moved quickly, making
        it necessary to interpolate the inbetween values"""
        pass
        #if self.last_scale_value > self.h_zoom_slider.value():
        #    for i in range(self.h_zoom_slider.value(), self.last_scale_value):
        #        this_audio_items_viewer.set_zoom(0.97)
        #else:
        #    for i in range(self.last_scale_value, self.h_zoom_slider.value()):
        #        this_audio_items_viewer.set_zoom(1.03)
        #self.last_scale_value = self.h_zoom_slider.value()




global_automation_point_diameter = 15.0
global_automation_point_radius = global_automation_point_diameter * 0.5
global_automation_ruler_width = 24
global_automation_width = 690
global_automation_height = 300

global_automation_total_height = global_automation_ruler_width +  global_automation_height - global_automation_point_radius
global_automation_total_width = global_automation_ruler_width + global_automation_width - global_automation_point_radius
global_automation_min_height = global_automation_ruler_width - global_automation_point_radius

global_automation_gradient = QtGui.QLinearGradient(0, 0, global_automation_point_diameter, global_automation_point_diameter)
global_automation_gradient.setColorAt(0, QtGui.QColor(240, 10, 10))
global_automation_gradient.setColorAt(1, QtGui.QColor(250, 90, 90))

class automation_item(QtGui.QGraphicsEllipseItem):
    def __init__(self, a_time, a_value, a_cc, a_view, a_is_cc, a_item_index):
        QtGui.QGraphicsEllipseItem.__init__(self, 0, 0, global_automation_point_diameter, global_automation_point_diameter)
        self.item_index = a_item_index
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.setPos(a_time-global_automation_point_radius, a_value - global_automation_point_radius)
        self.setBrush(global_automation_gradient)
        f_pen = QtGui.QPen()
        f_pen.setWidth(2)
        f_pen.setColor(QtGui.QColor(170,0,0))
        self.setPen(f_pen)
        self.cc_item = a_cc
        self.parent_view = a_view
        self.is_cc = a_is_cc

    def mousePressEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mousePressEvent(self, a_event)
        self.setGraphicsEffect(QtGui.QGraphicsOpacityEffect())

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseMoveEvent(self, a_event)
        for f_point in self.parent_view.automation_points:
            if f_point.isSelected():
                if f_point.pos().x() < global_automation_min_height:
                    f_point.setPos(global_automation_min_height, f_point.pos().y())
                elif f_point.pos().x() > global_automation_total_width:
                    f_point.setPos(global_automation_total_width, f_point.pos().y())
                if f_point.pos().y() < global_automation_min_height:
                    f_point.setPos(f_point.pos().x(), global_automation_min_height)
                elif f_point.pos().y() > global_automation_total_height:
                    f_point.setPos(f_point.pos().x(), global_automation_total_height)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseReleaseEvent(self, a_event)
        self.setGraphicsEffect(None)
        for f_point in self.parent_view.automation_points:
            if f_point.isSelected():
                f_cc_start = round((((f_point.pos().x() - global_automation_min_height) / global_automation_width) * 4.0), 4)
                if f_cc_start > 4.0:
                    f_cc_start = 4.0
                elif f_cc_start < 0.0:
                    f_cc_start = 0.0
                if self.is_cc:
                    f_cc_val = int(127.0 - (((f_point.pos().y() - global_automation_min_height) / global_automation_height) * 127.0))
                    f_point.cc_item.start = f_cc_start
                    f_point.cc_item.set_val(f_cc_val)
                else:
                    f_cc_val = (1.0 - (((f_point.pos().y() - global_automation_min_height) / global_automation_height) * 2.0))
                    f_point.cc_item.start = f_cc_start
                    f_point.cc_item.set_val(f_cc_val)
        this_item_editor.save_and_reload()

class automation_viewer(QtGui.QGraphicsView):
    def __init__(self, a_item_length=4, a_grid_div=16, a_is_cc=True):
        self.is_cc = a_is_cc
        self.item_length = float(a_item_length)
        self.viewer_width = global_automation_width
        self.viewer_height = global_automation_height
        self.grid_div = a_grid_div
        self.automation_points = []

        self.axis_size = global_automation_ruler_width

        self.beat_width = self.viewer_width / self.item_length
        self.value_width = self.beat_width / self.grid_div
        self.lines = []

        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(100,100,100))
        self.scene.mouseDoubleClickEvent = self.sceneMouseDoubleClickEvent
        self.setAlignment(QtCore.Qt.AlignLeft)
        self.setScene(self.scene)
        self.draw_axis()
        self.draw_grid()
        self.setDragMode(QtGui.QGraphicsView.RubberBandDrag)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.setResizeAnchor(QtGui.QGraphicsView.AnchorViewCenter)
        self.cc_num = 1

    def keyPressEvent(self, a_event):
        QtGui.QGraphicsScene.keyPressEvent(self.scene, a_event)
        if not this_item_editor.enabled:
            return
        if a_event.key() == QtCore.Qt.Key_Delete:
            for f_point in self.automation_points:
                if f_point.isSelected():
                    if self.is_cc:
                        this_item_editor.items[f_point.item_index].remove_cc(f_point.cc_item)
                    else:
                        this_item_editor.items[f_point.item_index].remove_pb(f_point.cc_item)
        this_item_editor.save_and_reload()

    def sceneMouseDoubleClickEvent(self, a_event):
        if not this_item_editor.enabled:
            this_item_editor.show_not_enabled_warning()
            return
        f_pos_x = a_event.scenePos().x()
        f_pos_y = a_event.scenePos().y()
        f_cc_start = ((f_pos_x - global_automation_min_height) / global_automation_width) * 4.0
        if f_cc_start > 4.0:
            f_cc_start = 4.0
        elif f_cc_start < 0.0:
            f_cc_start = 0.0
        if self.is_cc:
            f_cc_val = int(127.0 - (((f_pos_y - global_automation_min_height) / global_automation_height) * 127.0))
            if f_cc_val > 127:
                f_cc_val = 127
            elif f_cc_val < 0:
                f_cc_val = 0
            this_item_editor.add_cc(pydaw_cc(round(f_cc_start, 4), self.cc_num, f_cc_val))
        else:
            f_cc_val = 1.0 - (((f_pos_y - global_automation_min_height) / global_automation_height) * 2.0)
            if f_cc_val > 1.0:
                f_cc_val = 1.0
            elif f_cc_val < -1.0:
                f_cc_val = -1.0
            this_item_editor.add_pb(pydaw_pitchbend(round(f_cc_start, 4), round(f_cc_val, 4)))
        QtGui.QGraphicsScene.mouseDoubleClickEvent(self.scene, a_event)
        this_item_editor.save_and_reload()

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsView.mouseMoveEvent(self, a_event)
        if self.scene.mouseGrabberItem():
            self.connect_points()

    def draw_axis(self):
        self.x_axis = QtGui.QGraphicsRectItem(0, 0, self.viewer_width, self.axis_size)
        self.x_axis.setPos(self.axis_size, 0)
        self.scene.addItem(self.x_axis)
        self.y_axis = QtGui.QGraphicsRectItem(0, 0, self.axis_size, self.viewer_height)
        self.y_axis.setPos(0, self.axis_size)
        self.scene.addItem(self.y_axis)

    def draw_grid(self):
        f_beat_pen = QtGui.QPen()
        f_beat_pen.setWidth(2)
        f_line_pen = QtGui.QPen()
        f_line_pen.setColor(QtGui.QColor(0,0,0,40))
        if self.is_cc:
            f_labels = [0, '127', 0, '64', 0, '0']
        else:
            f_labels = [0, '1.0', 0, '0', 0, '-1.0']
        for i in range(1,6):
            f_line = QtGui.QGraphicsLineItem(0, 0, self.viewer_width, 0, self.y_axis)
            f_line.setPos(self.axis_size,self.viewer_height*(i-1)/4)
            if i % 2:
                f_label = QtGui.QGraphicsSimpleTextItem(f_labels[i], self.y_axis)
                f_label.setPos(1, self.viewer_height*(i-1)/4)
                f_label.setBrush(QtCore.Qt.white)
            if i == 3:
                f_line.setPen(f_beat_pen)

        for i in range(0, int(self.item_length)+1):
            f_beat = QtGui.QGraphicsLineItem(0, 0, 0, self.viewer_height+self.axis_size-f_beat_pen.width(), self.x_axis)
            f_beat.setPos(self.beat_width * i, 0.5*f_beat_pen.width())
            f_beat.setPen(f_beat_pen)
            if i < self.item_length:
                f_number = QtGui.QGraphicsSimpleTextItem(str(i), self.x_axis)
                f_number.setPos(self.beat_width * i + 5, 2)
                f_number.setBrush(QtCore.Qt.white)
                for j in range(0, self.grid_div):
                    f_line = QtGui.QGraphicsLineItem(0, 0, 0, self.viewer_height, self.x_axis)
                    if float(j) == self.grid_div / 2.0:
                        f_line.setLine(0, 0, 0, self.viewer_height)
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.axis_size)
                    else:
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.axis_size)
                        f_line.setPen(f_line_pen)

    def set_zoom(self, a_scale):
        self.scale(a_scale, 1.0)

    def clear_drawn_items(self):
        self.scene.clear()
        self.automation_points = []
        self.lines = []
        self.draw_axis()
        self.draw_grid()

    def connect_points(self):
        if self.lines:
            for i in range(len(self.lines)):
                self.scene.removeItem(self.lines[i])
        #sort list based on x
        if len(self.automation_points) > 1:
            self.lines = (len(self.automation_points)-1)*[None]
            self.automation_points.sort(key=lambda point: point.pos().x())
            f_line_pen = QtGui.QPen()
            f_line_pen.setColor(QtGui.QColor(255,60,60))
            f_line_pen.setWidth(2)
            for i in range(1, len(self.automation_points)):
                f_start_x = self.automation_points[i-1].pos().x()
                f_start_y = self.automation_points[i-1].pos().y()
                f_end_x = self.automation_points[i].pos().x()
                f_end_y = self.automation_points[i].pos().y()
                f_pos_x = f_end_x - f_start_x
                f_pos_y = f_end_y - f_start_y
                f_line = QtGui.QGraphicsLineItem(0, 0, f_pos_x, f_pos_y)
                f_line.setPos(7.5+f_start_x, 7.5+f_start_y)
                f_line.setPen(f_line_pen)
                self.scene.addItem(f_line)
                self.lines[i-1] = f_line

    def set_cc_num(self, a_cc_num):
        self.cc_num = a_cc_num
        self.clear_drawn_items()
        self.draw_item()

    def draw_item(self):
        self.clear_drawn_items()
        if not this_item_editor.enabled:
            return
        f_item_index = 0
        for f_item in this_item_editor.items:
            if self.is_cc:
                for f_cc in f_item.ccs:
                    if f_cc.cc_num == self.cc_num:
                        self.draw_point(f_cc, f_item_index)
            else:
                for f_pb in f_item.pitchbends:
                    self.draw_point(f_pb, f_item_index)
            f_item_index += 1

    def draw_point(self, a_cc, a_item_index):
        """ a_cc is an instance of the pydaw_cc class"""
        f_time = self.axis_size + (((float(a_item_index) * 4.0) + a_cc.start) * self.beat_width)
        if self.is_cc:
            f_value = self.axis_size +  self.viewer_height/127.0 * (127.0 - a_cc.cc_val)
        else:
            f_value = self.axis_size +  self.viewer_height/2.0 * (1.0 - a_cc.pb_val)
        f_point = automation_item(f_time, f_value, a_cc, self, self.is_cc, a_item_index)
        self.automation_points.append(f_point)
        self.scene.addItem(f_point)
        self.connect_points()

class automation_viewer_widget:
    def plugin_changed(self, a_val=None):
        self.control_combobox.clear()
        self.control_combobox.addItems(global_cc_maps[str(self.plugin_combobox.currentText())].keys())

    def control_changed(self, a_val=None):
        f_plugin_str = str(self.plugin_combobox.currentText())
        f_control_str = str(self.control_combobox.currentText())
        if f_plugin_str != '' and f_control_str != '':
            f_value = int(global_cc_maps[f_plugin_str][f_control_str])
            self.cc_spinbox.setValue(f_value)

    def cc_num_changed(self, a_val=None):
        self.set_cc_num(self.cc_spinbox.value())

    def set_cc_num(self, a_num):
        self.cc_spinbox.setValue(a_num)
        self.automation_viewer.set_cc_num(a_num)

    def smooth_pressed(self):
        if self.is_cc:
            pydaw_smooth_automation_points(this_item_editor.items, self.is_cc, self.cc_spinbox.value())
        else:
            pydaw_smooth_automation_points(this_item_editor.items, self.is_cc)
        this_item_editor.save_and_reload()

    def __init__(self, a_viewer, a_is_cc=True):
        self.is_cc = a_is_cc
        self.widget = QtGui.QGroupBox()
        self.vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.vlayout)
        self.automation_viewer = a_viewer
        self.vlayout.addWidget(self.automation_viewer)
        self.hlayout = QtGui.QHBoxLayout()
        self.vlayout.addLayout(self.hlayout)

        if a_is_cc:
            self.cc_spinbox = QtGui.QSpinBox()
            self.cc_spinbox.setRange(1, 127)
            self.hlayout.addWidget(QtGui.QLabel("CC#:"))
            self.hlayout.addWidget(self.cc_spinbox)
            self.cc_spinbox.valueChanged.connect(self.cc_num_changed)

            self.plugin_combobox = QtGui.QComboBox()
            self.plugin_combobox.setMinimumWidth(120)
            self.plugin_combobox.addItems(global_cc_maps.keys())
            self.hlayout.addWidget(QtGui.QLabel("Plugin"))
            self.hlayout.addWidget(self.plugin_combobox)

            self.control_combobox = QtGui.QComboBox()
            self.control_combobox.setMinimumWidth(180)
            self.hlayout.addWidget(QtGui.QLabel("Control"))
            self.hlayout.addWidget(self.control_combobox)
            self.plugin_combobox.currentIndexChanged.connect(self.plugin_changed)
            self.control_combobox.currentIndexChanged.connect(self.control_changed)

        self.smooth_button = QtGui.QPushButton("Smooth")
        self.smooth_button.setToolTip("By default, the control points are steppy, this button draws extra points between the exisiting points.")
        self.smooth_button.pressed.connect(self.smooth_pressed)
        self.hlayout.addWidget(self.smooth_button)
        self.hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))
        self.widget.setMinimumSize(750, 420)
        self.widget.setMaximumSize(750, 420)

class item_list_editor:
    def clear_notes(self):
        if self.enabled:
            self.item.notes = []
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
    def clear_ccs(self):
        if self.enabled:
            self.item.ccs = []
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
    def clear_pb(self):
        if self.enabled:
            self.item.pitchbends = []
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()

    def clear_new(self):
        self.enabled = False
        #self.item_name_line_edit.setText("")
        self.item_name_combobox.clear()
        self.item_name_combobox.clearEditText()
        self.ccs_table_widget.clearContents()
        self.notes_table_widget.clearContents()
        self.pitchbend_table_widget.clearContents()
        this_piano_roll_editor.clear_drawn_items()

    def save_and_reload(self):
        for f_i in range(len(self.item_names)):
            this_pydaw_project.save_item(self.item_names[f_i], self.items[f_i])
        self.open_item()

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

    def quantize_dialog(self):
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

        def quantize_ok_handler():
            f_quantize_index = f_quantize_combobox.currentIndex()
            self.events_follow_default = f_events_follow_notes.isChecked()
            if f_multiselect:
                self.item.quantize(f_quantize_index, f_events_follow_notes.isChecked(), f_ms_rows)
            else:
                self.item.quantize(f_quantize_index, f_events_follow_notes.isChecked())
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Quantize item '" + self.item_name + "'")
            f_window.close()

        def quantize_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Quantize")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_layout.addWidget(QtGui.QLabel("Quantize(beats)"), 0, 0)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.setCurrentIndex(5)
        f_layout.addWidget(f_quantize_combobox, 0, 1)
        f_events_follow_notes = QtGui.QCheckBox("CCs and pitchbend follow notes?")
        f_events_follow_notes.setChecked(self.events_follow_default)
        f_layout.addWidget(f_events_follow_notes, 1, 1)
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(quantize_ok_handler)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_ok_cancel_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_cancel_layout, 3, 1)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(quantize_cancel_handler)
        f_ok_cancel_layout.addWidget(f_cancel)
        f_window.exec_()

    def velocity_dialog(self):
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

        f_start_beat_val = 3.99
        f_end_beat_val = 0.0
        if f_multiselect:
            for f_note in f_ms_rows:
                if f_note.start < f_start_beat_val:
                    f_start_beat_val = f_note.start
                elif f_note.start > f_end_beat_val:
                    f_end_beat_val = f_note.start
        else:
            for f_note in self.item.notes:
                if f_note.start < f_start_beat_val:
                    f_start_beat_val = f_note.start
                elif f_note.start > f_end_beat_val:
                    f_end_beat_val = f_note.start

        def ok_handler():
            if f_multiselect:
                self.item.velocity_mod(f_amount.value(), f_start_beat.value(), f_end_beat.value(), f_draw_line.isChecked(), \
                f_end_amount.value(), f_add_values.isChecked(), f_ms_rows)
            else:
                self.item.velocity_mod(f_amount.value(), f_start_beat.value(), f_end_beat.value(), f_draw_line.isChecked(), \
                f_end_amount.value(), f_add_values.isChecked())
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Velocity mod item '" + self.item_name + "'")
            f_window.close()

        def cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Velocity Mod")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_layout.addWidget(QtGui.QLabel("Amount"), 0, 0)
        f_amount = QtGui.QSpinBox()
        f_amount.setRange(-127, 127)
        f_amount.setValue(64)
        f_layout.addWidget(f_amount, 0, 1)
        f_draw_line = QtGui.QCheckBox("Draw line?")
        f_layout.addWidget(f_draw_line, 1, 1)

        f_layout.addWidget(QtGui.QLabel("End Amount"), 2, 0)
        f_end_amount = QtGui.QSpinBox()
        f_end_amount.setRange(-127, 127)
        f_layout.addWidget(f_end_amount, 2, 1)

        f_layout.addWidget(QtGui.QLabel("Start Beat"), 3, 0)
        f_start_beat = QtGui.QDoubleSpinBox()
        f_start_beat.setRange(0.0, 3.99)
        f_start_beat.setValue(f_start_beat_val)
        f_layout.addWidget(f_start_beat, 3, 1)

        f_layout.addWidget(QtGui.QLabel("End Beat"), 4, 0)
        f_end_beat = QtGui.QDoubleSpinBox()
        f_end_beat.setRange(0.01, 4.0)
        f_end_beat.setValue(f_end_beat_val)
        f_layout.addWidget(f_end_beat, 4, 1)

        f_add_values = QtGui.QCheckBox("Add Values?")
        f_add_values.setToolTip("Check this to add Amount to the existing value, or leave\nunchecked to set the value to Amount.")
        f_layout.addWidget(f_add_values, 5, 1)

        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(ok_handler)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_ok_cancel_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_cancel_layout, 10, 1)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(cancel_handler)
        f_ok_cancel_layout.addWidget(f_cancel)
        f_window.exec_()

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
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Transpose item '" + self.item_name + "'")
            f_window.close()

        def transpose_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
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
            if f_quantize_checkbox.isChecked():
                f_quantize_index = f_quantize_combobox.currentIndex()
            else:
                f_quantize_index = None
            self.events_follow_default = f_events_follow_notes.isChecked()
            if f_multiselect:
                self.item.time_shift(f_shift.value(), f_events_follow_notes.isChecked(), f_ms_rows, a_quantize=f_quantize_index)
            else:
                self.item.time_shift(f_shift.value(), f_events_follow_notes.isChecked(), a_quantize=f_quantize_index)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Time shift item '" + self.item_name + "'")
            f_window.close()

        def time_shift_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Time Shift")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_shift = QtGui.QDoubleSpinBox()
        f_shift.setSingleStep(0.25)
        f_shift.setRange(-4.0, 4.0)
        f_layout.addWidget(QtGui.QLabel("Shift(beats)"), 0, 0)
        f_layout.addWidget(f_shift, 0, 1)
        f_events_follow_notes = QtGui.QCheckBox("CCs and pitchbend follow notes?")
        f_events_follow_notes.setChecked(self.events_follow_default)
        f_layout.addWidget(f_events_follow_notes, 1, 1)
        f_quantize_checkbox = QtGui.QCheckBox("Quantize?(beats)")
        f_layout.addWidget(f_quantize_checkbox, 2, 0)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.setCurrentIndex(5)
        f_layout.addWidget(f_quantize_combobox, 2, 1)
        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(time_shift_ok_handler)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_ok_cancel_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_cancel_layout, 3, 1)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(time_shift_cancel_handler)
        f_ok_cancel_layout.addWidget(f_cancel)
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

        def length_shift_ok_handler():
            if f_quantize_checkbox.isChecked():
                f_quantize_index = f_quantize_combobox.currentIndex()
            else:
                f_quantize_index = None

            if f_min_max_checkbox.isChecked():
                f_min_max_value = f_min_max.value()
            else:
                f_min_max_value = None

            if f_multiselect:
                self.item.length_shift(f_shift.value(), f_min_max_value, f_ms_rows, a_quantize=f_quantize_index)
            else:
                self.item.length_shift(f_shift.value(), f_min_max_value, a_quantize=f_quantize_index)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Length shift item '" + self.item_name + "'")
            f_window.close()

        def length_shift_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Length Shift")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_shift = QtGui.QDoubleSpinBox()
        f_shift.setSingleStep(0.25)
        f_shift.setRange(-16.0, 16.0)
        f_layout.addWidget(QtGui.QLabel("Shift(beats)"), 0, 0)
        f_layout.addWidget(f_shift, 0, 1)

        f_min_max = QtGui.QDoubleSpinBox()
        f_min_max.setRange(0.01, 16.0)
        f_min_max.setValue(1.0)
        f_min_max_checkbox = QtGui.QCheckBox("Min/Max(beats)")
        f_layout.addWidget(f_min_max_checkbox, 1, 0)
        f_layout.addWidget(f_min_max, 1, 1)

        f_quantize_checkbox = QtGui.QCheckBox("Quantize?(beats)")
        f_layout.addWidget(f_quantize_checkbox, 2, 0)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.setCurrentIndex(5)
        f_layout.addWidget(f_quantize_combobox, 2, 1)

        f_ok = QtGui.QPushButton("OK")
        f_ok.pressed.connect(length_shift_ok_handler)
        f_layout.addWidget(f_ok, 3, 0)
        f_cancel = QtGui.QPushButton("Cancel")
        f_cancel.pressed.connect(length_shift_cancel_handler)
        f_layout.addWidget(f_cancel, 3, 1)
        f_window.exec_()


    def on_template_open(self):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        f_path= expanduser("~") + "/" + global_pydaw_version_string + "/item_templates/" + str(self.template_combobox.currentText()) + ".pyitem"
        if not os.path.isfile(f_path):
            QtGui.QMessageBox.warning(self.notes_table_widget, "Error", "Cannot find specified template")
        else:
            f_item_handle = open(f_path, "r")
            f_item = pydaw_item.from_str(f_item_handle.read())
            f_item_handle.close()
            self.item = f_item
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Open template '" + str(self.template_combobox.currentText()) + "' as item '" + self.item_name + "'")

    def on_template_save_as(self):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        def ok_handler():
            if str(f_name.text()) == "":
                QtGui.QMessageBox.warning(f_window, "Error", "Name cannot be empty")
                return
            f_path= expanduser("~") + "/" + global_pydaw_version_string + "/item_templates/" + str(f_name.text()) + ".pyitem"
            f_handle = open(f_path, "w")
            f_handle.write(self.item.__str__())
            f_handle.close()
            self.load_templates()
            f_window.close()

        def cancel_handler():
            f_window.close()

        def f_name_text_changed():
            f_name.setText(pydaw_remove_bad_chars(f_name.text()))

        f_window = QtGui.QDialog(this_main_window)
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
        f_path= expanduser("~") + "/" + global_pydaw_version_string + "/item_templates"
        if not os.path.isdir(f_path):
            os.makedirs(f_path)
        else:
            f_file_list = os.listdir(f_path)
            for f_name in f_file_list:
                if f_name.endswith(".pyitem"):
                    self.template_combobox.addItem(f_name.split(".")[0])

    def __init__(self):
        self.enabled = False
        self.items = []
        self.item_names = []
        self.events_follow_default = True

        self.widget = QtGui.QWidget()
        self.master_vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.master_vlayout)
        self.tab_widget = QtGui.QTabWidget()

        self.piano_roll_tab = QtGui.QGroupBox()
        self.tab_widget.addTab(self.piano_roll_tab, "Piano Roll")

        self.notes_tab = QtGui.QGroupBox()
        self.tab_widget.addTab(self.notes_tab, "Notes")

        self.group_box = QtGui.QGroupBox()
        self.tab_widget.addTab(self.group_box, "CCs")

        self.pitchbend_tab = QtGui.QGroupBox()
        self.tab_widget.addTab(self.pitchbend_tab, "Pitchbend")

        self.main_vlayout = QtGui.QVBoxLayout()
        self.main_hlayout = QtGui.QHBoxLayout()
        self.group_box.setLayout(self.main_vlayout)

        self.editing_hboxlayout = QtGui.QHBoxLayout()
        self.master_vlayout.addLayout(self.editing_hboxlayout)

        self.master_vlayout.addWidget(self.tab_widget)

        self.editing_hboxlayout.addWidget(QtGui.QLabel("Editing Item:"))
        self.item_name_combobox = QtGui.QComboBox()
        self.item_name_combobox.setMinimumWidth(150)
        self.item_name_combobox.setEditable(False)
        self.item_name_combobox.currentIndexChanged.connect(self.item_index_changed)
        self.item_index_enabled = True
        self.editing_hboxlayout.addWidget(self.item_name_combobox)

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

        self.main_vlayout.addLayout(self.main_hlayout)

        self.notes_groupbox = QtGui.QGroupBox("Note Tools")
        self.notes_groupbox.setMinimumWidth(591)
        self.notes_groupbox.setMaximumWidth(591)
        self.notes_vlayout = QtGui.QVBoxLayout(self.notes_groupbox)

        self.edit_mode_groupbox = QtGui.QGroupBox()
        self.edit_mode_hlayout0 = QtGui.QHBoxLayout(self.edit_mode_groupbox)
        self.edit_mode_hlayout0.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        self.edit_mode_hlayout0.addWidget(QtGui.QLabel("List Edit Mode:"))
        self.add_radiobutton = QtGui.QRadioButton("Add/Edit")
        self.edit_mode_hlayout0.addWidget(self.add_radiobutton)
        self.multiselect_radiobutton = QtGui.QRadioButton("Multiselect")
        self.edit_mode_hlayout0.addWidget(self.multiselect_radiobutton)
        self.delete_radiobutton = QtGui.QRadioButton("Delete")
        self.edit_mode_hlayout0.addWidget(self.delete_radiobutton)
        self.add_radiobutton.setChecked(True)
        self.editing_hboxlayout.addWidget(self.edit_mode_groupbox, alignment=QtCore.Qt.AlignLeft)
        self.editing_hboxlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))

        self.notes_gridlayout = QtGui.QGridLayout()

        f_button_width = 82
        self.notes_quantize_button = QtGui.QPushButton("Quantize")
        self.notes_quantize_button.setMinimumWidth(f_button_width)
        self.notes_quantize_button.pressed.connect(self.quantize_dialog)
        self.notes_gridlayout.addWidget(self.notes_quantize_button, 0, 1)
        self.notes_transpose_button = QtGui.QPushButton("Transpose")
        self.notes_transpose_button.setMinimumWidth(f_button_width)
        self.notes_transpose_button.pressed.connect(self.transpose_dialog)
        self.notes_gridlayout.addWidget(self.notes_transpose_button, 0, 2)
        self.notes_shift_button = QtGui.QPushButton("Shift")
        self.notes_shift_button.setMinimumWidth(f_button_width)
        self.notes_shift_button.pressed.connect(self.time_shift_dialog)
        self.notes_gridlayout.addWidget(self.notes_shift_button, 0, 3)
        self.notes_length_button = QtGui.QPushButton("Length")
        self.notes_length_button.setMinimumWidth(f_button_width)
        self.notes_length_button.pressed.connect(self.length_shift_dialog)
        self.notes_gridlayout.addWidget(self.notes_length_button, 0, 4)

        self.notes_velocity_button = QtGui.QPushButton("Velocity")
        self.notes_velocity_button.setMinimumWidth(f_button_width)
        self.notes_velocity_button.pressed.connect(self.velocity_dialog)
        self.notes_gridlayout.addWidget(self.notes_velocity_button, 0, 5)

        self.notes_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 6)

        self.notes_clear_button = QtGui.QPushButton("Clear")
        self.notes_clear_button.setMinimumWidth(f_button_width)
        self.notes_clear_button.pressed.connect(self.clear_notes)
        self.notes_gridlayout.addWidget(self.notes_clear_button, 0, 7)
        self.notes_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 6, 1, 1)
        self.notes_vlayout.addLayout(self.notes_gridlayout)
        self.notes_table_widget = QtGui.QTableWidget()
        self.notes_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.notes_table_widget.setColumnCount(5)
        self.notes_table_widget.cellClicked.connect(self.notes_click_handler)
        self.notes_table_widget.setSortingEnabled(True)
        self.notes_table_widget.sortItems(0)
        self.notes_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.notes_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.notes_table_widget.keyPressEvent = self.notes_keyPressEvent
        self.notes_vlayout.addWidget(self.notes_table_widget)

        self.notes_hlayout = QtGui.QHBoxLayout()
        self.notes_tab.setLayout(self.notes_hlayout)
        self.notes_hlayout.addWidget(self.notes_groupbox)
        self.notes_hlayout.addItem(QtGui.QSpacerItem(0, 0, QtGui.QSizePolicy.Expanding))

        self.piano_roll_hlayout = QtGui.QHBoxLayout()
        self.piano_roll_tab.setLayout(self.piano_roll_hlayout)
        self.piano_roll_hlayout.addWidget(this_piano_roll_editor_widget.widget)

        self.ccs_groupbox = QtGui.QGroupBox("CCs")
        self.ccs_groupbox.setMaximumWidth(390)
        self.ccs_groupbox.setMinimumWidth(390)
        self.ccs_vlayout = QtGui.QVBoxLayout(self.ccs_groupbox)
        self.ccs_gridlayout = QtGui.QGridLayout()
        self.ccs_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 0, 1, 1)
        self.ccs_clear_button = QtGui.QPushButton("Clear")
        self.ccs_clear_button.setMinimumWidth(f_button_width)
        self.ccs_clear_button.pressed.connect(self.clear_ccs)
        self.ccs_gridlayout.addWidget(self.ccs_clear_button, 0, 1)
        self.ccs_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 2, 1, 1)
        self.ccs_vlayout.addLayout(self.ccs_gridlayout)
        self.ccs_table_widget = QtGui.QTableWidget()
        self.ccs_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.ccs_table_widget.setColumnCount(3)
        self.ccs_table_widget.cellClicked.connect(self.ccs_click_handler)
        self.ccs_table_widget.setSortingEnabled(True)
        self.ccs_table_widget.sortItems(0)
        self.ccs_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.ccs_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.ccs_table_widget.keyPressEvent = self.ccs_keyPressEvent
        self.ccs_vlayout.addWidget(self.ccs_table_widget)
        self.main_hlayout.addWidget(self.ccs_groupbox)

        self.cc_auto_viewer_scrollarea = QtGui.QScrollArea()
        self.cc_auto_viewer_scrollarea.setMinimumWidth(790)
        self.cc_auto_viewer_scrollarea.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.cc_auto_viewer_scrollarea.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.cc_auto_viewer_scrollarea_widget = QtGui.QWidget()
        self.cc_auto_viewer_scrollarea_widget.setMinimumSize(770, 1270)
        self.cc_auto_viewer_scrollarea.setWidget(self.cc_auto_viewer_scrollarea_widget)
        self.cc_auto_viewer_vlayout = QtGui.QVBoxLayout(self.cc_auto_viewer_scrollarea_widget)

        self.cc_auto_viewers = []
        for i in range(3):
            self.cc_auto_viewers.append(automation_viewer_widget(this_cc_automation_viewers[i]))
            self.cc_auto_viewer_vlayout.addWidget(self.cc_auto_viewers[i].widget)
        self.cc_auto_viewer_vlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding))
        self.main_hlayout.addWidget(self.cc_auto_viewer_scrollarea)
        self.main_hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))

        self.pb_hlayout = QtGui.QHBoxLayout()
        self.pitchbend_tab.setLayout(self.pb_hlayout)
        self.pb_groupbox = QtGui.QGroupBox("Pitchbend")
        self.pb_groupbox.setMaximumWidth(300)
        self.pb_groupbox.setMinimumWidth(300)
        self.pb_vlayout = QtGui.QVBoxLayout(self.pb_groupbox)
        self.pb_gridlayout = QtGui.QGridLayout()
        self.pb_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 0, 1, 1)
        self.pb_clear_button = QtGui.QPushButton("Clear")
        self.pb_clear_button.setMinimumWidth(f_button_width)
        self.pb_clear_button.pressed.connect(self.clear_pb)
        self.pb_gridlayout.addWidget(self.pb_clear_button, 0, 1)
        self.pb_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum), 0, 2, 1, 1)
        self.pb_vlayout.addLayout(self.pb_gridlayout)
        self.pitchbend_table_widget = QtGui.QTableWidget()
        self.pitchbend_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.pitchbend_table_widget.setColumnCount(2)
        self.pitchbend_table_widget.cellClicked.connect(self.pitchbend_click_handler)
        self.pitchbend_table_widget.setSortingEnabled(True)
        self.pitchbend_table_widget.sortItems(0)
        self.pitchbend_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.pitchbend_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.pitchbend_table_widget.keyPressEvent = self.pbs_keyPressEvent
        self.pb_vlayout.addWidget(self.pitchbend_table_widget)
        self.pb_hlayout.addWidget(self.pb_groupbox)
        self.pb_auto_vlayout = QtGui.QVBoxLayout()
        self.pb_hlayout.addLayout(self.pb_auto_vlayout)
        self.pb_viewer_widget = automation_viewer_widget(this_pb_automation_viewer, False)
        self.pb_auto_vlayout.addWidget(self.pb_viewer_widget.widget)
        self.pb_auto_vlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding))

        self.pb_hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))

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

    def item_index_changed(self, a_index=None):
        if self.item_index_enabled:
            self.open_item([self.item_name_combobox.currentText()])

    def set_row_counts(self):
        f_factor = len(self.item_names)
        self.notes_table_widget.setRowCount(128 * f_factor)
        self.ccs_table_widget.setRowCount(256 * f_factor)
        self.pitchbend_table_widget.setRowCount(128 * f_factor)

    def add_cc(self, a_cc):
        f_index, f_start = pydaw_beats_to_index(a_cc.start)
        a_cc.start = f_start
        self.items[f_index].add_cc(a_cc)

    def add_note(self, a_note):
        f_index, f_start = pydaw_beats_to_index(a_note.start)
        a_note.start = f_start
        self.items[f_index].add_note(a_note)

    def add_pb(self, a_pb):
        f_index, f_start = pydaw_beats_to_index(a_pb.start)
        a_pb.start = f_start
        self.items[f_index].add_cc(a_pb)

    def open_item(self, a_items=None):
        """ a_items is a list of str, which are the names of the items.  Leave blank to open the existing list """
        for i in range(3):
            this_cc_automation_viewers[i].clear_drawn_items()
        this_pb_automation_viewer.clear_drawn_items()

        self.enabled = True

        if a_items is not None:
            self.item_names = a_items
            self.item_index_enabled = False
            self.item_name_combobox.clear()
            self.item_name_combobox.clearEditText()
            self.item_name_combobox.addItems(a_items)
            #self.item_name_combobox.setCurrentIndex(self.item_name_combobox.findText(a_item_name))
            self.item_index_enabled = True

        self.notes_table_widget.clear()
        self.ccs_table_widget.clear()
        self.pitchbend_table_widget.clear()
        self.set_headers()
        self.set_row_counts()
        self.items = []
        f_cc_dict = {}

        self.notes_table_widget.setSortingEnabled(False)
        self.ccs_table_widget.setSortingEnabled(False)
        self.pitchbend_table_widget.setSortingEnabled(False)

        f_beat_offset = 0.0
        for f_item_name in self.item_names:
            f_item = this_pydaw_project.get_item(f_item_name)
            self.items.append(f_item)
            f_i = 0
            for note in f_item.notes:
                f_note_str = note_num_to_string(note.note_num)
                self.notes_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(note.start + f_beat_offset)))
                self.notes_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(str(note.length)))
                self.notes_table_widget.setItem(f_i, 2, QtGui.QTableWidgetItem(f_note_str))
                self.notes_table_widget.setItem(f_i, 3, QtGui.QTableWidgetItem(str(note.note_num)))
                self.notes_table_widget.setItem(f_i, 4, QtGui.QTableWidgetItem(str(note.velocity)))
                f_i = f_i + 1
            f_i = 0
            for cc in f_item.ccs:
                if not f_cc_dict.has_key(cc.cc_num):
                    f_cc_dict[cc.cc_num] = []
                f_cc_dict[cc.cc_num] = cc
                self.ccs_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(cc.start + f_beat_offset)))
                self.ccs_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(str(cc.cc_num)))
                self.ccs_table_widget.setItem(f_i, 2, QtGui.QTableWidgetItem(str(cc.cc_val)))
                f_i = f_i + 1
            f_i = 0
            for pb in f_item.pitchbends:
                self.pitchbend_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(pb.start + f_beat_offset)))
                self.pitchbend_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(str(pb.pb_val)))
                f_i = f_i + 1
            f_beat_offset += 4.0
        self.notes_table_widget.setSortingEnabled(True)
        self.ccs_table_widget.setSortingEnabled(True)
        self.pitchbend_table_widget.setSortingEnabled(True)

        this_piano_roll_editor.draw_item()
        f_i = 0
        if a_items is not None:
            for f_cc_num in f_cc_dict.keys():
                self.cc_auto_viewers[f_i].set_cc_num(f_cc_num)
                f_i += 1
                if f_i >= len(self.cc_auto_viewers):
                    break
        else:
            for i in range(3):
                this_cc_automation_viewers[i].draw_item()
        this_pb_automation_viewer.draw_item()

    def notes_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_notes = self.get_notes_table_selected_rows()
                for f_note in f_notes:
                    self.item.remove_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Delete notes from item '" + self.item_name + "'")
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.notes_clipboard = self.get_notes_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_note in self.notes_clipboard:
                self.item.add_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Paste notes into item '" + self.item_name + "'")
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.notes_clipboard = self.get_notes_table_selected_rows()
            for f_note in self.notes_clipboard:
                self.item.remove_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Cut notes from item '" + self.item_name + "'")
        else:
            QtGui.QTableWidget.keyPressEvent(self.notes_table_widget, event)

    def ccs_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_ccs = self.get_ccs_table_selected_rows()
                for f_cc in f_ccs:
                    self.item.remove_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Delete CCs from item '" + self.item_name + "'")
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_cc in self.ccs_clipboard:
                self.item.add_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Paste CCs into item '" + self.item_name + "'")
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
            for f_cc in self.ccs_clipboard:
                self.item.remove_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Cut CCs from item '" + self.item_name + "'")
        else:
            QtGui.QTableWidget.keyPressEvent(self.ccs_table_widget, event)

    def pbs_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_pbs = self.get_pbs_table_selected_rows()
                for f_pb in f_pbs:
                    self.item.remove_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Delete pitchbends from item '" + self.item_name + "'")
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.pbs_clipboard = self.get_pbs_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_pb in self.pbs_clipboard:
                self.item.add_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Paste pitchbends into item '" + self.item_name + "'")
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.pbs_clipboard = self.get_pbs_table_selected_rows()
            for f_pb in self.pbs_clipboard:
                self.item.remove_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Cut pitchbends from item '" + self.item_name + "'")
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
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Delete note from item '" + self.item_name + "'")

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
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Delete CC from item '" + self.item_name + "'")

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
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Delete pitchbend from item '" + self.item_name + "'")

    def notes_show_event_dialog(self, x, y, a_note=None):
        if a_note is not None:
            self.is_existing_note = True
            f_note_item = a_note
            self.default_note_start = a_note.start
            self.default_note_length = a_note.length
            self.default_note_note = a_note.note_num % 12
            self.default_note_octave = (a_note.note_num / 12) - 2
            self.default_note_velocity = a_note.velocity
        else:
            f_cell = self.notes_table_widget.item(x, y)
            if f_cell is not None:
                f_note_item = pydaw_note(self.notes_table_widget.item(x, 0).text(), self.notes_table_widget.item(x, 1).text(), self.notes_table_widget.item(x, 3).text(), self.notes_table_widget.item(x, 4).text())
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
                self.item.remove_note(f_note_item)
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

            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Update notes for item '" + self.item_name + "'")

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

        def length_changed(a_val=None):
            f_frac = beat_frac_text_to_float(f_quantize_combobox.currentIndex())
            if f_length.value() < f_frac:
                f_length.setValue(f_frac)
            else:
                f_val = round(f_length.value()/f_frac) * f_frac
                f_length.setValue(f_val)

        def add_another_clicked(a_checked):
            if a_checked:
                f_cancel_button.setText("Close")
            else:
                f_cancel_button.setText("Cancel")

        f_window = QtGui.QDialog(this_main_window)
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
        f_note.setMinimumWidth(66)
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
        f_length.setRange(0.01, 16.0)
        f_length.setValue(self.default_note_length)
        f_layout.addWidget(f_length, 3, 1)
        f_length.valueChanged.connect(length_changed)
        f_velocity = QtGui.QSpinBox()
        f_velocity.setRange(1, 127)
        f_velocity.setValue(self.default_note_velocity)
        f_layout.addWidget(QtGui.QLabel("Velocity"), 4, 0)
        f_layout.addWidget(f_velocity, 4, 1)
        if not self.is_existing_note:
            f_add_another = QtGui.QCheckBox("Add another?")
            f_add_another.toggled.connect(add_another_clicked)
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
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Update CCs for item '" + self.item_name + "'")
            if not f_add_another.isChecked():
                f_window.close()

        def cc_cancel_handler():
            f_window.close()

        def quantize_changed(f_quantize_index):
            f_frac = beat_frac_text_to_float(f_quantize_index)
            f_start.setSingleStep(f_frac)
            self.default_quantize = f_quantize_index


        def add_another_clicked(a_checked):
            if a_checked:
                f_cancel_button.setText("Close")
            else:
                f_cancel_button.setText("Cancel")

        def plugin_changed(a_val=None):
            f_control_combobox.clear()
            f_control_combobox.addItems(global_cc_maps[str(f_plugin_combobox.currentText())].keys())

        def control_changed(a_val=None):
            f_plugin_str = str(f_plugin_combobox.currentText())
            f_control_str = str(f_control_combobox.currentText())
            if f_plugin_str != '' and f_control_str != '':
                f_value = int(global_cc_maps[f_plugin_str][f_control_str])
                f_cc.setValue(f_value)

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("CCs")
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.currentIndexChanged.connect(quantize_changed)
        f_layout.addWidget(QtGui.QLabel("Quantize(beats)"), 1, 0)
        f_layout.addWidget(f_quantize_combobox, 1, 1)

        f_plugin_combobox = QtGui.QComboBox()
        f_plugin_combobox.addItems(global_cc_maps.keys())
        f_layout.addWidget(QtGui.QLabel("Plugin"), 2, 0)
        f_layout.addWidget(f_plugin_combobox, 2, 1)

        f_control_combobox = QtGui.QComboBox()
        f_layout.addWidget(QtGui.QLabel("Control"), 3, 0)
        f_layout.addWidget(f_control_combobox, 3, 1)

        plugin_changed()
        f_plugin_combobox.currentIndexChanged.connect(plugin_changed)
        f_control_combobox.currentIndexChanged.connect(control_changed)

        f_cc = QtGui.QSpinBox()
        f_cc.setRange(1, 127)
        f_cc.setValue(self.default_cc_num)
        f_layout.addWidget(QtGui.QLabel("CC"), 5, 0)
        f_layout.addWidget(f_cc, 5, 1)
        f_cc_value = QtGui.QSpinBox()
        f_cc_value.setRange(0, 127)
        f_cc_value.setValue(self.default_cc_val)
        f_layout.addWidget(QtGui.QLabel("Value"), 6, 0)
        f_layout.addWidget(f_cc_value, 6, 1)
        f_layout.addWidget(QtGui.QLabel("Start(beats)"), 7, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_start.setValue(self.default_cc_start)
        f_layout.addWidget(f_start, 7, 1)
        f_draw_line_checkbox = QtGui.QCheckBox("Draw line")
        f_layout.addWidget(f_draw_line_checkbox, 9, 1)
        f_layout.addWidget(QtGui.QLabel("End(beats)"), 12, 0)
        f_end = QtGui.QDoubleSpinBox()
        f_end.setRange(0, 3.99)
        f_layout.addWidget(f_end, 12, 1)
        f_layout.addWidget(QtGui.QLabel("End Value"), 13, 0)
        f_end_value = QtGui.QSpinBox()
        f_end_value.setRange(0, 127)
        f_layout.addWidget(f_end_value, 13, 1)
        f_add_another = QtGui.QCheckBox("Add another?")
        f_add_another.toggled.connect(add_another_clicked)
        f_layout.addWidget(f_add_another, 14, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_layout.addWidget(f_ok_button, 15, 0)
        f_ok_button.clicked.connect(cc_ok_handler)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_layout.addWidget(f_cancel_button, 15, 1)
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
            self.open_item()
            this_pydaw_project.git_repo.git_commit("-a", "Update pitchbends for item '" + self.item_name + "'")
            if not f_add_another.isChecked():
                f_window.close()

        def pb_cancel_handler():
            f_window.close()

        def quantize_changed(f_quantize_index):
            f_frac = beat_frac_text_to_float(f_quantize_index)
            f_start.setSingleStep(f_frac)
            self.default_pb_quantize = f_quantize_index

        def add_another_clicked(a_checked):
            if a_checked:
                f_cancel_button.setText("Close")
            else:
                f_cancel_button.setText("Cancel")

        f_window = QtGui.QDialog(this_main_window)
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
        f_add_another.toggled.connect(add_another_clicked)
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
            if self.is_instrument:
                this_pydaw_project.this_dssi_gui.pydaw_set_vol(self.track_number, self.volume_slider.value(), 0)
            else:
                this_pydaw_project.this_dssi_gui.pydaw_set_vol(self.track_number, self.volume_slider.value(), 1)

    def on_vol_released(self):
        if self.is_instrument:
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.git_repo.git_commit("-a", "Set volume for MIDI track " + str(self.track_number) + " to " + str(self.volume_slider.value()))
        else:
            f_tracks = this_pydaw_project.get_bus_tracks()
            f_tracks.busses[self.track_number].vol = self.volume_slider.value()
            this_pydaw_project.save_busses(f_tracks)
            this_pydaw_project.git_repo.git_commit("-a", "Set volume for bus track " + str(self.track_number) + " to " + str(self.volume_slider.value()))
    def on_pan_change(self, value):
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    def on_solo(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_solo(self.track_number, self.solo_checkbox.isChecked(), 0)
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
        this_pydaw_project.git_repo.git_commit("-a", "Set solo for MIDI track " + str(self.track_number) + " to " + str(self.solo_checkbox.isChecked()))
    def on_mute(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_mute(self.track_number, self.mute_checkbox.isChecked(), 0)
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.git_repo.git_commit("-a", "Set mute for MIDI track " + str(self.track_number) + " to " + str(self.mute_checkbox.isChecked()))
    def on_rec(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_track_rec(self.track_number, self.record_radiobutton.isChecked())
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.git_repo.git_commit("-a", "Set rec for MIDI track " + str(self.track_number) + " to " + str(self.record_radiobutton.isChecked()))
    def on_name_changed(self):
        if self.is_instrument:
            self.track_name_lineedit.setText(pydaw_remove_bad_chars(self.track_name_lineedit.text()))
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.this_dssi_gui.pydaw_save_track_name(self.track_number, self.track_name_lineedit.text(), 0)
            this_pydaw_project.git_repo.git_commit("-a", "Set name for MIDI track " + str(self.track_number) + " to " + str(self.track_name_lineedit.text()))
    def on_instrument_change(self, selected_instrument):
        if not self.suppress_osc:
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.this_dssi_gui.pydaw_set_instrument_index(self.track_number, selected_instrument)
            sleep(0.3)
            this_pydaw_project.git_repo.git_commit("-a", "Set instrument for MIDI track " + str(self.track_number) + " to " + str(self.instrument_combobox.currentText()))
    def on_show_ui(self):
        if self.instrument_combobox.currentIndex() > 0:
            this_pydaw_project.this_dssi_gui.pydaw_show_ui(self.track_number)
    def on_show_fx(self):
        if not self.is_instrument or self.instrument_combobox.currentIndex() > 0:
            if self.is_instrument:
                this_pydaw_project.this_dssi_gui.pydaw_show_fx(self.track_number, 0)
            else:
                this_pydaw_project.this_dssi_gui.pydaw_show_fx(self.track_number, 1)
    def on_bus_changed(self, a_value=0):
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
        this_pydaw_project.this_dssi_gui.pydaw_set_bus(self.track_number, self.bus_combobox.currentIndex(), 0)
        this_pydaw_project.git_repo.git_commit("-a", "Set bus for MIDI track " + str(self.track_number) + " to " + str(self.bus_combobox.currentIndex()))

    def __init__(self, a_track_num, a_track_text="track", a_instrument=True):
        self.is_instrument = a_instrument
        self.suppress_osc = True
        self.track_number = a_track_num
        self.group_box = QtGui.QWidget()
        self.group_box.setAutoFillBackground(True)
        self.group_box.setPalette(QtGui.QPalette(QtCore.Qt.black))
        #self.group_box.setObjectName("seqtrack")
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
        self.fx_button = QtGui.QPushButton("FX")
        self.fx_button.pressed.connect(self.on_show_fx)
        self.fx_button.setObjectName("fxbutton")
        self.fx_button.setMinimumWidth(24)
        self.fx_button.setMaximumWidth(24)
        if a_instrument:
            self.instrument_combobox = QtGui.QComboBox()
            self.instrument_combobox.addItems(["None", "Euphoria", "Ray-V", "Way-V"])
            self.instrument_combobox.currentIndexChanged.connect(self.on_instrument_change)
            self.instrument_combobox.setSizeAdjustPolicy(QtGui.QComboBox.AdjustToMinimumContentsLengthWithIcon)
            self.instrument_combobox.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
            self.hlayout3.addWidget(self.instrument_combobox)
            self.ui_button = QtGui.QPushButton("UI")
            self.ui_button.pressed.connect(self.on_show_ui)
            self.ui_button.setObjectName("uibutton")
            self.ui_button.setMinimumWidth(24)
            self.ui_button.setMaximumWidth(24)
            self.hlayout3.addWidget(self.ui_button)
            self.bus_combobox = QtGui.QComboBox()
            self.bus_combobox.addItems(['M', '1','2','3','4'])
            self.bus_combobox.setMinimumWidth(54)
            self.bus_combobox.currentIndexChanged.connect(self.on_bus_changed)
            self.hlayout2.addWidget(QtGui.QLabel("Bus:"))
            self.hlayout2.addWidget(self.bus_combobox)
            self.hlayout3.addWidget(self.fx_button)
            self.solo_checkbox = QtGui.QCheckBox()
            self.solo_checkbox.clicked.connect(self.on_solo)
            self.solo_checkbox.setObjectName("solo_checkbox")
            self.hlayout3.addWidget(self.solo_checkbox)
            self.mute_checkbox = QtGui.QCheckBox()
            self.mute_checkbox.clicked.connect(self.on_mute)
            self.mute_checkbox.setObjectName("mute_checkbox")
            self.hlayout3.addWidget(self.mute_checkbox)
            self.record_radiobutton = QtGui.QRadioButton()
            rec_button_group.addButton(self.record_radiobutton)
            self.record_radiobutton.toggled.connect(self.on_rec)
            self.record_radiobutton.setObjectName("rec_arm_radiobutton")
            self.hlayout3.addWidget(self.record_radiobutton)
        else:
            self.track_name_lineedit.setReadOnly(True)
            self.hlayout3.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
            self.hlayout3.addWidget(self.fx_button)

        self.suppress_osc = False

    def open_track(self, a_track, a_notify_osc=False):
        if not a_notify_osc:
            self.suppress_osc = True
        self.volume_slider.setValue(a_track.vol)
        if self.is_instrument:
            self.record_radiobutton.setChecked(a_track.rec)
            self.track_name_lineedit.setText(a_track.name)
            self.instrument_combobox.setCurrentIndex(a_track.inst)
            self.solo_checkbox.setChecked(a_track.solo)
            self.mute_checkbox.setChecked(a_track.mute)
            self.bus_combobox.setCurrentIndex(a_track.bus_num)
        self.suppress_osc = False

    def get_track(self):
        if self.is_instrument:
            return pydaw_track(self.solo_checkbox.isChecked(), self.mute_checkbox.isChecked(), self.record_radiobutton.isChecked(),
                           self.volume_slider.value(), str(self.track_name_lineedit.text()), self.instrument_combobox.currentIndex(), self.bus_combobox.currentIndex())
        else:
            return pydaw_track(False, False, self.record_radiobutton.isChecked(),
                           self.volume_slider.value(), str(self.track_name_lineedit.text()), -1)

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
    def on_spacebar(self):
        if self.is_playing or self.is_recording:
            self.stop_button.click()
        else:
            self.play_button.click()
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

    def show_audio_recording_dialog(self):
        f_inputs = this_pydaw_project.get_audio_input_tracks()
        f_audio_items = this_pydaw_project.get_audio_items()
        f_samplegraphs = this_pydaw_project.get_samplegraphs()
        f_result_list = []
        for f_i in range(5):
            if f_inputs.tracks[f_i].rec == 1:
                if os.path.isfile(this_pydaw_project.audio_tmp_folder + "/" + str(f_i) + ".wav"):
                    f_result_list.append(f_i)
                else:
                    print("Error:  Track " + str(f_i) + " was record-armed, but no .wav found")
        if len(f_result_list) > 0:
            for f_item_number in f_result_list:
                def ok_handler():
                    f_file_name = str(f_name.text())
                    if f_file_name is None or f_file_name == "":
                        QtGui.QMessageBox.warning(f_window, "Error", "You must select a name for the file")
                        return
                    os.system('mv "' + this_pydaw_project.audio_tmp_folder + "/" + str(f_item_number) + '.wav" "' + f_file_name + '"')
                    if f_next_index != -1 and f_import.isChecked():
                        f_audio_items.add_item(f_next_index, pydaw_audio_item(f_file_name, 0, 1000, self.last_region_num, self.last_bar, \
                        0.0, 0, 0, 0, 0.0, 0, 0.0, f_inputs.tracks[f_item_number].output, 0))
                    this_pydaw_project.save_audio_items(f_audio_items)
                    this_pydaw_project.git_repo.git_commit("-a", "Record audio item " + f_file_name)
                    f_sg_uid = pydaw_gen_uid()
                    this_pydaw_project.this_dssi_gui.pydaw_generate_sample_graph(f_file_name, f_sg_uid)
                    f_samplegraphs.add_ref(f_file_name, f_sg_uid)
                    this_pydaw_project.save_samplegraphs(f_samplegraphs)
                    f_window.close()

                def cancel_handler():
                    os.system('rm "' + this_pydaw_project.audio_tmp_folder + "/" + str(f_item_number) + '.wav"')
                    f_window.close()

                def file_dialog():
                    f_file_name = str(QtGui.QFileDialog.getSaveFileName(f_window, "Select a file name to save to...", self.last_open_dir, filter=".wav files(*.wav)"))
                    if not f_file_name is None and not f_file_name == "":
                        if not f_file_name.endswith(".wav"):
                            f_file_name = f_file_name + ".wav"
                        f_name.setText(f_file_name)
                        self.last_open_dir = os.path.dirname(f_file_name)

                f_next_index = f_audio_items.get_next_index()

                f_window = QtGui.QDialog(this_main_window)
                f_window.setWindowTitle("Save recorded file for audio input " + str(f_item_number))
                f_layout = QtGui.QGridLayout()
                f_window.setLayout(f_layout)
                f_layout.addWidget(QtGui.QLabel("File Name"), 0, 0)
                f_name = QtGui.QLineEdit()
                f_layout.addWidget(f_name, 0, 1)
                f_name.setReadOnly(True)
                f_name.setMinimumWidth(420)
                f_file_button = QtGui.QPushButton("Select File Name...")
                f_file_button.clicked.connect(file_dialog)
                f_layout.addWidget(f_file_button, 0, 2)
                if f_next_index == -1:
                    f_layout.addWidget(QtGui.QLabel("Cannot import into project, no available slots in item tab."), 1, 1)
                else:
                    f_import = QtGui.QCheckBox("Import into project?")
                    f_import.setChecked(True)
                    f_layout.addWidget(f_import, 1, 1)
                f_ok_button = QtGui.QPushButton("Save File")
                f_ok_button.clicked.connect(ok_handler)
                f_layout.addWidget(f_ok_button, 8,2)
                f_cancel_button = QtGui.QPushButton("Discard File")
                f_layout.addWidget(f_cancel_button, 8,3)
                f_cancel_button.clicked.connect(cancel_handler)
                f_window.exec_()
        this_audio_editor.open_items()

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
            self.show_audio_recording_dialog()
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
            this_pydaw_project.git_repo.git_commit("-a", "Set project tempo to " + str(a_tempo))
            this_audio_items_viewer.set_bpm(a_tempo)
            this_audio_editor.open_items()
    def on_loop_mode_changed(self, a_loop_mode):
        if not self.suppress_osc:
            this_pydaw_project.this_dssi_gui.pydaw_set_loop_mode(a_loop_mode)
            self.transport.loop_mode = a_loop_mode
            this_pydaw_project.save_transport(self.transport)
            this_pydaw_project.git_repo.git_commit("-a", "Set project loop mode to " + str(self.loop_mode_combobox.itemText(a_loop_mode)))
    def on_keybd_combobox_index_changed(self, a_index):
        self.alsa_output_ports.connect_to_pydaw(str(self.keybd_combobox.currentText()))
        if not self.suppress_osc:
            self.transport.midi_keybd = str(self.keybd_combobox.currentText())
            this_pydaw_project.save_transport(self.transport)
            this_pydaw_project.git_repo.git_commit("-a", "Set project MIDI in device to " + str(self.keybd_combobox.itemText(a_index)))
    def on_bar_changed(self, a_bar):
        self.transport.bar = a_bar
        if not self.suppress_osc and not self.is_playing and not self.is_recording:
            this_pydaw_project.save_transport(self.transport)
            this_pydaw_project.git_repo.git_commit("-a", "Set project playback bar to " + str(a_bar))
    def on_region_changed(self, a_region):
        self.transport.region = a_region
        if not self.is_playing and not self.is_recording:
            this_pydaw_project.save_transport(self.transport)
            this_pydaw_project.git_repo.git_commit("-a", "Set project playback bar to " + str(a_region))
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
        f_region_item = this_song_editor.table_widget.item(0, int(self.transport.region))
        if f_region_item and str(f_region_item.text()) != "":
            this_song_editor.table_widget.setItemSelected(f_region_item, True)
            this_region_editor.open_region(str(f_region_item.text()))
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
        self.last_open_dir = expanduser("~")
        self.transport = pydaw_transport()
        self.group_box = QtGui.QGroupBox()
        self.vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.vlayout)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.hlayout2 = QtGui.QHBoxLayout()
        self.vlayout.addLayout(self.hlayout1)
        self.vlayout.addLayout(self.hlayout2)
        self.play_button = QtGui.QRadioButton()
        self.play_button.setObjectName("play_button")
        self.play_button.clicked.connect(self.on_play)
        self.hlayout1.addWidget(self.play_button)
        self.stop_button = QtGui.QRadioButton()
        self.stop_button.setObjectName("stop_button")
        self.stop_button.clicked.connect(self.on_stop)
        self.hlayout1.addWidget(self.stop_button)
        self.rec_button = QtGui.QRadioButton()
        self.rec_button.setObjectName("rec_button")
        self.rec_button.clicked.connect(self.on_rec)
        self.hlayout1.addWidget(self.rec_button)
        self.hlayout1.addWidget(QtGui.QLabel("BPM:"))
        self.tempo_spinbox = QtGui.QSpinBox()
        self.tempo_spinbox.setObjectName("large_spinbox")
        self.tempo_spinbox.setRange(50, 200)
        self.tempo_spinbox.valueChanged.connect(self.on_tempo_changed)
        self.hlayout1.addWidget(self.tempo_spinbox)
        self.hlayout1.addWidget(QtGui.QLabel("Region:"))
        self.region_spinbox = QtGui.QSpinBox()
        self.region_spinbox.setObjectName("large_spinbox")
        self.region_spinbox.setRange(0, 300)
        self.region_spinbox.valueChanged.connect(self.on_region_changed)
        self.hlayout1.addWidget(self.region_spinbox)
        self.hlayout1.addWidget(QtGui.QLabel("Bar:"))
        self.bar_spinbox = QtGui.QSpinBox()
        self.bar_spinbox.setObjectName("large_spinbox")
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
        f_lower_ctrl_layout.addWidget(self.scope_button)
        f_loop_midi_gridlayout.addLayout(f_lower_ctrl_layout, 1, 1)
        self.hlayout1.addLayout(f_loop_midi_gridlayout)
        #This is an awful way to do this, I'll eventually have IPC that goes both directions...
        self.beat_timer = QtCore.QTimer()
        self.beat_timer.timeout.connect(self.beat_timeout)
        self.suppress_osc = False

class pydaw_main_window(QtGui.QMainWindow):
    def on_new(self):
        f_file = QtGui.QFileDialog.getSaveFileName(parent=self ,caption='New Project', directory=expanduser("~"), filter=global_pydaw_file_type_string)
        if not f_file is None and not str(f_file) == "":
            f_file = str(f_file)
            if not f_file.endswith("." + global_pydaw_version_string):
                f_file += "." + global_pydaw_version_string
            global_new_project(f_file)
    def on_open(self):
        f_file = QtGui.QFileDialog.getOpenFileName(parent=self ,caption='Open Project', directory=expanduser("~"), filter=global_pydaw_file_type_string)
        if not f_file is None and not str(f_file) == "":
            global_open_project(str(f_file))
    def on_save(self):
        this_pydaw_project.save_project()
    def on_save_as(self):
        f_new_file = QtGui.QFileDialog.getSaveFileName(self, "Save project as...", directory=expanduser("~") + "/" + this_pydaw_project.project_file + "." + global_pydaw_version_string)
        if not f_new_file is None and not str(f_new_file) == "":
            f_new_file = str(f_new_file)
            if not f_new_file.endswith("." + global_pydaw_version_string):
                f_new_file += "." + global_pydaw_version_string
            this_pydaw_project.save_project_as(f_new_file)
            set_window_title()
            set_default_project(f_new_file)

    def show_offline_rendering_wait_window(self, a_file_name):
        f_file_name = str(a_file_name) + ".finished"
        def ok_handler():
            f_window.close()

        def cancel_handler():
            f_window.close()

        def timeout_handler():
            if os.path.isfile(f_file_name):
                f_ok.setEnabled(True)
                f_timer.stop()
                f_time_label.setText("Finished in " + str(f_time_label.text()))
                os.system('rm "' + f_file_name + '"')
            else:
                f_elapsed_time = time() - f_start_time
                f_time_label.setText(str(round(f_elapsed_time, 1)))

        f_start_time = time()
        f_window = QtGui.QDialog(this_main_window)
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
            ((f_end_region.value() == f_start_region.value()) and (f_start_bar.value() >= f_end_bar.value())):
                QtGui.QMessageBox.warning(f_window, "Error", "End point is before start point.")
                return

            #TODO:  Check that the end is actually after the start....
            this_pydaw_project.this_dssi_gui.pydaw_offline_render(f_start_region.value(), f_start_bar.value(), f_end_region.value(), f_end_bar.value(), f_name.text())
            f_window.close()
            self.show_offline_rendering_wait_window(f_name.text())

        def cancel_handler():
            f_window.close()

        def file_name_select():
            f_file_name = str(QtGui.QFileDialog.getSaveFileName(f_window, "Select a file name to save to...", self.last_offline_dir))
            if not f_file_name is None and f_file_name != "":
                if not f_file_name.endswith(".wav"):
                    f_file_name += ".wav"
                if not f_file_name is None and not str(f_file_name) == "":
                    f_name.setText(f_file_name)
                self.last_offline_dir = os.path.dirname(f_file_name)

        f_start_reg = 0
        f_end_reg = 0

        for i in range(300):
            f_item = this_song_editor.table_widget.item(0, i)
            if not f_item is None and f_item.text() != "":
                f_start_reg = i
                break

        for i in range(f_start_reg + 1, 300):
            f_item = this_song_editor.table_widget.item(0, i)
            if f_item is None or f_item.text() == "":
                f_end_reg = i
                break

        f_window = QtGui.QDialog(this_main_window)
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
        self.last_offline_dir = expanduser("~")
        f_window.exec_()

    def on_undo(self):
        this_pydaw_project.git_repo.undo()
        global_ui_refresh_callback()

    def on_undo_history(self):
        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Undo history")
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_widget = pydaw_git_log_widget(this_pydaw_project.git_repo, global_ui_refresh_callback)
        f_widget.populate_table()
        f_layout.addWidget(f_widget)
        f_window.setGeometry(QtCore.QRect(f_window.x(), f_window.y(), 900, 720))
        f_window.exec_()

    def on_open_theme(self):
        f_file = str(QtGui.QFileDialog.getOpenFileName(self, "Open a theme file", "/usr/lib/pydaw2/themes", "PyDAW Style(style.txt)"))
        if not f_file is None and not f_file == "":
            f_style = pydaw_read_file_text(f_file)
            pydaw_write_file_text(self.user_style_file, f_file)
            self.setStyleSheet(f_style)

    def on_version(self):
        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Version Info")
        f_window.setFixedSize(150, 80)
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_version = QtGui.QLabel(pydaw_read_file_text("/usr/lib/pydaw2/pydaw2-version.txt"))
        #f_git_version = QtGui.QLabel("git revision")
        f_layout.addWidget(f_version)
        #f_layout.addWidget(f_git_version)
        f_window.exec_()

    def on_user_manual(self):
        QtGui.QDesktopServices.openUrl(QtCore.QUrl("http://pydaw.org/wiki/index.php?title=PyDAW2_Manual"))

    def on_website(self):
        QtGui.QDesktopServices.openUrl(QtCore.QUrl("http://pydaw.org/"))

    def show_help_file(self, a_file):
        f_window = QtGui.QDialog(this_main_window)
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

    def on_spacebar(self):
        this_transport.on_spacebar()

    def tab_changed(self):
        if this_audio_editor.delete_radiobutton.isChecked():
            this_audio_editor.add_radiobutton.setChecked(True)
        if this_item_editor.delete_radiobutton.isChecked():
            this_item_editor.add_radiobutton.setChecked(True)

    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        self.setObjectName("mainwindow")
        default_stylesheet_file = "/usr/lib/pydaw2/themes/default/style.txt"
        self.user_style_file = expanduser("~") + "/pydaw2/default-style.txt"
        if os.path.isfile(self.user_style_file):
            f_current_style_file_text = pydaw_read_file_text(self.user_style_file)
            if os.path.isfile(f_current_style_file_text):
                f_style = pydaw_read_file_text(f_current_style_file_text)
            else:
                f_style = pydaw_read_file_text(default_stylesheet_file)
        else:
            f_style = pydaw_read_file_text(default_stylesheet_file)

        self.setStyleSheet(f_style)

        self.central_widget = QtGui.QWidget()
        self.setCentralWidget(self.central_widget)

        self.main_layout = QtGui.QVBoxLayout()
        self.central_widget.setLayout(self.main_layout)

        self.spacebar_action = QtGui.QAction(self)
        self.addAction(self.spacebar_action)
        self.spacebar_action.triggered.connect(self.on_spacebar)
        self.spacebar_action.setShortcut(QtGui.QKeySequence(QtCore.Qt.Key_Space))

        #The context menus
        self.menu_bar = self.menuBar()
        self.menu_file = self.menu_bar.addMenu("&File")

        self.new_action = QtGui.QAction("New", self)
        self.menu_file.addAction(self.new_action)
        self.new_action.triggered.connect(self.on_new)
        self.new_action.setShortcut(QtGui.QKeySequence.New)

        self.open_action = QtGui.QAction("Open", self)
        self.menu_file.addAction(self.open_action)
        self.open_action.triggered.connect(self.on_open)
        self.open_action.setShortcut(QtGui.QKeySequence.Open)

        self.save_action = QtGui.QAction("Save", self)
        self.menu_file.addAction(self.save_action)
        self.save_action.triggered.connect(self.on_save)
        self.save_action.setShortcut(QtGui.QKeySequence.Save)

        self.save_as_action = QtGui.QAction("Save As...", self)
        self.menu_file.addAction(self.save_as_action)
        self.save_as_action.triggered.connect(self.on_save_as)
        self.save_as_action.setShortcut(QtGui.QKeySequence.SaveAs)

        self.offline_render_action = QtGui.QAction("Offline Render...", self)
        self.menu_file.addAction(self.offline_render_action)
        self.offline_render_action.triggered.connect(self.on_offline_render)

        self.menu_edit = self.menu_bar.addMenu("&Edit")

        #self.undo_action = QtGui.QAction("Undo", self)
        #self.menu_edit.addAction(self.undo_action)
        #self.undo_action.triggered.connect(self.on_undo)
        #self.undo_action.setShortcut(QtGui.QKeySequence.Undo)

        self.undo_history_action = QtGui.QAction("Undo History...", self)
        self.menu_edit.addAction(self.undo_history_action)
        self.undo_history_action.triggered.connect(self.on_undo_history)
        self.undo_history_action.setShortcut(QtGui.QKeySequence.Undo)

        self.menu_appearance = self.menu_bar.addMenu("&Appearance")

        self.open_theme_action = QtGui.QAction("Open Theme...", self)
        self.menu_appearance.addAction(self.open_theme_action)
        self.open_theme_action.triggered.connect(self.on_open_theme)

        self.menu_help = self.menu_bar.addMenu("&Help")

        self.website_action = QtGui.QAction("PyDAW Website...", self)
        self.menu_help.addAction(self.website_action)
        self.website_action.triggered.connect(self.on_website)

        self.manual_action = QtGui.QAction("Online User Manual...", self)
        self.menu_help.addAction(self.manual_action)
        self.manual_action.triggered.connect(self.on_user_manual)

        self.version_action = QtGui.QAction("Version Info...", self)
        self.menu_help.addAction(self.version_action)
        self.version_action.triggered.connect(self.on_version)

        self.transport_hlayout = QtGui.QHBoxLayout()
        self.main_layout.addLayout(self.transport_hlayout)

        self.transport_hlayout.addWidget(this_transport.group_box, alignment=QtCore.Qt.AlignLeft)

        #The tabs
        self.main_tabwidget = QtGui.QTabWidget()

        self.main_tabwidget.currentChanged.connect(self.tab_changed)

        self.main_layout.addWidget(self.main_tabwidget)

        self.main_tabwidget.addTab(this_region_editor.group_box, "Song/MIDI")
        this_region_editor.main_vlayout.addWidget(this_song_editor.table_widget, 0, 0)

        self.main_tabwidget.addTab(this_item_editor.widget, "MIDI Item")

        self.main_tabwidget.addTab(this_audio_editor.group_box, "Tracks")

        self.audio_items_tab = QtGui.QTabWidget()

        self.audio_items_tab.addTab(this_audio_items_viewer_widget.widget, "Viewer")
        self.audio_items_tab.addTab(this_audio_editor.items_groupbox, "Item List")

        self.main_tabwidget.addTab(self.audio_items_tab, "Audio Items")

        self.main_tabwidget.addTab(this_audio_editor.ccs_tab, "Automation")

        #Begin CC Map tab
        self.cc_map_tab = QtGui.QWidget()
        self.cc_map_tab.setObjectName("ccmaptabwidget")
        f_cc_map_main_vlayout = QtGui.QVBoxLayout(self.cc_map_tab)
        f_cc_map_label = QtGui.QLabel(
        """Below you can edit the MIDI CC maps for PyDAW's plugins. All CCs are sent to both Ray-V/Euphoria/Way-V and Modulex,
so the first 3 CC maps can overlap each other, but none of them should overlap with Modulex.
You must restart PyDAW for changes to the CC maps to take effect.

IMPORTANT:  Changing these will affect any existing CC automation you have in any projects.  The next major release
of PyDAW will not have this limitation, and will feature MIDI learn and the ability to automate parameters by
name instead of MIDI CC number""")
        f_cc_map_label.setMaximumWidth(1200)
        f_cc_map_main_vlayout.addWidget(f_cc_map_label)
        f_cc_map_hlayout = QtGui.QHBoxLayout()
        f_cc_map_main_vlayout.addLayout(f_cc_map_hlayout)
        self.cc_map_rayv = pydaw_cc_map_editor(2)
        f_cc_map_hlayout.addWidget(self.cc_map_rayv.groupbox)
        self.cc_map_euphoria = pydaw_cc_map_editor(1)
        f_cc_map_hlayout.addWidget(self.cc_map_euphoria.groupbox)
        self.cc_map_wayv = pydaw_cc_map_editor(3)
        f_cc_map_hlayout.addWidget(self.cc_map_wayv.groupbox)
        self.cc_map_modulex = pydaw_cc_map_editor(-1)
        f_cc_map_hlayout.addWidget(self.cc_map_modulex.groupbox)
        #f_cc_map_hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum))
        self.cc_map_scrollarea = QtGui.QScrollArea()
        self.cc_map_tab.setMinimumWidth(440 * 4)
        self.cc_map_tab.setMinimumHeight(4100)
        self.cc_map_scrollarea.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOn)
        self.cc_map_scrollarea.setWidget(self.cc_map_tab)
        self.main_tabwidget.addTab(self.cc_map_scrollarea, "CC Maps")

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

global_cc_maps = {"Euphoria":{}, "Way-V":{}, "Ray-V":{}, "Modulex":{}}  #A dict of dicts, with key/value pair:  control-name:control-value

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

        f_default_cc_num = int(self.cc_table.item(x, 0).text())

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle("Set CC for Control")
        f_window.setMinimumWidth(240)
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
            self.file_name = expanduser("~") + "/" + global_pydaw_version_string + "/lms_modulex-cc_map.txt"
            if not os.path.isfile(self.file_name):
                copyfile("/usr/lib/pydaw2/cc_maps/lms_modulex-cc_map.txt", self.file_name)
        elif a_index == 1:
            f_name = "Euphoria"
            self.file_name = expanduser("~") + "/" + global_pydaw_version_string + "/euphoria-cc_map.txt"
            if not os.path.isfile(self.file_name):
                copyfile("/usr/lib/pydaw2/cc_maps/euphoria-cc_map.txt", self.file_name)
        elif a_index == 2:
            f_name = "Ray-V"
            self.file_name = expanduser("~") + "/" + global_pydaw_version_string + "/ray_v-cc_map.txt"
            if not os.path.isfile(self.file_name):
                copyfile("/usr/lib/pydaw2/cc_maps/ray_v-cc_map.txt", self.file_name)
        elif a_index == 3:
            f_name = "Way-V"
            self.file_name = expanduser("~") + "/" + global_pydaw_version_string + "/way_v-cc_map.txt"
            if not os.path.isfile(self.file_name):
                copyfile("/usr/lib/pydaw2/cc_maps/way_v-cc_map.txt", self.file_name)
        self.groupbox = QtGui.QGroupBox(f_name)
        self.groupbox.setMaximumWidth(420)
        f_vlayout = QtGui.QVBoxLayout(self.groupbox)

        f_button_layout = QtGui.QHBoxLayout()
        f_vlayout.addLayout(f_button_layout)
        f_button_spacer = QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        f_button_layout.addItem(f_button_spacer)
        f_save_button = QtGui.QPushButton("Save")
        f_save_button.pressed.connect(self.on_save)
        f_button_layout.addWidget(f_save_button)

        self.cc_table = QtGui.QTableWidget(127, 3)
        self.cc_table.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.cc_table.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.cc_table.setHorizontalHeaderLabels(["CC", "Description", "LADSPA Port"])
        self.cc_table.cellClicked.connect(self.on_click)
        f_vlayout.addWidget(self.cc_table)

        try:
            f_cc_map_text = open(self.file_name, "r").read()
        except:
            return  #If we can't open the file, then just return

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
                    global_cc_maps[f_name][f_desc] = f_cc_num
                    f_row_index += 1

def set_default_project(a_project_path):
    f_def_file = expanduser("~") + "/" + global_pydaw_version_string + "/last-project.txt"
    f_handle = open(f_def_file, 'w')
    f_handle.write(str(a_project_path))
    f_handle.close()

def global_close_all():
    this_region_editor.clear_new()
    this_item_editor.clear_new()
    this_song_editor.table_widget.clearContents()
    this_audio_editor.audio_items_table_widget.clearContents()

def global_ui_refresh_callback():
    """ Use this to re-open all existing items/regions/song in their editors when the files have been changed externally"""
    if this_item_editor.enabled and os.path.isfile(this_pydaw_project.items_folder + "/" + this_item_editor.item_name + ".pyitem"):
        this_item_editor.open_item()
    else:
        this_item_editor.clear_new()
    if this_region_editor.enabled and os.path.isfile(this_pydaw_project.regions_folder + "/" + str(this_region_editor.region_name_lineedit.text()) + ".pyreg"):
        this_region_editor.open_region(this_region_editor.region_name_lineedit.text())
    else:
        this_region_editor.clear_new()
    this_audio_editor.open_items()
    this_audio_editor.open_tracks()
    this_audio_editor.automation_track_type_changed()
    this_region_editor.open_tracks()
    this_song_editor.open_song()
    this_transport.open_transport()
    this_pydaw_project.this_dssi_gui.pydaw_open_song(this_pydaw_project.project_folder)  #Re-open the project so that any changes can be caught by the back-end

def set_window_title():
    this_main_window.setWindowTitle('PyDAW2 - ' + this_pydaw_project.project_folder + "/" + this_pydaw_project.project_file + "." + global_pydaw_version_string)
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
    this_audio_editor.open_items()
    this_audio_editor.open_tracks()
    this_audio_editor.automation_track_type_changed()
    global_update_audio_track_comboboxes()
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
    #this_pydaw_project.save_project()  #Commenting out because AFAIK there would be nothing to save???
    this_song_editor.open_song()
    this_pydaw_project.save_song(this_song_editor.song)
    this_audio_editor.open_items()
    this_audio_editor.open_tracks()
    this_audio_editor.automation_track_type_changed()
    this_transport.open_transport()
    set_default_project(a_project_file)
    global_update_audio_track_comboboxes()
    set_window_title()

def about_to_quit():
    this_pydaw_project.quit_handler()

app = QtGui.QApplication(sys.argv)

global_timestretch_modes = ["None", "Pitch(affecting time)", "Time(affecting pitch"]
global_audio_track_names = {0:"track1", 1:"track2", 2:"track3", 3:"track4", 4:"track5", 5:"track6", 6:"track7", 7:"track8"}
global_suppress_audio_track_combobox_changes = False
global_audio_track_comboboxes = []
global_ai_sg_at_combobox = QtGui.QComboBox()
global_audio_track_comboboxes.append(global_ai_sg_at_combobox)

app.setWindowIcon(QtGui.QIcon('/usr/share/pixmaps/pydaw2.png'))
app.aboutToQuit.connect(about_to_quit)

this_pb_automation_viewer = automation_viewer(a_is_cc=False)
this_cc_automation_viewers = []
this_cc_automation_viewer0 = automation_viewer()
this_cc_automation_viewer1 = automation_viewer()
this_cc_automation_viewer2 = automation_viewer()
this_cc_automation_viewers.append(this_cc_automation_viewer0)
this_cc_automation_viewers.append(this_cc_automation_viewer1)
this_cc_automation_viewers.append(this_cc_automation_viewer2)

this_song_editor = song_editor()
this_region_editor = region_list_editor()
this_audio_editor = audio_list_editor()
this_piano_roll_editor = piano_roll_editor()
this_piano_roll_editor_widget = piano_roll_editor_widget()
this_item_editor = item_list_editor()
this_transport = transport_widget()
this_audio_items_viewer = audio_items_viewer()
this_audio_items_viewer_widget = audio_items_viewer_widget()

this_main_window = pydaw_main_window() #You must call this after instantiating the other widgets, as it relies on them existing
this_main_window.setWindowState(QtCore.Qt.WindowMaximized)
this_piano_roll_editor.verticalScrollBar().setSliderPosition(700)

for f_viewer in this_item_editor.cc_auto_viewers:  #Get the plugin/control comboboxes populated
    f_viewer.plugin_changed()
this_item_editor.cc_auto_viewers[1].set_cc_num(2)
this_item_editor.cc_auto_viewers[2].set_cc_num(3)

f_def_file = expanduser("~") + "/" + global_pydaw_version_string + "/last-project.txt"
if os.path.exists(f_def_file):
    f_handle = open(f_def_file, 'r')
    default_project_file = f_handle.read()
    f_handle.close()
    if not os.path.exists(default_project_file):
        default_project_file = expanduser("~") + "/" + global_pydaw_version_string + "/default-project/default." + global_pydaw_version_string
else:
    default_project_file = expanduser("~") + "/" + global_pydaw_version_string + "/default-project/default." + global_pydaw_version_string
if os.path.exists(default_project_file):
    global_open_project(default_project_file)
else:
    global_new_project(default_project_file)

sys.exit(app.exec_())

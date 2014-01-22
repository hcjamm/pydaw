#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import sys
import os
import operator
import subprocess
from time import sleep
import time
import random
from PyQt4 import QtGui, QtCore
from libpydaw import *
import libpydaw.liblo as liblo

from libpydaw.pydaw_util import *

import locale
import gettext

try:
    global_locale, global_encoding = locale.getdefaultlocale()
    global_language = gettext.translation("pydaw4",
        "{}/share/locale".format(pydaw_util.global_pydaw_install_prefix),
        [global_locale])
    global_language.install()
except Exception as ex:
    print("Exception while setting locale, falling back to English (hopefully)")
    def _(a_string): return a_string

global_transport_is_playing = False
global_region_lengths_dict = {}
global_audio_region_snap_px = {}
global_bar_count = 300 * 8

class pydaw_track_type_enum:
    midi = 0
    bus = 1
    audio = 2

def pydaw_update_region_lengths_dict():
    """ Call this any time the region length setup may have changed... """
    f_song = this_pydaw_project.get_song()
    global global_region_lengths_dict, global_audio_region_snap_px, global_bar_count
    global_region_lengths_dict = {}
    global_audio_region_snap_px = {}
    global_bar_count = 300 * 8
    for k, v in list(f_song.regions.items()):
        f_region = this_pydaw_project.get_region_by_uid(v)
        if f_region.region_length_bars != 0:
            global_region_lengths_dict[int(k)] = int(f_region.region_length_bars)
            global_bar_count = global_bar_count - 8 + int(f_region.region_length_bars)
    f_add = 0.0
    global_audio_region_snap_px[0] = 0.0
    for i in range(299):
        f_value = pydaw_get_region_length(i) * global_audio_px_per_bar
        global_audio_region_snap_px[i + 1] = f_value + f_add
        f_add += f_value

def pydaw_get_region_length(a_region_index):
    """ Get the length of the region at song index a_region_index from the cache """
    f_region_index = int(a_region_index)
    if f_region_index in global_region_lengths_dict:
        return global_region_lengths_dict[f_region_index]
    else:
        return 8

def pydaw_get_current_region_length():
    if global_current_region is None:
        return 8
    f_result = global_current_region.region_length_bars
    if f_result == 0:
        return 8
    else:
        return f_result

def pydaw_get_pos_in_bars(a_reg, a_bar, a_beat):
    f_result = 0.0
    for i in range(a_reg):
        f_result += pydaw_get_region_length(i)
    f_result += (a_bar) + (a_beat * 0.25)
    return f_result

def pydaw_bars_to_pos(a_bars):
    """ Convert a raw bar count to region, bar """
    f_bar_total = 0
    for i in range(299):
        f_bar_count = pydaw_get_region_length(i)
        if f_bar_total + f_bar_count > a_bars:
            return i, a_bars - f_bar_total
        else:
            f_bar_total += f_bar_count
    assert(False)

def pydaw_get_diff_in_bars(a_start_reg, a_start_bar, a_start_beat, a_end_reg, a_end_bar,
                           a_end_beat):
    """ Calculate the difference in bars, ie: 10.567 bars, between 2 points """
    f_result = 0.0
    if a_start_reg <= a_end_reg:
        for i in range(a_start_reg, a_end_reg):
            f_result += pydaw_get_region_length(i)
        f_result += (a_end_bar - a_start_bar)
        f_result += (a_end_beat - a_start_beat) * 0.25
    else:
        for i in range(a_start_reg, a_end_reg, -1):
            f_result -= pydaw_get_region_length(i)
        f_result += (a_start_bar - a_end_bar)
        f_result += (a_start_beat - a_end_beat) * 0.25
    return f_result

def pydaw_add_diff_in_bars(a_start_reg, a_start_bar, a_start_beat, a_bars):
    """ Add a fractional number of bars, and return a tuple of region, bar and beat """
    f_region = a_start_reg
    f_bar = a_start_bar
    f_beat = a_start_beat
    f_int_bars = int(a_bars)
    f_float_beats = (a_bars - float(f_int_bars)) * 4.0
    f_beat += f_float_beats
    if f_beat >= 4.0:
        f_beat -= 4.0
        f_bar += 1
    f_beat = round(f_beat, 4)
    f_bar += f_int_bars
    while True:
        f_reg_length = pydaw_get_region_length(f_region)
        if f_bar >= f_reg_length:
            f_bar -= f_reg_length
            f_region += 1
        else:
            break
    return (f_region, f_bar, f_beat)

def pydaw_center_widget_on_screen(a_widget):
    f_desktop_center = QtGui.QApplication.desktop().screen().rect().center()
    f_widget_center = a_widget.rect().center()
    f_x = pydaw_clip_value(f_desktop_center.x() - f_widget_center.x(), 0, 300)
    f_y = pydaw_clip_value(f_desktop_center.y() - f_widget_center.y(), 0, 200)
    a_widget.move(f_x, f_y)


def pydaw_print_generic_exception(a_ex):
    QtGui.QMessageBox.warning(this_main_window, _("Warning"), _("The following error happened:\n{}"
    "\nIf you are running PyDAW from a USB flash drive, this may be because file IO timed out "
    "due to the slow nature of flash drives.  If the problem persists, you should consider "
    "installing PyDAW-OS to your hard drive instead").format(a_ex))

global_tooltips_enabled = False

def pydaw_set_tooltips_enabled(a_enabled):
    """ Set extensive tooltips as an alternative to maintaining a separate user manual """
    global global_tooltips_enabled
    global_tooltips_enabled = a_enabled
    for f_automation_editor in global_automation_editors:
            f_automation_editor.set_tooltip(a_enabled)
    if a_enabled:
        pydaw_write_file_text("{}/tooltips.txt".format(global_pydaw_home), "True")
        this_song_editor.table_widget.setToolTip(
        _("This is the song editor.  A song is a timeline "
        "consisting of regions,\nclick here to add a region, click and drag to move a region, or "
        "press the 'delete' button to delete\nthe selected regions.  Click on a region to edit it "
        "in the region editor below.\n\nClick the 'tooltips' checkbox in the transport to disable "
        "these tooltips"))
        for f_region_editor in global_region_editors:
            f_region_editor.table_widget.setToolTip(_("This is a region editor, it consists of "
            "items and tracks.\nA track is either a plugin instrument, audio track or bus track.\n"
            "An item is one bar of MIDI notes or plugin automation.  Click an empty cell to add "
            "a new item\nDouble click an item to open it in the piano-roll-editor or select "
            "multiple and right-click->'edit multiple items as group'\n\n"
            "The selected items can be copied by pressing CTRL+C, cut with CTRL+X, pasted "
            "with CTRL+V, and deleted by pressing 'Delete'\n\n"
            "Additional functions can be found by right-clicking on the items, the term "
            "'unlink' means to create a new copy of the item that\n"
            "does not change it's parent item when edited (by default all items are "
            "'ghost items' that update all items with the same name)\n\n"
            "Click the 'tooltips' checkbox in the transport to disable these tooltips"))
        this_audio_items_viewer.setToolTip(_("Drag .wav files from the file browser onto here.  "
        "You can edit item properties with the\n"
        "'Edit' tab to the left, or by clicking and dragging the item handles."
        "\n\nClick the 'tooltips' checkbox in the transport to disable these tooltips"))
        this_audio_items_viewer_widget.folders_widget.setToolTip(_("Use this tab to browse your "
        "folders and files.\nDrag and drop one file at a time onto the sequencer.\n.wav files "
        "are the only supported audio file format.\nClick the 'Bookmark' button to save the "
        "current folder to your bookmarks located on the 'Bookmarks' tab."
        "\n\nClick the 'tooltips' checkbox in the transport to disable these tooltips"))
        this_audio_items_viewer_widget.modulex.widget.setToolTip(_("This tab allows you to set "
        "effects per-item.\nThe tab is only enabled when you have exactly one item selected, "
        "the copy and paste buttons allow you to copy settings between multipe items."))
        this_piano_roll_editor.setToolTip(_("Click+drag to draw notes\nCTRL+click+drag to "
        "marquee select multiple items\n"
        "Press the Del button to delete selected notes\nTo edit velocity, use the velocity "
        "button\n"
        "Double-click to edit note properties\nClick and drag the note end to change length\n"
        "Shift+click to delete a note\n"
        "To edit multiple items as one logical item, select multiple items in the region "
        "editor and right-click + 'Edit Selected Items as Group'\n"
        "The Quantize, Transpose and Velocity buttons open dialogs to manipulate the selected "
        "notes (or all notes if none are selected)"
        "\n\nClick the 'tooltips' checkbox in the transport to disable these tooltips"))
        this_transport.group_box.setToolTip(_("This is the transport, use this control to "
        "start/stop playback or recording.\n"
        "You can start or stop playback by pressing spacebar\n"
        "The MIDI controller used for recording is selected in the File->HardwareSettings menu\n"
        "The 'Loop Mode' combobox can be used to loop a region.\n"
        "The 'Follow' checkbox causes the UI to follow the project's playback position\n"
        "The 'Overdub' checkbox causes recorded MIDI notes to be appended to existing "
        "items, rather than placed in new items that replace the existing items.\n"
        "The panic button sends a note-off event on every note to every plugin.  "
        "Use this when you get a stuck note."
        "\n\nClick the 'tooltips' checkbox in the transport to disable these tooltips"))
        this_main_window.cc_map_tab.setToolTip(_("Use this tab to create CC maps for your "
        "MIDI controller to PyDAW's built-in plugins\n"
        "Each CC routes to a different control for each instrument, or if the CC is "
        "'Effects Only', it routes only to Modulex"))
        this_wave_editor_widget.widget.setToolTip(_("This tab allows you to A/B your track "
        "against a .wav file.\n"
        "Click the 'Open' button to open the .wav file, then click the 'Enabled?' checkbox "
        "to disable normal audio and enable the A/B track"))
        this_audio_item_editor_widget.set_tooltips(True)
        this_audio_items_viewer.set_tooltips(True)
        this_transport.set_tooltips(True)
    else:
        pydaw_write_file_text("{}/tooltips.txt".format(global_pydaw_home), "False")
        this_song_editor.table_widget.setToolTip("")
        for f_region_editor in global_region_editors:
            f_region_editor.table_widget.setToolTip("")
        this_audio_item_editor_widget.set_tooltips(False)
        this_audio_items_viewer.setToolTip("")
        this_audio_items_viewer_widget.folders_widget.setToolTip("")
        this_audio_items_viewer_widget.modulex.widget.setToolTip("")
        this_piano_roll_editor.setToolTip("")
        this_transport.group_box.setToolTip("")
        this_main_window.cc_map_tab.setToolTip("")
        this_wave_editor_widget.widget.setToolTip("")
        this_audio_items_viewer.set_tooltips(False)
        this_transport.set_tooltips(False)

def pydaw_global_current_region_is_none():
    if global_current_region is None:
        QtGui.QMessageBox.warning(this_main_window, _("Error"),
        _("You must create or select a region first by clicking in the song editor above."))
        return True
    return False

def pydaw_scale_to_rect(a_to_scale, a_scale_to):
    """ Returns a tuple that scales one QRectF to another """
    f_x = (a_scale_to.width() / a_to_scale.width())
    f_y = (a_scale_to.height() / a_to_scale.height())
    return (f_x, f_y)

def global_plugin_rel_callback(a_is_instrument, a_track_type, a_track_num, a_port, a_val):
    pass

def global_plugin_val_callback(a_is_instrument, a_track_type, a_track_num, a_port, a_val):
    this_pydaw_project.this_pydaw_osc.pydaw_update_plugin_control(a_is_instrument, a_track_type,
                                                                  a_track_num, a_port, a_val)

global_current_song_index = None

class song_editor:
    def add_qtablewidgetitem(self, a_name, a_region_num):
        """ Adds a properly formatted item.  This is not for creating empty items... """
        f_qtw_item = QtGui.QTableWidgetItem(a_name)
        f_qtw_item.setBackground(pydaw_region_gradient)
        f_qtw_item.setTextAlignment(QtCore.Qt.AlignCenter)
        f_qtw_item.setFlags(f_qtw_item.flags() | QtCore.Qt.ItemIsSelectable)
        self.table_widget.setItem(0, a_region_num, f_qtw_item)

    def open_song(self):
        """ This method clears the existing song from the editor and opens the
            one currently in this_pydaw_project
        """
        self.table_widget.clearContents()
        self.song = this_pydaw_project.get_song()
        f_region_dict = this_pydaw_project.get_regions_dict()
        for f_pos, f_region in list(self.song.regions.items()):
            self.add_qtablewidgetitem(f_region_dict.get_name_by_uid(f_region), f_pos)
        pydaw_update_region_lengths_dict()
        #global_open_audio_items()
        self.clipboard = []

    def cell_clicked(self, x, y):
        f_is_playing = False
        if (this_transport.is_playing or this_transport.is_recording) and \
        this_transport.follow_checkbox.isChecked():
            f_is_playing = True
            this_transport.follow_checkbox.setChecked(False)
            this_region_editor.table_widget.clearSelection()
            this_audio_items_viewer.stop_playback(0)
        f_cell = self.table_widget.item(x, y)
        if f_cell is None:
            def song_ok_handler():
                if f_new_radiobutton.isChecked():
                    f_uid = this_pydaw_project.create_empty_region(str(f_new_lineedit.text()))
                    f_msg = _("Create empty region '{}' at {}").format(f_new_lineedit.text(), y)
                elif f_copy_radiobutton.isChecked():
                    f_uid = this_pydaw_project.copy_region(str(f_copy_combobox.currentText()),
                                                           str(f_new_lineedit.text()))
                    f_msg = (_("Create new region '{}' at {} "
                            "copying from {}")).format(f_new_lineedit.text(), y,
                                                     f_copy_combobox.currentText())
                self.add_qtablewidgetitem(f_new_lineedit.text(), y)
                self.song.add_region_ref_by_uid(y, f_uid)
                this_region_settings.open_region(f_new_lineedit.text())
                global global_current_song_index
                global_current_song_index = y
                this_pydaw_project.save_song(self.song)
                this_pydaw_project.commit(f_msg)
                if not f_is_playing:
                    this_transport.set_region_value(y)
                    this_transport.set_bar_value(0)
                f_window.close()

            def song_cancel_handler():
                f_window.close()

            def on_name_changed():
                f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

            def on_current_index_changed(a_index):
                f_copy_radiobutton.setChecked(True)

            f_window = QtGui.QDialog(this_main_window)
            f_window.setWindowTitle(_("Add region to song..."))
            f_layout = QtGui.QGridLayout()
            f_window.setLayout(f_layout)
            f_new_radiobutton = QtGui.QRadioButton()
            f_new_radiobutton.setChecked(True)
            f_layout.addWidget(f_new_radiobutton, 0, 0)
            f_layout.addWidget(QtGui.QLabel(_("New:")), 0, 1)
            f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_region_name())
            f_new_lineedit.setMaxLength(24)
            f_new_lineedit.editingFinished.connect(on_name_changed)
            f_layout.addWidget(f_new_lineedit, 0, 2)
            f_copy_radiobutton = QtGui.QRadioButton()
            f_layout.addWidget(f_copy_radiobutton, 1, 0)
            f_copy_combobox = QtGui.QComboBox()
            f_copy_combobox.addItems(this_pydaw_project.get_region_list())
            f_copy_combobox.currentIndexChanged.connect(on_current_index_changed)
            f_layout.addWidget(QtGui.QLabel(_("Copy from:")), 1, 1)
            f_layout.addWidget(f_copy_combobox, 1, 2)
            f_ok_button = QtGui.QPushButton(_("OK"))
            f_layout.addWidget(f_ok_button, 5,0)
            f_ok_button.clicked.connect(song_ok_handler)
            f_cancel_button = QtGui.QPushButton(_("Cancel"))
            f_layout.addWidget(f_cancel_button, 5,1)
            f_cancel_button.clicked.connect(song_cancel_handler)
            f_window.exec_()
        else:
            this_region_settings.open_region(str(f_cell.text()))
            global global_current_song_index
            global_current_song_index = y
            if not f_is_playing:
                this_region_editor.table_widget.clearSelection()
                this_transport.set_region_value(y)
                this_transport.set_bar_value(0)

    def __init__(self):
        self.song = pydaw_song()
        self.main_vlayout = QtGui.QVBoxLayout()
        self.hlayout0 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout0)
        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setColumnCount(300)
        self.table_widget.setRowCount(1)
        self.table_widget.setFixedHeight(87)
        self.table_widget.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.table_widget.verticalHeader().setVisible(False)
        self.table_widget.setAutoScroll(True)
        self.table_widget.setAutoScrollMargin(1)
        self.table_widget.setRowHeight(0, 50)
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

        self.table_widget.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)
        self.rename_action = QtGui.QAction("Rename region", self.table_widget)
        self.rename_action.triggered.connect(self.on_rename_region)
        self.table_widget.addAction(self.rename_action)
        self.delete_action = QtGui.QAction("Delete (Del)", self.table_widget)
        self.delete_action.triggered.connect(self.on_delete)
        self.table_widget.addAction(self.delete_action)

    def on_delete(self):
        f_item = self.table_widget.currentItem()
        if f_item is None:
            return
        f_item_text = str(f_item.text())
        f_empty = QtGui.QTableWidgetItem() #Clear the item
        self.table_widget.setItem(f_item.row(), f_item.column(), f_empty)
        self.tablewidget_to_song()
        this_region_settings.clear_items()
        this_region_settings.region_name_lineedit.setText("")
        this_region_settings.enabled = False
        this_region_settings.update_region_length() #TODO:  Is this right?
        this_pydaw_project.commit(_("Remove {} from song").format(f_item_text))
        pydaw_update_region_lengths_dict()

    def on_rename_region(self):
        f_item = self.table_widget.currentItem()
        if f_item is None:
            return
        f_item_text = str(f_item.text())

        def ok_handler():
            f_new_name = str(f_new_lineedit.text())
            if f_new_name == "":
                QtGui.QMessageBox.warning(self.table_widget, _("Error"), _("Name cannot be blank"))
                return
            this_pydaw_project.rename_region(f_item_text, f_new_name)
            this_pydaw_project.commit(_("Rename region"))
            this_song_editor.open_song()
            this_region_settings.open_region(f_new_name)
            f_window.close()

        def cancel_handler():
            f_window.close()

        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Rename region..."))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_new_lineedit = QtGui.QLineEdit()
        f_new_lineedit.editingFinished.connect(on_name_changed)
        f_new_lineedit.setMaxLength(24)
        f_layout.addWidget(QtGui.QLabel(_("New name:")), 0, 0)
        f_layout.addWidget(f_new_lineedit, 0, 1)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(cancel_handler)
        f_window.exec_()

    def table_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            f_commit_msg = _("Deleted region references at ")
            for f_item in self.table_widget.selectedIndexes():
                f_commit_msg += str(f_item.column())
                f_empty = QtGui.QTableWidgetItem() #Clear the item
                self.table_widget.setItem(f_item.row(), f_item.column(), f_empty)
            self.tablewidget_to_song()
            this_region_settings.clear_items()
            this_region_settings.region_name_lineedit.setText("")
            this_region_settings.enabled = False
            this_region_settings.update_region_length() #TODO:  Is this right?
            this_pydaw_project.commit(f_commit_msg)
            pydaw_update_region_lengths_dict()
        else:
            QtGui.QTableWidget.keyPressEvent(self.table_widget, event)

    def table_drop_event(self, a_event):
        QtGui.QTableWidget.dropEvent(self.table_widget, a_event)
        a_event.acceptProposedAction()
        self.tablewidget_to_song()
        self.table_widget.clearSelection()
        this_pydaw_project.commit(_("Drag-n-drop song item(s)"))
        pydaw_update_region_lengths_dict()

    def tablewidget_to_song(self):
        """ Flush the edited content of the QTableWidget back to the native song class... """
        self.song.regions = {}
        f_uid_dict = this_pydaw_project.get_regions_dict()
        global global_current_song_index
        global_current_song_index = None
        for f_i in range(0, 300):
            f_item = self.table_widget.item(0, f_i)
            if not f_item is None:
                if str(f_item.text()) != "":
                    self.song.add_region_ref_by_name(f_i, f_item.text(), f_uid_dict)
                if str(f_item.text()) == global_current_region_name:
                    global_current_song_index = f_i
                    print((str(f_i)))
        this_pydaw_project.save_song(self.song)
        self.open_song()

    def open_first_region(self):
        for f_i in range(300):
            f_item = self.table_widget.item(0, f_i)
            if f_item is not None and str(f_item.text()) != "":
                this_region_settings.open_region(str(f_item.text()))
                this_transport.set_region_value(f_i)
                f_item.setSelected(True)
                break

global_current_region = None
global_current_region_name = None

class region_settings:
    def update_region_length(self, a_value=None):
        f_region_name = str(self.region_name_lineedit.text())
        global global_current_region
        if not this_transport.is_playing and not this_transport.is_recording and \
        global_current_region is not None and f_region_name != "":
            if not self.enabled or global_current_region is None:
                return
            if self.length_alternate_radiobutton.isChecked():
                f_region_length = self.length_alternate_spinbox.value()
                global_current_region.region_length_bars = f_region_length
                f_commit_message = \
                _("Set region '{}' length to {}").format(f_region_name,
                                                      self.length_alternate_spinbox.value())
            else:
                global_current_region.region_length_bars = 0
                f_region_length = 8
                f_commit_message = _("Set region '{}' length to default value").format(
                    f_region_name)
            this_pydaw_project.save_region(f_region_name, global_current_region)
            global_audio_items.set_region_length(f_region_length)
            this_pydaw_project.save_audio_region(global_current_region.uid, global_audio_items)
            self.open_region(self.region_name_lineedit.text())
            pydaw_update_region_lengths_dict()
            f_resave = False
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.clip_at_region_end():
                    f_resave = True
            if f_resave:
                this_pydaw_project.save_audio_region(global_current_region.uid, global_audio_items)
            this_pydaw_project.commit(f_commit_message)

    def __init__(self):
        self.enabled = False
        self.hlayout0 = QtGui.QHBoxLayout()
        self.region_num_label = QtGui.QLabel()
        self.region_num_label.setText(_("Region:"))
        self.hlayout0.addWidget(self.region_num_label)
        self.region_name_lineedit = QtGui.QLineEdit()
        self.region_name_lineedit.setEnabled(False)
        self.region_name_lineedit.setMaximumWidth(330)
        self.hlayout0.addWidget(self.region_name_lineedit)
        self.hlayout0.addItem(QtGui.QSpacerItem(10,10, QtGui.QSizePolicy.Expanding))
        self.split_button = QtGui.QPushButton(_("Split"))
        self.split_button.pressed.connect(self.on_split)
        self.hlayout0.addWidget(self.split_button)
        self.hlayout0.addWidget(QtGui.QLabel(_("Region Length:")))
        self.length_default_radiobutton = QtGui.QRadioButton(_("default"))
        self.length_default_radiobutton.setChecked(True)
        self.length_default_radiobutton.toggled.connect(self.update_region_length)
        self.hlayout0.addWidget(self.length_default_radiobutton)
        self.length_alternate_radiobutton = QtGui.QRadioButton()
        self.length_alternate_radiobutton.toggled.connect(self.update_region_length)
        self.hlayout0.addWidget(self.length_alternate_radiobutton)
        self.length_alternate_spinbox = QtGui.QSpinBox()
        self.length_alternate_spinbox.setKeyboardTracking(False)
        self.length_alternate_spinbox.setRange(1, pydaw_max_region_length)
        self.length_alternate_spinbox.setValue(8)
        self.length_alternate_spinbox.valueChanged.connect(self.update_region_length)
        self.hlayout0.addWidget(self.length_alternate_spinbox)

    def on_split(self):
        if global_current_region is None or this_transport.is_playing or \
        this_transport.is_recording or global_current_region.region_length_bars == 1:
            return
        def split_ok_handler():
            f_index = f_split_at.value()
            f_region_name = str(f_new_lineedit.text())
            f_new_uid = this_pydaw_project.create_empty_region(f_region_name)
            f_midi_tuple = global_current_region.split(f_index, f_new_uid)
            f_audio_tuple = global_audio_items.split(f_index)
            f_current_index = this_song_editor.song.get_index_of_region(global_current_region.uid)
            this_song_editor.song.insert_region(f_current_index + 1, f_new_uid)
            this_pydaw_project.save_song(this_song_editor.song)
            this_pydaw_project.save_region(global_current_region_name, f_midi_tuple[0])
            this_pydaw_project.save_region(f_region_name, f_midi_tuple[1])
            this_pydaw_project.save_audio_region(global_current_region.uid, f_audio_tuple[0])
            this_pydaw_project.save_audio_region(f_new_uid, f_audio_tuple[1])
            this_pydaw_project.commit(_("Split region {}  into {}").format(
                global_current_region_name, f_region_name))
            this_region_settings.open_region_by_uid(global_current_region.uid)
            this_song_editor.open_song()
            f_window.close()
        def split_cancel_handler():
            f_window.close()
        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Split region..."))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_vlayout0 = QtGui.QVBoxLayout()
        f_new_lineedit = QtGui.QLineEdit(this_pydaw_project.get_next_default_region_name())
        f_new_lineedit.editingFinished.connect(on_name_changed)
        f_new_lineedit.setMaxLength(24)
        f_layout.addWidget(QtGui.QLabel(_("New name:")), 0, 1)
        f_layout.addWidget(f_new_lineedit, 0, 2)
        f_layout.addLayout(f_vlayout0, 1, 0)
        f_split_at = QtGui.QSpinBox()
        f_split_at.setRange(1, pydaw_get_current_region_length() - 1)
        f_layout.addWidget(QtGui.QLabel(_("Split at:")), 2, 1)
        f_layout.addWidget(f_split_at, 2, 2)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_ok_cancel_layout, 5, 2)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_ok_cancel_layout.addWidget(f_ok_button)
        f_ok_button.clicked.connect(split_ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_ok_cancel_layout.addWidget(f_cancel_button)
        f_cancel_button.clicked.connect(split_cancel_handler)
        f_window.exec_()

    def open_region_by_uid(self, a_uid):
        f_regions_dict = this_pydaw_project.get_regions_dict()
        self.open_region(f_regions_dict.get_name_by_uid(a_uid))

    def open_region(self, a_file_name):
        self.enabled = False
        for f_editor in global_region_editors:
            f_editor.enabled = False
        self.clear_items()
        self.region_name_lineedit.setText(a_file_name)
        global global_current_region_name
        global_current_region_name = str(a_file_name)
        global global_current_region
        global_current_region = this_pydaw_project.get_region_by_name(a_file_name)
        if global_current_region.region_length_bars > 0:
            for f_editor in global_region_editors:
                f_editor.set_region_length(global_current_region.region_length_bars)
            self.length_alternate_spinbox.setValue(global_current_region.region_length_bars)
            this_transport.bar_spinbox.setRange(1, (global_current_region.region_length_bars))
            self.length_alternate_radiobutton.setChecked(True)
        else:
            for f_editor in global_region_editors:
                f_editor.set_region_length()
            self.length_alternate_spinbox.setValue(8)
            this_transport.bar_spinbox.setRange(1, 8)
            self.length_default_radiobutton.setChecked(True)
        self.enabled = True
        for f_editor in global_region_editors:
            f_editor.enabled = True
        f_items_dict = this_pydaw_project.get_items_dict()
        for f_item in global_current_region.items:
            if f_item.bar_num < global_current_region.region_length_bars or \
            (global_current_region.region_length_bars == 0 and f_item.bar_num < 8):
                f_item_name = f_items_dict.get_name_by_uid(f_item.item_uid)
                if f_item.track_num < pydaw_midi_track_count:
                    this_region_editor.add_qtablewidgetitem(f_item_name, f_item.track_num,
                                                            f_item.bar_num, a_is_offset=True)
                elif f_item.track_num < pydaw_midi_track_count + pydaw_bus_count:
                    this_region_bus_editor.add_qtablewidgetitem(f_item_name, f_item.track_num,
                                                                f_item.bar_num, a_is_offset=True)
                else:
                    this_region_audio_editor.add_qtablewidgetitem(f_item_name, f_item.track_num,
                                                                  f_item.bar_num, a_is_offset=True)
        this_audio_items_viewer.scale_to_region_size()
        global_open_audio_items()

    def clear_items(self):
        self.region_name_lineedit.setText("")
        self.length_alternate_spinbox.setValue(8)
        self.length_default_radiobutton.setChecked(True)
        for f_editor in global_region_editors:
            f_editor.clear_items()
        this_audio_items_viewer.clear_drawn_items()
        global global_current_region
        global_current_region = None

    def clear_new(self):
        self.region_name_lineedit.setText("")
        global global_current_region
        global_current_region = None
        for f_editor in global_region_editors:
            f_editor.clear_new()

    def on_play(self):
        self.length_default_radiobutton.setEnabled(False)
        self.length_alternate_radiobutton.setEnabled(False)
        self.length_alternate_spinbox.setEnabled(False)

    def on_stop(self):
        self.length_default_radiobutton.setEnabled(True)
        self.length_alternate_radiobutton.setEnabled(True)
        self.length_alternate_spinbox.setEnabled(True)

class region_list_editor:
    def add_qtablewidgetitem(self, a_name, a_track_num, a_bar_num, a_selected=False,
                             a_is_offset=False):
        """ Adds a properly formatted item.  This is not for creating empty items... """
        if a_is_offset:
            f_track_num = a_track_num - self.track_offset
        else:
            f_track_num = a_track_num
        f_qtw_item = QtGui.QTableWidgetItem(a_name)
        f_qtw_item.setBackground(pydaw_track_gradients[f_track_num]) # - self.track_offset
        f_qtw_item.setTextAlignment(QtCore.Qt.AlignCenter)
        f_qtw_item.setFlags(f_qtw_item.flags() | QtCore.Qt.ItemIsSelectable)
        self.table_widget.setItem(f_track_num, a_bar_num + 1, f_qtw_item)
        if a_selected:
            f_qtw_item.setSelected(True)

    def clear_new(self):
        """ Reset the region editor state to empty """
        self.clear_items()
        self.reset_tracks()
        self.enabled = False
        global global_region_clipboard
        global_region_clipboard = []

    def open_tracks(self):
        self.reset_tracks()
        if self.track_type == pydaw_track_type_enum.midi:
            f_tracks = this_pydaw_project.get_tracks()
            for key, f_track in list(f_tracks.tracks.items()):
                self.tracks[key].open_track(f_track)
        elif self.track_type == pydaw_track_type_enum.bus:
            f_tracks = this_pydaw_project.get_bus_tracks()
            for key, f_track in list(f_tracks.busses.items()):
                self.tracks[key].open_track(f_track)
        elif self.track_type == pydaw_track_type_enum.audio:
            f_tracks = this_pydaw_project.get_audio_tracks()
            for key, f_track in list(f_tracks.tracks.items()):
                self.tracks[key].open_track(f_track)

    def reset_tracks(self):
        self.tracks = []
        for i in range(0, self.track_count):
            if self.track_type == pydaw_track_type_enum.midi:
                track = seq_track(a_track_num=i, a_track_text=_("track{}").format(i + 1))
            elif self.track_type == pydaw_track_type_enum.bus:
                if i == 0:
                    track = seq_track(a_track_num=i, a_track_text=_("Master"),
                                      a_instrument=False)
                else:
                    track = seq_track(a_track_num=i, a_track_text=_("Bus{}").format(i),
                                      a_instrument=False)
            elif self.track_type == pydaw_track_type_enum.audio:
                track = audio_track(a_track_num=i, a_track_text=_("track{}").format(i + 1))
            self.tracks.append(track)
            self.table_widget.setCellWidget(i, 0, track.group_box)
        self.table_widget.setColumnWidth(0, 390)
        self.set_region_length()

    def set_region_length(self, a_length=8):
        self.region_length = a_length
        self.table_widget.setColumnCount(a_length + 1)
        f_headers = [_('Tracks')]
        for i in range(0, a_length):
            self.table_widget.setColumnWidth(i + 1, 100)
            f_headers.append(str(i + 1))
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
            f_item.setFlags(f_item.flags() & ~QtCore.Qt.ItemIsEditable & \
            ~QtCore.Qt.ItemIsSelectable & ~QtCore.Qt.ItemIsEnabled)
            self.table_widget.setItem(i, 0, f_item)
        self.enabled = False

    def get_tracks(self):
        if self.track_type == 0:
            f_result = pydaw_tracks()
            for f_i in range(0, self.track_count):
                f_result.add_track(f_i, self.tracks[f_i].get_track())
        elif self.track_type == 1:
            f_result = pydaw_busses()
            for f_i in range(0, self.track_count):
                f_result.add_bus(f_i, self.tracks[f_i].get_track())
        elif self.track_type == 2:
            f_result = pydaw_audio_tracks()
            for f_i in range(0, self.track_count):
                f_result.add_track(f_i, self.tracks[f_i].get_track())
        return f_result

    def warn_no_region_selected(self):
        QtGui.QMessageBox.warning(this_main_window, _("Error"),
        _("You must create or select a region first by clicking in the song editor above."))

    def cell_clicked(self, x, y):
        if y <= 0 or x < 0:
            return
        if not self.enabled:
            self.warn_no_region_selected()
            return
        if (this_transport.is_playing or this_transport.is_recording) and \
        this_transport.follow_checkbox.isChecked():
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
                global_open_items([f_item_name])
                this_main_window.main_tabwidget.setCurrentIndex(1)
            else:
                self.show_cell_dialog(x, y)

    def show_cell_dialog(self, x, y):
        def note_ok_handler():
            self.table_widget.clearSelection()
            global global_current_region
            if (f_new_radiobutton.isChecked() and f_item_count.value() == 1):
                f_cell_text = str(f_new_lineedit.text())
                if this_pydaw_project.item_exists(f_cell_text):
                    QtGui.QMessageBox.warning(self.table_widget, _("Error"),
                    _("An item named '{}' already exists.").format(f_cell_text))
                    return
                f_uid = this_pydaw_project.create_empty_item(f_cell_text)
                self.add_qtablewidgetitem(f_cell_text, x, y - 1, True)
                global_current_region.add_item_ref_by_uid(x + self.track_offset, y - 1, f_uid)
            elif f_new_radiobutton.isChecked() and f_item_count.value() > 1:
                f_name_suffix = 1
                f_cell_text = str(f_new_lineedit.text())
                for i in range(f_item_count.value()):
                    while this_pydaw_project.item_exists(_("{}-{}").format(f_cell_text,
                                                                        f_name_suffix)):
                        f_name_suffix += 1
                    f_item_name = _("{}-{}").format(f_cell_text, f_name_suffix)
                    f_uid = this_pydaw_project.create_empty_item(f_item_name)
                    self.add_qtablewidgetitem(f_item_name, x, y - 1 + i, True)
                    global_current_region.add_item_ref_by_uid(
                    x + self.track_offset, y - 1 + i, f_uid)
            elif f_copy_radiobutton.isChecked():
                f_cell_text = str(f_copy_combobox.currentText())
                self.add_qtablewidgetitem(f_cell_text, x, y - 1, True)
                global_current_region.add_item_ref_by_name(x + self.track_offset, y - 1,
                                                           f_cell_text,
                                                           this_pydaw_project.get_items_dict())
            elif f_copy_from_radiobutton.isChecked():
                f_cell_text = str(f_new_lineedit.text())
                f_copy_from_text = str(f_copy_combobox.currentText())
                if this_pydaw_project.item_exists(f_cell_text):
                    QtGui.QMessageBox.warning(self.table_widget, _("Error"),
                    _("An item named '{}' already exists.").format(f_cell_text))
                    return
                f_uid = this_pydaw_project.copy_item(f_copy_from_text, f_cell_text)
                self.add_qtablewidgetitem(f_cell_text, x, y - 1, True)
                global_current_region.add_item_ref_by_uid(x + self.track_offset, y - 1, f_uid)
            this_pydaw_project.save_region(str(this_region_settings.region_name_lineedit.text()),
                                           global_current_region)
            this_pydaw_project.commit(_("Add reference(s) to item "
                "(group) '{}' in region '{}'").format(
                f_cell_text, this_region_settings.region_name_lineedit.text()))
            self.last_item_copied = f_cell_text

            f_window.close()

        def paste_button_pressed():
            self.paste_clipboard()
            f_window.close()

        def paste_to_end_button_pressed():
            self.paste_to_region_end()
            f_window.close()

        def note_cancel_handler():
            f_window.close()

        def copy_combobox_index_changed(a_index):
            f_copy_radiobutton.setChecked(True)

        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Add item reference to region..."))
        f_layout = QtGui.QGridLayout()
        f_vlayout0 = QtGui.QVBoxLayout()
        f_vlayout1 = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_new_radiobutton = QtGui.QRadioButton()
        f_new_radiobutton.setChecked(True)
        f_layout.addWidget(f_new_radiobutton, 0, 0)
        f_layout.addWidget(QtGui.QLabel(_("New:")), 0, 1)
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
        f_vlayout1.addWidget(QtGui.QLabel(_("Copy from:")))
        f_vlayout1.addWidget(QtGui.QLabel(_("Existing:")))
        f_layout.addWidget(f_copy_combobox, 1, 2)
        f_layout.addWidget(QtGui.QLabel(_("Item Count:")), 2, 1)
        f_item_count = QtGui.QSpinBox()
        f_item_count.setRange(1, self.region_length - y + 1)
        f_item_count.setToolTip(_("Only used for 'New'"))
        f_layout.addWidget(f_item_count, 2, 2)

        if len(global_region_clipboard) > 0:
            f_paste_clipboard_button = QtGui.QPushButton(_("Paste Clipboard"))
            f_layout.addWidget(f_paste_clipboard_button, 4, 2)
            f_paste_clipboard_button.pressed.connect(paste_button_pressed)

        if len(global_region_clipboard) == 1:
            f_paste_to_end_button = QtGui.QPushButton(_("Paste to End"))
            f_layout.addWidget(f_paste_to_end_button, 7, 2)
            f_paste_to_end_button.pressed.connect(paste_to_end_button_pressed)

        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_ok_cancel_layout, 9, 2)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_ok_cancel_layout.addWidget(f_ok_button)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_ok_cancel_layout.addWidget(f_cancel_button)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_window.exec_()

    def column_clicked(self, a_val):
        if this_transport.is_playing or this_transport.is_recording:
            return
        if a_val > 0:
            this_transport.set_bar_value(a_val - 1)

    def __init__(self, a_track_type):
        self.enabled = False #Prevents user from editing a region before one has been selected
        self.track_type = a_track_type
        if a_track_type == 0:
            self.track_count = pydaw_midi_track_count
            self.track_offset = 0
        elif a_track_type == 1:
            self.track_count = pydaw_bus_count
            self.track_offset = pydaw_midi_track_count
        elif a_track_type == 2:
            self.track_count = pydaw_audio_track_count
            self.track_offset = pydaw_midi_track_count + pydaw_bus_count

        self.table_widget = QtGui.QTableWidget()
        self.table_widget.setAlternatingRowColors(True)
        self.table_widget.verticalHeader().setVisible(False)
        self.table_widget.horizontalHeader().sectionClicked.connect(self.column_clicked)
        self.table_widget.setMinimumHeight(150)
        self.table_widget.setAutoScroll(True)
        self.table_widget.setAutoScrollMargin(1)
        self.table_widget.setColumnCount(9)
        self.table_widget.setRowCount(self.track_count)
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

        self.edit_group_action = QtGui.QAction(_("Edit Selected Items as Group"),
                                               self.table_widget)
        self.edit_group_action.triggered.connect(self.edit_group)
        self.table_widget.addAction(self.edit_group_action)

        self.copy_action = QtGui.QAction(_("Copy (CTRL+C)"), self.table_widget)
        self.copy_action.triggered.connect(self.copy_selected)
        self.table_widget.addAction(self.copy_action)

        self.paste_action = QtGui.QAction(_("Paste (CTRL+V)"), self.table_widget)
        self.paste_action.triggered.connect(self.paste_clipboard)
        self.table_widget.addAction(self.paste_action)

        self.paste_to_end_action = QtGui.QAction(_("Paste to Region End (ALT+V)"),
                                                 self.table_widget)
        self.paste_to_end_action.triggered.connect(self.paste_to_region_end)
        self.table_widget.addAction(self.paste_to_end_action)

        self.rename_action = QtGui.QAction(_("Rename Selected Items"), self.table_widget)
        self.rename_action.triggered.connect(self.on_rename_items)
        self.table_widget.addAction(self.rename_action)
        self.unlink_action = QtGui.QAction(_("Unlink Single Item(CTRL+D)"), self.table_widget)
        self.unlink_action.triggered.connect(self.on_unlink_item)
        self.table_widget.addAction(self.unlink_action)
        self.unlink_selected_action = QtGui.QAction(_("Auto-Unlink Selected Items"),
                                                    self.table_widget)
        self.unlink_selected_action.triggered.connect(self.on_auto_unlink_selected)
        self.table_widget.addAction(self.unlink_selected_action)
        self.delete_action = QtGui.QAction(_("Delete (Del)"), self.table_widget)
        self.delete_action.triggered.connect(self.delete_selected)
        self.table_widget.addAction(self.delete_action)
        if a_track_type == 0:
            self.transpose_action = QtGui.QAction(_("Transpose"), self.table_widget)
            self.transpose_action.triggered.connect(self.transpose_dialog)
            self.table_widget.addAction(self.transpose_action)

        self.last_item_copied = None
        self.reset_tracks()
        self.last_cc_line_num = 1

    def get_selected_items(self):
        f_result = []
        for f_index in self.table_widget.selectedIndexes():
            f_cell = self.table_widget.item(f_index.row(), f_index.column())
            if not f_cell is None and not str(f_cell.text()) == "":
                f_result.append(str(f_cell.text()))
        return f_result

    def transpose_dialog(self):
        if pydaw_global_current_region_is_none():
            return

        f_item_list = self.get_selected_items()
        if len(f_item_list) == 0:
            QtGui.QMessageBox.warning(this_main_window, _("Error"), _("No items selected"))
            return

        def transpose_ok_handler():
            for f_item_name in f_item_list:
                f_item = this_pydaw_project.get_item_by_name(f_item_name)
                f_item.transpose(f_semitone.value(), f_octave.value(), a_selected_only=False,
                                 a_duplicate=f_duplicate_notes.isChecked())
                this_pydaw_project.save_item(f_item_name, f_item)
            this_pydaw_project.commit(_("Transpose item(s)"))
            if len(global_open_items_uids) > 0:
                global_open_items()
            f_window.close()

        def transpose_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Transpose"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_semitone = QtGui.QSpinBox()
        f_semitone.setRange(-12, 12)
        f_layout.addWidget(QtGui.QLabel(_("Semitones")), 0, 0)
        f_layout.addWidget(f_semitone, 0, 1)
        f_octave = QtGui.QSpinBox()
        f_octave.setRange(-5, 5)
        f_layout.addWidget(QtGui.QLabel(_("Octaves")), 1, 0)
        f_layout.addWidget(f_octave, 1, 1)
        f_duplicate_notes = QtGui.QCheckBox(_("Duplicate notes?"))
        f_duplicate_notes.setToolTip(_("Checking this box causes the transposed "
                                     "notes to be added rather than moving the existing notes."))
        f_layout.addWidget(f_duplicate_notes, 2, 1)
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(transpose_ok_handler)
        f_layout.addWidget(f_ok, 6, 0)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(transpose_cancel_handler)
        f_layout.addWidget(f_cancel, 6, 1)
        f_window.exec_()

    def table_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            self.delete_selected()
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.copy_selected()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            self.paste_clipboard()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() & QtCore.Qt.AltModifier:
            self.paste_to_region_end()
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.copy_selected()
            self.delete_selected()
        elif event.key() == QtCore.Qt.Key_D and event.modifiers() == QtCore.Qt.ControlModifier:
            self.on_unlink_item()
        else:
            QtGui.QTableWidget.keyPressEvent(self.table_widget, event)

    def edit_group(self):
        f_result = []
        for i in range(self.track_count):
            for i2 in range(1, self.region_length + 1):
                f_item = self.table_widget.item(i, i2)
                if not f_item is None and not str(f_item.text()) == "" and f_item.isSelected():
                    f_result_str = str(f_item.text())
                    if f_result_str in f_result:
                        QtGui.QMessageBox.warning(self.table_widget, _("Error"),
                        _("You cannot open multiple instances of the same item as a group.\n"
                        "You should unlink all duplicate instances of {} into their own "
                        "individual item names before editing as a group.").format(f_result_str))
                        return
                    f_result.append(f_result_str)
        global_open_items(f_result)
        this_main_window.main_tabwidget.setCurrentIndex(1)

    def on_rename_items(self):
        f_result = []
        for f_item in self.table_widget.selectedItems():
            f_item_name = str(f_item.text())
            if not f_item_name in f_result:
                f_result.append(f_item_name)
        if len(f_result) == 0:
            return

        def ok_handler():
            f_new_name = str(f_new_lineedit.text())
            if f_new_name == "":
                QtGui.QMessageBox.warning(self.group_box, _("Error"), _("Name cannot be blank"))
                return
            global global_region_clipboard
            #Clear the clipboard, otherwise the names could be invalid
            global_region_clipboard = []
            this_pydaw_project.rename_items(f_result, f_new_name)
            this_pydaw_project.commit(_("Rename items"))
            this_region_settings.open_region_by_uid(global_current_region.uid)
            global_update_items_label()
            f_window.close()

        def cancel_handler():
            f_window.close()

        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Rename selected items..."))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_new_lineedit = QtGui.QLineEdit()
        f_new_lineedit.editingFinished.connect(on_name_changed)
        f_new_lineedit.setMaxLength(24)
        f_layout.addWidget(QtGui.QLabel(_("New name:")), 0, 0)
        f_layout.addWidget(f_new_lineedit, 0, 1)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(cancel_handler)
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
            if f_cell_text == f_current_item_text:
                QtGui.QMessageBox.warning(self.group_box, _("Error"),
                _("You must choose a different name than the original item"))
                return
            if this_pydaw_project.item_exists(f_cell_text):
                QtGui.QMessageBox.warning(self.group_box, _("Error"),
                                          _("An item with this name already exists."))
                return
            f_uid = this_pydaw_project.copy_item(str(f_current_item.text()),
                                                 str(f_new_lineedit.text()))
            global_open_items([f_cell_text])
            self.last_item_copied = f_cell_text
            self.add_qtablewidgetitem(f_cell_text, x, y - 1)
            global_current_region.add_item_ref_by_uid(x + self.track_offset, y - 1, f_uid)
            this_pydaw_project.save_region(str(this_region_settings.region_name_lineedit.text()),
                                           global_current_region)
            this_pydaw_project.commit(_("Unlink item '{}' as '{}'").format(f_current_item_text,
                                                                        f_cell_text))
            f_window.close()

        def note_cancel_handler():
            f_window.close()

        def on_name_changed():
            f_new_lineedit.setText(pydaw_remove_bad_chars(f_new_lineedit.text()))

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Copy and unlink item..."))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_new_lineedit = QtGui.QLineEdit(f_current_item_text)
        f_new_lineedit.editingFinished.connect(on_name_changed)
        f_new_lineedit.setMaxLength(24)
        f_layout.addWidget(QtGui.QLabel(_("New name:")), 0, 0)
        f_layout.addWidget(f_new_lineedit, 0, 1)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_window.exec_()

    def on_auto_unlink_selected(self):
        """ Currently adds an automatic -N suffix, but this behavior may be changed later"""
        for i in range(self.track_count):
            for i2 in range(1, self.region_length + 1):
                f_item = self.table_widget.item(i, i2)
                if not f_item is None and not str(f_item.text()) == "" and f_item.isSelected():
                    f_item_name = str(f_item.text())
                    f_name_suffix = 1
                    while this_pydaw_project.item_exists("{}-{}".format(f_item_name,
                                                                        f_name_suffix)):
                        f_name_suffix += 1
                    f_cell_text = "{}-{}".format(f_item_name, f_name_suffix)
                    f_uid = this_pydaw_project.copy_item(f_item_name, f_cell_text)
                    self.add_qtablewidgetitem(f_cell_text, i, i2 - 1)
                    global_current_region.add_item_ref_by_uid(i + self.track_offset, i2 - 1, f_uid)
        this_pydaw_project.save_region(str(this_region_settings.region_name_lineedit.text()),
                                       global_current_region)
        this_pydaw_project.commit(_("Auto-Unlink items"))

    def paste_to_region_end(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return
        f_selected_cells = self.table_widget.selectedIndexes()
        if len(f_selected_cells) == 0:
            return
        if len(global_region_clipboard) != 1:
            QtGui.QMessageBox.warning(this_main_window, _("Error"), _("Paste to region end only "
                "works when you have exactly one item copied to the clipboard.\n"
                "You have {} items copied.").format(len(global_region_clipboard)))
            return
        f_base_row = f_selected_cells[0].row()
        f_base_column = f_selected_cells[0].column() - 1
        f_region_length = pydaw_get_current_region_length()
        f_item = global_region_clipboard[0]
        for f_column in range(f_base_column, f_region_length + 1):
            self.add_qtablewidgetitem(f_item[2], f_base_row, f_column)
        global_tablewidget_to_region()

    def paste_clipboard(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return
        f_selected_cells = self.table_widget.selectedIndexes()
        if len(f_selected_cells) == 0:
            return
        f_base_row = f_selected_cells[0].row()
        f_base_column = f_selected_cells[0].column() - 1
        f_region_length = pydaw_get_current_region_length()
        for f_item in global_region_clipboard:
            f_column = f_item[1] + f_base_column
            if f_column >= f_region_length or f_column < 0:
                continue
            f_row = f_item[0] + f_base_row
            if f_row >= self.track_count or f_row < 0:
                continue
            self.add_qtablewidgetitem(f_item[2], f_row, f_column)
        global_tablewidget_to_region()

    def delete_selected(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return
        for f_item in self.table_widget.selectedIndexes():
            f_empty = QtGui.QTableWidgetItem() #Clear the item
            self.table_widget.setItem(f_item.row(), f_item.column(), f_empty)
        global_tablewidget_to_region()
        self.table_widget.clearSelection()

    def copy_selected(self):
        if not self.enabled:
            self.warn_no_region_selected()
            return
        global global_region_clipboard
        global_region_clipboard = []  #Clear the clipboard
        for f_item in self.table_widget.selectedIndexes():
            f_cell = self.table_widget.item(f_item.row(), f_item.column())
            if not f_cell is None and not str(f_cell.text()) == "":
                global_region_clipboard.append([int(f_item.row()), int(f_item.column()) - 1,
                                                str(f_cell.text())])
        if len(global_region_clipboard) > 0:
            global_region_clipboard.sort(key=operator.itemgetter(0))
            f_row_offset = global_region_clipboard[0][0]
            for f_item in global_region_clipboard:
                f_item[0] -= f_row_offset
            global_region_clipboard.sort(key=operator.itemgetter(1))
            f_column_offset = global_region_clipboard[0][1]
            for f_item in global_region_clipboard:
                f_item[1] -= f_column_offset

    def table_drop_event(self, a_event):
        if a_event.pos().x() <= self.table_widget.columnWidth(0) or \
        a_event.pos().x() >= self.table_width:
            print(_("Drop event out of bounds, ignoring..."))
            a_event.ignore()
            return
        QtGui.QTableWidget.dropEvent(self.table_widget, a_event)
        a_event.acceptProposedAction()
        global_tablewidget_to_region()
        self.table_widget.clearSelection()

    def tablewidget_to_list(self):
        """ Convert an edited QTableWidget to a list of tuples for a region ref """
        f_result = []
        for i in range(0, self.track_count):
            for i2 in range(1, self.table_widget.columnCount()):
                f_item = self.table_widget.item(i, i2)
                if not f_item is None:
                    if f_item.text() != "":
                        f_result.append((i + self.track_offset, i2 - 1, str(f_item.text())))
        return f_result

global_region_clipboard = []

def global_tablewidget_to_region():
    global global_current_region
    global_current_region.items = []
    f_uid_dict = this_pydaw_project.get_items_dict()
    f_result = []
    for f_editor in global_region_editors:
        f_result += f_editor.tablewidget_to_list()
    for f_tuple in f_result:
        global_current_region.add_item_ref_by_name(f_tuple[0], f_tuple[1], f_tuple[2], f_uid_dict)
    this_pydaw_project.save_region(str(this_region_settings.region_name_lineedit.text()),
                                   global_current_region)
    this_pydaw_project.commit(_("Edit region"))


def global_update_audio_track_comboboxes(a_index=None, a_value=None):
    if not a_index is None and not a_value is None:
        global_audio_track_names[int(a_index)] = str(a_value)
    global global_suppress_audio_track_combobox_changes
    global_suppress_audio_track_combobox_changes = True
    for f_cbox in global_audio_track_comboboxes:
        f_current_index = f_cbox.currentIndex()
        f_cbox.clear()
        f_cbox.clearEditText()
        f_cbox.addItems(list(global_audio_track_names.values()))
        f_cbox.setCurrentIndex(f_current_index)

    global_suppress_audio_track_combobox_changes = False

global_bus_track_names = [_('Master'), _('Bus1'), _('Bus2'), _('Bus3'), _('Bus4')]

#TODO:  Clean these up...
global_beats_per_minute = 140.0
global_beats_per_second = global_beats_per_minute / 60.0
global_bars_per_second = global_beats_per_second * 0.25

def pydaw_set_bpm(a_bpm):
    global global_beats_per_minute, global_beats_per_second,  global_bars_per_second
    global_beats_per_minute = a_bpm
    global_beats_per_second = a_bpm / 60.0
    global_bars_per_second = global_beats_per_second * 0.25
    pydaw_widgets.set_global_tempo(a_bpm)

def pydaw_seconds_to_bars(a_seconds):
    '''converts seconds to regions'''
    return a_seconds * global_bars_per_second

global_audio_px_per_bar = 100.0
global_audio_px_per_beat = 100.0 / 4.0
global_audio_px_per_8th = 100.0 / 8.0
global_audio_px_per_12th = 100.0 / 12.0
global_audio_px_per_16th = 100.0 / 16.0

global_audio_quantize = False
global_audio_quantize_px = None
global_audio_quantize_amt = 1.0

global_audio_ruler_height = 20.0
global_audio_item_height = 75.0

global_audio_item_handle_height = 12.0
global_audio_item_handle_size = 6.25

global_audio_item_handle_brush = QtGui.QLinearGradient(0.0, 0.0, global_audio_item_handle_size,
                                                       global_audio_item_handle_height)
global_audio_item_handle_brush.setColorAt(0.0, QtGui.QColor.fromRgb(255, 255, 255, 120))
global_audio_item_handle_brush.setColorAt(0.0, QtGui.QColor.fromRgb(255, 255, 255, 90))

global_audio_item_handle_selected_brush = \
    QtGui.QLinearGradient(0.0, 0.0, global_audio_item_handle_size,
                          global_audio_item_handle_height)
global_audio_item_handle_selected_brush.setColorAt(0.0, QtGui.QColor.fromRgb(24, 24, 24, 120))
global_audio_item_handle_selected_brush.setColorAt(0.0, QtGui.QColor.fromRgb(24, 24, 24, 90))


global_audio_item_handle_pen = QtGui.QPen(QtCore.Qt.white)
global_audio_item_line_pen = QtGui.QPen(QtCore.Qt.white, 2.0)
global_audio_item_handle_selected_pen = QtGui.QPen(QtGui.QColor.fromRgb(24, 24, 24))
global_audio_item_line_selected_pen = QtGui.QPen(QtGui.QColor.fromRgb(24, 24, 24), 2.0)

global_audio_item_max_lane = 23
global_audio_item_lane_count = 24

global_last_audio_item_dir = global_home

class audio_viewer_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_track_num, a_audio_item, a_sample_length):
        QtGui.QGraphicsRectItem.__init__(self)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.setFlag(QtGui.QGraphicsItem.ItemClipsChildrenToShape)

        self.sample_length = a_sample_length
        self.audio_item = a_audio_item
        self.orig_string = str(a_audio_item)
        self.track_num = a_track_num
        f_graph = this_pydaw_project.get_sample_graph_by_uid(self.audio_item.uid)
        self.painter_paths = f_graph.create_sample_graph(True)
        self.y_inc = global_audio_item_height / len(self.painter_paths)
        f_y_pos = 0.0
        self.path_items = []
        for f_painter_path in self.painter_paths:
            f_path_item = QtGui.QGraphicsPathItem(f_painter_path)
            f_path_item.setBrush(pydaw_audio_item_scene_gradient)
            f_path_item.setParentItem(self)
            f_path_item.mapToParent(0.0, 0.0)
            self.path_items.append(f_path_item)
            f_y_pos += self.y_inc
        f_file_name = this_pydaw_project.get_wav_name_by_uid(self.audio_item.uid)
        f_file_name = this_pydaw_project.timestretch_lookup_orig_path(f_file_name)
        f_name_arr = f_file_name.split("/")
        f_name = f_name_arr[-1]
        self.label = QtGui.QGraphicsSimpleTextItem(f_name, parent=self)
        self.label.setPos(10, 28)
        self.label.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)

        self.start_handle = QtGui.QGraphicsRectItem(parent=self)
        self.start_handle.setAcceptHoverEvents(True)
        self.start_handle.hoverEnterEvent = self.generic_hoverEnterEvent
        self.start_handle.hoverLeaveEvent = self.generic_hoverLeaveEvent
        self.start_handle.setRect(QtCore.QRectF(0.0, 0.0, global_audio_item_handle_size,
                                                global_audio_item_handle_height))
        self.start_handle.mousePressEvent = self.start_handle_mouseClickEvent
        self.start_handle_line = QtGui.QGraphicsLineItem(0.0, global_audio_item_handle_height, 0.0,
        (global_audio_item_height * -1.0) + global_audio_item_handle_height, self.start_handle)

        self.start_handle_line.setPen(global_audio_item_line_pen)

        self.length_handle = QtGui.QGraphicsRectItem(parent=self)
        self.length_handle.setAcceptHoverEvents(True)
        self.length_handle.hoverEnterEvent = self.generic_hoverEnterEvent
        self.length_handle.hoverLeaveEvent = self.generic_hoverLeaveEvent
        self.length_handle.setRect(QtCore.QRectF(0.0, 0.0, global_audio_item_handle_size,
                                                 global_audio_item_handle_height))
        self.length_handle.mousePressEvent = self.length_handle_mouseClickEvent
        self.length_handle_line = QtGui.QGraphicsLineItem(global_audio_item_handle_size,
                                                  global_audio_item_handle_height,
                                                  global_audio_item_handle_size,
                                                  (global_audio_item_height * -1.0) +
                                                  global_audio_item_handle_height,
                                                  self.length_handle)

        self.fade_in_handle = QtGui.QGraphicsRectItem(parent=self)
        self.fade_in_handle.setAcceptHoverEvents(True)
        self.fade_in_handle.hoverEnterEvent = self.generic_hoverEnterEvent
        self.fade_in_handle.hoverLeaveEvent = self.generic_hoverLeaveEvent
        self.fade_in_handle.setRect(QtCore.QRectF(0.0, 0.0, global_audio_item_handle_size,
                                                  global_audio_item_handle_height))
        self.fade_in_handle.mousePressEvent = self.fade_in_handle_mouseClickEvent
        self.fade_in_handle_line = QtGui.QGraphicsLineItem(0.0, 0.0, 0.0, 0.0, self)

        self.fade_out_handle = QtGui.QGraphicsRectItem(parent=self)
        self.fade_out_handle.setAcceptHoverEvents(True)
        self.fade_out_handle.hoverEnterEvent = self.generic_hoverEnterEvent
        self.fade_out_handle.hoverLeaveEvent = self.generic_hoverLeaveEvent
        self.fade_out_handle.setRect(QtCore.QRectF(0.0, 0.0, global_audio_item_handle_size,
                                                   global_audio_item_handle_height))
        self.fade_out_handle.mousePressEvent = self.fade_out_handle_mouseClickEvent
        self.fade_out_handle_line = QtGui.QGraphicsLineItem(0.0, 0.0, 0.0, 0.0, self)

        self.stretch_handle = QtGui.QGraphicsRectItem(parent=self)
        self.stretch_handle.setAcceptHoverEvents(True)
        self.stretch_handle.hoverEnterEvent = self.generic_hoverEnterEvent
        self.stretch_handle.hoverLeaveEvent = self.generic_hoverLeaveEvent
        self.stretch_handle.setRect(QtCore.QRectF(0.0, 0.0, global_audio_item_handle_size,
                                                  global_audio_item_handle_height))
        self.stretch_handle.mousePressEvent = self.stretch_handle_mouseClickEvent
        self.stretch_handle_line = QtGui.QGraphicsLineItem(global_audio_item_handle_size,
            (global_audio_item_handle_height * 0.5) - (global_audio_item_height * 0.5),
            global_audio_item_handle_size,
            (global_audio_item_height * 0.5) + (global_audio_item_handle_height * 0.5),
            self.stretch_handle)
        self.stretch_handle.hide()

        self.split_line = QtGui.QGraphicsLineItem(0.0, 0.0, 0.0, global_audio_item_height, self)
        self.split_line.mapFromParent(0.0, 0.0)
        self.split_line.hide()
        self.split_line_is_shown = False

        self.setAcceptHoverEvents(True)

        self.is_start_resizing = False
        self.is_resizing = False
        self.is_copying = False
        self.is_fading_in = False
        self.is_fading_out = False
        self.is_stretching = False
        self.set_brush()
        self.waveforms_scaled = False
        self.event_pos_orig = None
        self.width_orig = None
        self.vol_linear = pydaw_db_to_lin(self.audio_item.vol)
        self.quantize_offset = 0.0
        if global_tooltips_enabled:
            self.set_tooltips(True)
        self.draw()

    def generic_hoverEnterEvent(self, a_event):
        QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.SizeHorCursor))

    def generic_hoverLeaveEvent(self, a_event):
        QtGui.QApplication.restoreOverrideCursor()

    def draw(self):
        f_temp_seconds = self.sample_length

        if self.audio_item.time_stretch_mode == 1 and \
        (self.audio_item.pitch_shift_end == self.audio_item.pitch_shift):
            f_temp_seconds /= pydaw_pitch_to_ratio(self.audio_item.pitch_shift)
        elif self.audio_item.time_stretch_mode == 2 and \
        (self.audio_item.timestretch_amt_end == self.audio_item.timestretch_amt):
            f_temp_seconds *= self.audio_item.timestretch_amt

        f_start = pydaw_get_pos_in_bars(0, self.audio_item.start_bar, self.audio_item.start_beat)
        f_start *= global_audio_px_per_bar

        f_length_seconds = pydaw_seconds_to_bars(f_temp_seconds) * global_audio_px_per_bar
        self.length_seconds_orig_px = f_length_seconds
        self.rect_orig = QtCore.QRectF(0.0, 0.0, f_length_seconds, global_audio_item_height)
        self.length_px_start = (self.audio_item.sample_start * 0.001 * f_length_seconds)
        self.length_px_minus_start = f_length_seconds - self.length_px_start
        self.length_px_minus_end = (self.audio_item.sample_end * 0.001 * f_length_seconds)
        f_length = self.length_px_minus_end - self.length_px_start

        f_track_num = \
        global_audio_ruler_height + (global_audio_item_height) * self.audio_item.lane_num

        f_fade_in = self.audio_item.fade_in * 0.001
        f_fade_out = self.audio_item.fade_out * 0.001
        self.setRect(0.0, 0.0, f_length, global_audio_item_height)
        f_fade_in_handle_pos = (f_length * f_fade_in)
        f_fade_in_handle_pos = pydaw_clip_value(f_fade_in_handle_pos, 0.0, (f_length - 6.0))
        f_fade_out_handle_pos = (f_length * f_fade_out) - global_audio_item_handle_size
        f_fade_out_handle_pos = pydaw_clip_value(f_fade_out_handle_pos,
                                                 (f_fade_in_handle_pos + 6.0), f_length)
        self.fade_in_handle.setPos(f_fade_in_handle_pos, 0.0)
        self.fade_out_handle.setPos(f_fade_out_handle_pos, 0.0)
        self.update_fade_in_line()
        self.update_fade_out_line()
        self.setPos(f_start, f_track_num)
        self.is_moving = False
        if self.audio_item.time_stretch_mode >= 3 or \
        (self.audio_item.time_stretch_mode == 2 and \
        (self.audio_item.timestretch_amt_end == self.audio_item.timestretch_amt)):
            self.stretch_width_default = f_length / self.audio_item.timestretch_amt

        self.sample_start_offset_px = \
        self.audio_item.sample_start * -0.001 * self.length_seconds_orig_px

        self.start_handle_scene_min = f_start + self.sample_start_offset_px
        self.start_handle_scene_max = self.start_handle_scene_min + self.length_seconds_orig_px

        if not self.waveforms_scaled:
            f_i_inc = 1.0 / len(self.painter_paths)
            f_i = f_i_inc * 1.0
            f_y_inc = 0.0
            f_y_offset = (1.0 - self.vol_linear) * self.y_inc * f_i_inc
            for f_path_item in self.path_items:
                if self.audio_item.reversed:
                    f_path_item.setPos(self.sample_start_offset_px + self.length_seconds_orig_px,
                                       self.y_inc + (f_y_offset * -1.0) + (f_y_inc * f_i))
                    f_path_item.rotate(-180.0)
                else:
                    f_path_item.setPos(self.sample_start_offset_px, f_y_offset + (f_y_inc * f_i))
                f_x_scale, f_y_scale = pydaw_scale_to_rect(pydaw_audio_item_scene_rect,
                                                           self.rect_orig)
                f_y_scale *= self.vol_linear
                f_path_item.scale(f_x_scale, f_y_scale)
                f_i += f_i_inc
                f_y_inc += self.y_inc
        self.waveforms_scaled = True

        self.length_handle.setPos(f_length - global_audio_item_handle_size,
                                  global_audio_item_height - global_audio_item_handle_height)
        self.start_handle.setPos(0.0, global_audio_item_height - global_audio_item_handle_height)
        if self.audio_item.time_stretch_mode >= 2 and \
        (((self.audio_item.time_stretch_mode != 5) and (self.audio_item.time_stretch_mode != 2)) \
        or (self.audio_item.timestretch_amt_end == self.audio_item.timestretch_amt)):
            self.stretch_handle.show()
            self.stretch_handle.setPos(f_length - global_audio_item_handle_size,
                                       (global_audio_item_height * 0.5) -
                                       (global_audio_item_handle_height * 0.5))

    def set_tooltips(self, a_on):
        if a_on:
            self.setToolTip(_("Double click to open editor dialog\nClick and "
                "drag selected to move.\n"
                "Shift+click to split items\nCtrl+drag to copy selected items\n"
                "You can multi-select individual items by CTRL+Alt clicking on them.\n\n"
                "You can glue together multiple items by selecting items and pressing CTRL+G\n"
                ", the glued item will retain all of the fades, stretches and per-item fx of "
                "the original items.\n"))
            self.start_handle.setToolTip(_("Use this handle to resize the item "
                                         "by changing the start point."))
            self.length_handle.setToolTip(_("Use this handle to resize the item "
                                          "by changing the end point."))
            self.fade_in_handle.setToolTip(_("Use this handle to change the fade in."))
            self.fade_out_handle.setToolTip(_("Use this handle to change the fade out."))
            self.stretch_handle.setToolTip(_("Use this handle to resize the item by "
                                           "time-stretching it."))
        else:
            self.setToolTip("")
            self.start_handle.setToolTip("")
            self.length_handle.setToolTip("")
            self.fade_in_handle.setToolTip("")
            self.fade_out_handle.setToolTip("")
            self.stretch_handle.setToolTip("")

    def clip_at_region_end(self):
        f_current_region_length = pydaw_get_current_region_length()
        f_max_x = f_current_region_length * global_audio_px_per_bar
        f_pos_x = self.pos().x()
        f_end = f_pos_x + self.rect().width()
        if f_end > f_max_x:
            f_end_px = f_max_x - f_pos_x
            self.setRect(0.0, 0.0, f_end_px, global_audio_item_height)
            self.audio_item.sample_end = \
                ((self.rect().width() + self.length_px_start) / self.length_seconds_orig_px) \
                * 1000.0
            self.audio_item.sample_end = pydaw_util.pydaw_clip_value(self.audio_item.sample_end,
                                                                     1.0, 1000.0, True)
            self.draw()
            return True
        else:
            return False

    def set_brush(self, a_index=None):
        if self.isSelected():
            self.setBrush(pydaw_selected_gradient)
            self.start_handle.setPen(global_audio_item_handle_selected_pen)
            self.length_handle.setPen(global_audio_item_handle_selected_pen)
            self.fade_in_handle.setPen(global_audio_item_handle_selected_pen)
            self.fade_out_handle.setPen(global_audio_item_handle_selected_pen)
            self.stretch_handle.setPen(global_audio_item_handle_selected_pen)
            self.split_line.setPen(global_audio_item_handle_selected_pen)

            self.start_handle_line.setPen(global_audio_item_line_selected_pen)
            self.length_handle_line.setPen(global_audio_item_line_selected_pen)
            self.fade_in_handle_line.setPen(global_audio_item_line_selected_pen)
            self.fade_out_handle_line.setPen(global_audio_item_line_selected_pen)
            self.stretch_handle_line.setPen(global_audio_item_line_selected_pen)

            self.label.setBrush(QtCore.Qt.darkGray)
            self.start_handle.setBrush(global_audio_item_handle_selected_brush)
            self.length_handle.setBrush(global_audio_item_handle_selected_brush)
            self.fade_in_handle.setBrush(global_audio_item_handle_selected_brush)
            self.fade_out_handle.setBrush(global_audio_item_handle_selected_brush)
            self.stretch_handle.setBrush(global_audio_item_handle_selected_brush)
        else:
            self.start_handle.setPen(global_audio_item_handle_pen)
            self.length_handle.setPen(global_audio_item_handle_pen)
            self.fade_in_handle.setPen(global_audio_item_handle_pen)
            self.fade_out_handle.setPen(global_audio_item_handle_pen)
            self.stretch_handle.setPen(global_audio_item_handle_pen)
            self.split_line.setPen(global_audio_item_handle_pen)

            self.start_handle_line.setPen(global_audio_item_line_pen)
            self.length_handle_line.setPen(global_audio_item_line_pen)
            self.fade_in_handle_line.setPen(global_audio_item_line_pen)
            self.fade_out_handle_line.setPen(global_audio_item_line_pen)
            self.stretch_handle_line.setPen(global_audio_item_line_pen)

            self.label.setBrush(QtCore.Qt.white)
            self.start_handle.setBrush(global_audio_item_handle_brush)
            self.length_handle.setBrush(global_audio_item_handle_brush)
            self.fade_in_handle.setBrush(global_audio_item_handle_brush)
            self.fade_out_handle.setBrush(global_audio_item_handle_brush)
            self.stretch_handle.setBrush(global_audio_item_handle_brush)
            if a_index is None:
                self.setBrush(pydaw_track_gradients[
                self.audio_item.lane_num % len(pydaw_track_gradients)])
            else:
                self.setBrush(pydaw_track_gradients[a_index % len(pydaw_track_gradients)])

    def pos_to_musical_time(self, a_pos):
        f_bar_frac = a_pos / global_audio_px_per_bar
        f_pos_bars = int(f_bar_frac)
        f_pos_beats = round((f_bar_frac - f_pos_bars) * 4.0, 6)
        return(f_pos_bars, f_pos_beats)

    def start_handle_mouseClickEvent(self, a_event):
        if global_transport_is_playing:
            return
        self.check_selected_status()
        a_event.setAccepted(True)
        QtGui.QGraphicsRectItem.mousePressEvent(self.length_handle, a_event)
        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected():
                f_item.min_start = f_item.pos().x() * -1.0
                f_item.is_start_resizing = True
                f_item.setFlag(QtGui.QGraphicsItem.ItemClipsChildrenToShape, False)

    def length_handle_mouseClickEvent(self, a_event):
        if global_transport_is_playing:
            return
        self.check_selected_status()
        a_event.setAccepted(True)
        QtGui.QGraphicsRectItem.mousePressEvent(self.length_handle, a_event)
        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected():
                f_item.is_resizing = True
                f_item.setFlag(QtGui.QGraphicsItem.ItemClipsChildrenToShape, False)

    def fade_in_handle_mouseClickEvent(self, a_event):
        if global_transport_is_playing:
            return
        self.check_selected_status()
        a_event.setAccepted(True)
        QtGui.QGraphicsRectItem.mousePressEvent(self.fade_in_handle, a_event)
        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected():
                f_item.is_fading_in = True

    def fade_out_handle_mouseClickEvent(self, a_event):
        if global_transport_is_playing:
            return
        self.check_selected_status()
        a_event.setAccepted(True)
        QtGui.QGraphicsRectItem.mousePressEvent(self.fade_out_handle, a_event)
        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected():
                f_item.is_fading_out = True

    def stretch_handle_mouseClickEvent(self, a_event):
        if global_transport_is_playing:
            return
        self.check_selected_status()
        a_event.setAccepted(True)
        QtGui.QGraphicsRectItem.mousePressEvent(self.stretch_handle, a_event)
        f_max_region_pos = global_audio_px_per_bar * pydaw_get_current_region_length()
        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected() and f_item.audio_item.time_stretch_mode >= 2:
                f_item.is_stretching = True
                f_item.max_stretch = f_max_region_pos - f_item.pos().x()
                f_item.setFlag(QtGui.QGraphicsItem.ItemClipsChildrenToShape, False)
                #for f_path in f_item.path_items:
                #    f_path.hide()

    def check_selected_status(self):
        """ If a handle is clicked and not selected, clear the selection
            and select only this item
        """
        if not self.isSelected():
            this_audio_items_viewer.scene.clearSelection()
            self.setSelected(True)

    def show_context_menu(self):
        f_menu = QtGui.QMenu(this_main_window)
        f_save_a_copy_action = f_menu.addAction(_("Save a copy"))
        f_save_a_copy_action.triggered.connect(self.save_a_copy)
        f_open_folder_action = f_menu.addAction(_("Open parent folder in browser"))
        f_open_folder_action.triggered.connect(self.open_item_folder)
        f_copy_file_path_action = f_menu.addAction(_("Copy file path to clipboard"))
        f_copy_file_path_action.triggered.connect(self.copy_file_path_to_clipboard)
        f_menu.exec_(QtGui.QCursor.pos())

    def copy_file_path_to_clipboard(self):
        f_path = this_pydaw_project.get_wav_path_by_uid(self.audio_item.uid)
        f_clipboard = QtGui.QApplication.clipboard()
        f_clipboard.setText(f_path)

    def save_a_copy(self):
        global global_last_audio_item_dir
        f_file = QtGui.QFileDialog.getSaveFileName(parent=this_audio_items_viewer,
                                                   caption=_('Save audio item as .wav'),
                                                   directory=global_last_audio_item_dir)
        if not f_file is None and not str(f_file) == "":
            f_file = str(f_file)
            if not f_file.endswith(".wav"):
                f_file += ".wav"
            global_last_audio_item_dir = os.path.dirname(f_file)
            f_orig_path = this_pydaw_project.get_wav_name_by_uid(self.audio_item.uid)
            f_cmd = 'cp "{}" "{}"'.format(f_orig_path, f_file)
            print(f_cmd)
            os.system(f_cmd)

    def open_item_folder(self):
        this_audio_items_viewer_widget.folders_tab_widget.setCurrentIndex(0)
        f_path = this_pydaw_project.get_wav_name_by_uid(self.audio_item.uid)
        f_dir = os.path.dirname(f_path)
        if os.path.isdir(f_dir):
            this_audio_items_viewer_widget.set_folder(f_dir, True)
            f_file = os.path.basename(f_path)
            this_audio_items_viewer_widget.select_file(f_file)
        else:
            QtGui.QMessageBox.warning(this_main_window, _("Error"), \
            _("The folder did not exist:\n\n{}").format(f_dir))

    def mousePressEvent(self, a_event):
        if global_transport_is_playing:
            return

        if a_event.modifiers() == QtCore.Qt.ControlModifier | QtCore.Qt.AltModifier:
            self.setSelected((not self.isSelected()))
            return

        if a_event.button() == QtCore.Qt.RightButton:
            self.show_context_menu()
            return

        if a_event.modifiers() == QtCore.Qt.ShiftModifier:
            f_per_item_fx_dict = \
            this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)

            f_item = self.audio_item
            f_item_old = f_item.clone()
            f_item.fade_in = 0.0
            f_item_old.fade_out = 999.0
            f_width_percent = a_event.pos().x() / self.rect().width()
            f_item.fade_out = pydaw_clip_value(f_item.fade_out, (f_item.fade_in + 90.0),
                                               999.0, True)
            f_item_old.fade_in /= f_width_percent
            f_item_old.fade_in = pydaw_clip_value(f_item_old.fade_in, 0.0,
                                                  (f_item_old.fade_out - 90.0), True)

            f_index = global_audio_items.get_next_index()
            if f_index == -1:
                QtGui.QMessageBox.warning(self, _("Error"),
                _("No more available audio item slots, max per region is {}").format(
                    pydaw_max_audio_item_count))
                return
            else:
                global_audio_items.add_item(f_index, f_item_old)
                f_per_item_fx = f_per_item_fx_dict.get_row(self.track_num)
                if f_per_item_fx is not None:
                    f_per_item_fx_dict.set_row(f_index, f_per_item_fx)
                    f_save_paif = True
                else:
                    f_save_paif = False

            f_event_pos = a_event.pos().x()
            # for items that are not quantized
            f_pos = f_event_pos - (f_event_pos - self.quantize(f_event_pos))
            f_scene_pos = self.quantize(a_event.scenePos().x())
            f_musical_pos = self.pos_to_musical_time(f_scene_pos)
            f_sample_shown = f_item.sample_end - f_item.sample_start
            f_sample_rect_pos = f_pos / self.rect().width()
            f_item.sample_start = (f_sample_rect_pos * f_sample_shown) + f_item.sample_start
            f_item.sample_start = pydaw_clip_value(f_item.sample_start, 0.0, 999.0, True)
            f_item.start_bar = f_musical_pos[0]
            f_item.start_beat = f_musical_pos[1]
            f_item_old.sample_end = f_item.sample_start
            this_pydaw_project.save_audio_region(global_current_region.uid, global_audio_items)
            if f_save_paif:
                this_pydaw_project.save_audio_per_item_fx_region(global_current_region.uid,
                                                                 f_per_item_fx_dict, False)
                this_pydaw_project.this_pydaw_osc.pydaw_audio_per_item_fx_region(
                    global_current_region.uid)
            this_pydaw_project.commit(_("Split audio item"))
            global_open_audio_items(True)
        else:
            if a_event.modifiers() == QtCore.Qt.ControlModifier:
                f_per_item_fx_dict = this_pydaw_project.get_audio_per_item_fx_region(
                    global_current_region.uid)
            #this_audio_item_editor_widget.open_item(self.audio_item)
            QtGui.QGraphicsRectItem.mousePressEvent(self, a_event)
            self.event_pos_orig = a_event.pos().x()
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    f_item_pos = f_item.pos().x()
                    f_item.quantize_offset = f_item_pos - f_item.quantize_all(f_item_pos)
                    if a_event.modifiers() == QtCore.Qt.ControlModifier:
                        f_item.is_copying = True
                        f_item.width_orig = f_item.rect().width()
                        f_item.per_item_fx = f_per_item_fx_dict.get_row(f_item.track_num)
                        this_audio_items_viewer.draw_item(f_item.track_num, f_item.audio_item,
                                                          f_item.sample_length)
                    if self.is_fading_out:
                        f_item.fade_orig_pos = f_item.fade_out_handle.pos().x()
                    elif self.is_fading_in:
                        f_item.fade_orig_pos = f_item.fade_in_handle.pos().x()
                    if self.is_start_resizing:
                        f_item.width_orig = 0.0
                    else:
                        f_item.width_orig = f_item.rect().width()

    def mouseDoubleClickEvent(self, a_event):
        this_audio_items_viewer_widget.folders_tab_widget.setCurrentIndex(2)

    def hoverEnterEvent(self, a_event):
        f_item_pos = self.pos().x()
        self.quantize_offset = f_item_pos - self.quantize_all(f_item_pos)

    def hoverMoveEvent(self, a_event):
        if a_event.modifiers() == QtCore.Qt.ShiftModifier:
            if not self.split_line_is_shown:
                self.split_line_is_shown = True
                self.split_line.show()
            f_x = a_event.pos().x()
            f_x = self.quantize_all(f_x)
            f_x -= self.quantize_offset
            self.split_line.setPos(f_x, 0.0)
        else:
            if self.split_line_is_shown:
                self.split_line_is_shown = False
                self.split_line.hide()

    def hoverLeaveEvent(self, a_event):
        if self.split_line_is_shown:
            self.split_line_is_shown = False
            self.split_line.hide()

    def y_pos_to_lane_number(self, a_y_pos):
        f_lane_num = int((a_y_pos - global_audio_ruler_height) / global_audio_item_height)
        f_lane_num = pydaw_clip_value(f_lane_num, 0, global_audio_item_max_lane)
        f_y_pos = (f_lane_num * global_audio_item_height) + global_audio_ruler_height
        return f_lane_num, f_y_pos

    def quantize_all(self, a_x):
        f_x = a_x
        if global_audio_quantize:
            f_x = round(f_x / global_audio_quantize_px) * global_audio_quantize_px
        return f_x

    def quantize(self, a_x):
        f_x = a_x
        f_x = self.quantize_all(f_x)
        if global_audio_quantize and f_x < global_audio_quantize_px:
            f_x = global_audio_quantize_px
        return f_x

    def quantize_start(self, a_x):
        f_x = a_x
        f_x = self.quantize_all(f_x)
        if f_x >= self.length_handle.pos().x():
            f_x -= global_audio_quantize_px
        return f_x

    def quantize_scene(self, a_x):
        f_x = a_x
        f_x = self.quantize_all(f_x)
        return f_x

    def update_fade_in_line(self):
        f_pos = self.fade_in_handle.pos()
        self.fade_in_handle_line.setLine(f_pos.x(), 0.0, 0.0, global_audio_item_height)

    def update_fade_out_line(self):
        f_pos = self.fade_out_handle.pos()
        self.fade_out_handle_line.setLine(f_pos.x() + global_audio_item_handle_size,
                                          0.0, self.rect().width(), global_audio_item_height)

    def mouseMoveEvent(self, a_event):
        if global_transport_is_playing or self.event_pos_orig is None:
            return
        f_event_pos = a_event.pos().x()
        f_event_diff = f_event_pos - self.event_pos_orig
        if self.is_resizing:
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    f_x = f_item.width_orig + f_event_diff + f_item.quantize_offset
                    f_x = pydaw_clip_value(f_x, global_audio_item_handle_size,
                                           f_item.length_px_minus_start)
                    f_x = f_item.quantize(f_x)
                    f_x -= f_item.quantize_offset
                    f_item.length_handle.setPos(f_x - global_audio_item_handle_size,
                                                global_audio_item_height -
                                                global_audio_item_handle_height)
        elif self.is_start_resizing:
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    f_x = f_item.width_orig + f_event_diff + f_item.quantize_offset
                    f_x = pydaw_clip_value(f_x, f_item.sample_start_offset_px,
                                           f_item.length_handle.pos().x())
                    f_x = pydaw_clip_min(f_x, f_item.min_start)
                    f_x = f_item.quantize_start(f_x)
                    f_x -= f_item.quantize_offset
                    f_item.start_handle.setPos(f_x, global_audio_item_height -
                                               global_audio_item_handle_height)
        elif self.is_fading_in:
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    #f_x = f_event_pos #f_item.width_orig + f_event_diff
                    f_x = f_item.fade_orig_pos + f_event_diff
                    f_x = pydaw_clip_value(f_x, 0.0, f_item.fade_out_handle.pos().x() - 4.0)
                    f_item.fade_in_handle.setPos(f_x, 0.0)
                    f_item.update_fade_in_line()
        elif self.is_fading_out:
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    f_x = f_item.fade_orig_pos + f_event_diff
                    f_x = pydaw_clip_value(f_x, f_item.fade_in_handle.pos().x() + 4.0,
                                           f_item.width_orig - global_audio_item_handle_size)
                    f_item.fade_out_handle.setPos(f_x, 0.0)
                    f_item.update_fade_out_line()
        elif self.is_stretching:
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected() and f_item.audio_item.time_stretch_mode >= 2:
                    f_x = f_item.width_orig + f_event_diff + f_item.quantize_offset
                    if f_item.audio_item.time_stretch_mode == 2:
                        f_x = pydaw_clip_value(f_x, f_item.stretch_width_default * 0.25,
                                               f_item.stretch_width_default * 4.0)
                    elif f_item.audio_item.time_stretch_mode == 6:
                        f_x = pydaw_clip_value(f_x, f_item.stretch_width_default * 0.5,
                                               f_item.stretch_width_default * 10.0)
                    else:
                        f_x = pydaw_clip_value(f_x, f_item.stretch_width_default * 0.1,
                                               f_item.stretch_width_default * 10.0)
                    f_x = pydaw_clip_max(f_x, f_item.max_stretch)
                    f_x = f_item.quantize(f_x)
                    f_x -= f_item.quantize_offset
                    f_item.stretch_handle.setPos(f_x - global_audio_item_handle_size,
                                                 (global_audio_item_height * 0.5) -
                                                 (global_audio_item_handle_height * 0.5))
        else:
            QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
            if global_audio_quantize:
                f_max_x = (pydaw_get_current_region_length() * global_audio_px_per_bar) - \
                           global_audio_quantize_px
            else:
                f_max_x = (pydaw_get_current_region_length() * global_audio_px_per_bar) - \
                           global_audio_item_handle_size
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    f_pos_x = f_item.pos().x()
                    f_pos_y = f_item.pos().y()
                    f_pos_x = pydaw_clip_value(f_pos_x, 0.0, f_max_x)
                    f_ignored, f_pos_y = f_item.y_pos_to_lane_number(f_pos_y)
                    f_pos_x = f_item.quantize_scene(f_pos_x)
                    f_item.setPos(f_pos_x, f_pos_y)
                    if not f_item.is_moving:
                        f_item.setGraphicsEffect(QtGui.QGraphicsOpacityEffect())
                        f_item.is_moving = True

    def mouseReleaseEvent(self, a_event):
        if global_transport_is_playing or self.event_pos_orig is None:
            return
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        QtGui.QApplication.restoreOverrideCursor()
        f_audio_items =  global_audio_items
        #Set to True when testing, set to False for better UI performance...
        f_reset_selection = True
        f_did_change = False
        f_was_stretching = False
        f_stretched_items = []
        f_event_pos = a_event.pos().x()
        f_event_diff = f_event_pos - self.event_pos_orig
        if self.is_copying:
            f_was_copying = True
            f_per_item_fx_dict = \
            this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)
        else:
            f_was_copying = False
        for f_audio_item in this_audio_items_viewer.audio_items:
            if f_audio_item.isSelected():
                f_item = f_audio_item.audio_item
                f_pos_x = f_audio_item.pos().x()
                if f_audio_item.is_resizing:
                    f_x = f_audio_item.width_orig + f_event_diff + f_audio_item.quantize_offset
                    f_x = pydaw_clip_value(f_x, global_audio_item_handle_size,
                                           f_audio_item.length_px_minus_start)
                    f_x = f_audio_item.quantize(f_x)
                    f_x -= f_audio_item.quantize_offset
                    f_audio_item.setRect(0.0, 0.0, f_x, global_audio_item_height)
                    f_item.sample_end = ((f_audio_item.rect().width() + \
                    f_audio_item.length_px_start) / f_audio_item.length_seconds_orig_px) * 1000.0
                    f_item.sample_end = \
                    pydaw_util.pydaw_clip_value(f_item.sample_end, 1.0, 1000.0, True)
                elif f_audio_item.is_start_resizing:
                    f_x = f_audio_item.start_handle.scenePos().x()
                    f_x = pydaw_clip_min(f_x, 0.0)
                    f_x = self.quantize_all(f_x)
                    if f_x < f_audio_item.sample_start_offset_px:
                        f_x = f_audio_item.sample_start_offset_px
                    f_start_result = self.pos_to_musical_time(f_x)
                    f_item.start_bar = f_start_result[0]
                    f_item.start_beat = f_start_result[1]
                    f_item.sample_start = ((f_x - f_audio_item.start_handle_scene_min) / \
                    (f_audio_item.start_handle_scene_max - f_audio_item.start_handle_scene_min)) \
                    * 1000.0
                    f_item.sample_start = pydaw_clip_value(f_item.sample_start, 0.0, 999.0, True)
                elif f_audio_item.is_fading_in:
                    f_pos = f_audio_item.fade_in_handle.pos().x()
                    f_val = (f_pos / f_audio_item.rect().width()) * 1000.0
                    f_item.fade_in = pydaw_clip_value(f_val, 0.0, 997.0, True)
                elif f_audio_item.is_fading_out:
                    f_pos = f_audio_item.fade_out_handle.pos().x()
                    f_val = \
                    ((f_pos + global_audio_item_handle_size) / (f_audio_item.rect().width())) \
                    * 1000.0
                    f_item.fade_out = pydaw_clip_value(f_val, 1.0, 998.0, True)
                elif f_audio_item.is_stretching and f_item.time_stretch_mode >= 2:
                    f_reset_selection = True
                    f_x = f_audio_item.width_orig + f_event_diff + f_audio_item.quantize_offset
                    if f_audio_item.audio_item.time_stretch_mode == 2:
                        f_x = pydaw_clip_value(f_x, f_audio_item.stretch_width_default * 0.25,
                                               f_audio_item.stretch_width_default * 4.0)
                    elif f_audio_item.audio_item.time_stretch_mode == 6:
                        f_x = pydaw_clip_value(f_x, f_audio_item.stretch_width_default * 0.5,
                                               f_audio_item.stretch_width_default * 10.0)
                    else:
                        f_x = pydaw_clip_value(f_x, f_audio_item.stretch_width_default * 0.1,
                                               f_audio_item.stretch_width_default * 10.0)
                    f_x = pydaw_clip_max(f_x, f_audio_item.max_stretch)
                    f_x = f_audio_item.quantize(f_x)
                    f_x -= f_audio_item.quantize_offset
                    f_item.timestretch_amt = f_x / f_audio_item.stretch_width_default
                    f_item.timestretch_amt_end = f_item.timestretch_amt
                    if f_item.time_stretch_mode >= 3 and f_audio_item.orig_string != str(f_item):
                        f_was_stretching = True
                        f_ts_result = this_pydaw_project.timestretch_audio_item(f_item)
                        if f_ts_result is not None:
                            f_stretched_items.append(f_ts_result)
                    f_audio_item.setRect(0.0, 0.0, f_x, global_audio_item_height)
                else:
                    f_pos_y = f_audio_item.pos().y()
                    if f_audio_item.is_copying:
                        f_reset_selection = True
                        f_item_old = f_item.clone()
                        f_index = f_audio_items.get_next_index()
                        if f_index == -1:
                            QtGui.QMessageBox.warning(self, _("Error"),
                            _("No more available audio item slots, max per region is"
                            " {}").format(pydaw_max_audio_item_count))
                            break
                        else:
                            f_audio_items.add_item(f_index, f_item_old)
                            if f_audio_item.per_item_fx is not None:
                                f_per_item_fx_dict.set_row(f_index, f_audio_item.per_item_fx)
                    else:
                        f_audio_item.set_brush(f_item.lane_num)
                    f_pos_x = self.quantize_all(f_pos_x)
                    f_item.lane_num, f_pos_y = self.y_pos_to_lane_number(f_pos_y)
                    f_audio_item.setPos(f_pos_x, f_pos_y)
                    f_start_result = f_audio_item.pos_to_musical_time(f_pos_x)
                    f_item.set_pos(f_start_result[0], f_start_result[1])
                f_audio_item.clip_at_region_end()
                f_item_str = str(f_item)
                if f_item_str != f_audio_item.orig_string:
                    f_audio_item.orig_string = f_item_str
                    f_did_change = True
                    if not f_reset_selection:
                        f_audio_item.draw()
            f_audio_item.is_moving = False
            f_audio_item.is_resizing = False
            f_audio_item.is_start_resizing = False
            f_audio_item.is_copying = False
            f_audio_item.is_fading_in = False
            f_audio_item.is_fading_out = False
            f_audio_item.is_stretching = False
            f_audio_item.setGraphicsEffect(None)
            f_audio_item.setFlag(QtGui.QGraphicsItem.ItemClipsChildrenToShape)
        if f_did_change:
            f_audio_items.deduplicate_items()
            if f_was_copying:
                this_pydaw_project.save_audio_per_item_fx_region(global_current_region.uid,
                                                                 f_per_item_fx_dict, False)
                this_pydaw_project.this_pydaw_osc.pydaw_audio_per_item_fx_region(
                    global_current_region.uid)
            if f_was_stretching:
                this_pydaw_project.save_stretch_dicts()
                for f_stretch_item in f_stretched_items:
                    f_stretch_item[2].wait()
                    this_pydaw_project.get_wav_uid_by_name(f_stretch_item[0],
                                                           a_uid=f_stretch_item[1])
            this_pydaw_project.save_audio_region(global_current_region.uid, f_audio_items)
            this_pydaw_project.commit(_("Update audio items"))
            global_open_audio_items(f_reset_selection)

global_audio_items_header_gradient = QtGui.QLinearGradient(0.0, 0.0, 0.0,
                                                           global_audio_ruler_height)
global_audio_items_header_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(61, 61, 61))
global_audio_items_header_gradient.setColorAt(0.5, QtGui.QColor.fromRgb(50,50, 50))
global_audio_items_header_gradient.setColorAt(0.6, QtGui.QColor.fromRgb(43, 43, 43))
global_audio_items_header_gradient.setColorAt(1.0, QtGui.QColor.fromRgb(65, 65, 65))

class audio_items_viewer(QtGui.QGraphicsView):
    def __init__(self):
        QtGui.QGraphicsView.__init__(self)
        self.reset_line_lists()
        self.h_zoom = 1.0
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.dropEvent = self.sceneDropEvent
        self.scene.dragEnterEvent = self.sceneDragEnterEvent
        self.scene.dragMoveEvent = self.sceneDragMoveEvent
        self.scene.contextMenuEvent = self.sceneContextMenuEvent
        self.scene.setBackgroundBrush(QtGui.QColor(90, 90, 90))
        self.scene.selectionChanged.connect(self.scene_selection_changed)
        self.setAcceptDrops(True)
        self.setScene(self.scene)
        self.audio_items = []
        self.track = 0
        self.gradient_index = 0
        self.playback_px = 0.0
        self.snap_draw_extra_lines = False
        self.snap_extra_lines_div = global_audio_px_per_8th
        self.snap_extra_lines_range = 8
        self.draw_headers(0)
        self.setAlignment(QtCore.Qt.AlignTop | QtCore.Qt.AlignLeft)
        self.setDragMode(QtGui.QGraphicsView.RubberBandDrag)
        self.is_playing = False
        self.last_x_scale = 1.0
        self.reselect_on_stop = []
        self.playback_cursor = None
        self.playback_inc_count = 0
        #Somewhat slow on my AMD 5450 using the FOSS driver
        #self.setRenderHint(QtGui.QPainter.Antialiasing)

    def reset_line_lists(self):
        self.text_list = []
        self.beat_line_list = []

    def prepare_to_quit(self):
        self.scene.clearSelection()
        self.scene.clear()

    def set_tooltips(self, a_on):
        for f_item in self.audio_items:
            f_item.set_tooltips(a_on)

    def resizeEvent(self, a_event):
        QtGui.QGraphicsView.resizeEvent(self, a_event)
        self.scale_to_region_size()

    def scale_to_region_size(self):
        f_width = float(self.rect().width()) - float(self.verticalScrollBar().width()) - 6.0
        if global_current_region is None:
            f_region_length = 8
        else:
            f_region_length = pydaw_get_current_region_length()
        f_region_px = f_region_length * global_audio_px_per_bar
        f_new_scale = f_width / f_region_px
        if self.last_x_scale != f_new_scale:
            self.scale(1.0 / self.last_x_scale, 1.0)
            self.last_x_scale = f_new_scale
            self.scale(self.last_x_scale, 1.0)
        self.horizontalScrollBar().setSliderPosition(0)

    def sceneContextMenuEvent(self, a_event):
        if self.check_running():
            return
        QtGui.QGraphicsScene.contextMenuEvent(self.scene, a_event)
        self.context_menu_pos = a_event.scenePos()
        f_menu = QtGui.QMenu(this_main_window)
        f_paste_action = QtGui.QAction(_("Paste file path from clipboard"), self)
        f_paste_action.triggered.connect(self.on_scene_paste_paths)
        f_menu.addAction(f_paste_action)
        f_menu.exec_(a_event.screenPos())

    def on_scene_paste_paths(self):
        f_clipboard = QtGui.QApplication.clipboard()
        f_path = f_clipboard.text()
        if f_path is None:
            QtGui.QMessageBox.warning(self, _("Error"), _("No text in the system clipboard"))
        else:
            f_path = str(f_path)
            if os.path.isfile(f_path):
                self.add_items(self.context_menu_pos.x(), self.context_menu_pos.y(), [f_path])
            else:
                f_path = f_path[100:]
                QtGui.QMessageBox.warning(self, _("Error"),
                                          _("{} is not a valid file").format(f_path))

    def scene_selection_changed(self):
        f_selected_items = []
        global global_current_audio_item_index
        for f_item in self.audio_items:
            f_item.set_brush()
            if f_item.isSelected():
                f_selected_items.append(f_item)
        if len(f_selected_items) == 1:
            global_current_audio_item_index = f_selected_items[0].track_num
            this_audio_items_viewer_widget.modulex.widget.setEnabled(True)
            f_paif = this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)
            this_audio_items_viewer_widget.modulex.set_from_list(
                f_paif.get_row(global_current_audio_item_index))
        elif len(f_selected_items) == 0:
            global_current_audio_item_index = None
            this_audio_items_viewer_widget.modulex.widget.setDisabled(True)
        else:
            this_audio_items_viewer_widget.modulex.widget.setDisabled(True)

        this_audio_items_viewer_widget.set_paif_buttons_enabled(len(f_selected_items))

        f_timestretch_checked = True
        if len( f_selected_items) > 1:
            f_time_stretch_mode_val = f_selected_items[0].audio_item.time_stretch_mode
            f_time_stretch_amt_val = f_selected_items[0].audio_item.timestretch_amt
            f_pitch_val = f_selected_items[0].audio_item.pitch_shift
            f_time_stretch_amt_end_val = f_selected_items[0].audio_item.timestretch_amt_end
            f_pitch_end_val = f_selected_items[0].audio_item.pitch_shift_end
            f_crispness_val = f_selected_items[0].audio_item.crispness
            for f_item in f_selected_items[1:]:
                if f_item.audio_item.time_stretch_mode != f_time_stretch_mode_val or \
                f_item.audio_item.timestretch_amt != f_time_stretch_amt_val or \
                f_item.audio_item.pitch_shift != f_pitch_val or \
                f_item.audio_item.pitch_shift_end != f_pitch_end_val or \
                f_item.audio_item.timestretch_amt_end != f_time_stretch_amt_end_val or \
                f_item.audio_item.crispness != f_crispness_val:
                    f_timestretch_checked = False
                    break
        this_audio_item_editor_widget.timestretch_checkbox.setChecked(f_timestretch_checked)
        f_output_checked = True
        if len( f_selected_items) > 1:
            f_output_val = f_selected_items[0].audio_item.output_track
            for f_item in f_selected_items[1:]:
                if f_item.audio_item.output_track != f_output_val:
                    f_output_checked = False
                    break
        this_audio_item_editor_widget.output_checkbox.setChecked(f_output_checked)
        f_vol_checked = True
        if len( f_selected_items) > 1:
            f_vol_val = f_selected_items[0].audio_item.vol
            for f_item in f_selected_items[1:]:
                if f_item.audio_item.vol != f_vol_val:
                    f_vol_checked = False
                    break
        this_audio_item_editor_widget.vol_checkbox.setChecked(f_vol_checked)

        f_reverse_checked = True
        if len( f_selected_items) > 1:
            f_reverse_val = f_selected_items[0].audio_item.reversed
            for f_item in f_selected_items[1:]:
                if f_item.audio_item.reversed != f_reverse_val:
                    f_reverse_checked = False
                    break
        this_audio_item_editor_widget.reversed_checkbox.setChecked(f_reverse_checked)


        f_fadein_vol_checked = True
        if len( f_selected_items) > 1:
            f_fadein_val = f_selected_items[0].audio_item.fadein_vol
            for f_item in f_selected_items[1:]:
                if f_item.audio_item.fadein_vol != f_fadein_val:
                    f_fadein_vol_checked = False
                    break
        this_audio_item_editor_widget.fadein_vol_checkbox.setChecked(f_fadein_vol_checked)

        f_fadeout_vol_checked = True
        if len( f_selected_items) > 1:
            f_fadeout_val = f_selected_items[0].audio_item.fadeout_vol
            for f_item in f_selected_items[1:]:
                if f_item.audio_item.fadeout_vol != f_fadeout_val:
                    f_fadeout_vol_checked = False
                    break
        this_audio_item_editor_widget.fadeout_vol_checkbox.setChecked(f_fadeout_vol_checked)


        if len(f_selected_items) > 0:
            if f_timestretch_checked:
                this_audio_item_editor_widget.timestretch_mode.setCurrentIndex(
                f_selected_items[0].audio_item.time_stretch_mode)

                if f_selected_items[0].audio_item.timestretch_amt_end != \
                f_selected_items[0].audio_item.timestretch_amt:
                    this_audio_item_editor_widget.timestretch_amt_end_checkbox.setChecked(True)
                else:
                    this_audio_item_editor_widget.timestretch_amt_end_checkbox.setChecked(False)

                if f_selected_items[0].audio_item.pitch_shift_end != \
                f_selected_items[0].audio_item.pitch_shift:
                    this_audio_item_editor_widget.pitch_shift_end_checkbox.setChecked(True)
                else:
                    this_audio_item_editor_widget.pitch_shift_end_checkbox.setChecked(False)

                this_audio_item_editor_widget.pitch_shift.setValue(
                    f_selected_items[0].audio_item.pitch_shift)
                this_audio_item_editor_widget.timestretch_amt.setValue(
                    f_selected_items[0].audio_item.timestretch_amt)
                this_audio_item_editor_widget.pitch_shift_end.setValue(
                    f_selected_items[0].audio_item.pitch_shift_end)
                this_audio_item_editor_widget.timestretch_amt_end.setValue(
                f_selected_items[0].audio_item.timestretch_amt_end)
                this_audio_item_editor_widget.crispness_combobox.setCurrentIndex(
                f_selected_items[0].audio_item.crispness)
            if f_output_checked:
                this_audio_item_editor_widget.output_combobox.setCurrentIndex(
                f_selected_items[0].audio_item.output_track)
            if f_vol_checked:
                this_audio_item_editor_widget.sample_vol_slider.setValue(
                    f_selected_items[0].audio_item.vol)
            if f_reverse_checked:
                this_audio_item_editor_widget.is_reversed_checkbox.setChecked(
                    f_selected_items[0].audio_item.reversed)
            if f_fadein_vol_checked:
                this_audio_item_editor_widget.fadein_vol_spinbox.setValue(
                    f_selected_items[0].audio_item.fadein_vol)
            if f_fadeout_vol_checked:
                this_audio_item_editor_widget.fadeout_vol_spinbox.setValue(
                    f_selected_items[0].audio_item.fadeout_vol)

    def sceneDragEnterEvent(self, a_event):
        a_event.setAccepted(True)

    def sceneDragMoveEvent(self, a_event):
        a_event.setDropAction(QtCore.Qt.CopyAction)

    def check_running(self):
        if pydaw_global_current_region_is_none() or \
        this_transport.is_playing or this_transport.is_recording:
            return True
        if global_pydaw_subprocess is None:
            QtGui.QMessageBox.warning(this_main_window, _("Error"),
            _("The audio engine is not running, audio items cannot be added.\n"
            "If the audio engine crashed, you will need to restart PyDAW."))
            return True
        return False

    def sceneDropEvent(self, a_event):
        if len(global_audio_items_to_drop) == 0:
            return
        f_x = a_event.scenePos().x()
        f_y = a_event.scenePos().y()
        self.add_items(f_x, f_y, global_audio_items_to_drop)

    def add_items(self, f_x, f_y, a_item_list):
        if self.check_running():
            return
        if global_current_region.region_length_bars == 0:
            f_max_start = 7
        else:
            f_max_start = global_current_region.region_length_bars - 1

        f_pos_bars = int(f_x / global_audio_px_per_bar)
        f_pos_bars = pydaw_clip_value(f_pos_bars, 0, f_max_start)

        if f_pos_bars == f_max_start:
            f_beat_frac = 0.0
        else:
            f_beat_frac = ((f_x % global_audio_px_per_bar) / global_audio_px_per_bar) * 4.0
            f_beat_frac = pydaw_clip_value(f_beat_frac, 0.0, 3.99, a_round=True)
        print("{}".format(f_beat_frac))
        if global_audio_quantize:
            f_beat_frac = \
                int(f_beat_frac * global_audio_quantize_amt) / global_audio_quantize_amt

        print("{} {}".format(f_pos_bars, f_beat_frac))

        f_lane_num = int((f_y - global_audio_ruler_height) / global_audio_item_height)
        f_lane_num = pydaw_clip_value(f_lane_num, 0, global_audio_item_max_lane)

        f_items = this_pydaw_project.get_audio_region(global_current_region.uid)

        for f_file_name in a_item_list:
            f_file_name_str = str(f_file_name)
            if not f_file_name_str is None and not f_file_name_str == "":
                f_index = f_items.get_next_index()
                if f_index == -1:
                    QtGui.QMessageBox.warning(self, _("Error"),
                    _("No more available audio item slots, "
                    "max per region is {}").format(pydaw_max_audio_item_count))
                    break
                else:
                    f_uid = this_pydaw_project.get_wav_uid_by_name(f_file_name_str)
                    f_item = pydaw_audio_item(f_uid, a_start_bar=f_pos_bars,
                                              a_start_beat=f_beat_frac,
                                              a_lane_num=f_lane_num)
                    f_items.add_item(f_index, f_item)
                    f_graph = this_pydaw_project.get_sample_graph_by_uid(f_uid)
                    f_audio_item = this_audio_items_viewer.draw_item(f_index, f_item,
                                                                     f_graph.length_in_seconds)
                    f_audio_item.clip_at_region_end()
        this_pydaw_project.save_audio_region(global_current_region.uid, f_items)
        this_pydaw_project.commit(
            _("Added audio items to region {}").format(global_current_region.uid))
        global_open_audio_items()
        self.last_open_dir = os.path.dirname(f_file_name_str)

    def keyPressEvent(self, a_event):
        if pydaw_global_current_region_is_none():
            return
        if a_event.key() == QtCore.Qt.Key_Delete:
            f_items = this_pydaw_project.get_audio_region(global_current_region.uid)
            f_paif = this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)
            for f_item in self.audio_items:
                if f_item.isSelected():
                    f_items.remove_item(f_item.track_num)
                    f_paif.clear_row_if_exists(f_item.track_num)
            this_pydaw_project.save_audio_region(global_current_region.uid, f_items)
            this_pydaw_project.save_audio_per_item_fx_region(global_current_region.uid,
                                                             f_paif, False)
            this_pydaw_project.commit(_("Delete audio item(s)"))
            global_open_audio_items(True)
        if a_event.key() == QtCore.Qt.Key_G and a_event.modifiers() == QtCore.Qt.ControlModifier:
            f_region_uid = global_current_region.uid
            f_indexes = []
            f_start_bar = None
            f_end_bar = None
            f_lane = None
            f_audio_track_num = None
            for f_item in self.audio_items:
                if f_item.isSelected():
                    f_indexes.append(f_item.track_num)
                    if f_start_bar is None or f_start_bar > f_item.audio_item.start_bar:
                        f_start_bar = f_item.audio_item.start_bar
                        f_lane = f_item.audio_item.lane_num
                        f_audio_track_num = f_item.audio_item.output_track
                    f_end, f_beat = \
                    f_item.pos_to_musical_time(f_item.pos().x() + f_item.rect().width())
                    if f_beat > 0.0:
                        f_end += 1
                    if f_end_bar is None or f_end_bar < f_end:
                        f_end_bar = f_end
            if len(f_indexes) == 0:
                print(_("No audio items selected, not glueing"))
                return
            f_path = this_pydaw_project.get_next_glued_file_name()
            this_pydaw_project.this_pydaw_osc.pydaw_glue_audio(f_path, global_current_song_index,
                                                               f_start_bar, f_end_bar, f_indexes)
            f_items = this_pydaw_project.get_audio_region(f_region_uid)
            f_paif = this_pydaw_project.get_audio_per_item_fx_region(f_region_uid)
            for f_index in f_indexes:
                f_items.remove_item(f_index)
                f_paif.clear_row_if_exists(f_index)
            f_index = f_items.get_next_index()
            f_uid = this_pydaw_project.get_wav_uid_by_name(f_path)
            f_item = pydaw_audio_item(f_uid, a_start_bar=f_start_bar, a_lane_num=f_lane,
                                      a_output_track=f_audio_track_num)
            f_items.add_item(f_index, f_item)

            this_pydaw_project.save_audio_region(f_region_uid, f_items)
            this_pydaw_project.save_audio_per_item_fx_region(f_region_uid, f_paif)
            this_pydaw_project.this_pydaw_osc.pydaw_audio_per_item_fx_region(f_region_uid)
            this_pydaw_project.commit(_("Glued audio items"))
            global_open_audio_items()


    def set_playback_pos(self, a_bar=None, a_beat=0.0):
        if a_bar is None:
            f_bar = this_transport.get_bar_value()
        else:
            f_bar = int(a_bar)
        f_beat = float(a_beat)
        f_pos = (f_bar * global_audio_px_per_bar) + (f_beat * global_audio_px_per_beat)
        self.playback_cursor.setPos(f_pos, 0.0)
        self.playback_inc_count = 0

    def set_playback_clipboard(self):
        self.reselect_on_stop = []
        for f_item in self.audio_items:
            if f_item.isSelected():
                self.reselect_on_stop.append(str(f_item.audio_item))

    def start_playback(self, a_bpm):
        self.is_playing = True

    def stop_playback(self, a_bar=None):
        if self.is_playing:
            self.is_playing = False
            self.reset_selection()
            self.set_playback_pos(a_bar)

    def reset_selection(self):
        for f_item in self.audio_items:
            if str(f_item.audio_item) in self.reselect_on_stop:
                f_item.setSelected(True)

    def set_zoom(self, a_scale):
        self.scale(a_scale, 1.0)

    def set_h_zoom_value(self, a_scale):
        self.h_zoom = a_scale

    def set_v_zoom(self, a_scale):
        self.scale(1.0, a_scale)

    def set_grid_div(self, a_enabled, a_div, a_range):
        self.snap_draw_extra_lines = a_enabled
        self.snap_extra_lines_div = float(a_div)
        self.snap_extra_lines_range = int(a_range)
        self.snap_extra_lines_beat_skip = self.snap_extra_lines_range / 4
        if global_current_region is None:
            self.clear_drawn_items()
        else:
            global_open_audio_items(True)

    def ruler_click_event(self, a_event):
        if not global_transport_is_playing:
            f_val = int(a_event.pos().x() / global_audio_px_per_bar)
            this_transport.set_bar_value(f_val)

    def check_line_count(self):
        """ Check that there are not too many vertical lines on the screen """
        f_num_count = len(self.text_list)
        if f_num_count == 0:
            return
        f_num_visible_count = int(f_num_count / pydaw_clip_min(self.h_zoom, 1))

        if f_num_visible_count > 24:
            for f_line in self.beat_line_list:
                f_line.setVisible(False)
            f_factor = f_num_visible_count // 24
            if f_factor == 1:
                for f_num in self.text_list:
                    f_num.setVisible(True)
            else:
                f_factor = int(round(f_factor / 2.0) * 2)
                for f_num in self.text_list:
                    f_num.setVisible(False)
                for f_num in self.text_list[::f_factor]:
                    f_num.setVisible(True)
        else:
            for f_line in self.beat_line_list:
                f_line.setVisible(True)
            for f_num in self.text_list:
                f_num.setVisible(True)


    def draw_headers(self, a_cursor_pos=None, a_default_length=False):
        if a_default_length:
            f_region_length = 8
        else:
            f_region_length = pydaw_get_current_region_length()

        f_size = global_audio_px_per_bar * f_region_length
        f_ruler = QtGui.QGraphicsRectItem(0, 0, f_size, global_audio_ruler_height)
        f_ruler.setBrush(global_audio_items_header_gradient)
        f_ruler.mousePressEvent = self.ruler_click_event
        self.scene.addItem(f_ruler)
        f_v_pen = QtGui.QPen(QtCore.Qt.black)
        f_beat_pen = QtGui.QPen(QtGui.QColor(210, 210, 210))
        f_16th_pen = QtGui.QPen(QtGui.QColor(120, 120, 120))
        f_reg_pen = QtGui.QPen(QtCore.Qt.white)
        f_total_height = (global_audio_item_lane_count * (global_audio_item_height)) + \
            global_audio_ruler_height
        self.scene.setSceneRect(0.0, 0.0, f_size, f_total_height)
        self.playback_cursor = self.scene.addLine(0.0, 0.0, 0.0, f_total_height,
                                                  QtGui.QPen(QtCore.Qt.red, 2.0))
        self.playback_cursor.setZValue(1000.0)
        i3 = 0.0
        for i in range(f_region_length):
            f_number = QtGui.QGraphicsSimpleTextItem("{}".format(i + 1), f_ruler)
            f_number.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
            f_number.setBrush(QtCore.Qt.white)
            f_number.setZValue(1000.0)
            self.text_list.append(f_number)
            self.scene.addLine(i3, 0.0, i3, f_total_height, f_v_pen)
            f_number.setPos(i3 + 3.0, 2)
            for f_beat_i in range(1, 4):
                f_beat_x = i3 + (global_audio_px_per_beat * f_beat_i)
                f_line = self.scene.addLine(f_beat_x, 0.0, f_beat_x, f_total_height, f_beat_pen)
                self.beat_line_list.append(f_line)
            if self.snap_draw_extra_lines:
                for f_16th_i in range(1, self.snap_extra_lines_range):
                    if f_16th_i % self.snap_extra_lines_beat_skip != 0:
                        f_16th_x = i3 + (self.snap_extra_lines_div * f_16th_i)
                        f_line = self.scene.addLine(f_16th_x, global_audio_ruler_height,
                                                    f_16th_x, f_total_height, f_16th_pen)
                        self.beat_line_list.append(f_line)
            i3 += global_audio_px_per_bar
        self.scene.addLine(i3, global_audio_ruler_height, i3, f_total_height, f_reg_pen)
        for i2 in range(global_audio_item_lane_count):
            f_y = ((global_audio_item_height) * (i2 + 1)) + global_audio_ruler_height
            self.scene.addLine(0, f_y, f_size, f_y)
        self.set_playback_pos(a_cursor_pos)
        self.check_line_count()

    def clear_drawn_items(self, a_default_length=False):
        if self.is_playing:
            f_was_playing = True
            self.is_playing = False
        else:
            f_was_playing = False
        self.reset_line_lists()
        self.audio_items = []
        self.scene.clear()
        self.draw_headers(a_default_length=a_default_length)
        if f_was_playing:
            self.is_playing = True

    def draw_item(self, a_audio_item_index, a_audio_item, a_sample_length):
        """a_start in seconds, a_length in seconds"""
        f_audio_item = audio_viewer_item(a_audio_item_index, a_audio_item, a_sample_length)
        self.audio_items.append(f_audio_item)
        self.scene.addItem(f_audio_item)
        return f_audio_item

global_audio_items_to_drop = []

global_current_audio_item_index = None

def global_paif_val_callback(a_port, a_val):
    if global_current_region is not None and global_current_audio_item_index is not None:
        this_pydaw_project.this_pydaw_osc.pydaw_audio_per_item_fx(global_current_region.uid,
                                                                  global_current_audio_item_index,
                                                                  a_port, a_val)

def global_paif_rel_callback(a_port, a_val):
    if global_current_region is not None and global_current_audio_item_index is not None:
        f_paif = this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)
        f_index_list = this_audio_items_viewer_widget.modulex.get_list()
        f_paif.set_row(global_current_audio_item_index, f_index_list)
        this_pydaw_project.save_audio_per_item_fx_region(global_current_region.uid, f_paif)

class audio_items_viewer_widget(pydaw_widgets.pydaw_abstract_file_browser_widget):
    def __init__(self):
        pydaw_widgets.pydaw_abstract_file_browser_widget.__init__(self)

        self.list_file.setDragEnabled(True)
        self.list_file.mousePressEvent = self.file_mouse_press_event
        self.preview_button.pressed.connect(self.on_preview)
        self.stop_preview_button.pressed.connect(self.on_stop_preview)
        self.folders_tab_widget.addTab(this_audio_item_editor_widget.widget, _("Edit"))

        self.modulex = pydaw_widgets.pydaw_per_audio_item_fx_widget(global_paif_rel_callback,
                                                                    global_paif_val_callback)

        self.modulex_widget = QtGui.QWidget()
        self.modulex_widget.setObjectName("plugin_ui")
        self.modulex_vlayout = QtGui.QVBoxLayout(self.modulex_widget)
        self.folders_tab_widget.addTab(self.modulex_widget, _("Per-Item FX"))
        self.modulex.widget.setDisabled(True)

        self.modulex_copy_button = QtGui.QPushButton(_("Copy"))
        self.modulex_copy_button.setFixedWidth(105)
        self.modulex_copy_button.pressed.connect(self.on_modulex_copy)
        self.modulex_paste_button = QtGui.QPushButton(_("Paste"))
        self.modulex_paste_button.setFixedWidth(105)
        self.modulex_paste_button.pressed.connect(self.on_modulex_paste)
        self.modulex_clear_button = QtGui.QPushButton(_("Clear"))
        self.modulex_clear_button.setFixedWidth(105)
        self.modulex_clear_button.pressed.connect(self.on_modulex_clear)
        self.modulex_hlayout = QtGui.QHBoxLayout()
        self.modulex_hlayout.addWidget(self.modulex_copy_button)
        self.modulex_hlayout.addWidget(self.modulex_paste_button)
        self.modulex_hlayout.addWidget(self.modulex_clear_button)
        self.modulex_hlayout.addItem(QtGui.QSpacerItem(1, 1, QtGui.QSizePolicy.Expanding))
        self.modulex_vlayout.addLayout(self.modulex_hlayout)
        self.modulex_vlayout.addWidget(self.modulex.scroll_area)
        self.set_paif_buttons_enabled(0)

        self.widget = QtGui.QWidget()
        self.hsplitter.addWidget(self.widget)
        self.vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.vlayout)
        self.controls_grid_layout = QtGui.QGridLayout()
        self.controls_grid_layout.addItem(QtGui.QSpacerItem(10, 10,
                                                            QtGui.QSizePolicy.Expanding), 0, 30)
        self.vlayout.addLayout(self.controls_grid_layout)
        self.vlayout.addWidget(this_audio_items_viewer)
        self.snap_combobox = QtGui.QComboBox()
        self.snap_combobox.setFixedWidth(105)
        self.snap_combobox.addItems([_("None"), _("Bar"), _("Beat"), "1/8th", "1/12th", "1/16th"])
        self.controls_grid_layout.addWidget(QtGui.QLabel(_("Snap:")), 0, 0)
        self.controls_grid_layout.addWidget(self.snap_combobox, 0, 1)
        self.snap_combobox.currentIndexChanged.connect(self.set_snap)
        self.snap_combobox.setCurrentIndex(2)

        self.menu_button = QtGui.QPushButton(_("Menu"))
        self.controls_grid_layout.addWidget(self.menu_button, 0, 10)
        self.action_menu = QtGui.QMenu(self.widget)
        self.clone_action = QtGui.QAction(_("Clone from region..."), self.widget)
        self.clone_action.triggered.connect(self.on_clone)
        self.action_menu.addAction(self.clone_action)
        self.copy_action = QtGui.QAction(_("Copy selected"), self.widget)
        self.copy_action.triggered.connect(self.on_copy)
        self.action_menu.addAction(self.copy_action)
        self.paste_action = QtGui.QAction(_("Paste"), self.widget)
        self.paste_action.triggered.connect(self.on_paste)
        self.action_menu.addAction(self.paste_action)
        self.menu_button.setMenu(self.action_menu)

        self.controls_grid_layout.addWidget(QtGui.QLabel(_("V-Zoom:")), 0, 45)
        self.v_zoom_combobox = QtGui.QComboBox()
        self.v_zoom_combobox.addItems([_("Small"), _("Medium"), _("Large")])
        self.v_zoom_combobox.setFixedWidth(105)
        self.v_zoom_combobox.currentIndexChanged.connect(self.set_v_zoom)
        self.controls_grid_layout.addWidget(self.v_zoom_combobox, 0, 46)
        self.h_zoom_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
        self.h_zoom_slider.setRange(0, 100)
        self.h_zoom_slider.setMaximumWidth(600)
        self.h_zoom_slider.setValue(0)
        self.last_scale_value = 0
        self.h_zoom_slider.valueChanged.connect(self.set_zoom)
        self.controls_grid_layout.addWidget(QtGui.QLabel(_("H-Zoom:")), 0, 49)
        self.controls_grid_layout.addWidget(self.h_zoom_slider, 0, 50)
        self.v_zoom = 1.0

    def file_mouse_press_event(self, a_event):
        QtGui.QListWidget.mousePressEvent(self.list_file, a_event)
        global global_audio_items_to_drop
        global_audio_items_to_drop = []
        for f_item in self.list_file.selectedItems():
            global_audio_items_to_drop.append("{}/{}".format(self.last_open_dir, f_item.text()))

    def on_preview(self):
        f_list = self.list_file.selectedItems()
        if f_list:
            this_pydaw_project.this_pydaw_osc.pydaw_preview_audio(
                "{}/{}".format(self.last_open_dir, f_list[0].text()))

    def on_stop_preview(self):
        this_pydaw_project.this_pydaw_osc.pydaw_stop_preview()

    def set_paif_buttons_enabled(self, a_count):
        if a_count == 0:
            self.modulex_copy_button.setEnabled(False)
            self.modulex_paste_button.setEnabled(False)
            self.modulex_clear_button.setEnabled(False)
        elif a_count == 1:
            self.modulex_copy_button.setEnabled(True)
            self.modulex_paste_button.setEnabled(True)
            self.modulex_clear_button.setEnabled(True)
        elif a_count > 1:
            self.modulex_copy_button.setEnabled(False)
            self.modulex_paste_button.setEnabled(True)
            self.modulex_clear_button.setEnabled(True)

    def on_modulex_copy(self):
        if global_current_audio_item_index is not None and global_current_region is not None:
            f_paif = this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)
            self.modulex_clipboard = f_paif.get_row(global_current_audio_item_index)

    def on_modulex_paste(self):
        if self.modulex_clipboard is not None and global_current_region is not None:
            f_paif = this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    f_paif.set_row(f_item.track_num, self.modulex_clipboard)
            this_pydaw_project.save_audio_per_item_fx_region(global_current_region.uid, f_paif)
            this_pydaw_project.this_pydaw_osc.pydaw_audio_per_item_fx_region(
                global_current_region.uid)
            this_audio_items_viewer_widget.modulex.set_from_list(self.modulex_clipboard)

    def on_modulex_clear(self):
        if global_current_region is not None:
            f_paif = this_pydaw_project.get_audio_per_item_fx_region(global_current_region.uid)
            for f_item in this_audio_items_viewer.audio_items:
                if f_item.isSelected():
                    f_paif.clear_row(f_item.track_num)
            this_pydaw_project.save_audio_per_item_fx_region(global_current_region.uid, f_paif)
            this_pydaw_project.this_pydaw_osc.pydaw_audio_per_item_fx_region(
                global_current_region.uid)
            self.modulex.clear_effects()

    def on_copy(self):
        if global_current_region is None or global_transport_is_playing:
            return
        self.audio_items_clipboard = []
        f_per_item_fx_dict = this_pydaw_project.get_audio_per_item_fx_region(
            global_current_region.uid)
        f_count = False
        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected():
                f_count = True
                self.audio_items_clipboard.append((str(f_item.audio_item),
                                                   f_per_item_fx_dict.get_row(f_item.track_num,
                                                                              True)))
        if not f_count:
            QtGui.QMessageBox.warning(self.widget, _("Error"), _("Nothing selected."))

    def on_paste(self):
        if global_current_region is None or global_transport_is_playing:
            return
        if len(self.audio_items_clipboard) == 0:
            QtGui.QMessageBox.warning(self.widget, _("Error"),
                                      _("Nothing copied to the clipboard."))
        f_per_item_fx_dict = this_pydaw_project.get_audio_per_item_fx_region(
            global_current_region.uid)
        for f_str, f_list in self.audio_items_clipboard:
            f_index = global_audio_items.get_next_index()
            if f_index == -1:
                break
            f_item = pydaw_audio_item.from_str(f_str)
            global_audio_items.add_item(f_index, f_item)
            if f_list is not None:
                f_per_item_fx_dict.set_row(f_index, f_list)
        global_audio_items.deduplicate_items()
        this_pydaw_project.save_audio_region(global_current_region.uid, global_audio_items)
        this_pydaw_project.save_audio_per_item_fx_region(global_current_region.uid,
                                                         f_per_item_fx_dict, False)
        this_pydaw_project.this_pydaw_osc.pydaw_audio_per_item_fx_region(
            global_current_region.uid)
        this_pydaw_project.commit(_("Paste audio items"))
        global_open_audio_items(True)

    def on_clone(self):
        if global_current_region is None or global_transport_is_playing:
            return
        def ok_handler():
            f_region_name = str(f_region_combobox.currentText())
            this_pydaw_project.region_audio_clone(global_current_region.uid, f_region_name)
            global_open_audio_items(True)
            f_window.close()

        def cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Clone audio from region..."))
        f_window.setMinimumWidth(270)
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_layout.addWidget(QtGui.QLabel(_("Clone from:")), 0, 0)
        f_region_combobox = QtGui.QComboBox()
        f_regions_dict = this_pydaw_project.get_regions_dict()
        f_regions_list = list(f_regions_dict.uid_lookup.keys())
        f_regions_list.sort()
        f_region_combobox.addItems(f_regions_list)
        f_layout.addWidget(f_region_combobox, 0, 1)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_layout.addWidget(f_ok_button, 5,0)
        f_ok_button.clicked.connect(ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_layout.addWidget(f_cancel_button, 5,1)
        f_cancel_button.clicked.connect(cancel_handler)
        f_window.exec_()

    def set_v_zoom(self, a_val=None):
        this_audio_items_viewer.set_v_zoom(1.0 / self.v_zoom)
        if self.v_zoom_combobox.currentIndex() == 0:
            self.v_zoom = 1.0
        elif self.v_zoom_combobox.currentIndex() == 1:
            self.v_zoom = 4.0
        elif self.v_zoom_combobox.currentIndex() == 2:
            self.v_zoom = 10.0
        this_audio_items_viewer.set_v_zoom(self.v_zoom)

    def set_snap(self, a_val=None):
        global global_audio_quantize, global_audio_quantize_px, global_audio_quantize_amt
        global_audio_quantize = True
        f_lines_enabled = True
        f_snap_range = 8
        if a_val == 0:
            global_audio_quantize = False
            global_audio_quantize_px = global_audio_px_per_beat
            f_lines_enabled = False
        elif a_val == 1:
            global_audio_quantize_px = global_audio_px_per_bar
            f_lines_enabled = False
            global_audio_quantize_amt = 0.25
        elif a_val == 2:
            global_audio_quantize_px = global_audio_px_per_beat
            f_lines_enabled = False
            global_audio_quantize_amt = 1.0
        elif a_val == 3:
            global_audio_quantize_px = global_audio_px_per_8th
            f_snap_range = 8
            global_audio_quantize_amt = 2.0
        elif a_val == 4:
            global_audio_quantize_px = global_audio_px_per_12th
            f_snap_range = 12
            global_audio_quantize_amt = 3.0
        elif a_val == 5:
            global_audio_quantize_px = global_audio_px_per_16th
            f_snap_range = 16
            global_audio_quantize_amt = 4.0
        this_audio_items_viewer.set_grid_div(f_lines_enabled,
                                             global_audio_quantize_px, f_snap_range)

    def set_zoom(self, a_val=None):
        """ This is a ridiculously convoluted way to do this, but I see no other way
            in the Qt docs.  When Scaling, 1.0 does not return to it's original scale,
            and QSlider skips values when moved quickly, making it necessary to
            interpolate the inbetween values
        """
        if self.last_scale_value > self.h_zoom_slider.value():
            for i in range(self.h_zoom_slider.value(), self.last_scale_value):
                this_audio_items_viewer.set_zoom(1.0 / 1.03)
        else:
            for i in range(self.last_scale_value, self.h_zoom_slider.value()):
                this_audio_items_viewer.set_zoom(1.03)
        self.last_scale_value = self.h_zoom_slider.value()
        this_audio_items_viewer.set_h_zoom_value(self.last_scale_value)
        this_audio_items_viewer.check_line_count()


class audio_item_editor_widget:
    def __init__(self):
        self.widget = QtGui.QWidget()
        self.widget.setMaximumWidth(480)
        self.main_vlayout = QtGui.QVBoxLayout(self.widget)

        self.layout = QtGui.QGridLayout()
        self.main_vlayout.addLayout(self.layout)

        self.sample_vol_layout = QtGui.QVBoxLayout()
        self.vol_checkbox = QtGui.QCheckBox(_("Vol"))
        self.sample_vol_layout.addWidget(self.vol_checkbox)
        self.sample_vol_slider = QtGui.QSlider(QtCore.Qt.Vertical)
        self.sample_vol_slider.setRange(-24, 24)
        self.sample_vol_slider.setValue(0)
        self.sample_vol_slider.valueChanged.connect(self.sample_vol_changed)
        self.sample_vol_layout.addWidget(self.sample_vol_slider)
        self.sample_vol_label = QtGui.QLabel("0db")
        self.sample_vol_label.setMinimumWidth(48)
        self.sample_vol_layout.addWidget(self.sample_vol_label)
        self.layout.addLayout(self.sample_vol_layout, 1, 2)
        self.vlayout2 = QtGui.QVBoxLayout()
        self.layout.addLayout(self.vlayout2, 1, 1)
        self.start_hlayout = QtGui.QHBoxLayout()
        self.vlayout2.addLayout(self.start_hlayout)

        self.timestretch_checkbox = QtGui.QCheckBox(_("Time Stretching:"))
        self.vlayout2.addWidget(self.timestretch_checkbox)
        self.timestretch_hlayout = QtGui.QHBoxLayout()
        self.time_pitch_gridlayout = QtGui.QGridLayout()
        self.vlayout2.addLayout(self.timestretch_hlayout)
        self.vlayout2.addLayout(self.time_pitch_gridlayout)
        self.timestretch_hlayout.addWidget(QtGui.QLabel(_("Mode:")))
        self.timestretch_mode = QtGui.QComboBox()

        self.timestretch_mode.setMinimumWidth(240)
        self.timestretch_hlayout.addWidget(self.timestretch_mode)
        self.timestretch_mode.addItems(global_timestretch_modes)
        self.timestretch_mode.currentIndexChanged.connect(self.timestretch_mode_changed)
        self.time_pitch_gridlayout.addWidget(QtGui.QLabel(_("Pitch:")), 0, 0)
        self.pitch_shift = QtGui.QDoubleSpinBox()
        self.pitch_shift.setRange(-36, 36)
        self.pitch_shift.setValue(0.0)
        self.pitch_shift.setDecimals(6)
        self.time_pitch_gridlayout.addWidget(self.pitch_shift, 0, 1)

        self.pitch_shift_end_checkbox = QtGui.QCheckBox(_("End:"))
        self.pitch_shift_end_checkbox.toggled.connect(self.pitch_end_mode_changed)
        self.time_pitch_gridlayout.addWidget(self.pitch_shift_end_checkbox, 0, 2)
        self.pitch_shift_end = QtGui.QDoubleSpinBox()
        self.pitch_shift_end.setRange(-36, 36)
        self.pitch_shift_end.setValue(0.0)
        self.pitch_shift_end.setDecimals(6)
        self.time_pitch_gridlayout.addWidget(self.pitch_shift_end, 0, 3)

        self.time_pitch_gridlayout.addWidget(QtGui.QLabel(_("Time:")), 1, 0)
        self.timestretch_amt = QtGui.QDoubleSpinBox()
        self.timestretch_amt.setRange(0.2, 4.0)
        self.timestretch_amt.setDecimals(6)
        self.timestretch_amt.setSingleStep(0.1)
        self.timestretch_amt.setValue(1.0)
        self.time_pitch_gridlayout.addWidget(self.timestretch_amt, 1, 1)

        self.crispness_layout = QtGui.QHBoxLayout()
        self.vlayout2.addLayout(self.crispness_layout)
        self.crispness_layout.addWidget(QtGui.QLabel(_("Crispness")))
        self.crispness_combobox = QtGui.QComboBox()
        self.crispness_combobox.addItems([_("0 (smeared)"), _("1 (piano)"), "2", "3",
                                          "4", "5 (normal)", _("6 (sharp, drums)")])
        self.crispness_combobox.setCurrentIndex(5)
        self.crispness_layout.addWidget(self.crispness_combobox)

        self.timestretch_amt_end_checkbox = QtGui.QCheckBox(_("End:"))
        self.timestretch_amt_end_checkbox.toggled.connect(self.timestretch_end_mode_changed)
        self.time_pitch_gridlayout.addWidget(self.timestretch_amt_end_checkbox, 1, 2)
        self.timestretch_amt_end = QtGui.QDoubleSpinBox()
        self.timestretch_amt_end.setRange(0.2, 4.0)
        self.timestretch_amt_end.setDecimals(6)
        self.timestretch_amt_end.setSingleStep(0.1)
        self.timestretch_amt_end.setValue(1.0)
        self.time_pitch_gridlayout.addWidget(self.timestretch_amt_end, 1, 3)

        self.timestretch_mode_changed(0)

        self.timestretch_mode.currentIndexChanged.connect(self.timestretch_changed)
        self.pitch_shift.valueChanged.connect(self.timestretch_changed)
        self.pitch_shift_end.valueChanged.connect(self.timestretch_changed)
        self.timestretch_amt.valueChanged.connect(self.timestretch_changed)
        self.timestretch_amt_end.valueChanged.connect(self.timestretch_changed)
        self.crispness_combobox.currentIndexChanged.connect(self.timestretch_changed)

        self.vlayout2.addSpacerItem(QtGui.QSpacerItem(1, 10))
        self.output_hlayout = QtGui.QHBoxLayout()
        self.output_checkbox = QtGui.QCheckBox(_("Output:"))
        self.output_hlayout.addWidget(self.output_checkbox)
        self.output_combobox = QtGui.QComboBox()
        global global_audio_track_comboboxes
        global_audio_track_comboboxes.append(self.output_combobox)
        self.output_combobox.setMinimumWidth(210)
        self.output_combobox.addItems(list(global_audio_track_names.values()))
        self.output_combobox.currentIndexChanged.connect(self.output_changed)
        self.output_hlayout.addWidget(self.output_combobox)
        self.vlayout2.addLayout(self.output_hlayout)

        self.vlayout2.addSpacerItem(QtGui.QSpacerItem(1, 10))
        self.reversed_layout = QtGui.QHBoxLayout()
        self.reversed_checkbox = QtGui.QCheckBox()
        self.reversed_layout.addWidget(self.reversed_checkbox)
        self.is_reversed_checkbox = QtGui.QCheckBox(_("Reverse"))
        self.is_reversed_checkbox.clicked.connect(self.reverse_changed)
        self.reversed_layout.addWidget(self.is_reversed_checkbox)
        self.reversed_layout.addItem(QtGui.QSpacerItem(5, 5, QtGui.QSizePolicy.Expanding))
        self.vlayout2.addLayout(self.reversed_layout)

        self.vlayout2.addSpacerItem(QtGui.QSpacerItem(1, 10))
        self.fadein_vol_layout = QtGui.QHBoxLayout()
        self.fadein_vol_checkbox = QtGui.QCheckBox(_("Fade-in:"))
        self.fadein_vol_layout.addWidget(self.fadein_vol_checkbox)
        self.fadein_vol_spinbox = QtGui.QSpinBox()
        self.fadein_vol_spinbox.setRange(-50, -6)
        self.fadein_vol_spinbox.setValue(-40)
        self.fadein_vol_spinbox.valueChanged.connect(self.fadein_vol_changed)
        self.fadein_vol_layout.addWidget(self.fadein_vol_spinbox)
        self.fadein_vol_layout.addItem(QtGui.QSpacerItem(5, 5, QtGui.QSizePolicy.Expanding))
        self.vlayout2.addLayout(self.fadein_vol_layout)

        self.fadeout_vol_checkbox = QtGui.QCheckBox(_("Fade-out:"))
        self.fadein_vol_layout.addWidget(self.fadeout_vol_checkbox)
        self.fadeout_vol_spinbox = QtGui.QSpinBox()
        self.fadeout_vol_spinbox.setRange(-50, -6)
        self.fadeout_vol_spinbox.setValue(-40)
        self.fadeout_vol_spinbox.valueChanged.connect(self.fadeout_vol_changed)
        self.fadein_vol_layout.addWidget(self.fadeout_vol_spinbox)

        self.vlayout2.addSpacerItem(QtGui.QSpacerItem(1, 1, vPolicy=QtGui.QSizePolicy.Expanding))
        self.ok_layout = QtGui.QHBoxLayout()
        self.ok = QtGui.QPushButton(_("Save Changes"))
        self.ok.pressed.connect(self.ok_handler)
        self.ok_layout.addWidget(self.ok)
        self.vlayout2.addLayout(self.ok_layout)

        self.last_open_dir = global_home

    def set_tooltips(self, a_on):
        if a_on:
            f_sbsms_tooltip = _("This control is only valid for the SBSMS and {} modes,\n"
            "the start/end values are for the full sample length, not the edited "
            "start/end points\n"
            "setting the start/end time to different values will cause the timestretch "
            "handle to disappear on the audio item.")
            self.timestretch_amt_end.setToolTip(f_sbsms_tooltip.format(
                _("Time(affecting pitch)")))
            self.pitch_shift_end.setToolTip(f_sbsms_tooltip.format(_("Pitch(affecting time)")))
            self.ok.setToolTip(_("Changes are not saved until you push this button"))
            self.widget.setToolTip(_("To edit the properties of one or more audio item(s),\n"
            "click or marquee select items, then change their properties and click "
            "'Save Changes'\n"
            "Only the control section(s) whose checkbox is checked will be updated.\n\n"
            "Click the 'tooltips' checkbox in the transport to disable these tooltips"))
            self.crispness_combobox.setToolTip(_("Affects the sharpness of transients, only "
            "for modes using Rubberband"))
            self.timestretch_mode.setToolTip(_("Modes:\n\nNone:  No stretching or "
            "pitch adjustment\n"
            "Pitch affecting time:  Repitch the item, it will become shorter at higher "
            "pitches, and longer at "
            "lower pitches\n"
            "Time affecting pitch:  Stretch the item to the desired length, it will have "
            "lower pitch at longer "
            "lengths, and higher pitch at shorter lengths\n"
            "Rubberband:  Adjust pitch and time independently\nRubberband (formants): "
            "Same as Rubberband, but preserves formants\n"
            "SBSMS:  Adjust pitch and time independently, also with the ability to "
            "set start/end pitch/time differently\n"
            "Paulstretch:  Mostly for stretching items very long, creates a very smeared, "
            "atmospheric sound"))
            self.output_combobox.setToolTip(_("Use this combobox to select the output "
            "audio track on the 'Audio Tracks' tab\n"
            "where you can apply effects and automation.  Please note that if you use a "
            "lot of audio sequencing in your projects,\n"
            "you must assign audio items to multiple tracks to take advantage of "
            "multiple CPU cores, otherwise all items will be \n"
            "processed on a single core"))
            self.sample_vol_slider.setToolTip(_("Use this to set the sample volume. "
            "If you need to automate volume changes, either\n"
            "use the fade-in/fade-out handles, or automate the volume on the audio "
            "track specified in the Output: combobox."))
            self.is_reversed_checkbox.setToolTip(_("Checking this causes the sample to "
                                                 "play backwards"))
            self.fadein_vol_spinbox.setToolTip(_("Sets the initial volume in dB when fading in."))
            self.fadeout_vol_spinbox.setToolTip(_("Sets the end volume in dB when fading out."))
        else:
            self.timestretch_amt_end.setToolTip("")
            self.pitch_shift_end.setToolTip("")
            self.ok.setToolTip("")
            self.widget.setToolTip("")
            self.crispness_combobox.setToolTip("")
            self.timestretch_mode.setToolTip("")
            self.output_combobox.setToolTip("")
            self.sample_vol_slider.setToolTip("")
            self.is_reversed_checkbox.setToolTip("")
            self.fadein_vol_spinbox.setToolTip("")
            self.fadeout_vol_spinbox.setToolTip("")

    def reverse_changed(self, a_val=None):
        self.reversed_checkbox.setChecked(True)

    def fadein_vol_changed(self, a_val=None):
        self.fadein_vol_checkbox.setChecked(True)

    def fadeout_vol_changed(self, a_val=None):
        self.fadeout_vol_checkbox.setChecked(True)

    def timestretch_end_mode_changed(self, a_val=None):
        if not self.timestretch_amt_end_checkbox.isChecked():
            self.timestretch_amt_end.setValue(self.timestretch_amt.value())

    def pitch_end_mode_changed(self, a_val=None):
        if not self.pitch_shift_end_checkbox.isChecked():
            self.pitch_shift_end.setValue(self.pitch_shift.value())

    def end_mode_changed(self, a_val=None):
        self.end_mode_checkbox.setChecked(True)

    def timestretch_changed(self, a_val=None):
        self.timestretch_checkbox.setChecked(True)
        if not self.pitch_shift_end_checkbox.isChecked():
            self.pitch_shift_end.setValue(self.pitch_shift.value())
        if not self.timestretch_amt_end_checkbox.isChecked():
            self.timestretch_amt_end.setValue(self.timestretch_amt.value())

    def output_changed(self, a_val=None):
        self.output_checkbox.setChecked(True)

    def timestretch_mode_changed(self, a_val=None):
        if a_val == 0:
            self.pitch_shift.setEnabled(False)
            self.timestretch_amt.setEnabled(False)
            self.pitch_shift.setValue(0.0)
            self.pitch_shift_end.setValue(0.0)
            self.timestretch_amt.setValue(1.0)
            self.timestretch_amt_end.setValue(1.0)
            self.timestretch_amt_end_checkbox.setEnabled(False)
            self.timestretch_amt_end_checkbox.setChecked(False)
            self.pitch_shift_end_checkbox.setEnabled(False)
            self.pitch_shift_end_checkbox.setChecked(False)
            self.crispness_combobox.setCurrentIndex(5)
            self.crispness_combobox.setEnabled(False)
        elif a_val == 1:
            self.pitch_shift.setEnabled(True)
            self.timestretch_amt.setEnabled(False)
            self.timestretch_amt.setValue(1.0)
            self.timestretch_amt_end.setValue(1.0)
            self.timestretch_amt_end.setEnabled(False)
            self.timestretch_amt_end_checkbox.setEnabled(False)
            self.timestretch_amt_end_checkbox.setChecked(False)
            self.pitch_shift_end_checkbox.setEnabled(True)
            self.pitch_shift_end.setEnabled(True)
            self.crispness_combobox.setCurrentIndex(5)
            self.crispness_combobox.setEnabled(False)
        elif a_val == 2:
            self.pitch_shift.setEnabled(False)
            self.timestretch_amt.setEnabled(True)
            self.pitch_shift.setValue(0.0)
            self.pitch_shift_end.setValue(0.0)
            self.pitch_shift_end.setEnabled(False)
            self.timestretch_amt.setRange(0.2, 4.0)
            self.timestretch_amt_end.setRange(0.2, 4.0)
            self.timestretch_amt_end.setEnabled(True)
            self.timestretch_amt_end_checkbox.setEnabled(True)
            self.pitch_shift_end_checkbox.setEnabled(False)
            self.pitch_shift_end_checkbox.setChecked(False)
            self.crispness_combobox.setCurrentIndex(5)
            self.crispness_combobox.setEnabled(False)
        elif a_val == 3 or a_val == 4:
            self.pitch_shift.setEnabled(True)
            self.pitch_shift_end.setEnabled(False)
            self.timestretch_amt.setEnabled(True)
            self.timestretch_amt.setRange(0.1, 10.0)
            self.timestretch_amt_end.setRange(0.1, 10.0)
            self.timestretch_amt_end_checkbox.setEnabled(False)
            self.timestretch_amt_end_checkbox.setChecked(False)
            self.pitch_shift_end_checkbox.setEnabled(False)
            self.pitch_shift_end_checkbox.setChecked(False)
            self.crispness_combobox.setEnabled(True)
        elif a_val == 5:
            self.pitch_shift.setEnabled(True)
            self.pitch_shift_end.setEnabled(True)
            self.timestretch_amt.setEnabled(True)
            self.timestretch_amt_end.setEnabled(True)
            self.timestretch_amt.setRange(0.1, 10.0)
            self.timestretch_amt_end.setRange(0.1, 10.0)
            self.timestretch_amt_end_checkbox.setEnabled(True)
            self.pitch_shift_end_checkbox.setEnabled(True)
            self.crispness_combobox.setCurrentIndex(5)
            self.crispness_combobox.setEnabled(False)
        elif a_val == 6:
            self.pitch_shift.setEnabled(True)
            self.timestretch_amt.setEnabled(True)
            self.timestretch_amt_end.setEnabled(False)
            self.pitch_shift_end.setEnabled(False)
            self.timestretch_amt.setRange(0.5, 10.0)
            self.timestretch_amt_end.setRange(0.5, 10.0)
            self.timestretch_amt_end_checkbox.setEnabled(False)
            self.timestretch_amt_end_checkbox.setChecked(False)
            self.pitch_shift_end_checkbox.setEnabled(False)
            self.pitch_shift_end_checkbox.setChecked(False)
            self.crispness_combobox.setCurrentIndex(5)
            self.crispness_combobox.setEnabled(False)

    def open_item(self, a_item):
        print("audio_item_editor_widget.open_item()")
        print((str(a_item)))
        if a_item is None:
            pass #TODO:  Reset values to default
        else:
            if a_item.end_mode == 1:
                self.end_musical_time.setChecked(True)
            else:
                self.end_sample_length.setChecked(True)
            if a_item.timestretch_amt_end == a_item.timestretch_amt:
                self.timestretch_amt_end_checkbox.setChecked(False)
            else:
                self.timestretch_amt_end_checkbox.setChecked(True)
            if a_item.pitch_shift_end == a_item.pitch_shift:
                self.pitch_shift_end_checkbox.setChecked(False)
            else:
                self.pitch_shift_end_checkbox.setChecked(True)
            self.timestretch_mode.setCurrentIndex(a_item.time_stretch_mode)
            self.pitch_shift.setValue(a_item.pitch_shift)
            self.timestretch_amt.setValue(a_item.timestretch_amt)
            self.output_combobox.setCurrentIndex(a_item.output_track)
            self.sample_vol_slider.setValue(a_item.vol)
            self.pitch_shift_end.setValue(a_item.pitch_shift_end)
            self.timestretch_amt_end.setValue(a_item.timestretch_amt_end)
            self.crispness_combobox.setCurrentIndex(a_item.crispness)
            self.is_reversed_checkbox.setChecked(a_item.reversed)

    def ok_handler(self):
        if global_transport_is_playing:
            QtGui.QMessageBox.warning(self.widget, _("Error"),
                                      _("Cannot edit audio items during playback"))
            return

        self.end_mode = 0

        f_selected_count = 0

        f_region_length = global_current_region.region_length_bars
        if f_region_length == 0:
            f_region_length = 8
        f_region_length -= 1

        f_was_stretching = False
        f_stretched_items = []

        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected():
                if self.output_checkbox.isChecked():
                    f_item.audio_item.output_track = self.output_combobox.currentIndex()
                if self.vol_checkbox.isChecked():
                    f_item.audio_item.vol = self.sample_vol_slider.value()
                if self.timestretch_checkbox.isChecked():
                    f_new_ts_mode = self.timestretch_mode.currentIndex()
                    f_new_ts = round(self.timestretch_amt.value(), 6)
                    f_new_ps = round(self.pitch_shift.value(), 6)
                    if self.timestretch_amt_end_checkbox.isChecked():
                        f_new_ts_end = round(self.timestretch_amt_end.value(), 6)
                    else:
                        f_new_ts_end = f_new_ts
                    if self.pitch_shift_end_checkbox.isChecked():
                        f_new_ps_end = round(self.pitch_shift_end.value(), 6)
                    else:
                        f_new_ps_end = f_new_ps
                    f_item.audio_item.crispness = self.crispness_combobox.currentIndex()

                    if ((f_item.audio_item.time_stretch_mode >= 3) or
                    (f_item.audio_item.time_stretch_mode == 1 and \
                    f_item.audio_item.pitch_shift_end != f_item.audio_item.pitch_shift) or \
                    (f_item.audio_item.time_stretch_mode == 2 and \
                    f_item.audio_item.timestretch_amt_end != f_item.audio_item.timestretch_amt)) \
                    and \
                    ((f_new_ts_mode == 0) or \
                    (f_new_ts_mode == 1 and f_new_ps == f_new_ps_end) or \
                    (f_new_ts_mode == 2 and f_new_ts == f_new_ts_end)):
                        f_item.audio_item.uid = \
                        this_pydaw_project.timestretch_get_orig_file_uid(f_item.audio_item.uid)

                    f_item.audio_item.time_stretch_mode = f_new_ts_mode
                    f_item.audio_item.pitch_shift = f_new_ps
                    f_item.audio_item.timestretch_amt = f_new_ts
                    f_item.audio_item.pitch_shift_end = f_new_ps_end
                    f_item.audio_item.timestretch_amt_end = f_new_ts_end
                    f_item.draw()
                    f_item.clip_at_region_end()
                    if (f_new_ts_mode >= 3) or \
                    (f_new_ts_mode == 1 and f_new_ps != f_new_ps_end) or \
                    (f_new_ts_mode == 2 and f_new_ts != f_new_ts_end) and \
                    (f_item.orig_string != str(f_item.audio_item)):
                        f_was_stretching = True
                        f_ts_result = this_pydaw_project.timestretch_audio_item(f_item.audio_item)
                        if f_ts_result is not None:
                            f_stretched_items.append((f_ts_result, f_item.audio_item))

                if self.reversed_checkbox.isChecked():
                    f_is_reversed = self.is_reversed_checkbox.isChecked()
                    if f_item.audio_item.reversed != f_is_reversed:
                        f_new_start = 1000.0 - f_item.audio_item.sample_end
                        f_new_end = 1000.0 - f_item.audio_item.sample_start
                        f_item.audio_item.sample_start = f_new_start
                        f_item.audio_item.sample_end = f_new_end
                    f_item.audio_item.reversed = f_is_reversed
                if self.fadein_vol_checkbox.isChecked():
                    f_item.audio_item.fadein_vol = self.fadein_vol_spinbox.value()
                if self.fadeout_vol_checkbox.isChecked():
                    f_item.audio_item.fadeout_vol = self.fadeout_vol_spinbox.value()
                f_item.draw()
                f_selected_count += 1
        if f_selected_count == 0:
            QtGui.QMessageBox.warning(self.widget, _("Error"), _("No items selected"))
        else:
            if f_was_stretching:
                f_current_region_length = pydaw_get_current_region_length()
                f_global_tempo = float(this_transport.tempo_spinbox.value())
                this_pydaw_project.save_stretch_dicts()
                for f_stretch_item, f_audio_item in f_stretched_items:
                    f_stretch_item[2].wait()
                    f_new_uid = this_pydaw_project.get_wav_uid_by_name(f_stretch_item[0],
                                                                       a_uid=f_stretch_item[1])
                    f_graph = this_pydaw_project.get_sample_graph_by_uid(f_new_uid)
                    f_audio_item.clip_at_region_end(f_current_region_length, f_global_tempo,
                                                    f_graph.length_in_seconds)
            this_pydaw_project.save_audio_region(global_current_region.uid, global_audio_items)
            global_open_audio_items(True)
            this_pydaw_project.commit(_("Update audio items"))

    def sample_vol_changed(self, a_val=None):
        self.sample_vol_label.setText("{}dB".format(self.sample_vol_slider.value()))
        self.vol_checkbox.setChecked(True)

global_audio_items = None

def global_open_audio_items(a_update_viewer=True):
    global global_audio_items
    global_audio_items = this_pydaw_project.get_audio_region(global_current_region.uid)
    if a_update_viewer:
        f_selected_list = []
        for f_item in this_audio_items_viewer.audio_items:
            if f_item.isSelected():
                f_selected_list.append(str(f_item.audio_item))
        this_audio_items_viewer.clear_drawn_items()
        for k, v in list(global_audio_items.items.items()):
            try:
                f_graph = this_pydaw_project.get_sample_graph_by_uid(v.uid)
                if f_graph is None:
                    print(_("Error drawing item for {}, could not get "
                    "sample graph object").format(v.uid))
                    continue
                this_audio_items_viewer.draw_item(k, v, f_graph.length_in_seconds)
            except:
                if global_transport_is_playing:
                    print(_("Exception while loading {}".format(v.uid)))
                else:
                    f_path = this_pydaw_project.get_wav_path_by_uid(v.uid)
                    if os.path.isfile(f_path):
                        f_error_msg = _("Unknown error loading sample f_path {}, \n\n{}").format(
                            f_path, locals())
                    else:
                        f_error_msg = _("Error loading '{}', file does not exist.").format(f_path)
                    QtGui.QMessageBox.warning(this_main_window, _("Error"), f_error_msg)
        for f_item in this_audio_items_viewer.audio_items:
            if str(f_item.audio_item) in f_selected_list:
                f_item.setSelected(True)


def global_save_all_region_tracks():
    this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    this_pydaw_project.save_audio_tracks(this_region_audio_editor.get_tracks())
    this_pydaw_project.save_busses(this_region_bus_editor.get_tracks())

class audio_track:
    def on_vol_change(self, value):
        self.volume_label.setText("{} dB".format(value))
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_vol(self.track_number,
                                                            self.volume_slider.value(), 2)

    def on_vol_released(self):
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].vol = self.volume_slider.value()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.commit(_("Set audio track {} to {}").format(self.track_number,
                                  self.volume_slider.value()))

    def on_solo(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_solo(self.track_number,
                                                             self.solo_checkbox.isChecked(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].solo = self.solo_checkbox.isChecked()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.commit(_("Set audio track {} soloed to {}").format(self.track_number,
                                  self.solo_checkbox.isChecked()))

    def on_mute(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_mute(self.track_number,
                                                             self.mute_checkbox.isChecked(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].mute = self.mute_checkbox.isChecked()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.commit(_("Set audio track {} muted to {}").format(self.track_number,
                                  self.mute_checkbox.isChecked()))

    def on_rec(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_track_rec(2, self.track_number,
                                                              self.record_radiobutton.isChecked())

    def on_name_changed(self):
        self.track_name_lineedit.setText(pydaw_remove_bad_chars(self.track_name_lineedit.text()))
        this_pydaw_project.this_pydaw_osc.pydaw_save_track_name(self.track_number,
                                                                self.track_name_lineedit.text(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].name = str(self.track_name_lineedit.text())
        this_pydaw_project.save_audio_tracks(f_tracks)
        global_update_audio_track_comboboxes(self.track_number, self.track_name_lineedit.text())
        this_pydaw_project.commit(_("Set audio track {} name to {}").format(self.track_number,
                                  self.track_name_lineedit.text()))
        global_fx_set_window_title(2, self.track_number,
                                   _("Audio Track: {}").format(self.track_name_lineedit.text()))

    def on_show_fx(self):
        global_open_fx_ui(self.track_number, pydaw_folder_audiofx, 2,
        _("Audio Track: {}").format(self.track_name_lineedit.text()))

    def on_bus_changed(self, a_value=0):
        this_pydaw_project.this_pydaw_osc.pydaw_set_bus(self.track_number,
                                                        self.bus_combobox.currentIndex(), 2)
        f_tracks = this_pydaw_project.get_audio_tracks()
        f_tracks.tracks[self.track_number].bus_num = self.bus_combobox.currentIndex()
        this_pydaw_project.save_audio_tracks(f_tracks)
        this_pydaw_project.commit(_("Set audio track {} bus to {}").format(self.track_number,
                                  self.track_name_lineedit.text()))

    def wheel_event(self, a_event=None):
        pass

    def context_menu_event(self, a_event=None):
        pass

    def __init__(self, a_track_num, a_track_text=_("track")):
        self.suppress_osc = True
        self.track_number = a_track_num
        self.group_box = QtGui.QWidget()
        self.group_box.contextMenuEvent = self.context_menu_event
        self.group_box.setObjectName("track_panel")
        self.group_box.setMinimumWidth(330)
        self.main_vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.main_vlayout)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout2)
        self.volume_slider = QtGui.QSlider()
        self.volume_slider.setRange(-50, 12)
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
        self.hlayout3.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                QtGui.QSizePolicy.Minimum))
        self.fx_button = QtGui.QPushButton("FX")
        self.fx_button.pressed.connect(self.on_show_fx)
        self.fx_button.setObjectName("fxbutton")
        self.fx_button.setFixedWidth(24)
        self.bus_combobox = QtGui.QComboBox()
        self.bus_combobox.wheelEvent = self.wheel_event
        self.bus_combobox.addItems(['M', '1','2','3','4'])
        self.bus_combobox.setMinimumWidth(54)
        self.bus_combobox.currentIndexChanged.connect(self.on_bus_changed)
        self.hlayout3.addWidget(QtGui.QLabel(_("Bus:")))
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
        self.record_radiobutton = QtGui.QRadioButton()
        rec_button_group.addButton(self.record_radiobutton)
        self.record_radiobutton.toggled.connect(self.on_rec)
        self.record_radiobutton.setObjectName("rec_arm_radiobutton")
        self.hlayout3.addWidget(self.record_radiobutton)
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
        return pydaw_audio_track(self.solo_checkbox.isChecked(), self.mute_checkbox.isChecked(),
                                 self.volume_slider.value(), str(self.track_name_lineedit.text()),
                                 self.bus_combobox.currentIndex(), self.track_number)

global_item_editing_count = 1

global_piano_roll_snap = False
global_piano_roll_grid_width = 1000.0
global_piano_keys_width = 34  #Width of the piano keys in px
global_piano_roll_grid_max_start_time = 999.0 + global_piano_keys_width
global_piano_roll_note_height = 15
global_piano_roll_snap_divisor = 16.0
global_piano_roll_snap_beats = 4.0/global_piano_roll_snap_divisor
global_piano_roll_snap_value = global_piano_roll_grid_width / global_piano_roll_snap_divisor
global_piano_roll_snap_divisor_beats = global_piano_roll_snap_divisor / 4.0
global_piano_roll_note_count = 120
global_piano_roll_header_height = 20
global_piano_roll_total_height = 1000  #gets updated by the piano roll to it's real value

pydaw_note_selected_gradient = QtGui.QLinearGradient(QtCore.QPointF(0, 0), QtCore.QPointF(0, 12))
pydaw_note_selected_gradient.setColorAt(0, QtGui.QColor(180, 172, 100))
pydaw_note_selected_gradient.setColorAt(1, QtGui.QColor(240, 240, 240))

global_selected_piano_note = None   #Used for mouse click hackery

def pydaw_set_piano_roll_quantize(a_index):
    global global_piano_roll_snap
    global global_piano_roll_snap_value
    global global_piano_roll_snap_divisor
    global global_piano_roll_snap_divisor_beats
    global global_piano_roll_snap_beats

    if a_index == 0:
        global_piano_roll_snap = False
    else:
        global_piano_roll_snap = True

    if a_index == 0:
        global_piano_roll_snap_divisor = 16.0  #For grid lines in the piano roll
    elif a_index == 7:
        global_piano_roll_snap_divisor = 128.0
    elif a_index == 6:
        global_piano_roll_snap_divisor = 64.0
    elif a_index == 5:
        global_piano_roll_snap_divisor = 32.0
    elif a_index == 4:
        global_piano_roll_snap_divisor = 16.0
    elif a_index == 3:
        global_piano_roll_snap_divisor =  12.0
    elif a_index == 2:
        global_piano_roll_snap_divisor =  8.0
    elif a_index == 1:
        global_piano_roll_snap_divisor =  4.0

    global_piano_roll_snap_beats = 4.0 / global_piano_roll_snap_divisor
    this_piano_roll_editor.set_grid_div(global_piano_roll_snap_divisor / 4.0)
    global_piano_roll_snap_divisor *= global_item_editing_count
    global_piano_roll_snap_value = \
    (global_piano_roll_grid_width * global_item_editing_count) / \
    (global_piano_roll_snap_divisor)
    global_piano_roll_snap_divisor_beats = global_piano_roll_snap_divisor / \
    (4.0 * global_item_editing_count)

global_piano_roll_min_note_length = global_piano_roll_grid_width / 128.0

global_piano_roll_note_labels = ["C", "C#", "D", "D#", "E", "F", "F#",
                                 "G", "G#", "A", "A#", "B"]


global_piano_note_gradient_tuple = \
((255, 0, 0), (255, 123, 0), (255, 255, 0), (123, 255, 0), (0, 255, 0),
 (0, 255, 123), (0, 255, 255), (0, 123, 255), (0, 0, 255), (0, 0, 255))

class piano_roll_note_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_length, a_note_height, a_note, a_note_item, a_item_index):
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, a_length, a_note_height)
        self.item_index = a_item_index
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.note_height = a_note_height
        self.note_item = a_note_item
        self.setAcceptHoverEvents(True)
        self.resize_start_pos = self.note_item.start
        self.is_copying = False
        if global_selected_piano_note is not None and a_note_item == global_selected_piano_note:
            self.is_resizing = True
            this_piano_roll_editor.click_enabled = True
        else:
            self.is_resizing = False
        self.showing_resize_cursor = False
        self.resize_rect = self.rect()
        self.setPen(QtGui.QPen(pydaw_track_gradients[3], 2))
        self.mouse_y_pos = QtGui.QCursor.pos().y()
        self.setZValue(1002.0)
        self.note_text = QtGui.QGraphicsSimpleTextItem(self)
        self.update_note_text()
        self.vel_line = QtGui.QGraphicsLineItem(self)
        self.set_vel_line()
        self.set_brush()

    def set_vel_line(self):
        f_vel = self.note_item.velocity
        f_rect = self.rect()
        f_y = (1.0 - (f_vel * 0.007874016)) * f_rect.height()
        f_width = f_rect.width()
        self.vel_line.setLine(0.0, f_y, f_width, f_y)

    def set_brush(self):
        f_val = (1.0 - (self.note_item.velocity / 127.0)) * 9.0
        f_val = pydaw_util.pydaw_clip_value(f_val, 0.0, 9.0)
        f_int = int(f_val)
        f_frac = f_val - f_int
        f_vals = []
        for f_i in range(3):
            f_val = (((global_piano_note_gradient_tuple[f_int + 1][f_i] -
                global_piano_note_gradient_tuple[f_int][f_i]) * f_frac) +
                global_piano_note_gradient_tuple[f_int][f_i])
            f_vals.append(f_val)
        self.setBrush(QtGui.QBrush(QtGui.QColor(*f_vals)))

    def update_note_text(self):
        f_octave = (self.note_item.note_num // 12) - 2
        f_note = global_piano_roll_note_labels[self.note_item.note_num % 12]
        self.note_text.setText("{}{}".format(f_note, f_octave))

    def mouse_is_at_end(self, a_pos):
        return (a_pos.x() > (self.rect().width() * 0.8))

    def hoverMoveEvent(self, a_event):
        #QtGui.QGraphicsRectItem.hoverMoveEvent(self, a_event)
        if not self.is_resizing:
            this_piano_roll_editor.click_enabled = False
            self.show_resize_cursor(a_event)

    def show_resize_cursor(self, a_event):
        f_is_at_end = self.mouse_is_at_end(a_event.pos())
        if f_is_at_end and not self.showing_resize_cursor:
            QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.SizeHorCursor))
            self.showing_resize_cursor = True
        elif not f_is_at_end and self.showing_resize_cursor:
            QtGui.QApplication.restoreOverrideCursor()
            self.showing_resize_cursor = False

    def get_selected_string(self):
        return "{}|{}".format(self.item_index, self.note_item)

    def hoverEnterEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverEnterEvent(self, a_event)
        this_piano_roll_editor.click_enabled = False

    def hoverLeaveEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverLeaveEvent(self, a_event)
        this_piano_roll_editor.click_enabled = True
        QtGui.QApplication.restoreOverrideCursor()
        self.showing_resize_cursor = False

    def mouseDoubleClickEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseDoubleClickEvent(self, a_event)
        QtGui.QApplication.restoreOverrideCursor()
        this_item_editor.notes_show_event_dialog(None, None,
                                                 self.note_item, self.item_index)

    def mousePressEvent(self, a_event):
        a_event.setAccepted(True)
        QtGui.QGraphicsRectItem.mousePressEvent(self, a_event)
        if a_event.modifiers() == QtCore.Qt.ShiftModifier:
            this_item_editor.items[self.item_index].remove_note(self.note_item)
            global_save_and_reload_items()
            QtGui.QApplication.restoreOverrideCursor()
            self.showing_resize_cursor = False
        else:
            self.setBrush(pydaw_note_selected_gradient)
            self.o_pos = self.pos()
            if self.mouse_is_at_end(a_event.pos()):
                self.is_resizing = True
                self.mouse_y_pos = QtGui.QCursor.pos().y()
                self.resize_last_mouse_pos = a_event.pos().x()
                for f_item in this_piano_roll_editor.note_items:
                    if f_item.isSelected():
                        f_item.resize_start_pos = f_item.note_item.start + \
                            (4.0 * f_item.item_index)
                        f_item.resize_pos = f_item.pos()
                        f_item.resize_rect = f_item.rect()
            elif a_event.modifiers() == QtCore.Qt.ControlModifier:
                self.is_copying = True
                for f_item in this_piano_roll_editor.note_items:
                    if f_item.isSelected():
                        this_piano_roll_editor.draw_note(f_item.note_item, f_item.item_index)
        this_piano_roll_editor.click_enabled = True

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        if self.is_resizing:
            f_adjusted_width_diff = (a_event.pos().x() - self.resize_last_mouse_pos)
            self.resize_last_mouse_pos = a_event.pos().x()
        for f_item in this_piano_roll_editor.note_items:
            if f_item.isSelected():
                if self.is_resizing:
                    f_adjusted_width = pydaw_clip_min(f_item.resize_rect.width() +
                                                      f_adjusted_width_diff,
                                                      global_piano_roll_min_note_length)
                    f_item.resize_rect.setWidth(f_adjusted_width)
                    f_item.setRect(f_item.resize_rect)
                    f_item.setPos(f_item.resize_pos.x(), f_item.resize_pos.y())
                    QtGui.QCursor.setPos(QtGui.QCursor.pos().x(), self.mouse_y_pos)
                else:
                    f_pos_x = f_item.pos().x()
                    f_pos_y = f_item.pos().y()
                    if f_pos_x < global_piano_keys_width:
                        f_pos_x = global_piano_keys_width
                    elif f_pos_x > global_piano_roll_grid_max_start_time:
                        f_pos_x = global_piano_roll_grid_max_start_time
                    if f_pos_y < global_piano_roll_header_height:
                        f_pos_y = global_piano_roll_header_height
                    elif f_pos_y > global_piano_roll_total_height:
                        f_pos_y = global_piano_roll_total_height
                    f_pos_y = \
                    (int((f_pos_y - global_piano_roll_header_height) /
                    self.note_height) * self.note_height) + global_piano_roll_header_height
                    if global_piano_roll_snap:
                        f_pos_x = (int((f_pos_x - global_piano_keys_width) /
                        global_piano_roll_snap_value) *
                        global_piano_roll_snap_value) + global_piano_keys_width
                    f_item.setPos(f_pos_x, f_pos_y)

    def mouseReleaseEvent(self, a_event):
        a_event.setAccepted(True)
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        global global_selected_piano_note
        if self.is_copying:
            f_new_selection = []
        for f_item in this_piano_roll_editor.note_items:
            if f_item.isSelected():
                f_pos_x = f_item.pos().x()
                f_pos_y = f_item.pos().y()
                if self.is_resizing:
                    if global_piano_roll_snap:
                        f_adjusted_width = \
                        (round(f_item.resize_rect.width() /
                        global_piano_roll_snap_value) * global_piano_roll_snap_value)
                        if f_adjusted_width == 0.0:
                            f_adjusted_width = global_piano_roll_snap_value
                        f_item.resize_rect.setWidth(f_adjusted_width)
                        f_item.setRect(f_item.resize_rect)
                    f_new_note_length = ((f_pos_x + f_item.rect().width() -
                    global_piano_keys_width) * 0.001 * 4.0) - f_item.resize_start_pos
                    if global_selected_piano_note is not None and \
                    self.note_item != global_selected_piano_note:
                        f_new_note_length -= (self.item_index * 4.0)
                    if global_piano_roll_snap and f_new_note_length < global_piano_roll_snap_beats:
                        f_new_note_length = global_piano_roll_snap_beats
                    elif f_new_note_length < pydaw_min_note_length:
                        f_new_note_length = pydaw_min_note_length
                    f_item.note_item.set_length(f_new_note_length)
                else:
                    f_new_note_start = (f_pos_x - global_piano_keys_width) * 4.0 * 0.001
                    f_new_note_num = int(global_piano_roll_note_count - \
                    ((f_pos_y - global_piano_roll_header_height) /
                    global_piano_roll_note_height))
                    if self.is_copying:
                        f_item.item_index, f_new_note_start = \
                            pydaw_beats_to_index(f_new_note_start)
                        f_new_note = pydaw_note(f_new_note_start, f_item.note_item.length,
                                                f_new_note_num, f_item.note_item.velocity)
                        this_item_editor.items[f_item.item_index].add_note(f_new_note, False)
                        #pass a ref instead of a str in case fix_overlaps() modifies it.
                        f_new_selection.append(f_item)
                    else:
                        this_item_editor.items[f_item.item_index].notes.remove(f_item.note_item)
                        f_item.item_index, f_new_note_start = \
                            pydaw_beats_to_index(f_new_note_start)
                        f_item.note_item.set_start(f_new_note_start)
                        f_item.note_item.note_num = f_new_note_num
                        this_item_editor.items[f_item.item_index].notes.append(f_item.note_item)
                        this_item_editor.items[f_item.item_index].notes.sort()
        for f_item in this_item_editor.items:
            f_item.fix_overlaps()
        global_selected_piano_note = None
        this_piano_roll_editor.selected_note_strings = []
        if self.is_copying:
            for f_new_item in f_new_selection:
                this_piano_roll_editor.selected_note_strings.append(
                    f_new_item.get_selected_string())
        else:
            for f_item in this_piano_roll_editor.note_items:
                if f_item.isSelected():
                    this_piano_roll_editor.selected_note_strings.append(
                        f_item.get_selected_string())
        for f_item in this_piano_roll_editor.note_items:
            f_item.is_resizing = False
            f_item.is_copying = False
        global_save_and_reload_items()  #<Was above the loop before, but I'm not sure why...
        self.showing_resize_cursor = False
        QtGui.QApplication.restoreOverrideCursor()
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
        QtGui.QApplication.restoreOverrideCursor()

    def hoverLeaveEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverLeaveEvent(self, a_event)
        self.setBrush(self.o_brush)

class piano_roll_editor(QtGui.QGraphicsView):
    def __init__(self):
        self.item_length = 4.0
        self.viewer_width = 1000
        self.grid_div = 16

        self.end_octave = 8
        self.start_octave = -2
        self.notes_in_octave = 12
        self.total_notes = global_piano_roll_note_count
        self.note_height = global_piano_roll_note_height
        self.octave_height = self.notes_in_octave * self.note_height

        self.header_height = global_piano_roll_header_height

        self.piano_height = self.note_height * self.total_notes
        self.piano_width = 32
        self.padding = 2
        self.piano_height = self.note_height * self.total_notes
        global global_piano_roll_total_height
        global_piano_roll_total_height = self.piano_height + global_piano_roll_header_height

        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(100,100,100))
        self.scene.mousePressEvent = self.sceneMousePressEvent
        self.scene.mouseReleaseEvent = self.sceneMouseReleaseEvent
        self.setAlignment(QtCore.Qt.AlignLeft)
        self.setScene(self.scene)
        self.first_open = True
        self.draw_header()
        self.draw_piano()
        self.draw_grid()

        self.has_selected = False

        self.setDragMode(QtGui.QGraphicsView.RubberBandDrag)
        self.note_items = []

        self.right_click = False
        self.left_click = False
        self.click_enabled = True
        self.last_scale = 1.0
        self.last_x_scale = 1.0
        self.scene.selectionChanged.connect(self.highlight_selected)
        self.selected_note_strings = []
        self.piano_keys = None
        self.vel_rand = 0
        self.vel_emphasis = 0

    def prepare_to_quit(self):
        self.scene.clearSelection()
        self.scene.clear()

    def highlight_keys(self, a_state, a_note):
        f_note = int(a_note)
        f_state = int(a_state)
        if self.piano_keys is not None and f_note in self.piano_keys:
            if f_state == 0:
                if self.piano_keys[f_note].is_black:
                    self.piano_keys[f_note].setBrush(QtGui.QColor(0, 0, 0))
                else:
                    self.piano_keys[f_note].setBrush(QtGui.QColor(255, 255, 255))
            elif f_state == 1:
                self.piano_keys[f_note].setBrush(QtGui.QColor(237, 150, 150))
            else:
                assert(False)

    def set_grid_div(self, a_div):
        self.grid_div = int(a_div)

    def scrollContentsBy(self, x, y):
        QtGui.QGraphicsView.scrollContentsBy(self, x, y)
        f_point = self.get_scene_pos()
        self.piano.setPos(f_point.x(), self.header_height)
        self.header.setPos(self.piano_width + self.padding, f_point.y())

    def get_scene_pos(self):
        return QtCore.QPointF(self.horizontalScrollBar().value(),
                              self.verticalScrollBar().value())

    def highlight_selected(self):
        self.has_selected = False
        for f_item in self.note_items:
            if f_item.isSelected():
                f_item.setBrush(pydaw_note_selected_gradient)
                f_item.note_item.is_selected = True
                self.has_selected = True
            else:
                f_item.note_item.is_selected = False
                f_item.set_brush()

    def keyPressEvent(self, a_event):
        QtGui.QGraphicsView.keyPressEvent(self, a_event)
        if a_event.key() == QtCore.Qt.Key_Delete:
            self.selected_note_strings = []
            for f_item in self.note_items:
                if f_item.isSelected():
                    this_item_editor.items[f_item.item_index].remove_note(f_item.note_item)
            global_save_and_reload_items()
        QtGui.QApplication.restoreOverrideCursor()

    def focusOutEvent(self, a_event):
        QtGui.QGraphicsView.focusOutEvent(self, a_event)
        QtGui.QApplication.restoreOverrideCursor()

    def sceneMouseReleaseEvent(self, a_event):
        QtGui.QGraphicsScene.mouseReleaseEvent(self.scene, a_event)
        self.click_enabled = True

    def sceneMousePressEvent(self, a_event):
        QtGui.QApplication.restoreOverrideCursor()
        if not this_item_editor.enabled:
            this_item_editor.show_not_enabled_warning()
            a_event.setAccepted(True)
            QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)
            return
        if a_event.modifiers() == QtCore.Qt.ControlModifier:
            self.hover_restore_cursor_event()
        elif self.click_enabled and this_item_editor.enabled:
            self.scene.clearSelection()
            f_pos_x = a_event.scenePos().x()
            f_pos_y = a_event.scenePos().y()
            if f_pos_x > global_piano_keys_width and \
            f_pos_x < global_piano_roll_grid_max_start_time and \
            f_pos_y > global_piano_roll_header_height and \
            f_pos_y < global_piano_roll_total_height:
                if self.vel_rand == 1:
                    pass
                elif self.vel_rand == 2:
                    pass
                f_note = int(self.total_notes - ((f_pos_y -
                             global_piano_roll_header_height) / self.note_height)) + 1
                if global_piano_roll_snap:
                    f_beat = (int((f_pos_x - global_piano_keys_width) /
                             global_piano_roll_snap_value) * \
                             global_piano_roll_snap_value) * 0.001 * 4.0
                    f_note_item = pydaw_note(f_beat, global_piano_roll_snap_beats, f_note,
                                             self.get_vel(f_beat))
                else:
                    f_beat = (f_pos_x - global_piano_keys_width) * 0.001 * 4.0
                    f_note_item = pydaw_note(f_beat, 0.25, f_note, self.get_vel(f_beat))
                f_note_index = this_item_editor.add_note(f_note_item)
                global global_selected_piano_note
                global_selected_piano_note = f_note_item
                f_drawn_note = self.draw_note(f_note_item, f_note_index)
                f_drawn_note.setSelected(True)
                f_drawn_note.resize_start_pos = \
                    f_drawn_note.note_item.start + (4.0 * f_drawn_note.item_index)
                f_drawn_note.resize_pos = f_drawn_note.pos()
                f_drawn_note.resize_rect = f_drawn_note.rect()
                f_drawn_note.is_resizing = True
                f_drawn_note.mouse_y_pos = QtGui.QCursor.pos().y()
                f_drawn_note.resize_last_mouse_pos = a_event.pos().x()
        a_event.setAccepted(True)
        QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)

    def hover_restore_cursor_event(self, a_event=None):
        QtGui.QApplication.restoreOverrideCursor()

    def draw_header(self):
        self.header = QtGui.QGraphicsRectItem(0, 0, self.viewer_width, self.header_height)
        self.header.hoverEnterEvent = self.hover_restore_cursor_event
        self.header.setBrush(QtGui.QColor.fromRgb(60, 60, 60, 255))
        self.scene.addItem(self.header)
        self.header.mapToScene(self.piano_width + self.padding, 0.0)
        self.beat_width = self.viewer_width / self.item_length
        self.value_width = self.beat_width / self.grid_div
        self.header.setZValue(1003.0)

    def draw_piano(self):
        self.piano_keys = {}
        f_black_notes = [2, 4, 6, 9, 11]
        f_piano_label = QtGui.QFont()
        f_piano_label.setPointSize(8)
        self.piano = QtGui.QGraphicsRectItem(0, 0, self.piano_width, self.piano_height)
        self.scene.addItem(self.piano)
        self.piano.mapToScene(0.0, self.header_height)
        f_key = piano_key_item(self.piano_width, self.note_height, self.piano)
        f_label = QtGui.QGraphicsSimpleTextItem("C8", f_key)
        f_label.setPen(QtCore.Qt.black)
        f_label.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
        f_label.setPos(4, 0)
        f_label.setFont(f_piano_label)
        f_key.setBrush(QtGui.QColor(255, 255, 255))
        f_note_index = 0
        f_note_num = 0

        for i in range(self.end_octave - self.start_octave,
                       self.start_octave - self.start_octave, -1):
            for j in range(self.notes_in_octave, 0, -1):
                f_key = piano_key_item(self.piano_width, self.note_height, self.piano)
                self.piano_keys[f_note_index] = f_key
                f_note_index += 1
                f_key.setPos(0, (self.note_height * j) + (self.octave_height * (i - 1)))

                f_key.setToolTip("{}hz".format(
                        round(pydaw_pitch_to_hz(f_note_num))))
                f_note_num += 1
                if j == 12:
                    f_label = QtGui.QGraphicsSimpleTextItem("C{}".format(
                        self.end_octave - i), f_key)
                    f_label.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
                    f_label.setPos(4, 0)
                    f_label.setFont(f_piano_label)
                    f_label.setPen(QtCore.Qt.black)
                if j in f_black_notes:
                    f_key.setBrush(QtGui.QColor(0, 0, 0))
                    f_key.is_black = True
                else:
                    f_key.setBrush(QtGui.QColor(255, 255, 255))
                    f_key.is_black = False
        self.piano.setZValue(1000.0)

    def draw_grid(self):
        f_black_key_brush = QtGui.QBrush(QtGui.QColor(30, 30, 30, 90))
        f_white_key_brush = QtGui.QBrush(QtGui.QColor(210, 210, 210, 90))
        f_base_brush = QtGui.QBrush(QtGui.QColor(255, 255, 255, 120))
        if self.first_open or \
        this_piano_roll_editor_widget.scale_combobox.currentIndex() == 0: #Major
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush , f_white_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 1: #Melodic Minor
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_white_key_brush, f_black_key_brush , f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 2: #Harmonic Minor
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_white_key_brush, f_black_key_brush, f_black_key_brush,
            f_white_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 3: #Natural Minor
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_white_key_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 4: #Pentatonic Major
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush, f_white_key_brush, f_black_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_black_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 5: #Pentatonic Minor
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 6: #Dorian
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_white_key_brush,
            f_black_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 7: #Phrygian
            f_octave_brushes = [f_base_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_white_key_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 8: #Lydian
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush, f_white_key_brush, f_black_key_brush, f_white_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 9: #Mixolydian
            f_octave_brushes = [f_base_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush, f_white_key_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_white_key_brush,
            f_black_key_brush]
        elif this_piano_roll_editor_widget.scale_combobox.currentIndex() == 10: #Locrian
            f_octave_brushes = [f_base_brush, f_white_key_brush, f_black_key_brush,
            f_white_key_brush, f_black_key_brush, f_white_key_brush, f_white_key_brush,
            f_black_key_brush, f_white_key_brush, f_black_key_brush, f_white_key_brush,
            f_black_key_brush]

        f_current_key = 0
        if not self.first_open:
            f_index = 12 - this_piano_roll_editor_widget.scale_key_combobox.currentIndex()
            f_octave_brushes = f_octave_brushes[f_index:] + f_octave_brushes[:f_index]
        self.first_open = False
        f_note_bar = QtGui.QGraphicsRectItem(0, 0, self.viewer_width,
                                             self.note_height, self.piano)
        f_note_bar.hoverMoveEvent = self.hover_restore_cursor_event
        f_note_bar.setBrush(f_base_brush)
        f_note_bar.setPos(self.piano_width + self.padding,  0.0)
        for i in range(self.end_octave-self.start_octave,
                       self.start_octave-self.start_octave, -1):
            for j in range(self.notes_in_octave, 0, -1):
                f_note_bar = QtGui.QGraphicsRectItem(0, 0, self.viewer_width,
                                                     self.note_height, self.piano)
                f_note_bar.setZValue(60.0)
                f_note_bar.setBrush(f_octave_brushes[f_current_key])
                f_current_key += 1
                if f_current_key >= len(f_octave_brushes):
                    f_current_key = 0
                f_note_bar.setPos(self.piano_width + self.padding,
                                  self.note_height * (j) + self.octave_height * (i-1))
        f_beat_pen = QtGui.QPen()
        f_beat_pen.setWidth(2)
        f_bar_pen = QtGui.QPen(QtGui.QColor(240, 30, 30), 12.0)
        f_line_pen = QtGui.QPen(QtGui.QColor(0, 0, 0))
        f_beat_y = self.piano_height + self.header_height + self.note_height
        for i in range(0, int(self.item_length) + 1):
            f_beat_x = (self.beat_width * i) + self.piano_width
            f_beat = self.scene.addLine(f_beat_x, 0, f_beat_x, f_beat_y)
            f_beat_number = i % 4
            if f_beat_number == 0 and not i == 0:
                f_beat.setPen(f_bar_pen)
            else:
                f_beat.setPen(f_beat_pen)
            if i < self.item_length:
                f_number = QtGui.QGraphicsSimpleTextItem(str(f_beat_number + 1), self.header)
                f_number.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
                f_number.setPos(self.beat_width * i + 5, 2)
                f_number.setBrush(QtCore.Qt.white)
                for j in range(0, self.grid_div):
                    f_x = (self.beat_width * i) + (self.value_width * j) + self.piano_width
                    f_line = self.scene.addLine(f_x, self.header_height, f_x, f_beat_y)
                    if float(j) != self.grid_div * 0.5:
                        f_line.setPen(f_line_pen)

    def resizeEvent(self, a_event):
        QtGui.QGraphicsView.resizeEvent(self, a_event)
        self.scale_to_width()

    def scale_to_width(self):
        if global_item_zoom_index == 0:
            self.scale(1.0 / self.last_x_scale, 1.0)
            self.last_x_scale = 1.0
        elif global_item_editing_count > 0 and global_item_zoom_index == 1:
            f_width = float(self.rect().width()) - float(self.verticalScrollBar().width()) - 6.0
            f_new_scale = f_width / self.viewer_width
            if self.last_x_scale != f_new_scale:
                self.scale(1.0 / self.last_x_scale, 1.0)
                self.last_x_scale = f_new_scale
                self.scale(self.last_x_scale, 1.0)
            self.horizontalScrollBar().setSliderPosition(0)

    def clear_drawn_items(self):
        self.note_items = []
        self.scene.clear()
        self.draw_header()
        self.draw_piano()
        self.draw_grid()

    def draw_item(self):
        """ Draw all notes in an instance of the pydaw_item class"""
        self.has_selected = False #Reset the selected-ness state...
        self.viewer_width = 1000 * global_item_editing_count
        self.item_length = float(4 * global_item_editing_count)
        #pydaw_set_piano_roll_quantize(this_piano_roll_editor_widget.snap_combobox.currentIndex())
        global global_piano_roll_grid_max_start_time
        global_piano_roll_grid_max_start_time = \
            (999.0 * global_item_editing_count) + global_piano_keys_width
        self.clear_drawn_items()
        if not this_item_editor.enabled:
            return
        f_beat_offset = 0
        for f_item in this_item_editor.items:
            for f_note in f_item.notes:
                f_note_item = self.draw_note(f_note, f_beat_offset)
                f_note_item.resize_last_mouse_pos = f_note_item.scenePos().x()
                f_note_item.resize_pos = f_note_item.scenePos()
                if f_note_item.get_selected_string() in self.selected_note_strings:
                    f_note_item.setSelected(True)
            f_beat_offset += 1
        self.scrollContentsBy(0, 0)

    def draw_note(self, a_note, a_item_index):
        """ a_note is an instance of the pydaw_note class"""
        f_start = self.piano_width + self.padding + self.beat_width * (a_note.start +
            (float(a_item_index) * 4.0))
        f_length = self.beat_width * a_note.length
        f_note = self.header_height + self.note_height * (self.total_notes - a_note.note_num)
        f_note_item = piano_roll_note_item(f_length, self.note_height,
                                           a_note.note_num, a_note, a_item_index)
        f_note_item.setPos(f_start, f_note)
        self.scene.addItem(f_note_item)
        self.note_items.append(f_note_item)
        return f_note_item

    def set_vel_rand(self, a_rand, a_emphasis):
        self.vel_rand = int(a_rand)
        self.vel_emphasis = int(a_emphasis)

    def get_vel(self, a_beat):
        if self.vel_rand == 0:
            return 100
        f_emph = self.get_beat_emphasis(a_beat)
        if self.vel_rand == 1:
            return random.randint(75 - f_emph, 100 - f_emph)
        elif self.vel_rand == 2:
            return random.randint(75 - f_emph, 100 - f_emph)
        else:
            assert(False)

    def get_beat_emphasis(self, a_beat, a_amt=25.0):
        if self.vel_emphasis == 0:
            return 0
        f_beat = a_beat
        if self.vel_emphasis == 2:
            f_beat += 0.5
        f_beat = f_beat % 1.0
        if f_beat > 0.5:
            f_beat = 0.5 - (f_beat - 0.5)
            f_beat = 0.5 - f_beat
        return int(f_beat * 2.0 * a_amt)

class piano_roll_editor_widget():
    def quantize_dialog(self):
        this_item_editor.quantize_dialog(False, this_piano_roll_editor.has_selected)

    def transpose_dialog(self):
        this_item_editor.transpose_dialog(False, this_piano_roll_editor.has_selected)

    def velocity_dialog(self):
        this_item_editor.velocity_dialog(False, this_piano_roll_editor.has_selected)

    def clear_notes(self):
        this_item_editor.clear_notes(False)

    def select_all(self):
        for f_note in this_piano_roll_editor.note_items:
            f_note.setSelected(True)

    def __init__(self):
        self.widget = QtGui.QWidget()
        self.vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.vlayout)

        self.controls_grid_layout = QtGui.QGridLayout()
        self.scale_key_combobox = QtGui.QComboBox()
        self.scale_key_combobox.setMinimumWidth(60)
        self.scale_key_combobox.addItems(["C", "C#", "D", "D#", "E", "F", "F#",
                                          "G", "G#", "A", "A#", "B"])
        self.scale_key_combobox.currentIndexChanged.connect(self.reload_handler)
        self.controls_grid_layout.addWidget(QtGui.QLabel("Key:"), 0, 3)
        self.controls_grid_layout.addWidget(self.scale_key_combobox, 0, 4)
        self.scale_combobox = QtGui.QComboBox()
        self.scale_combobox.setMinimumWidth(172)
        self.scale_combobox.addItems(["Major", "Melodic Minor", "Harmonic Minor",
                                      "Natural Minor", "Pentatonic Major", "Pentatonic Minor",
                                      "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian"])
        self.scale_combobox.currentIndexChanged.connect(self.reload_handler)
        self.controls_grid_layout.addWidget(QtGui.QLabel(_("Scale:")), 0, 5)
        self.controls_grid_layout.addWidget(self.scale_combobox, 0, 6)

        self.controls_grid_layout.addItem(
            QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding), 0, 30)

        self.vel_rand_combobox = QtGui.QComboBox()
        self.vel_rand_combobox.setMinimumWidth(90)
        self.vel_rand_combobox.addItems([_("None"), _("Tight"), _("Loose")])
        self.vel_rand_combobox.currentIndexChanged.connect(self.set_vel_rand)
        self.controls_grid_layout.addWidget(QtGui.QLabel(_("Vel. Rand.")), 0, 14)
        self.controls_grid_layout.addWidget(self.vel_rand_combobox, 0, 15)
        self.vel_emphasis_combobox = QtGui.QComboBox()
        self.vel_emphasis_combobox.setMinimumWidth(100)
        self.vel_emphasis_combobox.addItems([_("None"), _("On-beat"), _("Off-beat")])
        self.vel_emphasis_combobox.currentIndexChanged.connect(self.set_vel_rand)
        self.controls_grid_layout.addWidget(QtGui.QLabel(_("Emphasis")), 0, 18)
        self.controls_grid_layout.addWidget(self.vel_emphasis_combobox, 0, 19)

        self.edit_menu_button = QtGui.QPushButton(_("Edit"))
        self.edit_menu_button.setFixedWidth(60)
        self.edit_menu = QtGui.QMenu(self.widget)
        self.edit_menu_button.setMenu(self.edit_menu)
        self.controls_grid_layout.addWidget(self.edit_menu_button, 0, 30)

        self.quantize_action = QtGui.QAction(_("Quantize"), self.widget)
        self.edit_menu.addAction(self.quantize_action)
        self.quantize_action.triggered.connect(self.quantize_dialog)

        self.transpose_action = QtGui.QAction(_("Transpose"), self.widget)
        self.edit_menu.addAction(self.transpose_action)
        self.transpose_action.triggered.connect(self.transpose_dialog)

        self.velocity_action = QtGui.QAction(_("Velocity"), self.widget)
        self.edit_menu.addAction(self.velocity_action)
        self.velocity_action.triggered.connect(self.velocity_dialog)

        self.select_all_action = QtGui.QAction(_("Select All"), self.widget)
        self.edit_menu.addAction(self.select_all_action)
        self.select_all_action.triggered.connect(self.select_all)

        self.edit_menu.addSeparator()

        self.clear_action = QtGui.QAction(_("Clear All"), self.widget)
        self.edit_menu.addAction(self.clear_action)
        self.clear_action.triggered.connect(self.clear_notes)

        self.controls_grid_layout.addItem(
            QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding), 0, 31)

        self.vlayout.addLayout(self.controls_grid_layout)
        self.vlayout.addWidget(this_piano_roll_editor)
        self.snap_combobox = QtGui.QComboBox()
        self.snap_combobox.setMinimumWidth(90)
        self.snap_combobox.addItems([_("None"), "1/4", "1/8", "1/12", "1/16",
                                     "1/32", "1/64", "1/128"])
        self.controls_grid_layout.addWidget(QtGui.QLabel(_("Snap:")), 0, 0)
        self.controls_grid_layout.addWidget(self.snap_combobox, 0, 1)
        self.snap_combobox.currentIndexChanged.connect(self.set_snap)

    def set_vel_rand(self, a_val=None):
        this_piano_roll_editor.set_vel_rand(self.vel_rand_combobox.currentIndex(),
                                            self.vel_emphasis_combobox.currentIndex())

    def set_snap(self, a_val=None):
        f_index = self.snap_combobox.currentIndex()
        pydaw_set_piano_roll_quantize(f_index)
        if len(global_open_items_uids) > 0:
            global_open_items()
        else:
            this_piano_roll_editor.clear_drawn_items()

    def reload_handler(self, a_val=None):
        this_pydaw_project.set_midi_scale(self.scale_key_combobox.currentIndex(),
                                          self.scale_combobox.currentIndex())
        if len(global_open_items_uids) > 0:
            global_open_items()
        else:
            this_piano_roll_editor.clear_drawn_items()

global_automation_point_diameter = 15.0
global_automation_point_radius = global_automation_point_diameter * 0.5
global_automation_ruler_width = 24
global_automation_width = 690
global_automation_height = 300

global_automation_grid_max_start_time = \
global_automation_width + global_automation_ruler_width - global_automation_point_radius

global_automation_total_height = \
global_automation_ruler_width +  global_automation_height - global_automation_point_radius

global_automation_total_width = \
global_automation_ruler_width + global_automation_width - global_automation_point_radius

global_automation_min_height = global_automation_ruler_width - global_automation_point_radius

global_automation_gradient = QtGui.QLinearGradient(0, 0, global_automation_point_diameter,
                                                   global_automation_point_diameter)
global_automation_gradient.setColorAt(0, QtGui.QColor(240, 10, 10))
global_automation_gradient.setColorAt(1, QtGui.QColor(250, 90, 90))

global_automation_selected_gradient = QtGui.QLinearGradient(0, 0,
                                                            global_automation_point_diameter,
                                                            global_automation_point_diameter)
global_automation_selected_gradient.setColorAt(0, QtGui.QColor(255, 255, 255))
global_automation_selected_gradient.setColorAt(1, QtGui.QColor(240, 240, 240))

class automation_item(QtGui.QGraphicsEllipseItem):
    def __init__(self, a_time, a_value, a_cc, a_view, a_is_cc, a_item_index):
        QtGui.QGraphicsEllipseItem.__init__(self, 0, 0, global_automation_point_diameter,
                                            global_automation_point_diameter)
        self.item_index = a_item_index
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.setPos(a_time-global_automation_point_radius,
                    a_value - global_automation_point_radius)
        self.setBrush(global_automation_gradient)
        f_pen = QtGui.QPen()
        f_pen.setWidth(2)
        f_pen.setColor(QtGui.QColor(170,0,0))
        self.setPen(f_pen)
        self.cc_item = a_cc
        self.parent_view = a_view
        self.is_cc = a_is_cc
        self.is_copying = False

    def set_brush(self):
        if self.isSelected():
            self.setBrush(global_automation_selected_gradient)
        else:
            self.setBrush(global_automation_gradient)

    def mousePressEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mousePressEvent(self, a_event)
        if a_event.modifiers() == QtCore.Qt.ControlModifier:
            self.is_copying = True
            for f_item in self.parent_view.automation_points:
                if f_item.isSelected():
                    self.parent_view.draw_point(f_item.cc_item, f_item.item_index)
                    if self.is_cc:
                        this_item_editor.items[f_item.item_index].ccs.append(
                            f_item.cc_item.clone())
                    else:
                        this_item_editor.items[f_item.item_index].pitchbends.append(
                            f_item.cc_item.clone())
        else:
            self.is_copying = False

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseMoveEvent(self, a_event)
        for f_point in self.parent_view.automation_points:
            if f_point.isSelected():
                if f_point.pos().x() < global_automation_min_height:
                    f_point.setPos(global_automation_min_height, f_point.pos().y())
                elif f_point.pos().x() > global_automation_grid_max_start_time:
                    f_point.setPos(global_automation_grid_max_start_time, f_point.pos().y())
                if f_point.pos().y() < global_automation_min_height:
                    f_point.setPos(f_point.pos().x(), global_automation_min_height)
                elif f_point.pos().y() > global_automation_total_height:
                    f_point.setPos(f_point.pos().x(), global_automation_total_height)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseReleaseEvent(self, a_event)
        self.setGraphicsEffect(None)
        for f_point in self.parent_view.automation_points:
            if f_point.isSelected():
                f_cc_start = \
                round((((f_point.pos().x() - global_automation_min_height) /
                    global_automation_width) * 4.0), 4)

                if f_cc_start >= 4.0 * global_item_editing_count:
                    f_cc_start = (4.0 * global_item_editing_count) - 0.01
                elif f_cc_start < 0.0:
                    f_cc_start = 0.0
                f_new_item_index, f_cc_start = pydaw_beats_to_index(f_cc_start)
                if self.is_cc:
                    if not self.is_copying:
                        this_item_editor.items[f_point.item_index].ccs.remove(f_point.cc_item)
                    f_point.item_index = f_new_item_index
                    f_cc_val = int(127.0 - (((f_point.pos().y() - \
                    global_automation_min_height) / global_automation_height) * 127.0))

                    f_point.cc_item.start = f_cc_start
                    f_point.cc_item.set_val(f_cc_val)
                    this_item_editor.items[f_point.item_index].ccs.append(f_point.cc_item)
                    this_item_editor.items[f_point.item_index].ccs.sort()
                else:
                    if not self.is_copying:
                        this_item_editor.items[f_point.item_index].pitchbends.remove(
                        f_point.cc_item)
                    f_point.item_index = f_new_item_index
                    f_cc_val = (1.0 - (((f_point.pos().y() - \
                    global_automation_min_height) / global_automation_height) * 2.0))

                    f_point.cc_item.start = f_cc_start
                    f_point.cc_item.set_val(f_cc_val)
                    this_item_editor.items[f_point.item_index].pitchbends.append(f_point.cc_item)
                    this_item_editor.items[f_point.item_index].pitchbends.sort()
        global_save_and_reload_items()
        QtGui.QApplication.restoreOverrideCursor()

global_automation_editors = []

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
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.setResizeAnchor(QtGui.QGraphicsView.AnchorViewCenter)
        self.cc_num = 1
        self.last_scale = 1.0
        self.plugin_index = 0
        self.last_x_scale = 1.0
        global_automation_editors.append(self)
        self.selection_enabled = True
        self.scene.selectionChanged.connect(self.selection_changed)

    def selection_changed(self, a_event=None):
        if self.selection_enabled:
            for f_item in self.automation_points:
                f_item.set_brush()

    def set_tooltip(self, a_enabled=False):
        if a_enabled:
            if self.is_cc:
                f_start = _("Select the plugin/control you wish to automate using the comboboxes"
                          " below\n")
            else:
                f_start = ""
            self.setToolTip(
                _("{}Draw points by double-clicking, then click the 'smooth' button to "
                "draw extra points between them.\nClick+drag to select points\n"
                "Ctrl+click+drag selected points to copy\npress the 'delete' button"
                " to delete selected points.").format(f_start))
        else:
            self.setToolTip("")

    def prepare_to_quit(self):
        self.selection_enabled = False
        self.scene.clearSelection()
        self.scene.clear()

    def keyPressEvent(self, a_event):
        QtGui.QGraphicsScene.keyPressEvent(self.scene, a_event)
        if not this_item_editor.enabled:
            return
        if a_event.key() == QtCore.Qt.Key_Delete:
            self.selection_enabled = False
            for f_point in self.automation_points:
                if f_point.isSelected():
                    if self.is_cc:
                        this_item_editor.items[f_point.item_index].remove_cc(f_point.cc_item)
                    else:
                        this_item_editor.items[f_point.item_index].remove_pb(f_point.cc_item)
            global_save_and_reload_items()
            self.selection_enabled = True

    def clear_current_item(self):
        """ If this is a CC editor, it only clears the selected CC.  """
        self.selection_enabled = False
        if not self.automation_points:
            return
        for f_point in self.automation_points:
            if self.is_cc:
                this_item_editor.items[f_point.item_index].remove_cc(f_point.cc_item)
            else:
                this_item_editor.items[f_point.item_index].remove_pb(f_point.cc_item)
        global_save_and_reload_items()
        self.selection_enabled = True

    def sceneMouseDoubleClickEvent(self, a_event):
        if not this_item_editor.enabled:
            this_item_editor.show_not_enabled_warning()
            return
        f_pos_x = a_event.scenePos().x() - global_automation_point_radius
        f_pos_y = a_event.scenePos().y() - global_automation_point_radius
        f_cc_start = ((f_pos_x - global_automation_min_height) / global_automation_width) * 4.0
        f_cc_start = pydaw_clip_value(f_cc_start, 0.0,
                                      (4.0  * global_item_editing_count) - 0.01, a_round=True)
        if self.is_cc:
            f_cc_val = int(127.0 - (((f_pos_y - global_automation_min_height) /
                global_automation_height) * 127.0))
            f_cc_val = pydaw_clip_value(f_cc_val, 0, 127)
            this_item_editor.add_cc(pydaw_cc(round(f_cc_start, 4),
                                             self.plugin_index, self.cc_num, f_cc_val))
        else:
            f_cc_val = 1.0 - (((f_pos_y - global_automation_min_height) /
                global_automation_height) * 2.0)
            f_cc_val = pydaw_clip_value(f_cc_val, -1.0, 1.0)
            this_item_editor.add_pb(pydaw_pitchbend(round(f_cc_start, 4), round(f_cc_val, 4)))
        QtGui.QGraphicsScene.mouseDoubleClickEvent(self.scene, a_event)
        global_save_and_reload_items()

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
        f_bar_pen = QtGui.QPen()
        f_bar_pen.setWidth(2)
        f_bar_pen.setColor(QtGui.QColor(224, 60, 60))
        f_line_pen = QtGui.QPen()
        f_line_pen.setColor(QtGui.QColor(0, 0, 0, 40))
        if self.is_cc:
            f_labels = [0, '127', 0, '64', 0, '0']
        else:
            f_labels = [0, '1.0', 0, '0', 0, '-1.0']
        for i in range(1,6):
            f_line = QtGui.QGraphicsLineItem(0, 0, self.viewer_width, 0, self.y_axis)
            f_line.setPos(self.axis_size, self.viewer_height * (i - 1) / 4)
            if i % 2:
                f_label = QtGui.QGraphicsSimpleTextItem(f_labels[i], self.y_axis)
                f_label.setPos(1, self.viewer_height * (i - 1) / 4)
                f_label.setBrush(QtCore.Qt.white)
            if i == 3:
                f_line.setPen(f_beat_pen)

        for i in range(0, int(self.item_length) + 1):
            f_beat = QtGui.QGraphicsLineItem(0, 0, 0,
                                             self.viewer_height +
                                             self.axis_size-f_beat_pen.width(),
                                             self.x_axis)
            f_beat.setPos(self.beat_width * i, 0.5 * f_beat_pen.width())
            f_beat.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
            f_beat_number = i % 4
            if f_beat_number == 0 and not i == 0:
                f_beat.setPen(f_bar_pen)
                f_beat.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
            else:
                f_beat.setPen(f_beat_pen)
            if i < self.item_length:
                f_number = QtGui.QGraphicsSimpleTextItem(str(f_beat_number + 1), self.x_axis)
                f_number.setFlag(QtGui.QGraphicsItem.ItemIgnoresTransformations)
                f_number.setPos(self.beat_width * i + 5, 2)
                f_number.setBrush(QtCore.Qt.white)
                for j in range(0, self.grid_div):
                    f_line = QtGui.QGraphicsLineItem(0, 0, 0, self.viewer_height, self.x_axis)
                    if float(j) == self.grid_div / 2.0:
                        f_line.setLine(0, 0, 0, self.viewer_height)
                        f_line.setPos((self.beat_width*i)+(self.value_width * j), self.axis_size)
                    else:
                        f_line.setPos((self.beat_width * i) +
                            (self.value_width * j), self.axis_size)
                        f_line.setPen(f_line_pen)

    def clear_drawn_items(self):
        self.selection_enabled = False
        self.scene.clear()
        self.automation_points = []
        self.lines = []
        self.draw_axis()
        self.draw_grid()
        self.selection_enabled = True

    def resizeEvent(self, a_event):
        QtGui.QGraphicsView.resizeEvent(self, a_event)
        self.scale_to_width()

    def scale_to_width(self):
        if global_item_zoom_index == 0:
            self.scale(1.0 / self.last_x_scale, 1.0)
            self.last_x_scale = 1.0
        elif global_item_editing_count > 0 and global_item_zoom_index == 1:
            f_width = float(self.rect().width()) - float(self.verticalScrollBar().width()) - 6.0
            f_new_scale = f_width / self.viewer_width
            if self.last_x_scale != f_new_scale:
                self.scale(1.0 / self.last_x_scale, 1.0)
                self.last_x_scale = f_new_scale
                self.scale(self.last_x_scale, 1.0)
            self.horizontalScrollBar().setSliderPosition(0)

    #TODO:  Remove
    def connect_points(self):
        pass

    def set_cc_num(self, a_plugin_index, a_port_num):
        self.plugin_index = global_plugin_numbers[int(a_plugin_index)]
        self.cc_num = a_port_num
        self.clear_drawn_items()
        self.draw_item()

    def draw_item(self):
        self.viewer_width = global_automation_width * global_item_editing_count
        self.item_length = 4.0 * global_item_editing_count
        global global_automation_grid_max_start_time
        global_automation_grid_max_start_time = \
            (global_automation_width * global_item_editing_count) + \
            global_automation_ruler_width - global_automation_point_radius
        self.clear_drawn_items()
        if not this_item_editor.enabled:
            return
        f_item_index = 0
        f_pen = QtGui.QPen(pydaw_note_gradient, 2.0)
        f_note_height = (global_automation_height / 127.0)
        for f_item in this_item_editor.items:
            if self.is_cc:
                for f_cc in f_item.ccs:
                    if f_cc.cc_num == self.cc_num and f_cc.plugin_index == self.plugin_index:
                        self.draw_point(f_cc, f_item_index)
            else:
                for f_pb in f_item.pitchbends:
                    self.draw_point(f_pb, f_item_index)
            for f_note in f_item.notes:
                f_note_start = (f_item_index * global_automation_width) + (f_note.start * 0.25 *
                    global_automation_width) + global_automation_ruler_width
                f_note_end = f_note_start + (f_note.length * global_automation_width * 0.25)
                f_note_y = \
                    global_automation_ruler_width + ((127.0 - (f_note.note_num)) * f_note_height)
                f_note_item = QtGui.QGraphicsLineItem(f_note_start, f_note_y,
                                                      f_note_end, f_note_y)
                f_note_item.setPen(f_pen)
                self.scene.addItem(f_note_item)
            f_item_index += 1

    def draw_point(self, a_cc, a_item_index):
        """ a_cc is an instance of the pydaw_cc class"""
        f_time = self.axis_size + (((float(a_item_index) * 4.0) + a_cc.start) * self.beat_width)
        if self.is_cc:
            f_value = self.axis_size +  self.viewer_height / 127.0 * (127.0 - a_cc.cc_val)
        else:
            f_value = self.axis_size +  self.viewer_height / 2.0 * (1.0 - a_cc.pb_val)
        f_point = automation_item(f_time, f_value, a_cc, self, self.is_cc, a_item_index)
        self.automation_points.append(f_point)
        self.scene.addItem(f_point)
        self.connect_points()

global_last_ipb_value = 18  #For the 'add point' dialog to remember settings

class automation_viewer_widget:
    def plugin_changed(self, a_val=None):
        self.control_combobox.clear()
        self.control_combobox.addItems(global_cc_names[str(self.plugin_combobox.currentText())])
        self.automation_viewer.draw_item()

    def control_changed(self, a_val=None):
        self.set_cc_num()
        self.ccs_in_use_combobox.setCurrentIndex(0)

    def set_cc_num(self, a_val=None):
        f_port_name = str(self.control_combobox.currentText())
        if f_port_name != "":
            f_num = global_controller_port_name_dict[str(self.plugin_combobox.currentText())]\
                [f_port_name].port
            self.automation_viewer.set_cc_num(self.plugin_combobox.currentIndex(), f_num)

    def ccs_in_use_combobox_changed(self, a_val=None):
        if not self.suppress_ccs_in_use:
            f_str = str(self.ccs_in_use_combobox.currentText())
            if f_str != "":
                f_arr = f_str.split("|")
                self.plugin_combobox.setCurrentIndex(self.plugin_combobox.findText(f_arr[0]))
                self.control_combobox.setCurrentIndex(self.control_combobox.findText(f_arr[1]))

    def update_ccs_in_use(self, a_ccs):
        self.suppress_ccs_in_use = True
        self.ccs_in_use_combobox.clear()
        self.ccs_in_use_combobox.addItem("")
        for f_cc in a_ccs:
            f_key_split = f_cc.split("|")
            f_plugin_name = global_plugin_names[global_plugin_indexes[int(f_key_split[0])]]
            f_map = global_controller_port_num_dict[f_plugin_name][int(f_key_split[1])]
            self.ccs_in_use_combobox.addItem("{}|{}".format(f_plugin_name, f_map.name))
        self.suppress_ccs_in_use = False

    def smooth_pressed(self):
        if self.is_cc:
            f_map = global_controller_port_name_dict[str(self.plugin_combobox.currentText())]\
                [str(self.control_combobox.currentText())]
            pydaw_smooth_automation_points(
                this_item_editor.items, self.is_cc,
                global_plugin_numbers[self.plugin_combobox.currentIndex()], f_map.port)
        else:
            pydaw_smooth_automation_points(this_item_editor.items, self.is_cc)
        global_save_and_reload_items()

    def __init__(self, a_viewer, a_is_cc=True):
        self.is_cc = a_is_cc
        self.widget = QtGui.QGroupBox()
        self.vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.vlayout)
        self.automation_viewer = a_viewer
        self.vlayout.addWidget(self.automation_viewer)
        self.hlayout = QtGui.QHBoxLayout()

        if a_is_cc:
            self.hlayout2 = QtGui.QHBoxLayout()
            self.vlayout.addLayout(self.hlayout2)
            self.plugin_combobox = QtGui.QComboBox()
            self.plugin_combobox.setMinimumWidth(120)
            self.plugin_combobox.addItems(global_plugin_names)
            self.hlayout2.addWidget(QtGui.QLabel(_("Plugin")))
            self.hlayout2.addWidget(self.plugin_combobox)
            self.plugin_combobox.currentIndexChanged.connect(self.plugin_changed)
            self.control_combobox = QtGui.QComboBox()
            self.control_combobox.setMinimumWidth(240)
            self.hlayout2.addWidget(QtGui.QLabel(_("Control")))
            self.hlayout2.addWidget(self.control_combobox)
            self.hlayout2.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))
            self.control_combobox.currentIndexChanged.connect(self.control_changed)
            self.ccs_in_use_combobox = QtGui.QComboBox()
            self.ccs_in_use_combobox.setMinimumWidth(300)
            self.suppress_ccs_in_use = False
            self.ccs_in_use_combobox.currentIndexChanged.connect(self.ccs_in_use_combobox_changed)
            self.hlayout.addWidget(QtGui.QLabel(_("In Use:")))
            self.hlayout.addWidget(self.ccs_in_use_combobox)

        self.vlayout.addLayout(self.hlayout)
        self.smooth_button = QtGui.QPushButton(_("Smooth"))
        self.smooth_button.setToolTip(_("By default, the control points are steppy, "
            "this button draws extra points between the exisiting points."))
        self.smooth_button.pressed.connect(self.smooth_pressed)
        self.hlayout.addWidget(self.smooth_button)
        self.hlayout.addItem(QtGui.QSpacerItem(10, 10))
        self.edit_button = QtGui.QPushButton(_("Edit"))
        self.hlayout.addWidget(self.edit_button)
        self.edit_menu = QtGui.QMenu(self.widget)
        self.add_point_action = self.edit_menu.addAction(_("Add Point"))
        if self.is_cc:
            self.add_point_action.triggered.connect(self.add_cc_point)
            self.paste_point_action = self.edit_menu.addAction(_("Paste Point"))
            self.paste_point_action.triggered.connect(self.paste_cc_point)
        else:
            self.add_point_action.triggered.connect(self.add_pb_point)
        self.select_all_action = self.edit_menu.addAction(_("Select All"))
        self.select_all_action.triggered.connect(self.select_all)
        self.edit_menu.addSeparator()
        self.clear_action = self.edit_menu.addAction(_("Clear"))
        self.clear_action.triggered.connect(self.clear)
        self.edit_button.setMenu(self.edit_menu)
        self.hlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))

    def select_all(self):
        for f_item in self.automation_viewer.automation_points:
            f_item.setSelected(True)

    def clear(self):
        self.automation_viewer.clear_current_item()

    def paste_cc_point(self):
        if pydaw_widgets.global_cc_clipboard is None:
            QtGui.QMessageBox.warning(self.widget, _("Error"),
                                      _("Nothing copied to the clipboard.\n"
                                      "Right-click->'Copy Automation' on any knob on any plugin."))
            return
        self.add_cc_point(pydaw_widgets.global_cc_clipboard)

    def add_cc_point(self, a_value=None):
        if not this_item_editor.enabled:  #TODO:  Make this global...
            this_item_editor.show_not_enabled_warning()
            return

        def ok_handler():
            f_bar = f_bar_spinbox.value() - 1
            f_item = this_item_editor.items[f_bar]

            f_cc = pydaw_cc(f_pos_spinbox.value() - 1.0,
                            self.automation_viewer.plugin_index,
                            self.automation_viewer.cc_num,
                            f_value_spinbox.value())
            f_item.add_cc(f_cc)

            this_pydaw_project.save_item(this_item_editor.item_names[f_bar],
                                         this_item_editor.items[f_bar])
            global_open_items()
            this_pydaw_project.commit(_("Add automation point"))

        def cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Add automation point"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_layout.addWidget(QtGui.QLabel(_("Position (bars)")), 2, 0)
        f_bar_spinbox = QtGui.QSpinBox()
        f_bar_spinbox.setRange(1, len(global_open_items_uids))
        f_layout.addWidget(f_bar_spinbox, 2, 1)

        f_layout.addWidget(QtGui.QLabel(_("Position (beats)")), 5, 0)
        f_pos_spinbox = QtGui.QDoubleSpinBox()
        f_pos_spinbox.setRange(1.0, 4.99)
        f_pos_spinbox.setDecimals(2)
        f_pos_spinbox.setSingleStep(0.25)
        f_layout.addWidget(f_pos_spinbox, 5, 1)

        f_layout.addWidget(QtGui.QLabel(_("Value")), 10, 0)
        f_value_spinbox = QtGui.QDoubleSpinBox()
        f_value_spinbox.setRange(0.0, 127.0)
        f_value_spinbox.setDecimals(4)
        if a_value is not None:
            f_value_spinbox.setValue(a_value)
        f_layout.addWidget(f_value_spinbox, 10, 1)

        f_ok = QtGui.QPushButton(_("Add"))
        f_ok.pressed.connect(ok_handler)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_ok_cancel_layout.addWidget(f_ok)

        f_layout.addLayout(f_ok_cancel_layout, 40, 1)
        f_cancel = QtGui.QPushButton(_("Close"))
        f_cancel.pressed.connect(cancel_handler)
        f_ok_cancel_layout.addWidget(f_cancel)
        f_window.exec_()


    def add_pb_point(self):
        if not this_item_editor.enabled:  #TODO:  Make this global...
            this_item_editor.show_not_enabled_warning()
            return

        def ok_handler():
            f_bar = f_bar_spinbox.value() - 1
            f_item = this_item_editor.items[f_bar]

            f_value = pydaw_clip_value(f_epb_spinbox.value() / f_ipb_spinbox.value(),
                                       -1.0, 1.0, a_round=True)
            f_pb = pydaw_pitchbend(f_pos_spinbox.value() - 1.0, f_value)
            f_item.add_pb(f_pb)

            global global_last_ipb_value
            global_last_ipb_value = f_ipb_spinbox.value()

            this_pydaw_project.save_item(this_item_editor.item_names[f_bar],
                                         this_item_editor.items[f_bar])
            global_open_items()
            this_pydaw_project.commit(_("Add pitchbend automation point"))

        def cancel_handler():
            f_window.close()

        def ipb_changed(a_self=None, a_event=None):
            f_epb_spinbox.setRange(f_ipb_spinbox.value() * -1, f_ipb_spinbox.value())

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Add automation point"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_layout.addWidget(QtGui.QLabel(_("Position (bars)")), 2, 0)
        f_bar_spinbox = QtGui.QSpinBox()
        f_bar_spinbox.setRange(1, len(global_open_items_uids))
        f_layout.addWidget(f_bar_spinbox, 2, 1)

        f_layout.addWidget(QtGui.QLabel(_("Position (beats)")), 5, 0)
        f_pos_spinbox = QtGui.QDoubleSpinBox()
        f_pos_spinbox.setRange(1.0, 4.99)
        f_pos_spinbox.setDecimals(2)
        f_pos_spinbox.setSingleStep(0.25)
        f_layout.addWidget(f_pos_spinbox, 5, 1)

        f_layout.addWidget(QtGui.QLabel(_("Instrument Pitchbend")), 10, 0)
        f_ipb_spinbox = QtGui.QSpinBox()
        f_ipb_spinbox.setToolTip(
            _("Set this to the same setting that your instrument plugin uses"))
        f_ipb_spinbox.setRange(2, 36)
        f_ipb_spinbox.setValue(global_last_ipb_value)
        f_layout.addWidget(f_ipb_spinbox, 10, 1)
        f_ipb_spinbox.valueChanged.connect(ipb_changed)

        f_layout.addWidget(QtGui.QLabel(_("Effective Pitchbend")), 20, 0)
        f_epb_spinbox = QtGui.QSpinBox()
        f_epb_spinbox.setToolTip("")
        f_epb_spinbox.setRange(-18, 18)
        f_layout.addWidget(f_epb_spinbox, 20, 1)

        f_layout.addWidget(QtGui.QLabel(_("Pitchbend values are in semitones.\n\n"
            "Use this dialog to add points with precision,\n"
            "or double-click on the editor to add points.")), 30, 1)

        f_ok = QtGui.QPushButton(_("Add"))
        f_ok.pressed.connect(ok_handler)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_ok_cancel_layout.addWidget(f_ok)

        f_layout.addLayout(f_ok_cancel_layout, 40, 1)
        f_cancel = QtGui.QPushButton(_("Close"))
        f_cancel.pressed.connect(cancel_handler)
        f_ok_cancel_layout.addWidget(f_cancel)
        f_window.exec_()

global_open_items_uids = []

def global_update_items_label():
    """ Refresh the item tab labels, ie:  when rnemaing items """
    global global_open_items_uids
    this_item_editor.item_names = []
    f_items_dict = this_pydaw_project.get_items_dict()
    for f_item_uid in global_open_items_uids:
        this_item_editor.item_names.append(f_items_dict.get_name_by_uid(f_item_uid))
    f_label_text = ", ".join(this_item_editor.item_names)
    if len(f_label_text) >= 150:
        f_label_text = f_label_text[:150] + "..."
    this_item_editor.item_list_label.setText(f_label_text)

def global_check_midi_items():
    """ Return True if OK, otherwise clear the the item editors and return False """
    f_items_dict = this_pydaw_project.get_items_dict()
    f_invalid = False
    for f_uid in global_open_items_uids:
        if not f_items_dict.uid_exists(f_uid):
            f_invalid = True
            break
    if f_invalid:
        this_item_editor.clear_new()
        this_item_editor.item_list_label.setText("")
        return False
    else:
        return True

def global_open_items(a_items=None):
    """ a_items is a list of str, which are the names of the items.
        Leave blank to open the existing list
    """
    this_item_editor.enabled = True
    global global_open_items_uids

    if a_items is not None:
        this_piano_roll_editor.selected_note_strings = []
        f_index = this_item_editor.zoom_combobox.currentIndex()
        if f_index == 1:
            this_item_editor.zoom_combobox.setCurrentIndex(0)
        global global_item_editing_count
        global_item_editing_count = len(a_items)
        pydaw_set_piano_roll_quantize(this_piano_roll_editor_widget.snap_combobox.currentIndex())
        this_item_editor.item_names = a_items
        f_label_text = ", ".join(a_items)
        if len(f_label_text) >= 150:
            f_label_text = f_label_text[:150] + "..."
        this_item_editor.item_list_label.setText(f_label_text)
        this_item_editor.item_index_enabled = False
        this_item_editor.item_name_combobox.clear()
        this_item_editor.item_name_combobox.clearEditText()
        this_item_editor.item_name_combobox.addItems(a_items)
        this_item_editor.item_name_combobox.setCurrentIndex(0)
        this_item_editor.item_index_enabled = True
        this_piano_roll_editor.horizontalScrollBar().setSliderPosition(0)
        this_item_editor.set_zoom(a_is_refresh=True)
        f_items_dict = this_pydaw_project.get_items_dict()
        global_open_items_uids = []
        for f_item_name in a_items:
            global_open_items_uids.append(f_items_dict.get_uid_by_name(f_item_name))

    for i in range(3):
        this_cc_automation_viewers[i].clear_drawn_items()
    this_pb_automation_viewer.clear_drawn_items()

    this_item_editor.items = []
    f_cc_dict = {}

    for f_item_uid in global_open_items_uids:
        f_item = this_pydaw_project.get_item_by_uid(f_item_uid)
        this_item_editor.items.append(f_item)
        for cc in f_item.ccs:
            f_key = "{}|{}".format(cc.plugin_index, cc.cc_num)
            if not f_key in f_cc_dict:
                f_cc_dict[f_key] = []
            f_cc_dict[f_key] = cc

    this_piano_roll_editor.draw_item()
    for i in range(3):
        this_item_editor.cc_auto_viewers[i].update_ccs_in_use(list(f_cc_dict.keys()))
    f_i = 0
    if a_items is not None:
        for f_cc_num in list(f_cc_dict.keys()):
            this_item_editor.cc_auto_viewers[f_i].set_cc_num(f_cc_num)
            f_i += 1
            if f_i >= len(this_item_editor.cc_auto_viewers):
                break
    for i in range(3):
        this_cc_automation_viewers[i].draw_item()
    this_pb_automation_viewer.draw_item()
    this_item_editor.open_item_list()

    if a_items is not None and f_index == 1:
        this_item_editor.zoom_combobox.setCurrentIndex(1)

def global_save_and_reload_items():
    assert(len(this_item_editor.item_names) == len(this_item_editor.items))
    for f_i in range(len(this_item_editor.item_names)):
        this_pydaw_project.save_item(
            this_item_editor.item_names[f_i], this_item_editor.items[f_i])
    global_open_items()
    this_pydaw_project.commit(_("Edit item(s)"))

global_item_zoom_index = 0

class item_list_editor:
    def clear_notes(self, a_is_list=True):
        if self.enabled:
            if a_is_list:
                self.item.notes = []
                this_pydaw_project.save_item(self.item_name, self.item)
            else:
                for f_i in range(len(self.items)):
                    self.items[f_i].notes = []
                    this_pydaw_project.save_item(self.item_names[f_i], self.items[f_i])
            this_pydaw_project.commit(_("Clear notes"))
            global_open_items()
    def clear_ccs(self, a_is_list=True):
        if self.enabled:
            if a_is_list:
                self.item.ccs = []
                this_pydaw_project.save_item(self.item_name, self.item)
            else:
                for f_i in range(len(self.items)):
                    self.items[f_i].ccs = []
                    this_pydaw_project.save_item(self.item_names[f_i], self.items[f_i])
            this_pydaw_project.commit(_("Clear CCs"))
            global_open_items()
    def clear_pb(self, a_is_list=True):
        if self.enabled:
            if a_is_list:
                self.item.pitchbends = []
                this_pydaw_project.save_item(self.item_name, self.item)
            else:
                for f_i in range(len(self.items)):
                    self.items[f_i].pitchbends = []
                    this_pydaw_project.save_item(self.item_names[f_i], self.items[f_i])
            this_pydaw_project.commit(_("Clear pitchbends"))
            global_open_items()

    def clear_new(self):
        self.enabled = False
        self.ccs_table_widget.clearContents()
        self.notes_table_widget.clearContents()
        self.pitchbend_table_widget.clearContents()
        this_piano_roll_editor.clear_drawn_items()
        self.item = None
        self.items = []
        self.notes_clipboard = []
        self.ccs_clipboard = []
        self.pbs_clipboard = []

    def get_notes_table_selected_rows(self):
        f_result = []
        for i in range(0, self.notes_table_widget.rowCount()):
            f_item = self.notes_table_widget.item(i, 0)
            if not f_item is None and f_item.isSelected():
                f_result.append(pydaw_note(self.notes_table_widget.item(i, 0).text(),
                                           self.notes_table_widget.item(i, 1).text(),
                                           self.notes_table_widget.item(i, 3).text(),
                                           self.notes_table_widget.item(i, 4).text()))
        return f_result

    def get_ccs_table_selected_rows(self):
        f_result = []
        for i in range(0, self.ccs_table_widget.rowCount()):
            f_item = self.ccs_table_widget.item(i, 0)
            if not f_item is None and f_item.isSelected():
                f_result.append(pydaw_cc(self.ccs_table_widget.item(i, 0).text(),
                                         self.ccs_table_widget.item(i, 1).text(),
                                         self.ccs_table_widget.item(i, 2).text()))
        return f_result

    def get_pbs_table_selected_rows(self):
        f_result = []
        for i in range(0, self.pitchbend_table_widget.rowCount()):
            f_item = self.pitchbend_table_widget.item(i, 0)
            if not f_item is None and f_item.isSelected():
                f_result.append(pydaw_pitchbend(self.pitchbend_table_widget.item(i, 0).text(),
                                                self.pitchbend_table_widget.item(i, 1).text()))
        return f_result

    def quantize_dialog(self, a_is_list=True, a_selected_only=False):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        f_multiselect = False
        if a_is_list:
            if self.multiselect_radiobutton.isChecked():
                f_ms_rows = self.get_notes_table_selected_rows()
                if len(f_ms_rows) == 0:
                    QtGui.QMessageBox.warning(self.notes_table_widget, _("Error"),
                    _("You have editing in multiselect mode, but you have not selected "
                    "anything.  All items will be processed"))
                else:
                    f_multiselect = True

        def quantize_ok_handler():
            f_quantize_text = f_quantize_combobox.currentText()
            self.events_follow_default = f_events_follow_notes.isChecked()
            for f_i in range(len(self.items)):
                if f_multiselect:
                    self.items[f_i].quantize(f_quantize_text,
                                             f_events_follow_notes.isChecked(), f_ms_rows)
                else:
                    self.items[f_i].quantize(f_quantize_text,
                                             f_events_follow_notes.isChecked(),
                                             a_selected_only=a_selected_only)
                this_pydaw_project.save_item(self.item_names[f_i], self.items[f_i])
            global_open_items()
            this_pydaw_project.commit(_("Quantize item(s)"))
            f_window.close()

        def quantize_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Quantize"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_layout.addWidget(QtGui.QLabel(_("Quantize")), 0, 0)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(bar_fracs)
        f_layout.addWidget(f_quantize_combobox, 0, 1)
        f_events_follow_notes = QtGui.QCheckBox(_("CCs and pitchbend follow notes?"))
        f_events_follow_notes.setChecked(self.events_follow_default)
        f_layout.addWidget(f_events_follow_notes, 1, 1)
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(quantize_ok_handler)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_ok_cancel_layout.addWidget(f_ok)
        if a_selected_only:
            f_layout.addWidget(QtGui.QLabel(_("Only the selected notes will be quantized.")), 2, 1)
        f_layout.addLayout(f_ok_cancel_layout, 3, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(quantize_cancel_handler)
        f_ok_cancel_layout.addWidget(f_cancel)
        f_window.exec_()

    def velocity_dialog(self, a_is_list=True, a_selected_only=False):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        f_multiselect = False
        if a_is_list:
            if self.multiselect_radiobutton.isChecked():
                f_ms_rows = self.get_notes_table_selected_rows()
                if len(f_ms_rows) == 0:
                    QtGui.QMessageBox.warning(self.notes_table_widget, _("Error"),
                    _("You have editing in multiselect mode, but you have not selected anything.  "
                    "All items will be processed"))
                else:
                    f_multiselect = True
            f_start_beat_val = (4.0 * global_item_editing_count) - 0.01
            f_end_beat_val = 0.0
            if f_multiselect:
                for f_note in f_ms_rows:
                    if f_note.start < f_start_beat_val:
                        f_start_beat_val = f_note.start
                    elif f_note.start > f_end_beat_val:
                        f_end_beat_val = f_note.start

        def ok_handler():
            if f_draw_line.isChecked() and not f_add_values.isChecked() and \
            f_end_amount.value() < 1:
                QtGui.QMessageBox.warning(f_window, _("Error"),
                _("Cannot have end value less than 1 if not using 'Add Values'"))
                return

            if a_is_list:
                if f_multiselect:
                    self.item.velocity_mod(f_amount.value(), f_start_beat.value(),
                                           f_end_beat.value(), f_draw_line.isChecked(),
                                           f_end_amount.value(),
                                           f_add_values.isChecked(), f_ms_rows)
                else:
                    self.item.velocity_mod(f_amount.value(), f_start_beat.value(),
                                           f_end_beat.value(),
                                           f_draw_line.isChecked(), f_end_amount.value(),
                                           f_add_values.isChecked())
                this_pydaw_project.save_item(self.item_name, self.item)
            else:
                pydaw_velocity_mod(self.items, f_amount.value(), f_draw_line.isChecked(),
                                   f_end_amount.value(),
                                   f_add_values.isChecked(), a_selected_only=a_selected_only)
                for f_i in range(global_item_editing_count):
                    this_pydaw_project.save_item(self.item_names[f_i], self.items[f_i])
            global_open_items()
            this_pydaw_project.commit(_("Velocity mod item(s)"))
            f_window.close()

        def cancel_handler():
            f_window.close()

        def end_value_changed(a_val=None):
            f_draw_line.setChecked(True)

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Velocity Mod"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_layout.addWidget(QtGui.QLabel(_("Amount")), 0, 0)
        f_amount = QtGui.QSpinBox()
        f_amount.setRange(-127, 127)
        f_amount.setValue(100)
        f_layout.addWidget(f_amount, 0, 1)
        f_draw_line = QtGui.QCheckBox(_("Draw line?"))
        f_layout.addWidget(f_draw_line, 1, 1)

        f_layout.addWidget(QtGui.QLabel(_("End Amount")), 2, 0)
        f_end_amount = QtGui.QSpinBox()
        f_end_amount.setRange(-127, 127)
        f_end_amount.valueChanged.connect(end_value_changed)
        f_layout.addWidget(f_end_amount, 2, 1)

        if a_is_list:
            f_layout.addWidget(QtGui.QLabel(_("Start Beat")), 3, 0)
            f_start_beat = QtGui.QDoubleSpinBox()
            f_start_beat.setRange(0.0, 3.99)
            f_start_beat.setValue(f_start_beat_val)
            f_layout.addWidget(f_start_beat, 3, 1)

            f_layout.addWidget(QtGui.QLabel("End Beat"), 4, 0)
            f_end_beat = QtGui.QDoubleSpinBox()
            f_end_beat.setRange(0.01, 3.99)
            f_end_beat.setValue(f_end_beat_val)
            f_layout.addWidget(f_end_beat, 4, 1)

        f_add_values = QtGui.QCheckBox(_("Add Values?"))
        f_add_values.setToolTip(_("Check this to add Amount to the existing value, or leave\n"
                                "unchecked to set the value to Amount."))
        f_layout.addWidget(f_add_values, 5, 1)

        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_ok_cancel_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_cancel_layout, 10, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(cancel_handler)
        f_ok_cancel_layout.addWidget(f_cancel)
        f_window.exec_()

    def transpose_dialog(self, a_is_list=True, a_selected_only=False):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        f_multiselect = False
        if a_is_list:
            if self.multiselect_radiobutton.isChecked():
                f_ms_rows = self.get_notes_table_selected_rows()
                if len(f_ms_rows) == 0:
                    QtGui.QMessageBox.warning(self.notes_table_widget, _("Error"),
                                              _("You are editing in multiselect mode, "
                                              "but you have not selected "
                                              "anything.  All items will be processed"))
                else:
                    f_multiselect = True

        def transpose_ok_handler():
            if a_is_list:
                if f_multiselect:
                    self.item.transpose(f_semitone.value(), f_octave.value(), f_ms_rows)
                else:
                    self.item.transpose(f_semitone.value(), f_octave.value())
                this_pydaw_project.save_item(self.item_name, self.item)
            else:
                for f_i in range(len(self.items)):
                    self.items[f_i].transpose(f_semitone.value(), f_octave.value(),
                                              a_selected_only=a_selected_only,
                                              a_duplicate=f_duplicate_notes.isChecked())
                    this_pydaw_project.save_item(self.item_names[f_i], self.items[f_i])
            global_open_items()
            this_pydaw_project.commit(_("Transpose item(s)"))
            f_window.close()

        def transpose_cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Transpose"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_semitone = QtGui.QSpinBox()
        f_semitone.setRange(-12, 12)
        f_layout.addWidget(QtGui.QLabel(_("Semitones")), 0, 0)
        f_layout.addWidget(f_semitone, 0, 1)
        f_octave = QtGui.QSpinBox()
        f_octave.setRange(-5, 5)
        f_layout.addWidget(QtGui.QLabel(_("Octaves")), 1, 0)
        f_layout.addWidget(f_octave, 1, 1)
        f_duplicate_notes = QtGui.QCheckBox(_("Duplicate notes?"))
        f_duplicate_notes.setToolTip(_("Checking this box causes the transposed notes "
                                     "to be added rather than moving the existing notes."))
        f_layout.addWidget(f_duplicate_notes, 2, 1)
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(transpose_ok_handler)
        f_layout.addWidget(f_ok, 6, 0)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(transpose_cancel_handler)
        f_layout.addWidget(f_cancel, 6, 1)
        f_window.exec_()

    def show_not_enabled_warning(self):
        QtGui.QMessageBox.warning(this_main_window, _("Error"),
                                  _("You must open an item first by double-clicking on one in "
                                  "the region editor on the 'Song/Region' tab."))

    def set_zoom(self, a_value=None, a_is_refresh=False):
        if not self.enabled:
            return
        f_index = self.zoom_combobox.currentIndex()
        f_item_count = len(self.items)
        if not a_is_refresh and f_item_count < 2:
            return

        global global_item_zoom_index
        global_item_zoom_index = f_index

        for f_viewer in this_cc_automation_viewers:
            f_viewer.scale_to_width()
        this_piano_roll_editor.scale_to_width()
        this_pb_automation_viewer.scale_to_width()

    def tab_changed(self, a_val=None):
        this_piano_roll_editor.click_enabled = True

    def __init__(self):
        self.enabled = False
        self.items = []
        self.item_names = []
        self.events_follow_default = True

        self.widget = QtGui.QWidget()
        self.master_vlayout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.master_vlayout)
        self.master_hlayout = QtGui.QHBoxLayout()
        self.master_vlayout.addLayout(self.master_hlayout)
        self.item_list_label = QtGui.QLabel("")
        self.master_hlayout.addWidget(self.item_list_label,  QtCore.Qt.AlignLeft)
        self.zoom_combobox = QtGui.QComboBox()
        self.zoom_combobox.setMaximumWidth(120)
        self.zoom_combobox.addItems([_("Large"), _("Small")])
        self.zoom_combobox.currentIndexChanged.connect(self.set_zoom)
        self.master_hlayout.addWidget(self.zoom_combobox, QtCore.Qt.AlignRight)

        self.tab_widget = QtGui.QTabWidget()
        self.tab_widget.currentChanged.connect(self.tab_changed)
        self.piano_roll_tab = QtGui.QGroupBox()
        self.tab_widget.addTab(self.piano_roll_tab, _("Piano Roll"))
        self.notes_tab = QtGui.QGroupBox()
        self.group_box = QtGui.QGroupBox()
        self.tab_widget.addTab(self.group_box, _("CCs"))
        self.pitchbend_tab = QtGui.QGroupBox()
        self.tab_widget.addTab(self.pitchbend_tab, _("Pitchbend"))

        self.main_vlayout = QtGui.QVBoxLayout()
        self.main_hlayout = QtGui.QHBoxLayout()
        self.group_box.setLayout(self.main_vlayout)
        self.editing_hboxlayout = QtGui.QHBoxLayout()
        self.master_vlayout.addWidget(self.tab_widget)
        self.main_vlayout.addLayout(self.main_hlayout)

        self.notes_groupbox = QtGui.QGroupBox(_("Notes"))
        self.notes_vlayout = QtGui.QVBoxLayout(self.notes_groupbox)


        self.editing_hboxlayout.addWidget(QtGui.QLabel(_("Editing Item:")))
        self.item_name_combobox = QtGui.QComboBox()
        self.item_name_combobox.setMinimumWidth(150)
        self.item_name_combobox.setEditable(False)
        self.item_name_combobox.currentIndexChanged.connect(self.item_index_changed)
        self.item_index_enabled = True
        self.editing_hboxlayout.addWidget(self.item_name_combobox)

        self.edit_mode_groupbox = QtGui.QGroupBox()
        self.edit_mode_hlayout0 = QtGui.QHBoxLayout(self.edit_mode_groupbox)
        self.edit_mode_hlayout0.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                          QtGui.QSizePolicy.Minimum))
        self.edit_mode_hlayout0.addWidget(QtGui.QLabel(_("Mode")))
        self.add_radiobutton = QtGui.QRadioButton(_("Add/Edit"))
        self.edit_mode_hlayout0.addWidget(self.add_radiobutton)
        self.multiselect_radiobutton = QtGui.QRadioButton(_("Multiselect"))
        self.edit_mode_hlayout0.addWidget(self.multiselect_radiobutton)
        self.delete_radiobutton = QtGui.QRadioButton(_("Delete"))
        self.edit_mode_hlayout0.addWidget(self.delete_radiobutton)
        self.add_radiobutton.setChecked(True)
        self.editing_hboxlayout.addWidget(self.edit_mode_groupbox, alignment=QtCore.Qt.AlignLeft)
        self.editing_hboxlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                          QtGui.QSizePolicy.Minimum))

        self.notes_gridlayout = QtGui.QGridLayout()

        f_button_width = 82
        self.notes_quantize_button = QtGui.QPushButton(_("Quantize"))
        self.notes_quantize_button.setMinimumWidth(f_button_width)
        self.notes_quantize_button.pressed.connect(self.quantize_dialog)
        self.notes_gridlayout.addWidget(self.notes_quantize_button, 0, 0)
        self.notes_transpose_button = QtGui.QPushButton(_("Transpose"))
        self.notes_transpose_button.setMinimumWidth(f_button_width)
        self.notes_transpose_button.pressed.connect(self.transpose_dialog)
        self.notes_gridlayout.addWidget(self.notes_transpose_button, 0, 1)

        self.notes_velocity_button = QtGui.QPushButton(_("Velocity"))
        self.notes_velocity_button.setMinimumWidth(f_button_width)
        self.notes_velocity_button.pressed.connect(self.velocity_dialog)
        self.notes_gridlayout.addWidget(self.notes_velocity_button, 0, 2)

        self.notes_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                        QtGui.QSizePolicy.Minimum), 0, 3)

        self.notes_clear_button = QtGui.QPushButton(_("Clear"))
        self.notes_clear_button.setMinimumWidth(f_button_width)
        self.notes_clear_button.pressed.connect(self.clear_notes)
        self.notes_gridlayout.addWidget(self.notes_clear_button, 0, 4)
        self.notes_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                        QtGui.QSizePolicy.Minimum), 0, 6, 1, 1)
        self.notes_vlayout.addLayout(self.notes_gridlayout)
        self.notes_table_widget = QtGui.QTableWidget()
        self.notes_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.notes_table_widget.setColumnCount(5)
        self.notes_table_widget.setRowCount(256)
        self.notes_table_widget.cellClicked.connect(self.notes_click_handler)
        self.notes_table_widget.setSortingEnabled(True)
        self.notes_table_widget.sortItems(0)
        self.notes_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.notes_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.notes_table_widget.keyPressEvent = self.notes_keyPressEvent
        self.notes_vlayout.addWidget(self.notes_table_widget)
        self.notes_table_widget.resizeColumnsToContents()

        self.notes_hlayout = QtGui.QHBoxLayout()
        self.list_tab_vlayout = QtGui.QVBoxLayout()
        self.notes_tab.setLayout(self.list_tab_vlayout)
        self.list_tab_vlayout.addLayout(self.editing_hboxlayout)
        self.list_tab_vlayout.addLayout(self.notes_hlayout)
        self.notes_hlayout.addWidget(self.notes_groupbox)

        self.piano_roll_hlayout = QtGui.QHBoxLayout()
        self.piano_roll_tab.setLayout(self.piano_roll_hlayout)
        self.piano_roll_hlayout.addWidget(this_piano_roll_editor_widget.widget)

        self.ccs_groupbox = QtGui.QGroupBox(_("CCs"))
        self.ccs_vlayout = QtGui.QVBoxLayout(self.ccs_groupbox)
        self.ccs_gridlayout = QtGui.QGridLayout()
        self.ccs_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                      QtGui.QSizePolicy.Minimum), 0, 0, 1, 1)
        self.ccs_clear_button = QtGui.QPushButton(_("Clear"))
        self.ccs_clear_button.setMinimumWidth(f_button_width)
        self.ccs_clear_button.pressed.connect(self.clear_ccs)
        self.ccs_gridlayout.addWidget(self.ccs_clear_button, 0, 1)
        self.ccs_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                      QtGui.QSizePolicy.Minimum), 0, 2, 1, 1)
        self.ccs_vlayout.addLayout(self.ccs_gridlayout)
        self.ccs_table_widget = QtGui.QTableWidget()
        self.ccs_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.ccs_table_widget.setColumnCount(4)
        self.ccs_table_widget.setRowCount(256)
        self.ccs_table_widget.setSortingEnabled(True)
        self.ccs_table_widget.sortItems(0)
        self.ccs_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.ccs_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.ccs_table_widget.keyPressEvent = self.ccs_keyPressEvent
        self.ccs_table_widget.resizeColumnsToContents()
        self.ccs_vlayout.addWidget(self.ccs_table_widget)
        self.notes_hlayout.addWidget(self.ccs_groupbox)

        self.cc_auto_viewer_scrollarea = QtGui.QScrollArea()
        self.cc_auto_viewer_scrollarea.setMinimumWidth(900)
        self.cc_auto_viewer_scrollarea.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.cc_auto_viewer_scrollarea.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.cc_auto_viewer_scrollarea_widget = QtGui.QWidget()
        self.cc_auto_viewer_scrollarea_widget.setObjectName("plugin_ui")
        self.cc_auto_viewer_scrollarea_widget.setMinimumSize(880, 1330)
        self.cc_auto_viewer_scrollarea.setWidgetResizable(True)
        self.cc_auto_viewer_scrollarea.setWidget(self.cc_auto_viewer_scrollarea_widget)
        self.cc_auto_viewer_vlayout = QtGui.QVBoxLayout(self.cc_auto_viewer_scrollarea_widget)

        self.cc_auto_viewers = []
        for i in range(3):
            self.cc_auto_viewers.append(automation_viewer_widget(this_cc_automation_viewers[i]))
            self.cc_auto_viewer_vlayout.addWidget(self.cc_auto_viewers[i].widget)
        self.main_hlayout.addWidget(self.cc_auto_viewer_scrollarea)

        self.pb_hlayout = QtGui.QHBoxLayout()
        self.pitchbend_tab.setLayout(self.pb_hlayout)
        self.pb_groupbox = QtGui.QGroupBox(_("Pitchbend"))
        self.pb_groupbox.setFixedWidth(240)
        self.pb_vlayout = QtGui.QVBoxLayout(self.pb_groupbox)
        self.pb_gridlayout = QtGui.QGridLayout()
        self.pb_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                     QtGui.QSizePolicy.Minimum), 0, 0, 1, 1)
        self.pb_clear_button = QtGui.QPushButton(_("Clear"))
        self.pb_clear_button.setMinimumWidth(f_button_width)
        self.pb_clear_button.pressed.connect(self.clear_pb)
        self.pb_gridlayout.addWidget(self.pb_clear_button, 0, 1)
        self.pb_gridlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding,
                                                     QtGui.QSizePolicy.Minimum), 0, 2, 1, 1)
        self.pb_vlayout.addLayout(self.pb_gridlayout)
        self.pitchbend_table_widget = QtGui.QTableWidget()
        self.pitchbend_table_widget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.pitchbend_table_widget.setColumnCount(2)
        self.pitchbend_table_widget.setRowCount(256)
        self.pitchbend_table_widget.cellClicked.connect(self.pitchbend_click_handler)
        self.pitchbend_table_widget.setSortingEnabled(True)
        self.pitchbend_table_widget.sortItems(0)
        self.pitchbend_table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.pitchbend_table_widget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.pitchbend_table_widget.keyPressEvent = self.pbs_keyPressEvent
        self.pitchbend_table_widget.resizeColumnsToContents()
        self.pb_vlayout.addWidget(self.pitchbend_table_widget)
        self.notes_hlayout.addWidget(self.pb_groupbox)
        self.pb_auto_vlayout = QtGui.QVBoxLayout()
        self.pb_hlayout.addLayout(self.pb_auto_vlayout)
        self.pb_viewer_widget = automation_viewer_widget(this_pb_automation_viewer, False)
        self.pb_auto_vlayout.addWidget(self.pb_viewer_widget.widget)
        self.pb_auto_vlayout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Minimum,
                                                       QtGui.QSizePolicy.Expanding))

        self.tab_widget.addTab(self.notes_tab, _("List Editors"))

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


    def item_index_changed(self, a_index=None):
        if self.item_index_enabled:
            self.open_item_list()

    def set_headers(self): #Because clearing the table clears the headers
        self.notes_table_widget.setHorizontalHeaderLabels([_('Start'), _('Length'), _('Note'),
                                                           _('Note#'), _('Velocity')])
        self.ccs_table_widget.setHorizontalHeaderLabels([_('Start'), _('Plugin'), _('Control'),
                                                         _('Value')])
        self.pitchbend_table_widget.setHorizontalHeaderLabels([_('Start'), _('Value')])

    def set_row_counts(self):
        self.notes_table_widget.setRowCount(256)
        self.ccs_table_widget.setRowCount(256)
        self.pitchbend_table_widget.setRowCount(256)

    def add_cc(self, a_cc):
        f_index, f_start = pydaw_beats_to_index(a_cc.start)
        a_cc.start = f_start
        self.items[f_index].add_cc(a_cc)
        return f_index

    def add_note(self, a_note):
        f_index, f_start = pydaw_beats_to_index(a_note.start)
        a_note.start = f_start
        self.items[f_index].add_note(a_note, False)
        return f_index

    def add_pb(self, a_pb):
        f_index, f_start = pydaw_beats_to_index(a_pb.start)
        a_pb.start = f_start
        self.items[f_index].add_pb(a_pb)
        return f_index

    def open_item_list(self):
        self.notes_table_widget.clear()
        self.ccs_table_widget.clear()
        self.pitchbend_table_widget.clear()
        self.set_headers()
        self.item_name = str(self.item_name_combobox.currentText())
        self.item = this_pydaw_project.get_item_by_name(self.item_name)
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
            f_plugin_name = global_plugin_names[global_plugin_indexes[int(cc.plugin_index)]]
            f_port_name = global_controller_port_num_dict[f_plugin_name][int(cc.cc_num)].name
            self.ccs_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(cc.start)))
            self.ccs_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(f_plugin_name))
            self.ccs_table_widget.setItem(f_i, 2, QtGui.QTableWidgetItem(f_port_name))
            self.ccs_table_widget.setItem(f_i, 3, QtGui.QTableWidgetItem(str(cc.cc_val)))
            f_i = f_i + 1
        self.ccs_table_widget.setSortingEnabled(True)
        self.pitchbend_table_widget.setSortingEnabled(False)
        f_i = 0
        for pb in self.item.pitchbends:
            self.pitchbend_table_widget.setItem(f_i, 0, QtGui.QTableWidgetItem(str(pb.start)))
            self.pitchbend_table_widget.setItem(f_i, 1, QtGui.QTableWidgetItem(str(pb.pb_val)))
            f_i = f_i + 1
        self.pitchbend_table_widget.setSortingEnabled(True)
        self.notes_table_widget.resizeColumnsToContents()
        self.ccs_table_widget.resizeColumnsToContents()
        self.pitchbend_table_widget.resizeColumnsToContents()

    def notes_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_notes = self.get_notes_table_selected_rows()
                for f_note in f_notes:
                    self.item.remove_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Delete notes from item '{}'").format(self.item_name))
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.notes_clipboard = self.get_notes_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_note in self.notes_clipboard:
                self.item.add_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Paste notes into item '{}'").format(self.item_name))
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.notes_clipboard = self.get_notes_table_selected_rows()
            for f_note in self.notes_clipboard:
                self.item.remove_note(f_note)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Cut notes from item '{}'").format(self.item_name))
        else:
            QtGui.QTableWidget.keyPressEvent(self.notes_table_widget, event)

    def ccs_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_ccs = self.get_ccs_table_selected_rows()
                for f_cc in f_ccs:
                    self.item.remove_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Delete CCs from item '{}'").format(self.item_name))
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_cc in self.ccs_clipboard:
                self.item.add_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Paste CCs into item '{}'").format(self.item_name))
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.ccs_clipboard = self.get_ccs_table_selected_rows()
            for f_cc in self.ccs_clipboard:
                self.item.remove_cc(f_cc)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Cut CCs from item '{}'").format(self.item_name))
        else:
            QtGui.QTableWidget.keyPressEvent(self.ccs_table_widget, event)

    def pbs_keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            if self.multiselect_radiobutton.isChecked():
                f_pbs = self.get_pbs_table_selected_rows()
                for f_pb in f_pbs:
                    self.item.remove_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Delete pitchbends from item '{}'").format(self.item_name))
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ControlModifier:
            self.pbs_clipboard = self.get_pbs_table_selected_rows()
        elif event.key() == QtCore.Qt.Key_V and event.modifiers() == QtCore.Qt.ControlModifier:
            for f_pb in self.pbs_clipboard:
                self.item.add_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Paste pitchbends into item '{}'").format(self.item_name))
        elif event.key() == QtCore.Qt.Key_X and event.modifiers() == QtCore.Qt.ControlModifier:
            self.pbs_clipboard = self.get_pbs_table_selected_rows()
            for f_pb in self.pbs_clipboard:
                self.item.remove_pb(f_pb)
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Cut pitchbends from item '{}'").format(self.item_name))
        else:
            QtGui.QTableWidget.keyPressEvent(self.pitchbend_table_widget, event)

    def notes_click_handler(self, x, y):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        if self.add_radiobutton.isChecked():
            self.notes_show_event_dialog(x, y)
        elif self.delete_radiobutton.isChecked():
            if self.notes_table_widget.item(x, 0) is None or \
            str(self.notes_table_widget.item(x, 0).text()) == "":
                return
            self.item.remove_note(pydaw_note(self.notes_table_widget.item(x, 0).text(),
                                             self.notes_table_widget.item(x, 1).text(),
                                             self.notes_table_widget.item(x, 3).text(),
                                             self.notes_table_widget.item(x, 4).text()))
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Delete note from item '{}'").format(self.item_name))

    def pitchbend_click_handler(self, x, y):
        if not self.enabled:
            self.show_not_enabled_warning()
            return
        if self.add_radiobutton.isChecked():
            self.pitchbend_show_event_dialog(x, y)
        elif self.delete_radiobutton.isChecked():
            if self.pitchbend_table_widget.item(x, 0) is None:
                return
            self.item.remove_pb(pydaw_pitchbend(self.pitchbend_table_widget.item(x, 0).text(),
                                                self.pitchbend_table_widget.item(x, 1).text()))
            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Delete pitchbend from item '{}'").format(self.item_name))

    def notes_show_event_dialog(self, x, y, a_note=None, a_index=None):
        if a_note is not None:
            self.is_existing_note = True
            f_note_item = a_note
            if a_index is None:
                f_note_index, f_note_item.start = pydaw_beats_to_index( f_note_item.start)
            else:
                f_note_index = a_index
            self.default_note_start = f_note_item.start
            self.default_note_length = f_note_item.length
            self.default_note_note = f_note_item.note_num % 12
            self.default_note_octave = (f_note_item.note_num / 12) - 2
            self.default_note_velocity = f_note_item.velocity
        else:
            f_cell = self.notes_table_widget.item(x, y)
            if f_cell is not None:
                f_note_item = pydaw_note(self.notes_table_widget.item(x, 0).text(),
                                         self.notes_table_widget.item(x, 1).text(),
                                         self.notes_table_widget.item(x, 3).text(),
                                         self.notes_table_widget.item(x, 4).text())
                self.is_existing_note = True
                self.default_note_start = float(self.notes_table_widget.item(x, 0).text())
                self.default_note_length = float(self.notes_table_widget.item(x, 1).text())
                self.default_note_note = int(self.notes_table_widget.item(x, 3).text()) % 12
                self.default_note_octave = \
                    (int(self.notes_table_widget.item(x, 3).text()) / 12) - 2
                self.default_note_velocity = int(self.notes_table_widget.item(x, 4).text())
            else:
                self.is_existing_note = False

        def note_ok_handler():
            f_note_value = (int(f_note.currentIndex()) + (int(f_octave.value()) + 2) * 12)
            f_start_rounded = time_quantize_round(f_start.value())
            f_length_rounded = time_quantize_round(f_length.value())
            f_new_note = pydaw_note(f_start_rounded, f_length_rounded,
                                    f_note_value, f_velocity.value())

            if self.is_existing_note and a_note is not None:
                self.items[f_note_index].remove_note(f_note_item)
                self.items[f_note_index].add_note(f_new_note)
                this_pydaw_project.save_item(self.item_names[f_note_index],
                                             self.items[f_note_index])
            elif self.is_existing_note and a_note is None:
                self.item.remove_note(f_note_item)
                self.item.add_note(f_new_note)
                this_pydaw_project.save_item(self.item_name, self.item)
            else:
                f_item_index = self.add_note(f_new_note)
                this_pydaw_project.save_item(self.item_names[f_item_index],
                                             self.items[f_item_index])

            self.default_note_start = f_new_note.start
            self.default_note_length = f_length_rounded
            self.default_note_note = int(f_note.currentIndex())
            self.default_note_octave = int(f_octave.value())
            self.default_note_velocity = int(f_velocity.value())
            self.default_quantize = int(f_quantize_combobox.currentIndex())

            global_open_items()
            this_pydaw_project.commit(_("Update notes for item(s)"))

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
                f_cancel_button.setText(_("Close"))
            else:
                f_cancel_button.setText("Cancel")

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Notes"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.currentIndexChanged.connect(quantize_changed)
        f_layout.addWidget(QtGui.QLabel(_("Quantize(beats)")), 0, 0)
        f_layout.addWidget(f_quantize_combobox, 0, 1)
        f_note_layout = QtGui.QHBoxLayout()
        f_note = QtGui.QComboBox()
        f_note.setMinimumWidth(66)
        f_note.addItems(int_to_note_array)
        f_note.setCurrentIndex(self.default_note_note)
        f_note_layout.addWidget(f_note)
        f_layout.addWidget(QtGui.QLabel(_("Note")), 1, 0)
        f_octave = QtGui.QSpinBox()
        f_octave.setRange(-2, 8)
        f_octave.setValue(self.default_note_octave)
        f_note_layout.addWidget(f_octave)
        f_layout.addLayout(f_note_layout, 1, 1)
        f_layout.addWidget(QtGui.QLabel(_("Start(beats)")), 2, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_start.setValue(self.default_note_start)
        f_layout.addWidget(f_start, 2, 1)
        f_layout.addWidget(QtGui.QLabel(_("Length(beats)")), 3, 0)
        f_length = QtGui.QDoubleSpinBox()
        f_length.setRange(0.01, 32.0)
        f_length.setValue(self.default_note_length)
        f_layout.addWidget(f_length, 3, 1)
        f_length.valueChanged.connect(length_changed)
        f_velocity = QtGui.QSpinBox()
        f_velocity.setRange(1, 127)
        f_velocity.setValue(self.default_note_velocity)
        f_layout.addWidget(QtGui.QLabel(_("Velocity")), 4, 0)
        f_layout.addWidget(f_velocity, 4, 1)
        if not self.is_existing_note:
            f_add_another = QtGui.QCheckBox(_("Add another?"))
            f_add_another.toggled.connect(add_another_clicked)
            f_layout.addWidget(f_add_another, 5, 1)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_layout.addWidget(f_ok_button, 6,0)
        f_ok_button.clicked.connect(note_ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_layout.addWidget(f_cancel_button, 6,1)
        f_cancel_button.clicked.connect(note_cancel_handler)
        f_quantize_combobox.setCurrentIndex(self.default_quantize)
        f_window.exec_()

    def pitchbend_show_event_dialog(self, x, y):
        f_cell = self.pitchbend_table_widget.item(x, y)
        f_old_pb = None
        if f_cell is not None:
            self.default_pb_start = float(self.pitchbend_table_widget.item(x, 0).text())
            self.default_pb_val = float(self.pitchbend_table_widget.item(x, 1).text())
            f_old_pb = pydaw_pitchbend(self.default_pb_start, self.default_pb_val)

        def pb_ok_handler():
            f_start_rounded = time_quantize_round(f_start.value())
            if f_draw_line_checkbox.isChecked():
                self.item.draw_pb_line(f_start.value(), f_pb.value(),
                                       f_end.value(), f_end_value.value())
            else:
                if f_old_pb is not None:
                    self.item.remove_pb(f_old_pb)
                if not self.item.add_pb(pydaw_pitchbend(f_start_rounded, f_pb.value())):
                    QtGui.QMessageBox.warning(f_window, _("Error"), _("Duplicate pitchbend event"))
                    return

            self.default_pb_start = f_start_rounded
            self.default_pb_val = f_pb.value()

            this_pydaw_project.save_item(self.item_name, self.item)
            global_open_items()
            this_pydaw_project.commit(_("Update pitchbends for item '{}'").format(self.item_name))
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
                f_cancel_button.setText(_("Close"))
            else:
                f_cancel_button.setText("Cancel")

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Pitchbend"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_quantize_combobox = QtGui.QComboBox()
        f_quantize_combobox.addItems(beat_fracs)
        f_quantize_combobox.currentIndexChanged.connect(quantize_changed)
        f_layout.addWidget(QtGui.QLabel(_("Quantize(beats)")), 0, 0)
        f_layout.addWidget(f_quantize_combobox, 0, 1)
        f_pb = QtGui.QDoubleSpinBox()
        f_pb.setSingleStep(0.01)
        f_pb.setRange(-1, 1)
        f_pb.setValue(self.default_pb_val)
        f_layout.addWidget(QtGui.QLabel(_("Value")), 2, 0)
        f_layout.addWidget(f_pb, 2, 1)
        f_layout.addWidget(QtGui.QLabel(_("Position(beats)")), 3, 0)
        f_start = QtGui.QDoubleSpinBox()
        f_start.setRange(0.0, 3.99)
        f_start.setValue(self.default_pb_start)
        f_layout.addWidget(f_start, 3, 1)
        f_draw_line_checkbox = QtGui.QCheckBox(_("Draw line"))
        f_layout.addWidget(f_draw_line_checkbox, 4, 1)
        f_layout.addWidget(QtGui.QLabel(_("End(beats)")), 5, 0)
        f_end = QtGui.QDoubleSpinBox()
        f_end.setRange(0, 3.99)
        f_layout.addWidget(f_end, 5, 1)
        f_layout.addWidget(QtGui.QLabel(_("End Value")), 6, 0)
        f_end_value = QtGui.QDoubleSpinBox()
        f_end_value.setRange(-1, 1)
        f_end_value.setSingleStep(0.01)
        f_layout.addWidget(f_end_value, 6, 1)
        f_add_another = QtGui.QCheckBox(_("Add another?"))
        f_add_another.toggled.connect(add_another_clicked)
        f_layout.addWidget(f_add_another, 7, 1)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_layout.addWidget(f_ok_button, 8,0)
        f_ok_button.clicked.connect(pb_ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Cancel"))
        f_layout.addWidget(f_cancel_button, 8,1)
        f_cancel_button.clicked.connect(pb_cancel_handler)
        f_quantize_combobox.setCurrentIndex(self.default_pb_quantize)
        f_window.exec_()

rec_button_group = QtGui.QButtonGroup()

global_last_rec_armed_track = None

def global_set_record_armed_track():
    if global_last_rec_armed_track is None:
        return
    elif global_last_rec_armed_track < pydaw_midi_track_count:
        this_region_editor.tracks[global_last_rec_armed_track].record_radiobutton.setChecked(True)
    elif global_last_rec_armed_track < pydaw_midi_track_count + pydaw_bus_count:
        this_region_bus_editor.tracks[global_last_rec_armed_track - \
        pydaw_midi_track_count].record_radiobutton.setChecked(True)
    else:
        this_region_audio_editor.tracks[global_last_rec_armed_track - pydaw_midi_track_count - \
        pydaw_bus_count].record_radiobutton.setChecked(True)


class seq_track:
    def on_vol_change(self, value):
        self.volume_label.setText("{} dB".format(value))
        if not self.suppress_osc:
            if self.is_instrument:
                this_pydaw_project.this_pydaw_osc.pydaw_set_vol(self.track_number,
                                                                self.volume_slider.value(), 0)
            else:
                this_pydaw_project.this_pydaw_osc.pydaw_set_vol(self.track_number,
                                                                self.volume_slider.value(), 1)

    def on_vol_released(self):
        if self.is_instrument:
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.commit(_("Set volume for MIDI track "
                                      "{} to {}").format(self.track_number,
                                      self.volume_slider.value()))
        else:
            f_tracks = this_pydaw_project.get_bus_tracks()
            f_tracks.busses[self.track_number].vol = self.volume_slider.value()
            this_pydaw_project.save_busses(f_tracks)
            this_pydaw_project.commit(_("Set volume for bus track {} to {}").format(
                self.track_number, self.volume_slider.value()))
    def on_pan_change(self, value):
        this_pydaw_project.save_tracks(this_region_editor.get_tracks())
    def on_solo(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_solo(self.track_number,
                                                             self.solo_checkbox.isChecked(), 0)
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.commit(_("Set solo for MIDI track {} to {}").format(
                self.track_number, self.solo_checkbox.isChecked()))
    def on_mute(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_mute(self.track_number,
                                                             self.mute_checkbox.isChecked(), 0)
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.commit(_("Set mute for MIDI track {} to {}").format(
                self.track_number, self.mute_checkbox.isChecked()))
    def on_rec(self, value):
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_track_rec(
                self.track_type, self.track_number, self.record_radiobutton.isChecked())
            global global_last_rec_armed_track
            global_last_rec_armed_track = self.track_number

    def on_name_changed(self):
        if self.is_instrument:
            self.track_name_lineedit.setText(
                pydaw_remove_bad_chars(self.track_name_lineedit.text()))
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.this_pydaw_osc.pydaw_save_track_name(
                self.track_number, self.track_name_lineedit.text(), 0)
            this_pydaw_project.commit(
                _("Set name for MIDI track {} to {}").format(self.track_number,
                self.track_name_lineedit.text()))
            global_inst_set_window_title(self.track_number,
                _("MIDI Track: {}").format(self.track_name_lineedit.text()))
            global_fx_set_window_title(0, self.track_number,
                _("MIDI Track: {}").format(self.track_name_lineedit.text()))

    def on_instrument_change(self, selected_instrument):
        if not self.suppress_osc:
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.delete_inst_file(self.track_number)
            this_pydaw_project.this_pydaw_osc.pydaw_set_instrument_index(
                self.track_number, selected_instrument)
            global_close_inst_ui(self.track_number, True)
            this_pydaw_project.commit(
                _("Set instrument for MIDI track {} to {}").format(self.track_number,
                self.instrument_combobox.currentText()))

    def on_show_ui(self):
        f_index = self.instrument_combobox.currentIndex()
        if f_index == 0:
            pass
        global_open_inst_ui(self.track_number, f_index,
                            _("MIDI Track: {}").format(self.track_name_lineedit.text()))

    def on_show_fx(self):
        if not self.is_instrument or self.instrument_combobox.currentIndex() > 0:
            if self.is_instrument:
                if self.instrument_combobox.currentIndex() > 0:
                    global_open_fx_ui(self.track_number, pydaw_folder_instruments, 0,
                    _("MIDI Track: {}").format(self.track_name_lineedit.text()))
            else:
                global_open_fx_ui(self.track_number, pydaw_folder_busfx, 1,
                _("Bus Track: {}").format(self.track_name_lineedit.text()))
    def on_bus_changed(self, a_value=0):
        if not self.suppress_osc:
            this_pydaw_project.save_tracks(this_region_editor.get_tracks())
            this_pydaw_project.this_pydaw_osc.pydaw_set_bus(self.track_number,
                                                            self.bus_combobox.currentIndex(), 0)
            this_pydaw_project.commit(
                _("Set bus for MIDI track {} to {}").format(self.track_number,
                self.bus_combobox.currentIndex()))

    def wheel_event(self, a_event=None):
        pass

    def context_menu_event(self, a_event=None):
        pass

    def __init__(self, a_track_num, a_track_text=_("track"), a_instrument=True):
        self.is_instrument = a_instrument
        if a_instrument:
            self.track_type = 0
        else:
            self.track_type = 1
        self.suppress_osc = True
        self.track_number = a_track_num
        self.group_box = QtGui.QWidget()
        self.group_box.contextMenuEvent = self.context_menu_event
        self.group_box.setObjectName("track_panel")
        self.main_vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.main_vlayout)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.main_vlayout.addLayout(self.hlayout2)
        self.volume_slider = QtGui.QSlider()
        self.volume_slider.setRange(-50, 12)
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
        self.track_name_lineedit.setFixedWidth(90)
        self.track_name_lineedit.editingFinished.connect(self.on_name_changed)
        self.hlayout3.addWidget(self.track_name_lineedit)
        self.fx_button = QtGui.QPushButton("FX")
        self.fx_button.pressed.connect(self.on_show_fx)
        self.fx_button.setObjectName("fxbutton")
        self.fx_button.setFixedWidth(24)
        if a_instrument:
            self.instrument_combobox = QtGui.QComboBox()
            self.instrument_combobox.wheelEvent = self.wheel_event
            self.instrument_combobox.addItems(["None", "Euphoria", "Ray-V", "Way-V"])
            self.instrument_combobox.currentIndexChanged.connect(self.on_instrument_change)
            self.instrument_combobox.setSizeAdjustPolicy(
                QtGui.QComboBox.AdjustToMinimumContentsLengthWithIcon)
            self.instrument_combobox.setSizePolicy(
                QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
            self.hlayout3.addWidget(self.instrument_combobox)
            self.ui_button = QtGui.QPushButton("UI")
            self.ui_button.pressed.connect(self.on_show_ui)
            self.ui_button.setObjectName("uibutton")
            self.ui_button.setMinimumWidth(24)
            self.ui_button.setMaximumWidth(24)
            self.hlayout3.addWidget(self.ui_button)
            self.bus_combobox = QtGui.QComboBox()
            self.bus_combobox.wheelEvent = self.wheel_event
            self.bus_combobox.addItems(['M', '1','2','3','4'])
            self.bus_combobox.setMinimumWidth(54)
            self.bus_combobox.currentIndexChanged.connect(self.on_bus_changed)
            self.hlayout2.addWidget(QtGui.QLabel(_("Bus:")))
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
        else:
            self.track_name_lineedit.setReadOnly(True)
            self.hlayout3.addItem(QtGui.QSpacerItem(10, 10,
                                                    QtGui.QSizePolicy.Expanding,
                                                    QtGui.QSizePolicy.Minimum))
            self.hlayout3.addWidget(self.fx_button)
        self.record_radiobutton = QtGui.QRadioButton()
        rec_button_group.addButton(self.record_radiobutton)
        self.record_radiobutton.toggled.connect(self.on_rec)
        self.record_radiobutton.setObjectName("rec_arm_radiobutton")
        self.hlayout3.addWidget(self.record_radiobutton)

        self.suppress_osc = False

    def open_track(self, a_track, a_notify_osc=False):
        if not a_notify_osc:
            self.suppress_osc = True
        self.volume_slider.setValue(a_track.vol)
        if self.is_instrument:
            self.track_name_lineedit.setText(a_track.name)
            self.instrument_combobox.setCurrentIndex(a_track.inst)
            self.solo_checkbox.setChecked(a_track.solo)
            self.mute_checkbox.setChecked(a_track.mute)
            self.bus_combobox.setCurrentIndex(a_track.bus_num)
        self.suppress_osc = False

    def get_track(self):
        if self.is_instrument:
            return pydaw_track(self.solo_checkbox.isChecked(), self.mute_checkbox.isChecked(),
                               self.volume_slider.value(), str(self.track_name_lineedit.text()),
                               self.instrument_combobox.currentIndex(),
                               self.bus_combobox.currentIndex(), self.track_number)
        else:
            return pydaw_bus(self.volume_slider.value(),
                             self.record_radiobutton.isChecked(), self.track_number)

class transport_widget:
    def set_region_value(self, a_val):
        self.region_spinbox.setValue(int(a_val) + 1)

    def set_bar_value(self, a_val):
        self.bar_spinbox.setValue(int(a_val) + 1)

    def get_region_value(self):
        return self.region_spinbox.value() - 1

    def get_bar_value(self):
        return self.bar_spinbox.value() - 1

    def set_pos_from_cursor(self, a_region, a_bar):
        if self.follow_checkbox.isChecked() and (self.is_playing or self.is_recording):
            f_region = int(a_region)
            f_bar = int(a_bar)
            if self.region_spinbox.value() != f_region or \
            self.bar_spinbox.value() != f_bar:
                self.set_region_value(f_region)
                self.set_bar_value(f_bar)
                this_audio_items_viewer.set_playback_pos(f_bar)
                f_bar += 1
                this_region_audio_editor.table_widget.selectColumn(f_bar)
                this_region_editor.table_widget.selectColumn(f_bar)
                this_region_bus_editor.table_widget.selectColumn(f_bar)
                if f_region != self.last_region_num:
                    self.last_region_num = f_region
                    f_item = this_song_editor.table_widget.item(0, f_region)
                    this_song_editor.table_widget.selectColumn(f_region)
                    if not f_item is None and f_item.text() != "":
                        this_region_settings.open_region(f_item.text())
                    else:
                        this_region_settings.clear_items()
                        this_audio_items_viewer.clear_drawn_items(a_default_length=True)
                        this_audio_items_viewer.scale_to_region_size()
                        for f_region_editor in global_region_editors:
                            f_region_editor.set_region_length()

    def get_pos_in_seconds(self):
        f_bars = pydaw_get_pos_in_bars(self.get_region_value(), self.get_bar_value(), 0.0)
        f_seconds_per_bar = 60.0 / (self.tempo_spinbox.value() * 0.25)
        return f_bars * f_seconds_per_bar

    def set_pos_in_seconds(self, a_seconds):
        f_seconds_per_bar = 60.0 / (self.tempo_spinbox.value() * 0.25)
        f_bars_total = int(a_seconds / f_seconds_per_bar)
        f_region, f_bar = pydaw_bars_to_pos(f_bars_total)
        self.set_region_value(f_region)
        self.set_bar_value(f_bar)

    def init_playback_cursor(self, a_start=True):
        if not self.follow_checkbox.isChecked():
            return
        if this_song_editor.table_widget.item(0, self.get_region_value()) is not None:
            f_region_name = \
                str(this_song_editor.table_widget.item(0, self.get_region_value()).text())
            if not a_start or (global_current_region_name is not None and \
            f_region_name != global_current_region_name) or global_current_region is None:
                this_region_settings.open_region(f_region_name)
        else:
            this_region_editor.clear_items()
            this_region_audio_editor.clear_items()
            this_region_bus_editor.clear_items()
            this_audio_items_viewer.clear_drawn_items()
        if a_start:
            this_region_editor.table_widget.selectColumn(self.get_bar_value() + 1)
            this_region_audio_editor.table_widget.selectColumn(self.get_bar_value() + 1)
            this_region_bus_editor.table_widget.selectColumn(self.get_bar_value() + 1)
        else:
            this_region_editor.table_widget.clearSelection()
            this_region_audio_editor.table_widget.clearSelection()
            this_region_bus_editor.table_widget.clearSelection()
        this_song_editor.table_widget.selectColumn(self.get_region_value())

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
            self.set_region_value(self.start_region)
            self.set_bar_value(self.last_bar)
        this_song_editor.table_widget.setEnabled(False)
        this_region_settings.on_play()
        self.bar_spinbox.setEnabled(False)
        self.region_spinbox.setEnabled(False)
        global global_transport_is_playing
        global_transport_is_playing = True
        self.is_playing = True
        self.init_playback_cursor()
        self.last_region_num = self.get_region_value()
        self.start_region = self.get_region_value()
        self.last_bar = self.get_bar_value()
        this_pydaw_project.this_pydaw_osc.pydaw_play(a_region_num=self.get_region_value(),
                                                     a_bar=self.get_bar_value())
        self.trigger_audio_playback()
        this_wave_editor_widget.on_play()
        self.menu_button.setEnabled(False)
        this_audio_items_viewer.set_playback_clipboard()

    def trigger_audio_playback(self):
        if not self.follow_checkbox.isChecked():
            return
        this_audio_items_viewer.set_playback_pos(self.get_bar_value())
        this_audio_items_viewer.start_playback(self.tempo_spinbox.value())

    def on_stop(self):
        if not self.is_playing and not self.is_recording:
            return
        global global_transport_is_playing
        global_transport_is_playing = False
        this_song_editor.table_widget.setEnabled(True)
        this_region_settings.on_stop()
        self.bar_spinbox.setEnabled(True)
        self.region_spinbox.setEnabled(True)
        self.overdub_checkbox.setEnabled(True)
        this_pydaw_project.this_pydaw_osc.pydaw_stop()
        sleep(0.1)
        self.set_region_value(self.start_region)
        if self.is_recording:
            self.is_recording = False
            #As the history will be referenced when the recorded items are added to history
            this_pydaw_project.flush_history()
            sleep(2)  #Give it some time to flush the recorded items to disk...
            self.show_save_items_dialog()
            if global_current_region is not None and this_region_settings.enabled:
                this_region_settings.open_region_by_uid(global_current_region.uid)
            this_song_editor.open_song()
            this_pydaw_project.commit(_("Recording"))
        self.init_playback_cursor(a_start=False)
        self.is_playing = False
        self.set_bar_value(self.last_bar)
        f_song_table_item = this_song_editor.table_widget.item(0, self.get_region_value())
        if f_song_table_item is not None and str(f_song_table_item.text()) != None:
            f_song_table_item_str = str(f_song_table_item.text())
            this_region_settings.open_region(f_song_table_item_str)
        else:
            this_region_settings.clear_items()
        this_audio_items_viewer.stop_playback()
        this_audio_items_viewer.set_playback_pos(self.get_bar_value())
        this_wave_editor_widget.on_stop()
        self.menu_button.setEnabled(True)

    def show_save_items_dialog(self):
        def ok_handler():
            f_file_name = str(f_file.text())
            if f_file_name is None or f_file_name == "":
                QtGui.QMessageBox.warning(f_window, _("Error"),
                                          _("You must select a name for the item"))
                return
            this_pydaw_project.check_for_recorded_items(f_file_name)
            f_window.close()

        def text_edit_handler(a_val=None):
            f_file.setText(pydaw_remove_bad_chars(f_file.text()))

        f_window = QtGui.QDialog(this_main_window,
                                 QtCore.Qt.WindowTitleHint | QtCore.Qt.FramelessWindowHint)
        f_window.setMinimumWidth(330)
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_layout.addWidget(QtGui.QLabel(_("Save recorded MIDI items")), 0, 2)
        f_layout.addWidget(QtGui.QLabel(_("Item Name:")), 3, 1)
        f_file = QtGui.QLineEdit()
        f_file.setMaxLength(24)
        f_file.textEdited.connect(text_edit_handler)
        f_layout.addWidget(f_file, 3, 2)
        f_ok_button = QtGui.QPushButton(_("Save"))
        f_ok_button.clicked.connect(ok_handler)
        f_layout.addWidget(f_ok_button, 8,2)
        f_window.exec_()

    def on_rec(self):
        if self.is_playing:
            self.play_button.setChecked(True)
            return
        if self.overdub_checkbox.isChecked() and self.loop_mode_combobox.currentIndex() > 0:
            QtGui.QMessageBox.warning(self.group_box, _("Error"),
                                      _("Cannot use overdub mode with loop mode to record"))
            self.stop_button.setChecked(True)
            return
        this_song_editor.table_widget.setEnabled(False)
        this_wave_editor_widget.on_play()
        this_region_settings.on_play()
        self.bar_spinbox.setEnabled(False)
        self.region_spinbox.setEnabled(False)
        self.overdub_checkbox.setEnabled(False)
        global global_transport_is_playing
        global_transport_is_playing = True
        self.is_recording = True
        self.init_playback_cursor()
        self.last_region_num = self.get_region_value()
        self.start_region = self.get_region_value()
        self.last_bar = self.get_bar_value()
        this_pydaw_project.this_pydaw_osc.pydaw_rec(a_region_num=self.get_region_value(),
                                                    a_bar=self.get_bar_value())
        self.trigger_audio_playback()
        this_audio_items_viewer.set_playback_clipboard()
        self.menu_button.setEnabled(False)

    def on_tempo_changed(self, a_tempo):
        self.transport.bpm = a_tempo
        pydaw_set_bpm(a_tempo)
        if global_current_region is not None:
            global_open_audio_items()
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_tempo(a_tempo)
            this_pydaw_project.save_transport(self.transport)
            this_pydaw_project.commit(_("Set project tempo to {}").format(a_tempo))

    def on_loop_mode_changed(self, a_loop_mode):
        if not self.suppress_osc:
            this_pydaw_project.this_pydaw_osc.pydaw_set_loop_mode(a_loop_mode)

    def on_bar_changed(self, a_bar):
        self.transport.bar = a_bar
        if not self.suppress_osc and not self.is_playing and not self.is_recording:
            this_audio_items_viewer.set_playback_pos(self.get_bar_value())

    def on_region_changed(self, a_region):
        self.bar_spinbox.setRange(1, pydaw_get_region_length(a_region - 1))
        self.transport.region = a_region
        if not self.is_playing and not self.is_recording:
            this_audio_items_viewer.set_playback_pos(self.get_bar_value())

    def on_follow_cursor_check_changed(self):
        if self.follow_checkbox.isChecked():
            f_item = this_song_editor.table_widget.item(0, self.get_region_value())
            if not f_item is None and f_item.text() != "":
                this_region_settings.open_region(f_item.text())
            else:
                this_region_editor.clear_items()
                this_region_audio_editor.clear_items()
                this_region_bus_editor.clear_items()
            this_song_editor.table_widget.selectColumn(self.get_region_value())
            this_region_editor.table_widget.selectColumn(self.get_bar_value())
            this_region_audio_editor.table_widget.selectColumn(self.get_bar_value())
            this_region_bus_editor.table_widget.selectColumn(self.get_bar_value())
            if self.is_playing or self.is_recording:
                self.trigger_audio_playback()
        else:
            this_region_editor.table_widget.clearSelection()
            this_region_audio_editor.table_widget.clearSelection()
            this_region_bus_editor.table_widget.clearSelection()
            if self.is_playing or self.is_recording:
                this_audio_items_viewer.stop_playback(0)
            else:
                this_audio_items_viewer.stop_playback()

    def open_transport(self, a_notify_osc=False):
        if not a_notify_osc:
            self.suppress_osc = True
        self.transport = this_pydaw_project.get_transport()
        self.tempo_spinbox.setValue(int(self.transport.bpm))
        self.suppress_osc = False

    def on_overdub_changed(self, a_val=None):
        this_pydaw_project.this_pydaw_osc.pydaw_set_overdub_mode(
            self.overdub_checkbox.isChecked())

    def on_panic(self):
        this_pydaw_project.this_pydaw_osc.pydaw_panic()

    def __init__(self):
        self.suppress_osc = True
        self.is_recording = False
        self.is_playing = False
        self.start_region = 0
        self.last_bar = 0
        self.last_open_dir = global_home
        self.transport = pydaw_transport()
        self.group_box = QtGui.QGroupBox()
        self.group_box.setObjectName("transport_panel")
        self.vlayout = QtGui.QVBoxLayout()
        self.group_box.setLayout(self.vlayout)
        self.hlayout1 = QtGui.QHBoxLayout()
        self.vlayout.addLayout(self.hlayout1)
        self.play_button = QtGui.QRadioButton()
        self.play_button.setObjectName("play_button")
        self.play_button.clicked.connect(self.on_play)
        self.hlayout1.addWidget(self.play_button)
        self.stop_button = QtGui.QRadioButton()
        self.stop_button.setChecked(True)
        self.stop_button.setObjectName("stop_button")
        self.stop_button.clicked.connect(self.on_stop)
        self.hlayout1.addWidget(self.stop_button)
        self.rec_button = QtGui.QRadioButton()
        self.rec_button.setObjectName("rec_button")
        self.rec_button.clicked.connect(self.on_rec)
        self.hlayout1.addWidget(self.rec_button)
        self.grid_layout1 = QtGui.QGridLayout()
        self.hlayout1.addLayout(self.grid_layout1)
        self.grid_layout1.addWidget(QtGui.QLabel(_("BPM:")), 0, 0)
        self.tempo_spinbox = QtGui.QSpinBox()
        self.tempo_spinbox.setKeyboardTracking(False)
        self.tempo_spinbox.setObjectName("large_spinbox")
        self.tempo_spinbox.setRange(50, 200)
        self.tempo_spinbox.valueChanged.connect(self.on_tempo_changed)
        self.grid_layout1.addWidget(self.tempo_spinbox, 1, 0)
        self.grid_layout1.addWidget(QtGui.QLabel(_("Region:")), 0, 10)
        self.region_spinbox = QtGui.QSpinBox()
        self.region_spinbox.setObjectName("large_spinbox")
        self.region_spinbox.setRange(1, 300)
        self.region_spinbox.valueChanged.connect(self.on_region_changed)
        self.grid_layout1.addWidget(self.region_spinbox, 1, 10)
        self.grid_layout1.addWidget(QtGui.QLabel(_("Bar:")), 0, 20)
        self.bar_spinbox = QtGui.QSpinBox()
        self.bar_spinbox.setObjectName("large_spinbox")
        self.bar_spinbox.setRange(1, 8)
        self.bar_spinbox.valueChanged.connect(self.on_bar_changed)
        self.grid_layout1.addWidget(self.bar_spinbox, 1, 20)
        f_loop_midi_gridlayout = QtGui.QVBoxLayout()
        f_upper_ctrl_layout = QtGui.QHBoxLayout()
        f_loop_midi_gridlayout.addLayout(f_upper_ctrl_layout)
        f_lower_ctrl_layout = QtGui.QHBoxLayout()
        f_upper_ctrl_layout.addWidget(QtGui.QLabel(_("Loop Mode:")))
        self.loop_mode_combobox = QtGui.QComboBox()
        self.loop_mode_combobox.addItems([_("Off"), _("Region")])
        self.loop_mode_combobox.setMinimumWidth(90)
        self.loop_mode_combobox.currentIndexChanged.connect(self.on_loop_mode_changed)
        f_upper_ctrl_layout.addWidget(self.loop_mode_combobox)
        self.follow_checkbox = QtGui.QCheckBox(_("Follow"))
        self.follow_checkbox.setChecked(True)
        self.follow_checkbox.clicked.connect(self.on_follow_cursor_check_changed)
        f_lower_ctrl_layout.addWidget(self.follow_checkbox)
        self.overdub_checkbox = QtGui.QCheckBox(_("Overdub"))
        self.overdub_checkbox.clicked.connect(self.on_overdub_changed)
        f_lower_ctrl_layout.addWidget(self.overdub_checkbox)
        self.menu_button = QtGui.QPushButton(_("Menu"))
        f_lower_ctrl_layout.addWidget(self.menu_button)
        self.panic_button = QtGui.QPushButton(_("Panic"))
        self.panic_button.pressed.connect(self.on_panic)
        f_lower_ctrl_layout.addItem(QtGui.QSpacerItem(10, 10, QtGui.QSizePolicy.Expanding))
        f_lower_ctrl_layout.addWidget(self.panic_button)
        self.tooltips_checkbox = QtGui.QCheckBox(_("Tooltips"))
        self.tooltips_checkbox.stateChanged.connect(pydaw_set_tooltips_enabled)
        f_upper_ctrl_layout.addWidget(self.tooltips_checkbox)
        f_loop_midi_gridlayout.addLayout(f_lower_ctrl_layout)
        self.hlayout1.addLayout(f_loop_midi_gridlayout)
        self.last_region_num = -99
        self.suppress_osc = False

    def set_tooltips(self, a_enabled):
        if a_enabled:
            self.tooltips_checkbox.setToolTip(
                _("Check this box to show really annoying (but useful) tooltips "
                "for everything"))
            self.panic_button.setToolTip(
                _("Panic button:   Sends a note-off signal on every note to every instrument"))
            self.overdub_checkbox.setToolTip(
                _("Checking this box causes recording to unlink existing items and append "
                "new events to the existing events"))
            self.follow_checkbox.setToolTip(
                _("Checking this box causes the region editor to follow playback"))
        else:
            self.tooltips_checkbox.setToolTip("")
            self.panic_button.setToolTip("")
            self.overdub_checkbox.setToolTip("")
            self.follow_checkbox.setToolTip("")


global_open_fx_ui_dicts = [{}, {}, {}, {}, {}]
global_open_inst_ui_dict = {}

def global_open_fx_ui(a_track_num, a_folder, a_track_type, a_title):
    global global_open_fx_ui_dicts
    if not a_track_num in global_open_fx_ui_dicts[a_track_type]:
        f_modulex = pydaw_widgets.pydaw_modulex_plugin_ui(global_plugin_rel_callback,
                                                          global_plugin_val_callback,
                                                          a_track_num,
                                                          this_pydaw_project, a_folder,
                                                          a_track_type,
                                                          a_title, this_main_window.styleSheet(),
                                                          global_fx_closed_callback,
                                                          global_configure_plugin_callback)

        pydaw_center_widget_on_screen(f_modulex.widget)
        f_modulex.show_widget()
        global_open_fx_ui_dicts[a_track_type][a_track_num] = f_modulex
    else:
        if global_open_fx_ui_dicts[a_track_type][a_track_num].widget.isHidden():
            global_open_fx_ui_dicts[a_track_type][a_track_num].widget.show()
        global_open_fx_ui_dicts[a_track_type][a_track_num].widget.raise_()


def global_open_inst_ui(a_track_num, a_plugin_type, a_title):
    global global_open_inst_ui_dict
    f_track_num = int(a_track_num)
    if not f_track_num in global_open_inst_ui_dict:
        if a_plugin_type == 1:
            f_plugin = pydaw_widgets.pydaw_euphoria_plugin_ui(global_plugin_rel_callback,
                                                              global_plugin_val_callback,
                                                              f_track_num, this_pydaw_project,
                                                              pydaw_folder_instruments, 0,
                                                              a_title,
                                                              this_main_window.styleSheet(),
                                                              global_inst_closed_callback,
                                                              global_configure_plugin_callback)
        elif a_plugin_type == 2:
            f_plugin = pydaw_widgets.pydaw_rayv_plugin_ui(global_plugin_rel_callback,
                                                          global_plugin_val_callback,
                                                          f_track_num, this_pydaw_project,
                                                          pydaw_folder_instruments,
                                                          0, a_title,
                                                          this_main_window.styleSheet(),
                                                          global_inst_closed_callback,
                                                          global_configure_plugin_callback)
        elif a_plugin_type == 3:
            f_plugin = pydaw_widgets.pydaw_wayv_plugin_ui(global_plugin_rel_callback,
                                                          global_plugin_val_callback,
                                                          f_track_num, this_pydaw_project,
                                                          pydaw_folder_instruments,
                                                          0, a_title,
                                                          this_main_window.styleSheet(),
                                                          global_inst_closed_callback,
                                                          global_configure_plugin_callback)
        else:
            return

        pydaw_center_widget_on_screen(f_plugin.widget)
        f_plugin.show_widget()
        global_open_inst_ui_dict[f_track_num] = f_plugin
    else:
        if global_open_inst_ui_dict[f_track_num].widget.isHidden():
            global_open_inst_ui_dict[f_track_num].widget.show()
        global_open_inst_ui_dict[f_track_num].widget.raise_()


def global_close_inst_ui(a_track_num, a_delete_file=False):
    f_track_num = int(a_track_num)
    global global_open_inst_ui_dict
    if f_track_num in global_open_inst_ui_dict:
        if a_delete_file:
            global_open_inst_ui_dict[f_track_num].delete_plugin_file()
        global_open_inst_ui_dict[f_track_num].widget.close()
        global_open_inst_ui_dict.pop(f_track_num)

def global_inst_set_window_title(a_track_num, a_track_name):
    f_track_num = int(a_track_num)
    global global_open_inst_ui_dict
    if f_track_num in global_open_inst_ui_dict:
        global_open_inst_ui_dict[f_track_num].set_window_title(a_track_name)

def global_fx_set_window_title(a_track_type, a_track_num, a_track_name):
    f_track_num = int(a_track_num)
    f_track_type = int(a_track_type)
    global global_open_fx_ui_dicts
    if f_track_num in global_open_fx_ui_dicts[f_track_type]:
        global_open_fx_ui_dicts[f_track_type][f_track_num].set_window_title(a_track_name)

def global_fx_closed_callback(a_track_num, a_track_type):
    pass
    #global global_open_fx_ui_dicts
    #global_open_fx_ui_dicts[a_track_type].pop(a_track_num)  #Not doing anymore,just hiding

def global_inst_closed_callback(a_track_num, a_track_type=None):
    pass
    #global global_open_inst_ui_dict
    #global_open_inst_ui_dict.pop(a_track_num)  #Not doing anymore, just hiding

def global_configure_plugin_callback(a_is_instrument, a_track_type, a_track_num,
                                     a_key, a_message):
    this_pydaw_project.this_pydaw_osc.pydaw_configure_plugin(a_is_instrument,
                                                             a_track_type, a_track_num,
                                                             a_key, a_message)

def global_close_all_plugin_windows():
    global global_open_fx_ui_dicts, global_open_inst_ui_dict
    for f_dict in global_open_fx_ui_dicts:
        for v in list(f_dict.values()):
            v.is_quitting = True
            v.widget.close()
    for v in list(global_open_inst_ui_dict.values()):
        v.is_quitting = True
        v.widget.close()
    global_open_fx_ui_dicts = [{}, {}, {}, {}, {}]
    global_open_inst_ui_dict = {}

class pydaw_main_window(QtGui.QMainWindow):
    def check_for_empty_directory(self, a_file):
        """ Return true if directory is empty, show error message and
            return False if not
        """
        f_parent_dir = os.path.dirname(a_file)
        if not os.listdir(f_parent_dir) == []:
            QtGui.QMessageBox.warning(self, _("Error"),
            _("You must save the project file to an empty directory, use "
            "the 'Create Folder' button to create a directory."))
            return False
        else:
            return True

    def on_new(self):
        if global_transport_is_playing:
            return
        try:
            while True:
                f_file = QtGui.QFileDialog.getSaveFileName(
                    parent=self, caption=_('New Project'),
                    directory="{}/default.{}".format(global_home, global_pydaw_version_string),
                    filter=global_pydaw_file_type_string)
                if not f_file is None and not str(f_file) == "":
                    f_file = str(f_file)
                    if not self.check_for_empty_directory(f_file):
                        continue
                    if not f_file.endswith("." + global_pydaw_version_string):
                        f_file += "." + global_pydaw_version_string
                    global_new_project(f_file)
                break
        except Exception as ex:
            pydaw_print_generic_exception(ex)

    def on_open(self):
        if global_transport_is_playing:
            return
        try:
            f_file = QtGui.QFileDialog.getOpenFileName(parent=self ,caption=_('Open Project'),
                                                       directory=global_default_project_folder,
                                                       filter=global_pydaw_file_type_string)
            if f_file is None:
                return
            f_file_str = str(f_file)
            if f_file_str == "":
                return
            global_open_project(f_file_str)
        except Exception as ex:
            pydaw_print_generic_exception(ex)

    def on_save_as(self):
        if global_transport_is_playing:
            return
        try:
            while True:
                f_new_file = QtGui.QFileDialog.getSaveFileName(self, _("Save project as..."),
                                                               directory="{}/{}.{}".format(
                                                               global_default_project_folder,
                                                               this_pydaw_project.project_file,
                                                               global_pydaw_version_string))
                if not f_new_file is None and not str(f_new_file) == "":
                    f_new_file = str(f_new_file)
                    if not self.check_for_empty_directory(f_new_file):
                        continue
                    if not f_new_file.endswith(".{}".format(global_pydaw_version_string)):
                        f_new_file += ".{}".format(global_pydaw_version_string)
                    global_close_all_plugin_windows()
                    this_pydaw_project.save_project_as(f_new_file)
                    set_window_title()
                    set_default_project(f_new_file)
                    break
                else:
                    break
        except Exception as ex:
            pydaw_print_generic_exception(ex)

    def show_offline_rendering_wait_window(self, a_file_name):
        f_file_name = "{}.finished".format(a_file_name)
        def ok_handler():
            f_window.close()

        def cancel_handler():
            f_window.close()

        def timeout_handler():
            if os.path.isfile(f_file_name):
                f_ok.setEnabled(True)
                f_timer.stop()
                f_time_label.setText(_("Finished in {}").format(f_time_label.text()))
                os.system('rm "{}"'.format(f_file_name))
            else:
                f_elapsed_time = time.time() - f_start_time
                f_time_label.setText(str(round(f_elapsed_time, 1)))

        f_start_time = time.time()
        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Rendering to .wav, please wait"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_time_label = QtGui.QLabel("")
        f_time_label.setMinimumWidth(360)
        f_layout.addWidget(f_time_label, 1, 1)
        f_timer = QtCore.QTimer()
        f_timer.timeout.connect(timeout_handler)

        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok.setEnabled(False)
        f_layout.addWidget(f_ok)
        f_layout.addWidget(f_ok, 2, 2)
        #f_cancel = QtGui.QPushButton("Cancel")
        #f_cancel.pressed.connect(cancel_handler)
        #f_layout.addWidget(f_cancel, 9, 2)
        #TODO:  Send a 'cancel_offline_render' message to the engine...
        f_timer.start(100)
        f_window.exec_()

    def on_offline_render(self):
        def ok_handler():
            if str(f_name.text()) == "":
                QtGui.QMessageBox.warning(f_window, _("Error"), _("Name cannot be empty"))
                return
            if (f_end_region.value() < f_start_region.value()) or \
            ((f_end_region.value() == f_start_region.value()) and \
            (f_start_bar.value() >= f_end_bar.value())):
                QtGui.QMessageBox.warning(f_window, _("Error"),
                _("End point is before start point."))
                return

            if f_copy_to_clipboard_checkbox.isChecked():
                self.copy_to_clipboard_checked = True
                f_clipboard = QtGui.QApplication.clipboard()
                f_clipboard.setText(f_name.text())
            else:
                self.copy_to_clipboard_checked = False
            #TODO:  Check that the end is actually after the start....
            this_pydaw_project.this_pydaw_osc.pydaw_offline_render(f_start_region.value() - 1,
                                                                   f_start_bar.value() - 1,
                                                                   f_end_region.value() - 1,
                                                                   f_end_bar.value() - 1,
                                                                   f_name.text())
            self.start_reg = f_start_region.value()
            self.end_reg = f_end_region.value()
            self.start_bar = f_start_bar.value()
            self.end_bar = f_end_bar.value()
            self.last_offline_dir = os.path.dirname(str(f_name.text()))
            f_window.close()
            self.show_offline_rendering_wait_window(f_name.text())

        def cancel_handler():
            f_window.close()

        def file_name_select():
            try:
                if not os.path.isdir(self.last_offline_dir):
                    self.last_offline_dir = global_home
                f_file_name = str(QtGui.QFileDialog.getSaveFileName(
                    f_window,  _("Select a file name to save to..."), self.last_offline_dir))
                if not f_file_name is None and f_file_name != "":
                    if not f_file_name.endswith(".wav"):
                        f_file_name += ".wav"
                    if not f_file_name is None and not str(f_file_name) == "":
                        f_name.setText(f_file_name)
                    self.last_offline_dir = os.path.dirname(f_file_name)
            except Exception as ex:
                pydaw_print_generic_exception(ex)

        if self.first_offline_render:
            self.first_offline_render = False
            self.start_reg = 1
            self.end_reg = 1
            self.start_bar = 1
            self.end_bar = 2

            for i in range(300):
                f_item = this_song_editor.table_widget.item(0, i)
                if not f_item is None and f_item.text() != "":
                    self.start_reg = i + 1
                    break

            for i in range(self.start_reg, 300):
                f_item = this_song_editor.table_widget.item(0, i)
                if f_item is None or f_item.text() == "":
                    self.end_reg = i + 1
                    break

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Offline Render"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_name = QtGui.QLineEdit()
        f_name.setReadOnly(True)
        f_name.setMinimumWidth(360)
        f_layout.addWidget(QtGui.QLabel(_("File Name:")), 0, 0)
        f_layout.addWidget(f_name, 0, 1)
        f_select_file = QtGui.QPushButton(_("Select"))
        f_select_file.pressed.connect(file_name_select)
        f_layout.addWidget(f_select_file, 0, 2)

        f_layout.addWidget(QtGui.QLabel(_("Start:")), 1, 0)
        f_start_hlayout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_start_hlayout, 1, 1)
        f_start_hlayout.addWidget(QtGui.QLabel(_("Region:")))
        f_start_region = QtGui.QSpinBox()
        f_start_region.setRange(1, 299)
        f_start_region.setValue(self.start_reg)
        f_start_hlayout.addWidget(f_start_region)
        f_start_hlayout.addWidget(QtGui.QLabel(_("Bar:")))
        f_start_bar = QtGui.QSpinBox()
        f_start_bar.setRange(1, 8)
        f_start_bar.setValue(self.start_bar)
        f_start_hlayout.addWidget(f_start_bar)

        f_layout.addWidget(QtGui.QLabel(_("End:")), 2, 0)
        f_end_hlayout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_end_hlayout, 2, 1)
        f_end_hlayout.addWidget(QtGui.QLabel(_("Region:")))
        f_end_region = QtGui.QSpinBox()
        f_end_region.setRange(1, 299)
        f_end_region.setValue(self.end_reg)
        f_end_hlayout.addWidget(f_end_region)
        f_end_hlayout.addWidget(QtGui.QLabel(_("Bar:")))
        f_end_bar = QtGui.QSpinBox()
        f_end_bar.setRange(1, 8)
        f_end_bar.setValue(self.end_bar)
        f_end_hlayout.addWidget(f_end_bar)
        f_layout.addWidget(QtGui.QLabel(
        _("File is exported to 32 bit .wav at the sample rate your audio "
        "interface is running at.\nYou can convert "
        "the format using other programs such as Audacity")), 3, 1)
        f_copy_to_clipboard_checkbox = QtGui.QCheckBox(
        _("Copy export path to clipboard? (useful for right-click pasting "
        "back into the audio sequencer)"))
        f_copy_to_clipboard_checkbox.setChecked(self.copy_to_clipboard_checked)
        f_layout.addWidget(f_copy_to_clipboard_checkbox, 4, 1)
        f_ok_layout = QtGui.QHBoxLayout()
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10,
                                              QtGui.QSizePolicy.Expanding,
                                              QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 9, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 9, 2)
        f_window.exec_()

    def on_undo(self):
        if global_transport_is_playing:
            return
        if this_pydaw_project.undo():
            global_ui_refresh_callback()
        else:
            self.on_undo_history()

    def on_redo(self):
        if global_transport_is_playing:
            return
        this_pydaw_project.redo()
        global_ui_refresh_callback()

    def on_undo_history(self):
        if global_transport_is_playing:
            return
        this_pydaw_project.flush_history()
        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Undo history"))
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_widget = pydaw_history_log_widget(this_pydaw_project.history,
                                            global_ui_refresh_callback)
        f_widget.populate_table()
        f_layout.addWidget(f_widget)
        f_window.setGeometry(QtCore.QRect(f_window.x(), f_window.y(), 900, 720))
        f_window.exec_()

    def on_verify_history(self):
        if global_transport_is_playing:
            return
        f_str = this_pydaw_project.verify_history()
        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Verify Project History Database"))
        f_window.setFixedSize(800, 600)
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_text = QtGui.QTextEdit(f_str)
        f_text.setReadOnly(True)
        f_layout.addWidget(f_text)
        f_window.exec_()

    def on_change_audio_settings(self):
        f_dialog = pydaw_device_dialog.pydaw_device_dialog(True)
        f_dialog.show_device_dialog(a_notify=True)

    def on_open_theme(self):
        try:
            f_file = QtGui.QFileDialog.getOpenFileName(self,
                    _("Open a theme file"), "{}/lib/{}/themes".format(
                        pydaw_util.global_pydaw_install_prefix, global_pydaw_version_string),
                    "PyDAW Style(*.pytheme)")
            if f_file is not None and str(f_file) != "":
                f_file = str(f_file)
                f_style = pydaw_read_file_text(f_file)
                f_dir = os.path.dirname(f_file)
                f_style = pydaw_escape_stylesheet(f_style, f_dir)
                pydaw_write_file_text(global_user_style_file, f_file)
                QtGui.QMessageBox.warning(this_main_window, _("Theme Applied..."),
                                          _("Please restart PyDAW to update the UI"))
        except Exception as ex:
            pydaw_print_generic_exception(ex)

    def on_version(self):
        def on_ok():
            f_window.close()
        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Version Info"))
        f_window.setFixedSize(420, 90)
        f_layout = QtGui.QVBoxLayout()
        f_window.setLayout(f_layout)
        f_version = QtGui.QLabel(pydaw_read_file_text( "{}/lib/{}/{}-version.txt".format(
            pydaw_util.global_pydaw_install_prefix, global_pydaw_version_string,
            global_pydaw_version_string)))
        f_version.setTextInteractionFlags(QtCore.Qt.TextSelectableByMouse)
        f_layout.addWidget(f_version)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_layout.addWidget(f_ok_button)
        f_ok_button.pressed.connect(on_ok)
        f_window.exec_()

    def on_website(self):
        QtGui.QDesktopServices.openUrl(
            QtCore.QUrl("http://sourceforge.net/projects/libmodsynth/"))

    def on_spacebar(self):
        this_transport.on_spacebar()

    def tab_changed(self):
        if this_item_editor.delete_radiobutton.isChecked():
            this_item_editor.add_radiobutton.setChecked(True)

    def on_import_midi(self):
        self.midi_file = None

        def ok_handler():
            if self.midi_file is None:
                QtGui.QMessageBox.warning(f_window, _("Error"),
                                          _("File name cannot be empty"))
                return
            f_item_name_str = str(f_item_name.text())
            if f_item_name_str == "":
                QtGui.QMessageBox.warning(f_window, _("Error"), _("File name cannot be empty"))
                return
            if not self.midi_file.populate_region_from_track_map(
            this_pydaw_project, f_item_name_str):
                QtGui.QMessageBox.warning(f_window, _("Error"),
                _("No available slots for inserting a region, delete an existing "
                "region from the song editor first"))
            else:
                this_pydaw_project.commit(_("Import MIDI file"))
                this_song_editor.open_song()
            f_window.close()

        def cancel_handler():
            f_window.close()

        def file_name_select():
            if not os.path.isdir(self.last_offline_dir):
                self.last_offline_dir = global_home
            if self.last_midi_dir is None:
                f_dir_name = global_default_project_folder
            else:
                f_dir_name = self.last_midi_dir
            f_file_name = QtGui.QFileDialog.getOpenFileName(
                parent=self, caption=_('Open MIDI File'),
                directory=f_dir_name, filter='MIDI File (*.mid)')
            if not f_file_name is None and not str(f_file_name) == "":
                self.midi_file = pydaw_midi_file_to_items(f_file_name)
                f_name.setText(f_file_name)
                self.last_midi_dir = os.path.dirname(str(f_file_name))
                if str(f_item_name.text()).strip() == "":
                    f_item_name.setText(pydaw_remove_bad_chars(
                        f_file_name.split("/")[-1].replace(".", "-")))

        def item_name_changed(a_val=None):
            f_item_name.setText(pydaw_remove_bad_chars(f_item_name.text()))

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Import MIDI File..."))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_name = QtGui.QLineEdit()
        f_name.setReadOnly(True)
        f_name.setMinimumWidth(360)
        f_layout.addWidget(QtGui.QLabel(_("File Name:")), 0, 0)
        f_layout.addWidget(f_name, 0, 1)
        f_select_file = QtGui.QPushButton(_("Select"))
        f_select_file.pressed.connect(file_name_select)
        f_layout.addWidget(f_select_file, 0, 2)

        f_item_name = QtGui.QLineEdit()
        f_item_name.setMaxLength(24)
        f_layout.addWidget(QtGui.QLabel(_("Item Name:")), 2, 0)
        f_item_name.editingFinished.connect(item_name_changed)
        f_layout.addWidget(f_item_name, 2, 1)

        f_info_label = QtGui.QLabel()
        f_layout.addWidget(f_info_label, 4, 1)

        f_ok_layout = QtGui.QHBoxLayout()
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10,
                                              QtGui.QSizePolicy.Expanding,
                                              QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 9, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 9, 2)
        f_window.exec_()

    def on_collapse_splitters(self):
        self.song_region_splitter.setSizes([0, 9999])
        self.transport_splitter.setSizes([0, 9999])

    def on_restore_splitters(self):
        self.song_region_splitter.setSizes([100, 9999])
        self.transport_splitter.setSizes([100, 9999])

    def on_edit_notes(self, a_event=None):
        QtGui.QTextEdit.leaveEvent(self.notes_tab, a_event)
        this_pydaw_project.write_notes(self.notes_tab.toPlainText())

    def mp3_converter_dialog(self):
        if pydaw_which("avconv") is None and pydaw_which("ffmpeg") is not None:
            f_avconv = "ffmpeg"
        else:
            f_avconv = "avconv"
        f_lame = "lame"
        for f_app in (f_avconv, f_lame):
            if pydaw_which(f_app) is None:
                QtGui.QMessageBox.warning(self, _("Error"),
                _("Please ensure that avconv(or ffmpeg) and lame are installed, can't "
                "open mp3 converter dialog.\n"
                "Check your normal sources for packages or visit:\n\n"
                "http://lame.sourceforge.net/\n"
                "http://libav.org\n\nCan't find {}").format(f_app))
                return
        self.audio_converter_dialog("lame", "avconv", "mp3")

    def ogg_converter_dialog(self):
        if pydaw_which("oggenc") is None or pydaw_which("oggdec") is None:
            QtGui.QMessageBox.warning(self, _("Error"),
                _("Error, vorbis-tools are not installed"))
            return
        self.audio_converter_dialog("oggenc", "oggdec", "ogg")

    def audio_converter_dialog(self, a_enc, a_dec, a_label):
        def ok_handler():
            f_input_file = str(f_name.text())
            f_output_file = str(f_output_name.text())
            if f_input_file == "" or f_output_file == "":
                QtGui.QMessageBox.warning(f_window, _("Error"), _("File names cannot be empty"))
                return
            if f_wav_radiobutton.isChecked():
                if a_dec == "avconv" or a_dec == "ffmpeg":
                    f_cmd = [a_dec, "-i", f_input_file, f_output_file]
                elif a_dec == "oggdec":
                    f_cmd = [a_dec, "--output", f_output_file, f_input_file]
            else:
                if a_enc == "oggenc":
                    f_cmd = [a_enc, "-b", "{}k".format(f_mp3_br_combobox.currentText()),
                         "-o", f_output_file, f_input_file]
                elif a_enc == "lame":
                    f_cmd = [a_enc, "-b", str(f_mp3_br_combobox.currentText()),
                         f_input_file, f_output_file]
            f_proc = subprocess.Popen(f_cmd)
            f_proc.communicate()
            f_window.close()
            QtGui.QMessageBox.warning(self, _("Success"), _("Created file"))

        def cancel_handler():
            f_window.close()

        def set_output_file_name():
            if str(f_output_name.text()) == "":
                f_file_name = str(f_name.text()).rsplit('.')[0] + self.ac_ext
                f_output_name.setText(f_file_name)

        def file_name_select():
            try:
                if not os.path.isdir(self.last_ac_dir):
                    self.last_ac_dir = global_home
                f_file_name = QtGui.QFileDialog.getOpenFileName(
                    f_window, _("Select a file name to save to..."),
                    self.last_ac_dir,
                    filter=_("Audio Files {}").format('(*.wav *.{})'.format(a_label)))
                if not f_file_name is None and str(f_file_name) != "":
                    f_name.setText(str(f_file_name))
                    self.last_ac_dir = os.path.dirname(f_file_name)
                if f_file_name.lower().endswith(".{}".format(a_label)):
                    f_wav_radiobutton.setChecked(True)
                else:
                    f_mp3_radiobutton.setChecked(True)
                set_output_file_name()
            except Exception as ex:
                pydaw_print_generic_exception(ex)

        def file_name_select_output():
            try:
                if not os.path.isdir(self.last_ac_dir):
                    self.last_ac_dir = global_home
                f_file_name = str(QtGui.QFileDialog.getSaveFileName(
                    f_window, _("Select a file name to save to..."), self.last_ac_dir))
                if not f_file_name is None and f_file_name != "":
                    if not f_file_name.endswith(self.ac_ext):
                        f_file_name += self.ac_ext
                    f_output_name.setText(f_file_name)
                    self.last_ac_dir = os.path.dirname(f_file_name)
            except Exception as ex:
                pydaw_print_generic_exception(ex)

        def format_changed(a_val=None):
            if f_wav_radiobutton.isChecked():
                self.ac_ext = ".wav"
            else:
                self.ac_ext = ".{}".format(a_label)
            f_str = str(f_output_name.text()).strip()
            if f_str != "" and not f_str.endswith(self.ac_ext):
                f_arr = f_str.rsplit(".")
                f_output_name.setText(f_arr[0] + self.ac_ext)

        self.ac_ext = ".wav"
        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Audio File Converter"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_name = QtGui.QLineEdit()
        f_name.setReadOnly(True)
        f_name.setMinimumWidth(360)
        f_layout.addWidget(QtGui.QLabel(_("Input File:")), 0, 0)
        f_layout.addWidget(f_name, 0, 1)
        f_select_file = QtGui.QPushButton(_("Select"))
        f_select_file.pressed.connect(file_name_select)
        f_layout.addWidget(f_select_file, 0, 2)

        f_output_name = QtGui.QLineEdit()
        f_output_name.setReadOnly(True)
        f_output_name.setMinimumWidth(360)
        f_layout.addWidget(QtGui.QLabel(_("Output File:")), 1, 0)
        f_layout.addWidget(f_output_name, 1, 1)
        f_select_file_output = QtGui.QPushButton(_("Select"))
        f_select_file_output.pressed.connect(file_name_select_output)
        f_layout.addWidget(f_select_file_output, 1, 2)

        f_rb_group = QtGui.QButtonGroup()
        f_wav_radiobutton = QtGui.QRadioButton("wav")
        f_wav_radiobutton.setEnabled(True)
        f_wav_radiobutton.setChecked(True)
        f_rb_group.addButton(f_wav_radiobutton)
        f_wav_layout = QtGui.QHBoxLayout()
        f_wav_layout.addWidget(f_wav_radiobutton)
        f_layout.addLayout(f_wav_layout, 2, 1)
        f_wav_radiobutton.toggled.connect(format_changed)

        f_mp3_radiobutton = QtGui.QRadioButton(a_label)
        f_mp3_radiobutton.setEnabled(False)
        f_rb_group.addButton(f_mp3_radiobutton)
        f_mp3_layout = QtGui.QHBoxLayout()
        f_mp3_layout.addWidget(f_mp3_radiobutton)
        f_mp3_radiobutton.toggled.connect(format_changed)
        f_mp3_br_combobox = QtGui.QComboBox()
        f_mp3_br_combobox.addItems(["320", "160", "128"])
        f_mp3_layout.addWidget(QtGui.QLabel(_("Bitrate")))
        f_mp3_layout.addWidget(f_mp3_br_combobox)
        f_layout.addLayout(f_mp3_layout, 3, 1)

        f_ok_layout = QtGui.QHBoxLayout()
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10,
                                              QtGui.QSizePolicy.Expanding,
                                              QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 9, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 9, 2)
        f_window.exec_()

    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        #self.setMinimumSize(1100, 600)
        self.setObjectName("mainwindow")

        self.setStyleSheet(global_stylesheet)
        self.first_offline_render = True
        self.last_offline_dir = global_home
        self.last_ac_dir = global_home
        self.copy_to_clipboard_checked = True
        self.last_midi_dir = None

        self.central_widget = QtGui.QScrollArea()
        self.central_widget.setObjectName("plugin_ui")
        self.central_widget.setMinimumSize(500, 500)
        self.widget = QtGui.QWidget()
        self.widget.setObjectName("plugin_ui")
        self.setCentralWidget(self.central_widget)
        self.central_widget.setWidget(self.widget)
        self.central_widget.setWidgetResizable(True)
        self.central_widget.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.central_widget.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)

        self.main_layout = QtGui.QVBoxLayout()
        self.widget.setLayout(self.main_layout)
        self.transport_splitter = QtGui.QSplitter(QtCore.Qt.Vertical)
        self.main_layout.addWidget(self.transport_splitter)

        self.spacebar_action = QtGui.QAction(self)
        self.addAction(self.spacebar_action)
        self.spacebar_action.triggered.connect(self.on_spacebar)
        self.spacebar_action.setShortcut(QtGui.QKeySequence(QtCore.Qt.Key_Space))

        #The menus
        self.menu_bar = QtGui.QMenu(self)
        # Dirty hack, rather than moving the methods to the transport
        this_transport.menu_button.setMenu(self.menu_bar)
        self.menu_file = self.menu_bar.addMenu(_("&File"))

        self.new_action = self.menu_file.addAction(_("New"))
        self.new_action.triggered.connect(self.on_new)
        self.new_action.setShortcut(QtGui.QKeySequence.New)

        self.open_action = self.menu_file.addAction(_("Open"))
        self.open_action.triggered.connect(self.on_open)
        self.open_action.setShortcut(QtGui.QKeySequence.Open)

        self.save_as_action = self.menu_file.addAction(
            _("Save As...(projects are automatically saved, this creates a copy)"))
        self.save_as_action.triggered.connect(self.on_save_as)
        self.save_as_action.setShortcut(QtGui.QKeySequence.SaveAs)
        self.menu_file.addSeparator()

        self.offline_render_action = self.menu_file.addAction(_("Offline Render..."))
        self.offline_render_action.triggered.connect(self.on_offline_render)

        self.import_midi_action = self.menu_file.addAction(_("Import MIDI File..."))
        self.import_midi_action.triggered.connect(self.on_import_midi)
        self.menu_file.addSeparator()

        self.audio_device_action = self.menu_file.addAction(_("Hardware Settings..."))
        self.audio_device_action.triggered.connect(self.on_change_audio_settings)
        self.menu_file.addSeparator()

        self.quit_action = self.menu_file.addAction(_("Quit"))
        self.quit_action.triggered.connect(self.close)
        self.quit_action.setShortcut(QtGui.QKeySequence.Quit)

        self.menu_edit = self.menu_bar.addMenu(_("&Edit"))

        self.undo_action = self.menu_edit.addAction(_("Undo"))
        self.undo_action.triggered.connect(self.on_undo)
        self.undo_action.setShortcut(QtGui.QKeySequence.Undo)

        self.redo_action = self.menu_edit.addAction(_("Redo"))
        self.redo_action.triggered.connect(self.on_redo)
        self.redo_action.setShortcut(QtGui.QKeySequence.Redo)

        self.menu_edit.addSeparator()

        self.undo_history_action = self.menu_edit.addAction(_("Undo History..."))
        self.undo_history_action.triggered.connect(self.on_undo_history)

        self.verify_history_action = self.menu_edit.addAction(_("Verify History DB..."))
        self.verify_history_action.triggered.connect(self.on_verify_history)

        self.menu_appearance = self.menu_bar.addMenu(_("&Appearance"))

        self.collapse_splitters_action = self.menu_appearance.addAction(
            _("Collapse transport and song editor"))
        self.collapse_splitters_action.triggered.connect(self.on_collapse_splitters)
        self.collapse_splitters_action.setShortcut(QtGui.QKeySequence("CTRL+Up"))

        self.restore_splitters_action = self.menu_appearance.addAction(
            _("Restore transport and song editor"))
        self.restore_splitters_action.triggered.connect(self.on_restore_splitters)
        self.restore_splitters_action.setShortcut(QtGui.QKeySequence("CTRL+Down"))

        self.menu_appearance.addSeparator()

        self.open_theme_action = self.menu_appearance.addAction(_("Open Theme..."))
        self.open_theme_action.triggered.connect(self.on_open_theme)

        self.menu_tools = self.menu_bar.addMenu(_("&Tools"))

        self.ac_action = self.menu_tools.addAction(_("MP3 Converter..."))
        self.ac_action.triggered.connect(self.mp3_converter_dialog)

        self.ac_action = self.menu_tools.addAction(_("Ogg Converter..."))
        self.ac_action.triggered.connect(self.ogg_converter_dialog)

        self.menu_help = self.menu_bar.addMenu(_("&Help"))

        self.website_action = self.menu_help.addAction(_("PyDAW Website..."))
        self.website_action.triggered.connect(self.on_website)

        self.version_action = self.menu_help.addAction(_("Version Info..."))
        self.version_action.triggered.connect(self.on_version)

        self.transport_hlayout = QtGui.QHBoxLayout()
        self.transport_widget = QtGui.QWidget()
        self.transport_widget.setLayout(self.transport_hlayout)
        self.transport_splitter.addWidget(self.transport_widget)
        self.transport_widget.setSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Minimum)

        self.transport_hlayout.addWidget(this_transport.group_box, alignment=QtCore.Qt.AlignLeft)
        #The tabs
        self.main_tabwidget = QtGui.QTabWidget()
        self.main_tabwidget.currentChanged.connect(self.tab_changed)
        self.transport_splitter.addWidget(self.main_tabwidget)

        self.regions_tab_widget = QtGui.QTabWidget()
        self.song_region_tab = QtGui.QWidget()
        self.song_region_vlayout = QtGui.QVBoxLayout()
        self.song_region_tab.setLayout(self.song_region_vlayout)
        self.song_region_splitter = QtGui.QSplitter(QtCore.Qt.Vertical)
        self.song_region_splitter.addWidget(self.song_region_tab)
        self.main_tabwidget.addTab(self.song_region_splitter, _("Song/Region"))

        self.song_region_vlayout.addWidget(this_song_editor.table_widget)
        self.song_region_vlayout.addLayout(this_region_settings.hlayout0)

        self.song_region_splitter.addWidget(self.regions_tab_widget)
        self.regions_tab_widget.addTab(this_region_editor.table_widget, _("Instruments"))
        self.regions_tab_widget.addTab(this_region_bus_editor.table_widget, _("Busses"))
        self.regions_tab_widget.addTab(this_region_audio_editor.table_widget, _("Audio Tracks"))
        self.regions_tab_widget.addTab(this_audio_items_viewer_widget.hsplitter, _("Audio Seq"))

        self.main_tabwidget.addTab(this_item_editor.widget, _("MIDI Item"))

        self.cc_map_tab = QtGui.QWidget()
        self.cc_map_tab.setObjectName("ccmaptabwidget")
        f_cc_map_main_vlayout = QtGui.QVBoxLayout(self.cc_map_tab)
        f_cc_map_hlayout = QtGui.QHBoxLayout()
        f_cc_map_main_vlayout.addLayout(f_cc_map_hlayout)
        self.cc_map_table = pydaw_cc_map_editor()
        f_cc_map_hlayout.addWidget(self.cc_map_table.groupbox)
        f_cc_map_hlayout.addItem(QtGui.QSpacerItem(10, 10,
                                                   QtGui.QSizePolicy.Expanding,
                                                   QtGui.QSizePolicy.Minimum))
        self.main_tabwidget.addTab(self.cc_map_tab, _("CC Maps"))
        self.main_tabwidget.addTab(this_wave_editor_widget.widget, _("Wave Editor"))

        self.notes_tab = QtGui.QTextEdit(self)
        self.notes_tab.setAcceptRichText(False)
        self.notes_tab.leaveEvent = self.on_edit_notes
        self.main_tabwidget.addTab(self.notes_tab, _("Project Notes"))

        try:
            self.osc_server = liblo.Server(30321)
        except liblo.ServerError as err:
            print("Error creating OSC server: {}".format(err))
            self.osc_server = None
        if self.osc_server is not None:
            print((self.osc_server.get_url()))
            self.osc_server.add_method("pydaw/ui_configure", 's', self.configure_callback)
            self.osc_server.add_method(None, None, self.osc_fallback)
            self.osc_timer = QtCore.QTimer(self)
            self.osc_timer.setSingleShot(False)
            self.osc_timer.timeout.connect(self.osc_time_callback)
            self.osc_timer.start(20)
        if global_pydaw_with_audio:
            self.subprocess_timer = QtCore.QTimer(self)
            self.subprocess_timer.timeout.connect(self.subprocess_monitor)
            self.subprocess_timer.setSingleShot(False)
            self.subprocess_timer.start(1000)
        self.show()
        self.ignore_close_event = True

    def subprocess_monitor(self):
        if global_pydaw_subprocess.poll() != None:
            self.subprocess_timer.stop()
            exitCode = global_pydaw_subprocess.returncode
            if (exitCode != 0):
                QtGui.QMessageBox.warning(
                    self, _("Error"),
                    _("The audio engine died with error code {}, "
                    "please try restarting PyDAW").format(exitCode))

    def osc_time_callback(self):
        self.osc_server.recv(1)

    def osc_fallback(self, path, args, types, src):
        print(("got unknown message '{}' from '{}'".format(path, src)))
        for a, t in zip(args, types):
            print(("argument of type '{}': {}".format(t, a)))

    def configure_callback(self, path, arr):
        f_pc_dict = {}
        for f_line in arr[0].split("\n"):
            if f_line == "":
                break
            a_key, a_val = f_line.split("|", 1)
            if a_key == "pc":
                f_is_inst, f_track_num, f_port, f_val = a_val.split("|")
                f_track_type, f_track_num = track_all_to_type_and_index(f_track_num)
                f_pc_dict[(f_track_type, f_is_inst, f_track_num, f_port)] = f_val
            elif a_key == "cur":
                f_region, f_bar, f_beat = a_val.split("|")
                this_transport.set_pos_from_cursor(f_region, f_bar)
                this_audio_items_viewer.set_playback_pos(f_bar, f_beat)
            elif a_key == "ne":
                f_state, f_note = a_val.split("|")
                this_piano_roll_editor.highlight_keys(f_state, f_note)
            elif a_key == "ml":
                if self.cc_map_table.cc_spinbox is not None:
                    self.cc_map_table.cc_spinbox.setValue(int(a_val))
            elif a_key == "wec":
                this_wave_editor_widget.set_playback_cursor(float(a_val))
        #This prevents multiple events from moving the same control, only the last goes through
        for k, f_val in f_pc_dict.items():
            f_track_type, f_is_inst, f_track_num, f_port = k
            if int_to_bool(f_is_inst):
                if int(f_track_num) in global_open_inst_ui_dict:
                    global_open_inst_ui_dict[int(f_track_num)].set_control_val(
                        int(f_port), float(f_val))
            else:
                if int(f_track_num) in global_open_fx_ui_dicts[int(f_track_type)]:
                    global_open_fx_ui_dicts[int(f_track_type)]\
                    [int(f_track_num)].set_control_val(int(f_port), float(f_val))


    def closeEvent(self, event):
        if self.ignore_close_event:
            event.ignore()
            if global_transport_is_playing:
                return
            self.setEnabled(False)
            f_reply = QtGui.QMessageBox.question(
                self, _('Message'), _("Are you sure you want to quit?"),
                QtGui.QMessageBox.Yes | QtGui.QMessageBox.Cancel, QtGui.QMessageBox.Cancel)
            if f_reply == QtGui.QMessageBox.Cancel:
                self.setEnabled(True)
                return
            else:
                this_audio_items_viewer.prepare_to_quit()
                this_piano_roll_editor.prepare_to_quit()
                for f_viewer in this_cc_automation_viewers:
                    f_viewer.prepare_to_quit()
                sleep(0.5)
                global_close_all_plugin_windows()
                if self.osc_server is not None:
                    self.osc_timer.stop()
                if global_pydaw_with_audio: #Wait up to 6 seconds and then kill the process
                    self.subprocess_timer.stop()
                    if not "--debug" in sys.argv:
                        close_pydaw_engine()
                else:
                    this_pydaw_project.flush_history()
                if self.osc_server is not None:
                    self.osc_server.free()
                self.ignore_close_event = False
                f_quit_timer = QtCore.QTimer(self)
                f_quit_timer.setSingleShot(True)
                f_quit_timer.timeout.connect(self.close)
                f_quit_timer.start(1000)
        else:
            event.accept()


global_plugin_names = ["Euphoria", "Way-V", "Ray-V", "Modulex"]
global_plugin_numbers = [1, 3, 2, -1]
global_plugin_indexes = {1:0, 3:1, 2:2, -1:3}
global_cc_names = {"Euphoria":[], "Way-V":[], "Ray-V":[], "Modulex":[]}
global_controller_port_name_dict = {"Euphoria":{}, "Way-V":{}, "Ray-V":{}, "Modulex":{}}
global_controller_port_num_dict = {"Euphoria":{}, "Way-V":{}, "Ray-V":{}, "Modulex":{}}

class pydaw_controller_map_item:
    def __init__(self, a_name, a_port, a_transform_hint, a_min, a_max):
        self.name = str(a_name)
        self.transform_hint = int(a_transform_hint)
        self.port = int(a_port)
        self.min = float(a_min)
        self.max = float(a_max)

def pydaw_load_controller_maps():
    f_portmap_dict = \
    {"Euphoria":pydaw_ports.EUPHORIA_PORT_MAP, "Way-V":pydaw_ports.WAYV_PORT_MAP,
    "Ray-V":pydaw_ports.RAYV_PORT_MAP, "Modulex":pydaw_ports.MODULEX_PORT_MAP}
    #list(global_cc_names.keys())
    for k, v in f_portmap_dict.items():
        for f_line_arr in v:
            f_map  = pydaw_controller_map_item(f_line_arr[0], f_line_arr[1],
                                               f_line_arr[2], f_line_arr[3],
                                               f_line_arr[4])
            global_controller_port_name_dict[k][f_line_arr[0]] = f_map
            global_controller_port_num_dict[k][int(f_line_arr[1])] = f_map
            global_cc_names[k].append(f_line_arr[0])
        global_cc_names[k].sort()

def pydaw_get_cc_map(a_name):
    return pydaw_cc_map.from_str(
        pydaw_read_file_text("{}/{}".format(global_cc_map_folder, a_name)))

def pydaw_save_cc_map(a_name, a_map):
    pydaw_write_file_text("{}/{}".format(global_cc_map_folder, a_name), str(a_map))

class pydaw_cc_map_editor:
    def add_map(self, a_item):
        if not a_item in self.cc_maps_list:
            self.cc_maps_list.append(a_item)
        self.ignore_combobox = True
        self.map_combobox.clear()
        self.map_combobox.addItems(self.cc_maps_list)
        self.ignore_combobox = False
        self.map_combobox.setCurrentIndex(self.map_combobox.findText(a_item))

    def on_save_as(self):
        def ok_handler():
            f_str = str(f_name.text())
            if f_str == "":
                return
            f_map = pydaw_get_cc_map(self.current_map_name)
            self.current_map_name = f_str
            pydaw_save_cc_map(self.current_map_name, f_map)
            self.add_map(f_str)
            this_pydaw_project.this_pydaw_osc.pydaw_load_cc_map(self.current_map_name)
            f_window.close()

        def cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Save CC Map"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_name = QtGui.QLineEdit()
        f_name.setMinimumWidth(240)
        f_layout.addWidget(QtGui.QLabel(_("File Name:")), 0, 0)
        f_layout.addWidget(f_name, 0, 1)

        f_ok_layout = QtGui.QHBoxLayout()
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10,
                                              QtGui.QSizePolicy.Expanding,
                                              QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 9, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 9, 2)
        f_window.exec_()

    def on_new(self):
        def ok_handler():
            f_str = str(f_name.text())
            if f_str == "":
                return
            f_map = pydaw_cc_map()
            self.current_map_name = f_str
            pydaw_save_cc_map(self.current_map_name, f_map)
            self.add_map(f_str)
            this_pydaw_project.this_pydaw_osc.pydaw_load_cc_map(self.current_map_name)
            f_window.close()

        def cancel_handler():
            f_window.close()

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("New CC Map"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_name = QtGui.QLineEdit()
        f_name.setMinimumWidth(240)
        f_layout.addWidget(QtGui.QLabel(_("File Name:")), 0, 0)
        f_layout.addWidget(f_name, 0, 1)

        f_ok_layout = QtGui.QHBoxLayout()
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10,
                                              QtGui.QSizePolicy.Expanding,
                                              QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 9, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 9, 2)
        f_window.exec_()

    def on_open(self, a_val=None):
        if not self.ignore_combobox:
            self.current_map_name = str(self.map_combobox.currentText())
            self.open_map(self.current_map_name)
            this_pydaw_project.this_pydaw_osc.pydaw_load_cc_map(self.current_map_name)

    def on_new_cc(self):
        self.on_click()

    def on_click(self, x=None, y=None):
        def cc_ok_handler():
            f_map = pydaw_get_cc_map(self.current_map_name)
            f_map.add_item(self.cc_spinbox.value(), pydaw_cc_map_item(f_effects_cb.isChecked(),
                           global_controller_port_name_dict["Ray-V"]\
                                                           [str(f_rayv.currentText())].port,
                           global_controller_port_name_dict["Way-V"]\
                                                           [str(f_wayv.currentText())].port,
                           global_controller_port_name_dict["Euphoria"]\
                                                           [str(f_euphoria.currentText())].port,
                           global_controller_port_name_dict["Modulex"]\
                                                           [str(f_modulex.currentText())].port))
            pydaw_save_cc_map(self.current_map_name, f_map)
            self.open_map(self.current_map_name)
            this_pydaw_project.this_pydaw_osc.pydaw_load_cc_map(self.current_map_name)
            f_window.close()

        def cc_cancel_handler():
            f_map = pydaw_get_cc_map(self.current_map_name)
            try:
                f_map.map.pop(self.cc_spinbox.value())
                pydaw_save_cc_map(self.current_map_name, f_map)
                self.open_map(self.current_map_name)
                this_pydaw_project.this_pydaw_osc.pydaw_load_cc_map(self.current_map_name)
                self.cc_spinbox = None
            except KeyError:
                pass
            f_window.close()

        def window_close_event(a_val=None):
            this_pydaw_project.this_pydaw_osc.pydaw_midi_learn(False)

        f_window = QtGui.QDialog(this_main_window)
        f_window.closeEvent = window_close_event
        f_window.setWindowTitle(_("Map CC to Control(s)"))
        f_window.setMinimumWidth(240)
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)
        f_layout.addWidget(QtGui.QLabel(_("Move your MIDI controller to set the CC number,\n"
        "you must select a MIDI controller and record arm a track first.\n\n"
        "Checking the 'Effects tracks only?' box will cause the controller to only\n"
        "modify Modulex and all instrument controls will be ignored.")), 0, 1)
        self.cc_spinbox = QtGui.QSpinBox()
        self.cc_spinbox.setRange(1, 127)
        if x is not None:
            self.cc_spinbox.setValue(int(self.cc_table.item(x, 0).text()))
        f_layout.addWidget(QtGui.QLabel("CC"), 1, 0)
        f_layout.addWidget(self.cc_spinbox, 1, 1)
        f_effects_cb = QtGui.QCheckBox(_("Effects tracks only?"))
        f_layout.addWidget(f_effects_cb, 2, 1)
        if x is not None and str(self.cc_table.item(x, 1).text()) == "True":
            f_effects_cb.setChecked(True)

        f_euphoria = QtGui.QComboBox()
        f_list = list(global_controller_port_name_dict["Euphoria"].keys())
        f_list.sort()
        f_euphoria.addItems(f_list)
        f_layout.addWidget(QtGui.QLabel("Euphoria"), 3, 0)
        f_layout.addWidget(f_euphoria, 3, 1)
        if x is not None:
            f_euphoria.setCurrentIndex(f_euphoria.findText(str(self.cc_table.item(x, 2).text())))

        f_modulex = QtGui.QComboBox()
        f_modulex.setMinimumWidth(300)
        f_list = list(global_controller_port_name_dict["Modulex"].keys())
        f_list.sort()
        f_modulex.addItems(f_list)
        f_layout.addWidget(QtGui.QLabel("Modulex"), 4, 0)
        f_layout.addWidget(f_modulex, 4, 1)
        if x is not None:
            f_modulex.setCurrentIndex(f_modulex.findText(str(self.cc_table.item(x, 3).text())))

        f_rayv = QtGui.QComboBox()
        f_list = list(global_controller_port_name_dict["Ray-V"].keys())
        f_list.sort()
        f_rayv.addItems(f_list)
        f_layout.addWidget(QtGui.QLabel("Ray-V"), 5, 0)
        f_layout.addWidget(f_rayv, 5, 1)
        if x is not None:
            f_rayv.setCurrentIndex(f_rayv.findText( str(self.cc_table.item(x, 4).text()) ))

        f_wayv = QtGui.QComboBox()
        f_list = list(global_controller_port_name_dict["Way-V"].keys())
        f_list.sort()
        f_wayv.addItems(f_list)
        f_layout.addWidget(QtGui.QLabel("Way-V"), 6, 0)
        f_layout.addWidget(f_wayv, 6, 1)
        if x is not None:
            f_wayv.setCurrentIndex(f_wayv.findText( str(self.cc_table.item(x, 5).text()) ))

        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_layout.addLayout(f_ok_cancel_layout, 7,1)
        f_ok_button = QtGui.QPushButton(_("OK"))
        f_ok_cancel_layout.addWidget(f_ok_button)
        f_ok_button.clicked.connect(cc_ok_handler)
        f_cancel_button = QtGui.QPushButton(_("Clear"))
        f_ok_cancel_layout.addWidget(f_cancel_button)
        f_cancel_button.clicked.connect(cc_cancel_handler)
        this_pydaw_project.this_pydaw_osc.pydaw_midi_learn(True)
        f_window.exec_()

    def __init__(self):
        self.cc_spinbox = None
        self.ignore_combobox = False
        f_local_dir = global_pydaw_home
        if not os.path.isdir(f_local_dir):
            os.mkdir(f_local_dir)
        if not os.path.isfile("{}/default".format(global_cc_map_folder)):
            pydaw_save_cc_map("default", pydaw_cc_map())
        self.current_map_name = "default"
        self.cc_maps_list = os.listdir(global_cc_map_folder)
        self.cc_maps_list.sort()
        self.groupbox = QtGui.QGroupBox(_("Controllers"))
        self.groupbox.setFixedWidth(930)
        f_vlayout = QtGui.QVBoxLayout(self.groupbox)
        f_button_layout = QtGui.QHBoxLayout()
        f_vlayout.addLayout(f_button_layout)
        f_button_spacer = QtGui.QSpacerItem(10, 10,
                                            QtGui.QSizePolicy.Expanding,
                                            QtGui.QSizePolicy.Minimum)
        f_button_layout.addItem(f_button_spacer)
        f_new_cc_button = QtGui.QPushButton(_("New CC"))
        f_new_cc_button.pressed.connect(self.on_new_cc)
        f_button_layout.addWidget(f_new_cc_button)
        self.map_combobox = QtGui.QComboBox()
        self.map_combobox.setMinimumWidth(240)
        self.map_combobox.addItems(self.cc_maps_list)
        self.map_combobox.currentIndexChanged.connect(self.on_open)
        f_button_layout.addWidget(self.map_combobox)
        f_new_button = QtGui.QPushButton(_("New Map"))
        f_new_button.pressed.connect(self.on_new)
        f_button_layout.addWidget(f_new_button)
        f_save_as_button = QtGui.QPushButton(_("Save As"))
        f_save_as_button.pressed.connect(self.on_save_as)
        f_button_layout.addWidget(f_save_as_button)
        self.cc_table = QtGui.QTableWidget(0, 6)
        self.cc_table.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.cc_table.verticalHeader().setVisible(False)
        self.cc_table.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.cc_table.setHorizontalHeaderLabels(["CC", "Effects Only?", "Euphoria",
                                                 "Modulex", "Ray-V", "Way-V"])
        self.cc_table.cellClicked.connect(self.on_click)
        self.cc_table.setSortingEnabled(True)
        self.cc_table.sortByColumn(0)
        f_vlayout.addWidget(self.cc_table)
        self.open_map("default")

    def open_map(self, a_map_name):
        f_map = pydaw_get_cc_map(a_map_name)
        self.cc_table.clearContents()
        self.cc_table.setSortingEnabled(False)
        self.cc_table.setRowCount(len(f_map.map))
        f_row_pos = 0
        for k, v in list(f_map.map.items()):
            f_num = str(k).zfill(3)
            self.cc_table.setItem(f_row_pos, 0, QtGui.QTableWidgetItem(f_num))
            self.cc_table.setItem(f_row_pos, 1,
                                  QtGui.QTableWidgetItem(str(int_to_bool(v.effects_only))))
            self.cc_table.setItem(f_row_pos, 2,
                                  QtGui.QTableWidgetItem(
                                  global_controller_port_num_dict["Euphoria"]\
                                                                 [v.euphoria_port].name))
            self.cc_table.setItem(f_row_pos, 3,
                                  QtGui.QTableWidgetItem(
                                  global_controller_port_num_dict["Modulex"]\
                                                                 [v.modulex_port].name))
            self.cc_table.setItem(
                f_row_pos, 4,
                QtGui.QTableWidgetItem(
                    global_controller_port_num_dict["Ray-V"][v.rayv_port].name))
            self.cc_table.setItem(
                f_row_pos, 5,
                QtGui.QTableWidgetItem(
                    global_controller_port_num_dict["Way-V"][v.wayv_port].name))
            f_row_pos += 1
        self.cc_table.setSortingEnabled(True)
        self.cc_table.resizeColumnsToContents()


class pydaw_wave_editor_widget:
    def __init__(self):
        self.widget = QtGui.QWidget()
        self.layout = QtGui.QVBoxLayout(self.widget)
        self.right_widget = QtGui.QWidget()
        self.vlayout = QtGui.QVBoxLayout(self.right_widget)
        self.file_browser = pydaw_widgets.pydaw_file_browser_widget()
        self.file_browser.load_button.pressed.connect(self.on_file_open)
        self.file_browser.preview_button.pressed.connect(self.on_preview)
        self.file_browser.stop_preview_button.pressed.connect(self.on_stop_preview)
        self.file_browser.list_file.setSelectionMode(QtGui.QListWidget.SingleSelection)
        self.layout.addWidget(self.file_browser.hsplitter)
        self.file_browser.hsplitter.addWidget(self.right_widget)
        self.file_hlayout = QtGui.QHBoxLayout()
        self.enabled_checkbox = QtGui.QCheckBox(_("Enabled?"))
        self.enabled_checkbox.stateChanged.connect(self.enabled_changed)
        self.file_hlayout.addWidget(self.enabled_checkbox)

        self.menu = QtGui.QMenu(self.widget)
        self.menu_button = QtGui.QPushButton(_("Menu"))
        self.menu_button.setMenu(self.menu)
        self.file_hlayout.addWidget(self.menu_button)
        self.export_action = self.menu.addAction(_("Export"))
        self.export_action.triggered.connect(self.on_export)
        self.menu.addSeparator()
        self.copy_action = self.menu.addAction(_("Copy File to Clipboard"))
        self.copy_action.triggered.connect(self.copy_file_to_clipboard)
        self.paste_action = self.menu.addAction(_("Paste File from Clipboard"))
        self.paste_action.triggered.connect(self.open_file_from_clipboard)

        self.fx_button = QtGui.QPushButton(_("Effects"))
        self.fx_button.pressed.connect(self.on_show_fx)
        self.file_hlayout.addWidget(self.fx_button)
        self.file_lineedit = QtGui.QLineEdit()
        self.file_lineedit.setReadOnly(True)
        self.file_hlayout.addWidget(self.file_lineedit)
        self.vlayout.addLayout(self.file_hlayout)
        self.gridlayout = QtGui.QGridLayout()
        self.vlayout.addLayout(self.gridlayout)
        self.time_label = QtGui.QLabel("0:00")
        self.time_label.setMinimumWidth(60)
        self.gridlayout.addWidget(self.time_label, 0, 0)
        self.vol_label = QtGui.QLabel("0db")
        self.vol_label.setMinimumWidth(51)
        self.gridlayout.addWidget(self.vol_label, 0, 1)
        self.vol_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
        self.vol_slider.setRange(-24, 12)
        self.vol_slider.setValue(0)
        self.vol_slider.valueChanged.connect(self.vol_changed)
        self.gridlayout.addWidget(self.vol_slider, 0, 2)
        self.sample_graph = pydaw_audio_item_viewer_widget(self.marker_callback,
                                                           self.marker_callback,
                                                           self.marker_callback,
                                                           self.marker_callback)
        self.vlayout.addWidget(self.sample_graph)
        self.orig_pos = 0
        self.has_loaded_file = False
        self.duration = None
        self.sixty_recip = 1.0 / 60.0
        self.playback_cursor = None
        self.time_label_enabled = False
        self.file_browser.hsplitter.setSizes([420, 9999])
        self.copy_to_clipboard_checked = True
        self.last_offline_dir = global_home


    def on_export(self):
        if self.duration is None:
            return
        def ok_handler():
            if str(f_name.text()) == "":
                QtGui.QMessageBox.warning(f_window, _("Error"), _("Name cannot be empty"))
                return

            if f_copy_to_clipboard_checkbox.isChecked():
                self.copy_to_clipboard_checked = True
                f_clipboard = QtGui.QApplication.clipboard()
                f_clipboard.setText(f_name.text())
            else:
                self.copy_to_clipboard_checked = False

            this_pydaw_project.this_pydaw_osc.pydaw_we_export(f_name.text())
            self.last_offline_dir = os.path.dirname(str(f_name.text()))
            f_window.close()
            this_main_window.show_offline_rendering_wait_window(f_name.text())

        def cancel_handler():
            f_window.close()

        def file_name_select():
            try:
                if not os.path.isdir(self.last_offline_dir):
                    self.last_offline_dir = global_home
                f_file_name = str(QtGui.QFileDialog.getSaveFileName(
                    f_window,  _("Select a file name to save to..."), self.last_offline_dir))
                if not f_file_name is None and f_file_name != "":
                    if not f_file_name.endswith(".wav"):
                        f_file_name += ".wav"
                    if not f_file_name is None and not str(f_file_name) == "":
                        f_name.setText(f_file_name)
                    self.last_offline_dir = os.path.dirname(f_file_name)
            except Exception as ex:
                pydaw_print_generic_exception(ex)

        f_window = QtGui.QDialog(this_main_window)
        f_window.setWindowTitle(_("Offline Render"))
        f_layout = QtGui.QGridLayout()
        f_window.setLayout(f_layout)

        f_name = QtGui.QLineEdit()
        f_name.setReadOnly(True)
        f_name.setMinimumWidth(360)
        f_layout.addWidget(QtGui.QLabel(_("File Name:")), 0, 0)
        f_layout.addWidget(f_name, 0, 1)
        f_select_file = QtGui.QPushButton(_("Select"))
        f_select_file.pressed.connect(file_name_select)
        f_layout.addWidget(f_select_file, 0, 2)

        f_layout.addWidget(QtGui.QLabel(
        _("File is exported to 32 bit .wav at the sample rate your audio "
        "interface is running at.\nYou can convert "
        "the format using other programs such as Audacity")), 3, 1)
        f_copy_to_clipboard_checkbox = QtGui.QCheckBox(
        _("Copy export path to clipboard? (useful for right-click pasting "
        "back into the audio sequencer)"))
        f_copy_to_clipboard_checkbox.setChecked(self.copy_to_clipboard_checked)
        f_layout.addWidget(f_copy_to_clipboard_checkbox, 4, 1)
        f_ok_layout = QtGui.QHBoxLayout()
        f_ok_layout.addItem(QtGui.QSpacerItem(10, 10,
                                              QtGui.QSizePolicy.Expanding,
                                              QtGui.QSizePolicy.Minimum))
        f_ok = QtGui.QPushButton(_("OK"))
        f_ok.pressed.connect(ok_handler)
        f_ok_layout.addWidget(f_ok)
        f_layout.addLayout(f_ok_layout, 9, 1)
        f_cancel = QtGui.QPushButton(_("Cancel"))
        f_cancel.pressed.connect(cancel_handler)
        f_layout.addWidget(f_cancel, 9, 2)
        f_window.exec_()


    def on_reload(self):
        pass

    def on_show_fx(self):
        global_open_fx_ui(0, None, 4, _("Wave Editor"))

    def enabled_changed(self, a_val=None):
        this_pydaw_project.this_pydaw_osc.pydaw_ab_set(self.enabled_checkbox.isChecked())

    def vol_changed(self, a_val=None):
        f_result = self.vol_slider.value()
        self.marker_callback()
        self.vol_label.setText("{}dB".format(f_result))

    def on_preview(self):
        f_list = self.file_browser.files_selected()
        if f_list:
            this_pydaw_project.this_pydaw_osc.pydaw_preview_audio(f_list[0])

    def on_stop_preview(self):
        this_pydaw_project.this_pydaw_osc.pydaw_stop_preview()

    def on_file_open(self):
        f_file = self.file_browser.files_selected()
        if not f_file:
            return
        f_file_str = f_file[0]
        self.open_file(f_file_str)

    def copy_file_to_clipboard(self):
        f_clipboard = QtGui.QApplication.clipboard()
        f_clipboard.setText(str(self.file_lineedit.text()))

    def open_file_from_clipboard(self):
        f_clipboard = QtGui.QApplication.clipboard()
        f_text = str(f_clipboard.text())
        if len(f_text) < 1000 and os.path.isfile(f_text):
            self.open_file(f_text)
        else:
            QtGui.QMessageBox.warning(self.widget, _("Error"), _("No file path in the clipboard"))

    def open_file(self, a_file):
        self.clear_sample_graph()
        self.file_lineedit.setText(a_file)
        f_graph = self.set_sample_graph(a_file)
        self.duration = f_graph.frame_count / f_graph.sample_rate
        print("Duration:  {}".format(self.duration))
        self.has_loaded_file = True
        this_pydaw_project.this_pydaw_osc.pydaw_ab_open(a_file)
        self.marker_callback()

    def marker_callback(self, a_val=None):
        f_start = self.sample_graph.start_marker.value
        f_end = self.sample_graph.end_marker.value
        f_diff = f_end - f_start
        f_fade_in = ((self.sample_graph.fade_in_marker.value - f_start) / f_diff) * 1000.0
        f_fade_out = 1000.0 - \
            (((f_end - self.sample_graph.fade_out_marker.value) / f_diff) * 1000.0)

        f_item = pydaw_audio_item(0, a_sample_start=f_start, a_sample_end=f_end,
                                  a_vol=self.vol_slider.value(),
                                  a_fade_in=f_fade_in, a_fade_out=f_fade_out,
                                  a_fadein_vol=-36, a_fadeout_vol=-36)
        this_pydaw_project.this_pydaw_osc.pydaw_we_set("0|{}".format(f_item))
        self.set_time_label(f_start * 0.001, True)

    def set_playback_cursor(self, a_pos):
        if self.playback_cursor is not None:
            self.playback_cursor.setPos(
                a_pos * pydaw_widgets.pydaw_audio_item_scene_width, 0.0)
        self.set_time_label(a_pos)

    def set_time_label(self, a_value, a_override=False):
        if self.has_loaded_file and (a_override or self.time_label_enabled):
            f_seconds = self.duration * a_value
            f_minutes = int(f_seconds * self.sixty_recip)
            f_seconds = str(int(f_seconds % 60.0)).zfill(2)
            self.time_label.setText("{}:{}".format(f_minutes, f_seconds))

    def on_play(self):
        self.file_browser.load_button.setEnabled(False)
        self.file_browser.preview_button.setEnabled(False)
        self.file_browser.stop_preview_button.setEnabled(False)
        self.sample_graph.setEnabled(False)
        self.vol_slider.setEnabled(False)
        self.menu_button.setEnabled(False)
        if self.enabled_checkbox.isChecked():
            self.time_label_enabled = True
            self.playback_cursor = self.sample_graph.scene.addLine(
                                            self.sample_graph.start_marker.line.line(),
                                            self.sample_graph.start_marker.line.pen())

    def on_stop(self):
        self.file_browser.load_button.setEnabled(True)
        self.file_browser.preview_button.setEnabled(True)
        self.file_browser.stop_preview_button.setEnabled(True)
        self.sample_graph.setEnabled(True)
        self.vol_slider.setEnabled(True)
        self.menu_button.setEnabled(True)
        if self.playback_cursor is not None:
            self.sample_graph.scene.removeItem(self.playback_cursor)
            self.playback_cursor = None
        self.time_label_enabled = False
        if self.has_loaded_file:
            self.set_time_label(self.sample_graph.start_marker.value * 0.001, True)

    def set_sample_graph(self, a_file_name):
        this_pydaw_project.delete_sample_graph_by_name(a_file_name)
        f_graph = this_pydaw_project.get_sample_graph_by_name(a_file_name)
        self.sample_graph.draw_item(
            f_graph.create_sample_graph(True), 0.0, 1000.0, 0.0, 1000.0)
        return f_graph

    def clear_sample_graph(self):
        self.sample_graph.clear_drawn_items()


def set_default_project(a_project_path):
    f_def_file = "{}/last-project.txt".format(global_pydaw_home)
    f_handle = open(f_def_file, 'w')
    f_handle.write(str(a_project_path))
    f_handle.close()

def global_close_all():
    close_pydaw_engine()
    global_close_all_plugin_windows()
    this_region_settings.clear_new()
    this_item_editor.clear_new()
    this_song_editor.table_widget.clearContents()
    this_audio_items_viewer.clear_drawn_items()
    this_pb_automation_viewer.clear_drawn_items()
    for f_viewer in this_cc_automation_viewers:
        f_viewer.clear_drawn_items()
    for f_widget in this_item_editor.cc_auto_viewers:
        f_widget.update_ccs_in_use([])

def global_ui_refresh_callback(a_restore_all=False):
    """ Use this to re-open all existing items/regions/song in
        their editors when the files have been changed externally
    """
    for f_editor in global_region_editors:
        f_editor.open_tracks()
    f_regions_dict = this_pydaw_project.get_regions_dict()
    global global_current_region
    if global_current_region is not None and \
    f_regions_dict.uid_exists(global_current_region.uid):
        this_region_settings.open_region_by_uid(global_current_region.uid)
        global_open_audio_items()
        #this_audio_editor.open_tracks()
    else:
        this_region_settings.clear_new()
        global_current_region = None
    if this_item_editor.enabled and global_check_midi_items():
        global_open_items()
    this_song_editor.open_song()
    this_transport.open_transport()
    this_pydaw_project.this_pydaw_osc.pydaw_open_song(
        this_pydaw_project.project_folder, a_restore_all)
    global_set_record_armed_track()

def set_window_title():
    this_main_window.setWindowTitle('PyDAW4 - {}/{}.{}'.format(this_pydaw_project.project_folder,
                                                           this_pydaw_project.project_file,
                                                           global_pydaw_version_string))

#Opens or creates a new project
def global_open_project(a_project_file, a_wait=True):
    global_close_all()
    global this_pydaw_project
    if a_wait:
        sleep(3.0)
    open_pydaw_engine(a_project_file)
    this_pydaw_project = pydaw_project(global_pydaw_with_audio)
    this_pydaw_project.suppress_updates = True
    this_pydaw_project.open_project(a_project_file, False)
    this_wave_editor_widget.last_offline_dir = this_pydaw_project.user_folder
    this_song_editor.open_song()
    pydaw_update_region_lengths_dict()
    for f_editor in global_region_editors:
        f_editor.open_tracks()
    this_transport.open_transport()
    set_default_project(a_project_file)
    global_update_audio_track_comboboxes()
    set_window_title()
    this_pydaw_project.suppress_updates = False
    f_scale = this_pydaw_project.get_midi_scale()
    if f_scale is not None:
        this_piano_roll_editor_widget.scale_key_combobox.setCurrentIndex(f_scale[0])
        this_piano_roll_editor_widget.scale_combobox.setCurrentIndex(f_scale[1])
    this_song_editor.open_first_region()
    this_main_window.last_offline_dir = this_pydaw_project.user_folder
    this_main_window.notes_tab.setText(this_pydaw_project.get_notes())

def global_new_project(a_project_file, a_wait=True):
    global_close_all()
    global this_pydaw_project
    if a_wait:
        sleep(3.0)
    open_pydaw_engine(a_project_file)
    this_pydaw_project = pydaw_project(global_pydaw_with_audio)
    this_pydaw_project.new_project(a_project_file)
    this_pydaw_project.save_transport(this_transport.transport)
    this_wave_editor_widget.last_offline_dir = this_pydaw_project.user_folder
    this_song_editor.open_song()
    this_pydaw_project.save_song(this_song_editor.song)
    this_transport.open_transport()
    set_default_project(a_project_file)
    global_update_audio_track_comboboxes()
    set_window_title()
    this_main_window.last_offline_dir = this_pydaw_project.user_folder
    this_main_window.notes_tab.setText("")

this_pydaw_project = pydaw_project(global_pydaw_with_audio)

app = QtGui.QApplication(sys.argv)

global_cc_map_folder = "{}/cc_maps".format(global_pydaw_home)

if not os.path.isdir(global_cc_map_folder):
    os.makedirs(global_cc_map_folder)

pydaw_load_controller_maps()

global_timestretch_modes = [_("None"), _("Pitch(affecting time)"),
                            _("Time(affecting pitch)"), "Rubberband",
                            "Rubberband(formants)", "SBSMS", "Paulstretch"]
global_audio_track_names = {0:"track1", 1:"track2", 2:"track3", 3:"track4",
                            4:"track5", 5:"track6", 6:"track7", 7:"track8"}
global_suppress_audio_track_combobox_changes = False
global_audio_track_comboboxes = []

app.setWindowIcon(QtGui.QIcon("{}/share/pixmaps/{}.png".format(
                  pydaw_util.global_pydaw_install_prefix,
                  global_pydaw_version_string)))

this_pb_automation_viewer = automation_viewer(a_is_cc=False)
this_cc_automation_viewers = []
this_cc_automation_viewer0 = automation_viewer()
this_cc_automation_viewer1 = automation_viewer()
this_cc_automation_viewer2 = automation_viewer()
this_cc_automation_viewers.append(this_cc_automation_viewer0)
this_cc_automation_viewers.append(this_cc_automation_viewer1)
this_cc_automation_viewers.append(this_cc_automation_viewer2)

this_wave_editor_widget = pydaw_wave_editor_widget()
this_song_editor = song_editor()
this_region_settings = region_settings()
this_region_editor = region_list_editor(pydaw_track_type_enum.midi)
this_region_bus_editor = region_list_editor(pydaw_track_type_enum.bus)
this_region_audio_editor = region_list_editor(pydaw_track_type_enum.audio)
global_region_editors = (this_region_editor, this_region_bus_editor, this_region_audio_editor)

this_audio_item_editor_widget = audio_item_editor_widget()
this_piano_roll_editor = piano_roll_editor()
this_piano_roll_editor_widget = piano_roll_editor_widget()
this_item_editor = item_list_editor()
this_audio_items_viewer = audio_items_viewer()

if not os.path.isfile(pydaw_util.global_pydaw_device_config):
    f_dialog = pydaw_device_dialog.pydaw_device_dialog(a_is_running=True)
    f_dialog.show_device_dialog()
    sleep(1.0)
    pydaw_util.pydaw_read_device_config()

global_pydaw_subprocess = None

def close_pydaw_engine():
    """ Ask the engine to gracefully stop itself, then kill the process if it
    doesn't exit on it's own"""
    global global_pydaw_subprocess
    if global_pydaw_subprocess is not None:
        this_pydaw_project.quit_handler()
        f_exited = False
        for i in range(20):
            if global_pydaw_subprocess.poll() == None:
                f_exited = True
                break
            else:
                sleep(0.3)
        if not f_exited:
            try:
                if pydaw_util.global_pydaw_is_sandboxed:
                    print("global_pydaw_subprocess did not exit on it's own, "
                          "sending SIGTERM to helper script...")
                    global_pydaw_subprocess.terminate()
                else:
                    print("global_pydaw_subprocess did not exit on it's own, sending SIGKILL...")
                    global_pydaw_subprocess.kill()
            except Exception as ex:
                print("Exception raised while trying to kill process: {}".format(ex))
        global_pydaw_subprocess = None

def kill_pydaw_engine():
    """ Kill any zombie instances of the engine if they exist. Otherwise, the
    UI won't be able to control the engine"""
    try:
        f_val = subprocess.check_output(['ps', '-ef'])
    except Exception as ex:
        print("kill_pydaw_engine raised Exception during process search, "
              "assuming no zombie processes {}\n".format(ex))
        return
    f_engine_name = "{}-engine".format(global_pydaw_version_string)
    f_val = f_val.decode()
    f_result = []
    for f_line in f_val.split("\n"):
        #print(f_line)
        if f_engine_name in f_line:
            try:
                f_arr = f_line.split()
                f_result.append(int(f_arr[1]))
            except Exception as ex:
                print("kill_pydaw_engine Exception adding PID {}\n\t{}".format(f_arr[1], ex))

    if len(f_result) > 0:
        f_answer = QtGui.QMessageBox.warning(this_audio_item_editor_widget.widget, _("Warning"),
        _("Detected that there are instances of the PyDAW audio engine already running.\n"
        "This could mean that you already have PyDAW running, if so you should click 'Cancel'\n"
        "and close the other instance.\n\n"
        "This could also mean that for some reason the engine didn't properly terminate from "
        "another session.  If so, click 'OK' to kill the other process(es)"),
        buttons=QtGui.QMessageBox.Ok | QtGui.QMessageBox.Cancel)
        if f_answer == QtGui.QMessageBox.Cancel:
            exit(1)
        else:
            for f_pid in set(f_result):
                try:
                    f_kill = ["kill", "-9", f_arr[1]]
                    print(f_kill)
                    f_result = subprocess.check_output(f_kill)
                    print(f_result)
                except Exception as ex:
                    print("kill_pydaw_engine : Exception: {}".format(ex))
            sleep(3.0)

def open_pydaw_engine(a_project_path):
    if not global_pydaw_with_audio:
        print(_("Not starting audio because of the audio engine setting, "
              "you can change this in File->HardwareSettings"))
        return

    kill_pydaw_engine() #ensure no running instances of the engine
    f_project_dir = os.path.dirname(a_project_path)
    f_pid = os.getpid()
    print(_("Starting audio engine with {}").format(a_project_path))
    global global_pydaw_subprocess
    if pydaw_util.pydaw_which("pasuspender") is not None:
        f_pa_suspend = True
    else:
        f_pa_suspend = False

    if int(pydaw_util.global_device_val_dict["audioEngine"]) >= 3 \
    and pydaw_util.pydaw_which("x-terminal-emulator") is not None:
        f_sleep = "--sleep"
        if int(pydaw_util.global_device_val_dict["audioEngine"]) == 4 and \
        pydaw_util.pydaw_which("gdb") is not None:
            f_run_with = " gdb "
            f_sleep = ""
        elif int(pydaw_util.global_device_val_dict["audioEngine"]) == 5 and \
        pydaw_util.pydaw_which("valgrind") is not None:
            f_run_with = " valgrind "
            f_sleep = ""
        else:
            f_run_with = ""
        if f_pa_suspend:
            f_cmd = \
            ("""pasuspender -- x-terminal-emulator -e bash -c 'ulimit -c unlimited ; """
            """{} "{}" "{}" "{}" {} {} ; read' """).format(f_run_with,
                                                           pydaw_util.global_pydaw_bin_path,
                                                           global_pydaw_install_prefix,
                                                           f_project_dir, f_pid, f_sleep)
        else:
            f_cmd = \
            """x-terminal-emulator -e bash -c 'ulimit -c unlimited ; """
            """{} "{}" "{}" "{}" {} {} ; read' """.format(
            f_run_with, pydaw_util.global_pydaw_bin_path,
            pydaw_util.global_pydaw_install_prefix, f_project_dir, f_pid, f_sleep)
    else:
        if f_pa_suspend:
            f_cmd = 'pasuspender -- "{}" "{}" "{}" {}'.format(
                pydaw_util.global_pydaw_bin_path, pydaw_util.global_pydaw_install_prefix,
                f_project_dir, f_pid)
        else:
            f_cmd = '"{}" "{}" "{}" {}'.format(pydaw_util.global_pydaw_bin_path,
                                               pydaw_util.global_pydaw_install_prefix,
                                               f_project_dir, f_pid)
    print(f_cmd)
    global_pydaw_subprocess = subprocess.Popen([f_cmd], shell=True)

this_transport = transport_widget()
this_audio_items_viewer_widget = audio_items_viewer_widget()

#You must call this after instantiating the other widgets, as it relies on them existing
this_main_window = pydaw_main_window()
this_main_window.setWindowState(QtCore.Qt.WindowMaximized)
this_piano_roll_editor.verticalScrollBar().setSliderPosition(700)
this_piano_roll_editor_widget.snap_combobox.setCurrentIndex(4)

for f_viewer in this_item_editor.cc_auto_viewers:  #Get the plugin/control comboboxes populated
    f_viewer.plugin_changed()
this_item_editor.cc_auto_viewers[1].set_cc_num(2)
this_item_editor.cc_auto_viewers[2].set_cc_num(3)

# ^^^TODO:  Move the CC maps out of the main window class and instantiate earlier

f_def_file = "{}/last-project.txt".format(global_pydaw_home)
if os.path.exists(f_def_file):
    f_handle = open(f_def_file, 'r')
    default_project_file = f_handle.read()
    f_handle.close()
    if not os.path.exists(default_project_file):
        default_project_file = "{}/default-project/default.{}".format(
            global_pydaw_home, global_pydaw_version_string)
else:
    default_project_file = "{}/default-project/default.{}".format(
        global_pydaw_home, global_pydaw_version_string)
if os.path.exists(default_project_file):
    global_open_project(default_project_file, a_wait=False)
else:
    global_new_project(default_project_file, a_wait=False)

if global_show_create_folder_error:
    QtGui.QMessageBox.warning(
        this_main_window, _("Warning"),
        _("Error creating folder in {} , this is probably a permission issue"
        "if you didn't use FAT as the filesystem, settings will NOT persist between sessions "
        "until you make {} writable.").format(global_home, global_home))

global_tooltips_enabled_file = "{}/tooltips.txt".format(global_pydaw_home)

if os.path.isfile(global_tooltips_enabled_file):
    if pydaw_read_file_text(global_tooltips_enabled_file) == "True":
        this_transport.tooltips_checkbox.setChecked(True)
else:
    this_transport.tooltips_checkbox.setChecked(True)

QtCore.QTextCodec.setCodecForLocale(QtCore.QTextCodec.codecForName("UTF-8"))

sys.exit(app.exec_())

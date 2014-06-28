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

import os
import random
import traceback
import subprocess

from libpydaw.pydaw_util import *

from libpydaw.pydaw_osc import pydaw_osc
import wavefile
import datetime

import numpy
import scipy
import scipy.signal

from PyQt4 import QtGui, QtCore
from libpydaw import pydaw_history

pydaw_bus_count = 5
pydaw_audio_track_count = 8
pydaw_audio_input_count = 5
pydaw_midi_track_count = 20

def track_all_to_type_and_index(a_index):
    """ Convert global track number to track type + track number  """
    f_index = int(a_index)
    if f_index >= pydaw_midi_track_count + pydaw_bus_count:
        return 2, f_index - pydaw_midi_track_count - pydaw_bus_count
    elif f_index >= pydaw_midi_track_count:
        return 1, f_index - pydaw_midi_track_count
    else:
        return 0, f_index

def proj_file_str(a_val):
    f_val = a_val
    if isinstance(f_val, float):
        f_val = round(a_val, 6)
    return str(f_val)

pydaw_max_audio_item_count = 256
pydaw_max_region_length = 256 #bars

pydaw_folder_audio = "audio"
pydaw_folder_audiofx = "audiofx"
pydaw_folder_audio_per_item_fx = "audio_per_item_fx"
pydaw_folder_busfx = "busfx"
pydaw_folder_instruments = "instruments"
pydaw_folder_items = "items"
pydaw_folder_regions = "regions"
pydaw_folder_regions_audio = "regions_audio"
pydaw_folder_samplegraph = "samplegraph"
pydaw_folder_samples = "samples"
pydaw_folder_timestretch = "timestretch"
pydaw_folder_glued = "glued"
pydaw_folder_user = "user"
pydaw_folder_sendfx = "sendfx"

pydaw_file_pyregions = "default.pyregions"
pydaw_file_pyitems = "default.pyitems"
pydaw_file_pysong = "default.pysong"
pydaw_file_pytransport = "default.pytransport"
pydaw_file_pytracks = "default.pytracks"
pydaw_file_pyaudio = "default.pyaudio"
pydaw_file_pyaudioitem = "default.pyaudioitem"
pydaw_file_pybus = "default.pybus"
pydaw_file_pyinput = "default.pyinput"
pydaw_file_pywavs = "default.pywavs"
pydaw_file_pystretch = "default.pystretch"
pydaw_file_pystretch_map = "map.pystretch"
pydaw_file_notes = "notes.txt"
pydaw_file_wave_editor_bookmarks = "wave_editor_bookmarks.txt"

pydaw_min_note_length = 4.0/129.0  #Anything smaller gets deleted when doing a transform

pydaw_terminating_char = "\\"


class pydaw_project:
    def create_file(self, a_folder, a_file, a_text):
        """  Call save_file only if the file doesn't exist... """
        if not os.path.isfile("{}/{}/{}".format(self.project_folder, a_folder, a_file)):
            self.save_file(a_folder, a_file, a_text)

    def save_file(self, a_folder, a_file, a_text, a_force_new=False):
        """ Writes a file to disk and updates the project history to reflect the changes """
        f_full_path = "{}/{}/{}".format(self.project_folder, a_folder, a_file)
        if not a_force_new and os.path.isfile(f_full_path):
            f_old = pydaw_read_file_text(f_full_path)
            if f_old == a_text:
                return
            f_existed = 1
        else:
            f_old = ""
            f_existed = 0
        pydaw_write_file_text(f_full_path, a_text)
        f_history_file = pydaw_history.pydaw_history_file(a_folder, a_file, a_text,
                                                          f_old, f_existed)
        self.history_files.append(f_history_file)
        #TODO:  debug/verbose mode this output...
        print((str(f_history_file)))

    def commit(self, a_message):
        """ Commit the project history """
        if self.history_undo_cursor > 0:
            self.history_commits = self.history_commits[:self.history_undo_cursor]
            self.history_undo_cursor = 0
        if len(self.history_files) > 0:
            self.history_commits.append(pydaw_history.pydaw_history_commit(
                self.history_files, a_message))
            self.history_files = []

    def undo(self):
        if self.history_undo_cursor >= len(self.history_commits):
            return False #meaning show the window
        self.history_undo_cursor += 1
        self.history_commits[-1 * self.history_undo_cursor].undo(self.project_folder)
        return True

    def redo(self):
        if self.history_undo_cursor == 0:
            return
        self.history_commits[-1 * self.history_undo_cursor].redo(self.project_folder)
        self.history_undo_cursor -= 1

    def get_files_dict(self, a_folder, a_ext=None):
        f_result = {}
        f_files = []
        if a_ext is not None :
            for f_file in os.listdir(a_folder):
                if f_file.endswith(a_ext):
                    f_files.append(f_file)
        else:
            f_files = os.listdir(a_folder)
        for f_file in f_files:
            f_result[f_file] = pydaw_read_file_text("{}/{}".format(a_folder, f_file))
        return f_result

    def get_bus_fx_files(self):
        return os.listdir(self.busfx_folder)

    def get_audio_fx_files(self):
        return os.listdir(self.audiofx_folder)

    def delete_inst_file(self, a_track_num):
        f_file_path = "{}/{}.pyinst".format(self.instrument_folder, a_track_num)
        if os.path.isfile(f_file_path):
            os.system("rm -f '{}'".format(f_file_path))

    def flush_history(self):
        for f_commit in self.history_commits:
            self.history.commit(f_commit)
        self.history_commits = []
        self.history_undo_cursor = 0

    def save_project_as(self, a_file_name):
        f_file_name = str(a_file_name)
        print("Saving project as {} ...".format(f_file_name))
        f_new_project_folder = os.path.dirname(f_file_name)
        #The below is safe because we already checked that the folder
        #should be empty before calling this
        f_cmd = "rm -rf '{}'".format(f_new_project_folder)
        print(f_cmd)
        os.system(f_cmd)
        f_cmd = "cp -rf '{}' '{}'".format(self.project_folder, f_new_project_folder)
        print(f_cmd)
        os.system(f_cmd)
        print("{}/{} | {}".format(f_new_project_folder, self.project_file, a_file_name))
        self.set_project_folders(f_file_name)
        self.this_pydaw_osc.pydaw_open_song(self.project_folder)
        self.history = pydaw_history.pydaw_history(self.project_folder)

    def set_project_folders(self, a_project_file):
        #folders
        self.project_folder = os.path.dirname(a_project_file)
        self.project_file = os.path.splitext(os.path.basename(a_project_file))[0]
        self.instrument_folder = "{}/{}".format(self.project_folder, pydaw_folder_instruments)
        self.regions_folder = "{}/{}".format(self.project_folder, pydaw_folder_regions)
        self.regions_audio_folder = "{}/{}".format(self.project_folder,
                                                   pydaw_folder_regions_audio)
        self.items_folder = "{}/{}".format(self.project_folder, pydaw_folder_items)
        self.audio_folder = "{}/{}".format(self.project_folder, pydaw_folder_audio)
        self.audio_tmp_folder = "{}/audio/tmp".format(self.project_folder)
        self.samples_folder = "{}/{}".format(self.project_folder, pydaw_folder_samples)
        self.audiofx_folder = "{}/{}".format(self.project_folder, pydaw_folder_audiofx)
        self.audio_per_item_fx_folder = "{}/{}".format(self.project_folder,
                                                       pydaw_folder_audio_per_item_fx)
        self.busfx_folder = "{}/{}".format(self.project_folder, pydaw_folder_busfx)
        self.samplegraph_folder = "{}/{}".format(self.project_folder, pydaw_folder_samplegraph)
        self.timestretch_folder = "{}/{}".format(self.project_folder, pydaw_folder_timestretch)
        self.glued_folder = "{}/{}".format(self.project_folder, pydaw_folder_glued)
        self.user_folder = "{}/{}".format(self.project_folder, pydaw_folder_user)
        self.sendfx_folder = "{}/{}".format(self.project_folder, pydaw_folder_sendfx)
        #files
        self.pyregions_file = "{}/default.pyregions".format(self.project_folder)
        self.pyitems_file = "{}/default.pyitems".format(self.project_folder)
        self.pywavs_file = "{}/default.pywavs".format(self.project_folder)
        self.pystretch_file = "{}/{}".format(self.project_folder, pydaw_file_pystretch)
        self.pystretch_map_file = "{}/{}".format(self.project_folder, pydaw_file_pystretch_map)
        self.pyscale_file = "{}/default.pyscale".format(self.project_folder)
        self.pynotes_file = "{}/{}".format(self.project_folder, pydaw_file_notes)
        self.pywebm_file = "{}/{}".format(self.project_folder, pydaw_file_wave_editor_bookmarks)

        pydaw_clear_sample_graph_cache()

    def open_project(self, a_project_file, a_notify_osc=True):
        self.set_project_folders(a_project_file)
        if not os.path.exists(a_project_file):
            print("project file {} does not exist, creating as new project".format(a_project_file))
            self.new_project(a_project_file)
        else:
            self.history = pydaw_history.pydaw_history(self.project_folder)
            self.open_stretch_dicts()
        if a_notify_osc:
            self.this_pydaw_osc.pydaw_open_song(self.project_folder)

    def new_project(self, a_project_file, a_notify_osc=True):
        self.set_project_folders(a_project_file)

        project_folders = [self.project_folder, self.instrument_folder, self.regions_folder,
            self.items_folder, self.audio_folder, self.samples_folder,
            self.audiofx_folder, self.audio_per_item_fx_folder, self.busfx_folder,
            self.samplegraph_folder, self.audio_tmp_folder, self.regions_audio_folder,
            self.timestretch_folder, self.glued_folder, self.user_folder, self.sendfx_folder]

        for project_dir in project_folders:
            print(project_dir)
            if not os.path.isdir(project_dir):
                os.makedirs(project_dir)
        self.history = pydaw_history.pydaw_history(self.project_folder)

        self.create_file("", "version.txt",
                         "Created with {}-{}".format(global_pydaw_version_string,
                             pydaw_read_file_text("{}/lib/{}/minor-version.txt".format(
                                 global_pydaw_install_prefix, global_pydaw_version_string))))
        self.create_file("", os.path.basename(a_project_file),
                         "This file is not supposed to contain any data, it is "
                         "only a placeholder for saving and opening the project")
        self.create_file("", pydaw_file_pyregions, pydaw_terminating_char)
        self.create_file("", pydaw_file_pywavs, pydaw_terminating_char)
        self.create_file("", pydaw_file_pystretch_map, pydaw_terminating_char)
        self.create_file("", pydaw_file_pystretch, pydaw_terminating_char)
        self.create_file("", pydaw_file_pyitems, pydaw_terminating_char)
        self.create_file("", pydaw_file_pysong, pydaw_terminating_char)
        self.create_file("", pydaw_file_pytransport, str(pydaw_transport()))
        f_midi_tracks_instance = pydaw_tracks()
        for i in range(pydaw_midi_track_count):
            f_midi_tracks_instance.add_track(i, pydaw_track(a_name="track{}".format(i + 1),
                                                            a_track_pos=i))
        self.create_file("", pydaw_file_pytracks, str(f_midi_tracks_instance))
        f_pyaudio_instance = pydaw_audio_tracks()
        for i in range(pydaw_audio_track_count):
            f_pyaudio_instance.add_track(i, pydaw_audio_track(a_name="track{}".format(i + 1),
                                                              a_track_pos=i))
        self.create_file("", pydaw_file_pyaudio, str(f_pyaudio_instance))
        f_pybus_instance = pydaw_busses()
        for i in range(pydaw_bus_count):
            f_pybus_instance.add_bus(i, pydaw_bus(a_track_pos=i))
        self.create_file("", pydaw_file_pybus, str(f_pybus_instance))
        self.open_stretch_dicts()
        self.commit("Created project")
        if a_notify_osc:
            self.this_pydaw_osc.pydaw_open_song(self.project_folder)

    def get_next_glued_file_name(self):
        while True:
            self.glued_name_index += 1
            f_path = "{}/glued-{}.wav".format(self.glued_folder, self.glued_name_index)
            if not os.path.isfile(f_path):
                break
        return f_path

    def get_notes(self):
        if os.path.isfile(self.pynotes_file):
            return pydaw_read_file_text(self.pynotes_file)
        else:
            return ""

    def write_notes(self, a_text):
        pydaw_write_file_text(self.pynotes_file, a_text)

    def open_stretch_dicts(self):
        self.timestretch_cache = {}
        self.timestretch_reverse_lookup = {}

        f_cache_text = pydaw_read_file_text(self.pystretch_file)
        for f_line in f_cache_text.split("\n"):
            if f_line == pydaw_terminating_char:
                break
            f_line_arr = f_line.split("|", 5)
            f_file_path_and_uid = f_line_arr[5].split("|||")
            self.timestretch_cache[(int(f_line_arr[0]), float(f_line_arr[1]),
                                    float(f_line_arr[2]), float(f_line_arr[3]),
                                    float(f_line_arr[4]),
                                    f_file_path_and_uid[0])] = int(f_file_path_and_uid[1])

        f_map_text = pydaw_read_file_text(self.pystretch_map_file)
        for f_line in f_map_text.split("\n"):
            if f_line == pydaw_terminating_char:
                break
            f_line_arr = f_line.split("|||")
            self.timestretch_reverse_lookup[f_line_arr[0]] = f_line_arr[1]

    def save_stretch_dicts(self):
        f_stretch_text = ""
        for k, v in list(self.timestretch_cache.items()):
            for f_tuple_val in k:
                f_stretch_text += "{}|".format(f_tuple_val)
            f_stretch_text += "||{}\n".format(v)
        f_stretch_text += pydaw_terminating_char
        self.save_file("", pydaw_file_pystretch, f_stretch_text)

        f_map_text = ""
        for k, v in list(self.timestretch_reverse_lookup.items()):
            f_map_text += "{}|||{}\n".format(k, v)
        f_map_text += pydaw_terminating_char
        self.save_file("", pydaw_file_pystretch_map, f_map_text)

    def set_midi_scale(self, a_key, a_scale):
        pydaw_write_file_text(self.pyscale_file, "{}|{}".format(a_key, a_scale))

    def get_midi_scale(self):
        if os.path.exists(self.pyscale_file):
            f_list = pydaw_read_file_text(self.pyscale_file).split("|")
            return (int(f_list[0]), int(f_list[1]))
        else:
            return None

    def set_we_bm(self, a_file_list):
        f_list = [x for x in sorted(a_file_list) if len(x) < 1000]
        pydaw_write_file_text(self.pywebm_file, "\n".join(f_list))

    def get_we_bm(self):
        if os.path.exists(self.pywebm_file):
            f_list = pydaw_read_file_text(self.pywebm_file).split("\n")
            return [x for x in f_list if x]
        else:
            return []

    def get_regions_dict(self):
        try:
            f_file = open(self.pyregions_file, "r")
        except:
            return pydaw_name_uid_dict()
        f_str = f_file.read()
        f_file.close()
        return pydaw_name_uid_dict.from_str(f_str)

    def save_regions_dict(self, a_uid_dict):
        self.save_file("", pydaw_file_pyregions, str(a_uid_dict))

    def get_wavs_dict(self):
        try:
            f_file = open(self.pywavs_file, "r")
        except:
            return pydaw_name_uid_dict()
        f_str = f_file.read()
        f_file.close()
        return pydaw_name_uid_dict.from_str(f_str)

    def save_wavs_dict(self, a_uid_dict):
        pydaw_write_file_text (self.pywavs_file, str(a_uid_dict))
        #self.save_file("", pydaw_file_pywavs, str(a_uid_dict))

    def get_items_dict(self):
        try:
            f_file = open(self.pyitems_file, "r")
        except:
            return pydaw_name_uid_dict()
        f_str = f_file.read()
        f_file.close()
        return pydaw_name_uid_dict.from_str(f_str)

    def save_items_dict(self, a_uid_dict):
        self.save_file("", pydaw_file_pyitems, str(a_uid_dict))

    def get_song_string(self):
        try:
            f_file = open("{}/default.pysong".format(self.project_folder), "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_song(self):
        return pydaw_song.from_str(self.get_song_string())

    def get_region_string(self, a_region_uid):
        try:
            f_file = open("{}/{}".format(self.regions_folder, a_region_uid), "r")
        except:
            return "\\"  #TODO:  allow the exception to happen???
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_region_by_name(self, a_region_name):
        f_region_dict = self.get_regions_dict()
        f_region_name = str(a_region_name)
        f_uid = f_region_dict.get_uid_by_name(f_region_name)
        return pydaw_region.from_str(f_uid, self.get_region_string(f_uid))

    def get_region_by_uid(self, a_region_uid):
        f_uid = str(a_region_uid)
        return pydaw_region.from_str(f_uid, self.get_region_string(f_uid))

    def rename_items(self, a_item_names, a_new_item_name):
        f_items_dict = self.get_items_dict()
        if len(a_item_names) > 1 or f_items_dict.name_exists(a_new_item_name):
            f_suffix = 1
            f_new_item_name = "{}-".format(a_new_item_name)
            for f_item_name in a_item_names:
                while f_items_dict.name_exists("{}{}".format(f_new_item_name, f_suffix)):
                    f_suffix += 1
                f_items_dict.rename_item(f_item_name, f_new_item_name + str(f_suffix))
        else:
            f_items_dict.rename_item(a_item_names[0], a_new_item_name)
        self.save_items_dict(f_items_dict)

    def rename_region(self, a_old_name, a_new_name):
        f_regions_dict = self.get_regions_dict()
        if f_regions_dict.name_exists(a_new_name):
            f_suffix = 1
            f_new_name = "{}-".format(a_new_name)
            while f_regions_dict.name_exists(f_new_name + str(f_suffix)):
                f_suffix += 1
            f_regions_dict.rename_item(a_old_name, f_new_name)
        else:
            f_regions_dict.rename_item(a_old_name, a_new_name)
        self.save_regions_dict(f_regions_dict)

    def set_vol_for_all_audio_items(self, a_uid, a_vol, a_reverse=None, a_same_vol=False,
                                    a_old_vol=0):
        """ a_uid:  wav_pool uid
            a_vol:  dB
            a_reverse:  None=All, True=reversed-only, False=only-if-not-reversed
        """
        f_uid = int(a_uid)
        f_changed_any = False
        f_pysong = self.get_song()
        for f_region_uid in f_pysong.regions.values():
            f_audio_region = self.get_audio_region(f_region_uid)
            f_changed = False
            for f_audio_item in f_audio_region.items.values():
                if f_audio_item.uid == f_uid:
                    if a_reverse is None or \
                    (a_reverse and f_audio_item.reversed) or \
                    (not a_reverse and not f_audio_item.reversed):
                        if not a_same_vol or a_old_vol == f_audio_item.vol:
                            f_audio_item.vol = int(a_vol)
                            f_changed = True
            if f_changed:
                self.save_audio_region(f_region_uid, f_audio_region)
                f_changed_any = True
        if f_changed_any:
            self.commit("Changed volume for all audio items with uid {}".format(f_uid))

    def set_fades_for_all_audio_items(self, a_item):
        """ a_uid:  wav_pool uid
            a_item:  pydaw_audio_item
        """
        f_changed_any = False
        f_pysong = self.get_song()
        for f_region_uid in f_pysong.regions.values():
            f_audio_region = self.get_audio_region(f_region_uid)
            f_changed = False
            for f_audio_item in f_audio_region.items.values():
                if f_audio_item.uid == a_item.uid:
                    if a_item.reversed == f_audio_item.reversed and \
                    a_item.sample_start == f_audio_item.sample_start and \
                    a_item.sample_end == f_audio_item.sample_end:
                        f_audio_item.fade_in = a_item.fade_in
                        f_audio_item.fade_out = a_item.fade_out
                        f_audio_item.fadein_vol = a_item.fadein_vol
                        f_audio_item.fadeout_vol = a_item.fadeout_vol
                        f_changed = True
            if f_changed:
                self.save_audio_region(f_region_uid, f_audio_region)
                f_changed_any = True
        if f_changed_any:
            self.commit("Changed volume for all audio items with uid {}".format(a_item.uid))

    def set_output_for_all_audio_items(self, a_uid, a_index):
        """ a_uid:  wav_pool uid
            a_index:  output track
        """
        f_uid = int(a_uid)
        f_changed_any = False
        f_pysong = self.get_song()
        for f_region_uid in f_pysong.regions.values():
            f_audio_region = self.get_audio_region(f_region_uid)
            f_changed = False
            for f_audio_item in f_audio_region.items.values():
                if f_audio_item.uid == f_uid:
                    f_audio_item.output_track = int(a_index)
                    f_changed = True
            if f_changed:
                self.save_audio_region(f_region_uid, f_audio_region)
                f_changed_any = True
        if f_changed_any:
            self.commit("Changed output to {} for all "
                "audio items with uid {}".format(a_index, f_uid))

    def set_paif_for_all_audio_items(self, a_uid, a_paif):
        """ a_uid:  wav_pool uid
            a_paif:  a list that corresponds to a paif row
        """
        f_uid = int(a_uid)
        f_changed_any = False
        f_pysong = self.get_song()
        for f_region_uid in f_pysong.regions.values():
            f_audio_region = self.get_audio_region(f_region_uid)
            f_paif = self.get_audio_per_item_fx_region(f_region_uid)
            f_changed = False
            for f_index, f_audio_item in f_audio_region.items.items():
                if f_audio_item.uid == f_uid:
                    f_paif.set_row(f_index, a_paif)
                    self.save_audio_per_item_fx_region(f_region_uid, f_paif)
                    self.this_pydaw_osc.pydaw_audio_per_item_fx_region(f_region_uid)
                    f_changed = True
            if f_changed:
                self.save_audio_region(f_region_uid, f_audio_region)
                f_changed_any = True
        if f_changed_any:
            self.commit("Update per-audio-item-fx for all audio items with uid {}".format(f_uid))

    def get_item_string(self, a_item_uid):
        try:
            f_file = open("{}/{}".format(self.items_folder, a_item_uid), "r")
        except:
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_item_by_uid(self, a_item_uid):
        return pydaw_item.from_str(self.get_item_string(a_item_uid))

    def get_item_by_name(self, a_item_name):
        f_items_dict = self.get_items_dict()
        return pydaw_item.from_str(
            self.get_item_string(f_items_dict.get_uid_by_name(a_item_name)))

    def timestretch_lookup_orig_path(self, a_path):
        if a_path in self.timestretch_reverse_lookup:
            return self.timestretch_reverse_lookup[a_path]
        else:
            return a_path

    def timestretch_audio_item(self, a_audio_item):
        """ Return path, uid for a time-stretched audio item and update all project files,
        or None if the UID already exists in the cache"""
        a_audio_item.timestretch_amt = round(a_audio_item.timestretch_amt, 6)
        a_audio_item.pitch_shift = round(a_audio_item.pitch_shift, 6)
        a_audio_item.timestretch_amt_end = round(a_audio_item.timestretch_amt_end, 6)
        a_audio_item.pitch_shift_end = round(a_audio_item.pitch_shift_end, 6)

        f_src_path = self.get_wav_name_by_uid(a_audio_item.uid)
        if f_src_path in self.timestretch_reverse_lookup:
            f_src_path = self.timestretch_reverse_lookup[f_src_path]
        else:
            if (a_audio_item.timestretch_amt == 1.0 and \
            a_audio_item.pitch_shift == 0.0 and \
            a_audio_item.timestretch_amt_end == 1.0 and a_audio_item.pitch_shift_end == 0.0) or \
            (a_audio_item.time_stretch_mode == 1 and \
            a_audio_item.pitch_shift == a_audio_item.pitch_shift_end) or \
            (a_audio_item.time_stretch_mode == 2 and \
            a_audio_item.timestretch_amt == a_audio_item.timestretch_amt_end):
                return None  #Don't process if the file is not being stretched/shifted yet
        f_key = (a_audio_item.time_stretch_mode, a_audio_item.timestretch_amt,
                 a_audio_item.pitch_shift, a_audio_item.timestretch_amt_end,
                 a_audio_item.pitch_shift_end, a_audio_item.crispness, f_src_path)
        if f_key in self.timestretch_cache:
            a_audio_item.uid = self.timestretch_cache[f_key]
            return None
        else:
            f_uid = pydaw_gen_uid()
            f_dest_path = "{}/{}.wav".format(self.timestretch_folder, f_uid)

            f_cmd = None
            if a_audio_item.time_stretch_mode == 1:
                self.this_pydaw_osc.pydaw_pitch_env(f_src_path, f_dest_path,
                                                    a_audio_item.pitch_shift,
                                                    a_audio_item.pitch_shift_end)
                self.get_wav_uid_by_name(f_dest_path, a_uid=f_uid)  #add it to the pool
            elif a_audio_item.time_stretch_mode == 2:
                self.this_pydaw_osc.pydaw_rate_env(f_src_path, f_dest_path,
                                                   a_audio_item.timestretch_amt,
                                                   a_audio_item.timestretch_amt_end)
                self.get_wav_uid_by_name(f_dest_path, a_uid=f_uid)  #add it to the pool
            elif a_audio_item.time_stretch_mode == 3:
                f_cmd = [pydaw_rubberband_util, "-c", str(a_audio_item.crispness), "-t",
                         str(a_audio_item.timestretch_amt), "-p", str(a_audio_item.pitch_shift),
                         "-R", "--pitch-hq", f_src_path, f_dest_path]
            elif a_audio_item.time_stretch_mode == 4:
                f_cmd = [pydaw_rubberband_util, "-F", "-c", str(a_audio_item.crispness), "-t",
                         str(a_audio_item.timestretch_amt), "-p", str(a_audio_item.pitch_shift),
                         "-R", "--pitch-hq", f_src_path, f_dest_path]
            elif a_audio_item.time_stretch_mode == 5:
                f_cmd = [pydaw_sbsms_util, f_src_path, f_dest_path,
                         str(1.0 / a_audio_item.timestretch_amt),
                         str(1.0 / a_audio_item.timestretch_amt_end),
                         str(a_audio_item.pitch_shift), str(a_audio_item.pitch_shift_end) ]
            elif a_audio_item.time_stretch_mode == 6:
                if a_audio_item.pitch_shift != 0.0:
                    f_cmd = [pydaw_paulstretch_util,
                             "-s", str(a_audio_item.timestretch_amt), "-p",
                             str(a_audio_item.pitch_shift),
                             f_src_path, f_dest_path ]
                else:
                    f_cmd = [pydaw_paulstretch_util,
                             "-s", str(a_audio_item.timestretch_amt),
                             f_src_path, f_dest_path ]

            self.timestretch_cache[f_key] = f_uid
            self.timestretch_reverse_lookup[f_dest_path] = f_src_path
            a_audio_item.uid = self.timestretch_cache[f_key]

            if f_cmd is not None:
                print("Running {}".format(" ".join(f_cmd)))
                f_proc = subprocess.Popen(f_cmd)
                return f_dest_path, f_uid, f_proc
            else:
                return None

    def timestretch_get_orig_file_uid(self, a_uid):
        """ Return the UID of the original file """
        f_new_path = self.get_wav_path_by_uid(a_uid)
        if f_new_path in self.timestretch_reverse_lookup:
            f_old_path = self.timestretch_reverse_lookup[f_new_path]
            return self.get_wav_uid_by_name(f_old_path)
        else:
            print("\n####\n####\nWARNING:  timestretch_get_orig_file_uid could not find uid {}"
            "\n####\n####\n".format(a_uid))
            return a_uid

    def check_for_recorded_items(self, a_item_name):
        self.check_for_recorded_regions()
        f_item_name = "{}-".format(a_item_name)
        f_rec_items_file = "{}/recorded_items".format(self.project_folder)
        if os.path.isfile(f_rec_items_file):
            f_str_list = pydaw_read_file_text(f_rec_items_file).split("\n")
            os.remove(f_rec_items_file)
        else:
            return
        f_int_list = []
        for f_str in f_str_list:
            if f_str == "":
                break
            f_int = int(f_str)
            if not f_int in  f_int_list:
                f_int_list.append(f_int)
        f_int_list.sort()
        f_suffix = 1
        f_items_dict = self.get_items_dict()
        for f_int in f_int_list:
            f_item = self.get_item_by_uid(f_int)
            f_item.fix_overlaps()
            self.save_item_by_uid(f_int, f_item, a_new_item=True)
            while "{}{}".format(f_item_name, f_suffix) in f_items_dict.uid_lookup:
                f_suffix += 1
            f_items_dict.add_item(f_int, f_item_name + str(f_suffix))
            f_suffix += 1
        self.save_items_dict(f_items_dict)

    def check_for_recorded_regions(self):
        f_rec_items_file = "{}/recorded_regions".format(self.project_folder)
        if os.path.isfile(f_rec_items_file):
            f_str_list = pydaw_read_file_text(f_rec_items_file).split("\n")
            os.remove(f_rec_items_file)
        else:
            return
        f_int_list = []
        for f_str in f_str_list:
            if f_str == "":
                break
            f_int = int(f_str)
            if not f_int in  f_int_list:
                f_int_list.append(f_int)
        f_int_list.sort()
        f_suffix = 1
        f_regions_dict = self.get_regions_dict()
        for f_int in f_int_list:
            if not f_int in f_regions_dict.name_lookup:
                self.save_file(pydaw_folder_regions_audio, f_int, pydaw_terminating_char)
                while "recorded-{}".format(f_suffix) in f_regions_dict.uid_lookup:
                    f_suffix += 1
                f_regions_dict.add_item(f_int, "recorded-{}".format(f_suffix))
                f_suffix += 1
                f_old_text = ""
                f_existed = 0
            else:
                f_old_text = self.history.get_latest_version_of_file(pydaw_folder_regions, f_int)
                f_existed = 1
            self.history_files.append(
                pydaw_history.pydaw_history_file(pydaw_folder_regions, str(f_int),
            pydaw_read_file_text("{}/{}".format(self.regions_folder, f_int)),
                                 f_old_text, f_existed))

        self.save_regions_dict(f_regions_dict)
        f_old_text = self.history.get_latest_version_of_file("", pydaw_file_pysong)
        f_new_text = pydaw_read_file_text("{}/{}".format(self.project_folder, pydaw_file_pysong))
        if f_old_text is not None and f_new_text != f_old_text:
            print("Appending history file for pysong")
            self.history_files.append(pydaw_history.pydaw_history_file("", pydaw_file_pysong,
                                                                       f_new_text,
                                                                       f_old_text, 1))
        else:
            print("f_old_text: {}".format(f_old_text))

    def get_tracks_string(self):
        try:
            f_file = open("{}/default.pytracks".format(self.project_folder), "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_tracks(self):
        return pydaw_tracks.from_str(self.get_tracks_string())

    def get_bus_tracks_string(self):
        try:
            f_file = open("{}/default.pybus".format(self.project_folder), "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_bus_tracks(self):
        return pydaw_busses.from_str(self.get_bus_tracks_string())

    def get_audio_tracks_string(self):
        try:
            f_file = open("{}/default.pyaudio".format(self.project_folder), "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_audio_tracks(self):
        return pydaw_audio_tracks.from_str(self.get_audio_tracks_string())

    def get_audio_region_string(self, a_region_uid):
        f_file = open("{}/{}".format(self.regions_audio_folder, a_region_uid), "r")
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_audio_region(self, a_region_uid):
        return pydaw_audio_region.from_str(self.get_audio_region_string(a_region_uid))

    def get_audio_per_item_fx_region(self, a_region_uid):
        f_path = "{}/{}".format(self.audio_per_item_fx_folder, a_region_uid)
        #TODO:  Sort this out at PyDAWv4 and create an empty file first
        if not os.path.isfile(f_path):
            return pydaw_audio_item_fx_region()
        else:
            f_text = pydaw_read_file_text(f_path)
            return pydaw_audio_item_fx_region.from_str(f_text)

    def save_audio_per_item_fx_region(self, a_region_uid, a_paif, a_commit=True):
        if not self.suppress_updates:
            self.save_file(pydaw_folder_audio_per_item_fx, str(a_region_uid), str(a_paif))
            if a_commit:
                self.commit("Update per-audio-item effects")

    def get_sample_graph_by_name(self, a_path, a_uid_dict=None, a_cp=True):
        f_uid = self.get_wav_uid_by_name(a_path, a_cp=a_cp)
        return self.get_sample_graph_by_uid(f_uid)

    def get_sample_graph_by_uid(self, a_uid):
        f_pygraph_file = "{}/{}".format(self.samplegraph_folder, a_uid)
        f_result = pydaw_sample_graph.create(f_pygraph_file, self.samples_folder)
        if not f_result.is_valid(): # or not f_result.check_mtime():
            print("\n\nNot valid, or else mtime is newer than graph time, "
                  "deleting sample graph...\n")
            pydaw_remove_item_from_sg_cache(f_pygraph_file)
            self.create_sample_graph(self.get_wav_path_by_uid(a_uid), a_uid)
            return pydaw_sample_graph.create(f_pygraph_file, self.samples_folder)
        else:
            return f_result

    def delete_sample_graph_by_name(self, a_path):
        f_uid = self.get_wav_uid_by_name(a_path, a_cp=False)
        self.delete_sample_graph_by_uid(f_uid)

    def delete_sample_graph_by_uid(self, a_uid):
        f_pygraph_file = "{}/{}".format(self.samplegraph_folder, a_uid)
        pydaw_remove_item_from_sg_cache(f_pygraph_file)

    def get_wav_uid_by_name(self, a_path, a_uid_dict=None, a_uid=None, a_cp=True):
        """ Return the UID from the wav pool, or add to the pool if it does not exist """
        if a_uid_dict is None:
            f_uid_dict = self.get_wavs_dict()
        else:
            f_uid_dict = a_uid_dict
        f_path = str(a_path).replace("//", "/")
        if a_cp:
            self.cp_audio_file_to_cache(f_path)
        if f_uid_dict.name_exists(f_path):
            return f_uid_dict.get_uid_by_name(f_path)
        else:
            f_uid = f_uid_dict.add_new_item(f_path, a_uid)
            self.create_sample_graph(f_path, f_uid)
            self.save_wavs_dict(f_uid_dict)
            return f_uid

    def cp_audio_file_to_cache(self, a_file):
        if a_file in self.cached_audio_files:
            return
        f_cp_path = "{}{}".format(self.samples_folder, a_file)
        f_cp_dir = os.path.dirname(f_cp_path)
        if not os.path.isdir(f_cp_dir):
            os.makedirs(f_cp_dir)
        if not os.path.isfile(f_cp_path):
            f_cmd = "cp -f '{}' '{}'".format(a_file, f_cp_path)
            print(f_cmd)
            os.system(f_cmd)
        self.cached_audio_files.append(a_file)

    def get_wav_name_by_uid(self, a_uid, a_uid_dict=None):
        """ Return the UID from the wav pool, or add to the pool if it does not exist """
        if a_uid_dict is None:
            f_uid_dict = self.get_wavs_dict()
        else:
            f_uid_dict = a_uid_dict
        if f_uid_dict.uid_exists(a_uid):
            return f_uid_dict.get_name_by_uid(a_uid)
        else:
            raise Exception

    def get_wav_path_by_uid(self, a_uid):
        f_uid_dict = self.get_wavs_dict()
        return f_uid_dict.get_name_by_uid(a_uid)

    def create_sample_graph(self, a_path, a_uid):
        f_uid = int(a_uid)
        f_sample_dir_path = "{}{}".format(self.samples_folder, a_path)
        if os.path.isfile(a_path):
            f_path = a_path
        elif os.path.isfile(f_sample_dir_path):
            f_path = f_sample_dir_path
        else:
            raise Exception("Cannot create sample graph, the "
                "following do not exist:\n{}\n{}\n".format(
                a_path, f_sample_dir_path))

        # TODO:  This algorithm is somewhat screwed up in the C code,
        #  and this is a one-to-one port.  The f_peak_count and so on
        #  are not consistent with length, need to fix it.
        with wavefile.WaveReader(f_path) as f_reader:
            f_result = "meta|filename|{}\n".format(f_path)
            f_ts = int(datetime.datetime.now().strftime("%s"))
            f_result += "meta|timestamp|{}\n".format(f_ts)
            f_result += "meta|channels|{}\n".format(f_reader.channels)
            f_result += "meta|frame_count|{}\n".format(f_reader.frames)
            f_result += "meta|sample_rate|{}\n".format(
                int(f_reader.samplerate))
            f_length = float(f_reader.frames) / float(f_reader.samplerate)
            f_length = round(f_length, 6)
            f_result += "meta|length|{}\n".format(f_length)
            f_peak_count = int(f_length * 32.0)
            f_points = []
            f_count = 0
            for f_chunk in f_reader.read_iter(size=f_peak_count * 50):
                for f_i2 in range(50):
                    f_pos = f_i2 * f_peak_count
                    f_break = False
                    for f_i in range(f_chunk.shape[0]):
                        f_frame = f_chunk[f_i][f_pos:f_pos+f_peak_count]
                        if not len(f_frame):
                            f_break = True
                            continue
                        f_high = -1.0
                        f_low = 1.0
                        for f_i2 in range(0, f_frame.shape[0], 10):
                            f_val = f_frame[f_i2]
                            if f_val > f_high:
                                f_high = f_val
                            elif f_val < f_low:
                                f_low = f_val
                        f_high = round(float(f_high), 6)
                        f_points.append("p|{}|h|{}".format(f_i, f_high))
                        f_low = round(float(f_low), 6)
                        f_points.append("p|{}|l|{}".format(f_i, f_low))
                    f_count += 1
                    if f_break:
                        break
            f_result += "\n".join(f_points)
            f_result += "\nmeta|count|{}\n\\".format(f_count)
        self.this_pydaw_osc.pydaw_add_to_wav_pool(f_path, f_uid)
        f_pygraph_file = "{}/{}".format(self.samplegraph_folder, f_uid)
        with open(f_pygraph_file, "w") as f_handle:
            f_handle.write(f_result)

    def verify_history(self):
        self.flush_history()
        return self.history.verify_history()

    def get_transport(self):
        try:
            f_file = open("{}/default.pytransport".format(self.project_folder), "r")
        except:
            return pydaw_transport()  #defaults
        f_str = f_file.read()
        f_file.close()
        f_result = pydaw_transport.from_str(f_str)
        f_file_name = "{}/default.pymididevice".format(self.project_folder)
        if os.path.isfile(f_file_name):
            f_file = open(f_file_name)
            f_result.midi_keybd = f_file.read()
            f_file.close()
        return f_result

    def save_transport(self, a_transport):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pytransport, str(a_transport))

    def create_empty_region(self, a_region_name):
        #TODO:  Check for uniqueness, from a pydaw_project.check_for_uniqueness method...
        f_regions_dict = self.get_regions_dict()
        f_uid = f_regions_dict.add_new_item(a_region_name)
        self.save_file(pydaw_folder_regions, f_uid, pydaw_terminating_char)
        self.save_file(pydaw_folder_regions_audio, f_uid, pydaw_terminating_char)
        self.save_regions_dict(f_regions_dict)
        return f_uid

    def create_empty_item(self, a_item_name):
        f_items_dict = self.get_items_dict()
        f_uid = f_items_dict.add_new_item(a_item_name)
        self.save_file(pydaw_folder_items, str(f_uid), pydaw_terminating_char)
        self.this_pydaw_osc.pydaw_save_item(f_uid)
        self.save_items_dict(f_items_dict)
        return f_uid

    def copy_region(self, a_old_region_name, a_new_region_name):
        f_regions_dict = self.get_regions_dict()
        f_uid = f_regions_dict.add_new_item(a_new_region_name)
        f_old_uid = f_regions_dict.get_uid_by_name(a_old_region_name)
        self.save_file(pydaw_folder_regions,  str(f_uid),
                       pydaw_read_file_text("{}/{}".format(self.regions_folder, f_old_uid)))
        self.save_file(pydaw_folder_regions_audio,  str(f_uid),
                       pydaw_read_file_text("{}/{}".format(self.regions_audio_folder, f_old_uid)))
        f_paif_file = "{}/{}".format(self.audio_per_item_fx_folder, f_old_uid)
        if os.path.isfile(f_paif_file):
            self.save_file(pydaw_folder_audio_per_item_fx, str(f_uid),
                           pydaw_read_file_text(f_paif_file))
        self.save_regions_dict(f_regions_dict)
        return f_uid

    def region_audio_clone(self, a_dest_region_uid, a_src_region_name):
        f_regions_dict = self.get_regions_dict()
        f_uid = f_regions_dict.get_uid_by_name(a_src_region_name)
        self.save_file(pydaw_folder_regions_audio, str(a_dest_region_uid),
                       pydaw_read_file_text("{}/{}".format(self.regions_audio_folder, f_uid)))
        f_paif_file = "{}/{}".format(self.audio_per_item_fx_folder, f_uid)
        if os.path.isfile(f_paif_file):
            self.save_file(pydaw_folder_audio_per_item_fx, str(a_dest_region_uid),
                           pydaw_read_file_text(f_paif_file))
            self.this_pydaw_osc.pydaw_audio_per_item_fx_region(a_dest_region_uid)
        self.this_pydaw_osc.pydaw_reload_audio_items(a_dest_region_uid)
        self.commit("Clone audio from region {}".format(a_src_region_name))

    def copy_item(self, a_old_item, a_new_item):
        f_items_dict = self.get_items_dict()
        f_uid = f_items_dict.add_new_item(a_new_item)
        f_old_uid = f_items_dict.get_uid_by_name(a_old_item)
        self.save_file(pydaw_folder_items,  str(f_uid), pydaw_read_file_text(
            "{}/{}".format(self.items_folder, f_old_uid)))
        self.this_pydaw_osc.pydaw_save_item(f_uid)
        self.save_items_dict(f_items_dict)
        return f_uid

    def save_item(self, a_name, a_item):
        if not self.suppress_updates:
            f_items_dict = self.get_items_dict()
            f_uid = f_items_dict.get_uid_by_name(a_name)
            self.save_file(pydaw_folder_items, str(f_uid), str(a_item))
            self.this_pydaw_osc.pydaw_save_item(f_uid)

    def save_item_by_uid(self, a_uid, a_item, a_new_item=False):
        if not self.suppress_updates:
            f_uid = int(a_uid)
            self.save_file(pydaw_folder_items, str(f_uid), str(a_item), a_new_item)
            self.this_pydaw_osc.pydaw_save_item(f_uid)

    def save_region(self, a_name, a_region):
        if not self.suppress_updates:
            f_regions_dict = self.get_regions_dict()
            f_uid = f_regions_dict.get_uid_by_name(a_name)
            self.save_file(pydaw_folder_regions, str(f_uid), str(a_region))
            self.this_pydaw_osc.pydaw_save_region(f_uid)

    def save_song(self, a_song):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pysong, str(a_song))
            self.this_pydaw_osc.pydaw_save_song()

    def save_tracks(self, a_tracks):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pytracks, str(a_tracks))
            #Is there a need for a configure message here?

    def save_busses(self, a_tracks):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pybus, str(a_tracks))
            #Is there a need for a configure message here?

    def save_audio_tracks(self, a_tracks):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pyaudio, str(a_tracks))
            #Is there a need for a configure message here?

    def save_audio_inputs(self, a_tracks):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pyinput, str(a_tracks))

    def save_audio_region(self, a_region_uid, a_tracks):
        if not self.suppress_updates:
            self.save_file(pydaw_folder_regions_audio, str(a_region_uid), str(a_tracks))
            self.this_pydaw_osc.pydaw_reload_audio_items(a_region_uid)

    def item_exists(self, a_item_name, a_name_dict=None):
        if a_name_dict is None:
            f_name_dict = self.get_items_dict()
        else:
            f_name_dict = a_name_dict
        if str(a_item_name) in f_name_dict.uid_lookup:
            return True
        else:
            return False

    def get_next_default_item_name(self, a_item_name="item"):
        f_item_name = str(a_item_name)
        if f_item_name == "item":
            f_start = self.last_item_number
        else:
            f_start = 1
        f_items_dict = self.get_items_dict()
        for i in range(f_start, 10000):
            f_result = f_item_name + "-" + str(i)
            if not f_result in f_items_dict.uid_lookup:
                if f_item_name == "item":
                    self.last_item_number = i
                return f_result

    def get_next_default_region_name(self, a_region_name="region"):
        f_regions_dict = self.get_regions_dict()
        if str(a_region_name) != "region" and not str(a_region_name) in f_regions_dict.uid_lookup:
            return str(a_region_name)
        for i in range(self.last_region_number, 10000):
            f_result = str(a_region_name) + "-" + str(i)
            if not f_result in f_regions_dict.uid_lookup:
                if str(a_region_name) == "region":
                    self.last_region_number = i
                return f_result

    def get_item_list(self):
        f_result = self.get_items_dict()
        return sorted(f_result.uid_lookup.keys())

    def get_region_list(self):
        f_result = self.get_regions_dict()
        return sorted(f_result.uid_lookup.keys())

    def quit_handler(self):
        self.this_pydaw_osc.stop_server()
        self.flush_history()

    def check_audio_files(self):
        """ Verify that all audio files exist  """
        f_result = []
        f_regions = self.get_regions_dict()
        f_wav_pool = self.get_wavs_dict()
        f_to_delete = []
        f_commit = False
        for k, v in list(f_wav_pool.name_lookup.items()):
            if not os.path.isfile(v):
                f_to_delete.append(k)
        if len(f_to_delete) > 0:
            f_commit = True
            for f_key in f_to_delete:
                f_wav_pool.name_lookup.pop(f_key)
            self.save_wavs_dict(f_wav_pool)
            self.error_log_write("Removed missing audio item(s) from wav_pool")
        for f_uid in list(f_regions.uid_lookup.values()):
            f_to_delete = []
            f_region = self.get_audio_region(f_uid)
            for k, v in list(f_region.items.items()):
                if not f_wav_pool.uid_exists(v.uid):
                    f_to_delete.append(k)
            if len(f_to_delete) > 0:
                f_commit = True
                for f_key in f_to_delete:
                    f_region.remove_item(f_key)
                f_result += f_to_delete
                self.save_audio_region(f_uid, f_region)
                self.error_log_write("Removed missing audio item(s) from region {}".format(f_uid))
        if f_commit:
            self.commit("")
        return f_result

    def error_log_write(self, a_message):
        f_file = open("{}/error.log".format(self.project_folder), "a")
        f_file.write(a_message)
        f_file.close()

    def __init__(self, a_with_audio):
        self.last_item_number = 1
        self.last_region_number = 1
        self.history_files = []
        self.history_commits = []
        self.cached_audio_files = []
        self.history_undo_cursor = 0
        self.this_pydaw_osc = pydaw_osc(a_with_audio)
        self.glued_name_index = 0
        self.suppress_updates = False

class pydaw_song:
    def __init__(self):
        self.regions = {}

    def get_next_empty_pos(self):
        for f_i in range(300):
            if not f_i in self.regions:
                return f_i
        return None

    def get_index_of_region(self, a_uid):
        for k, v in list(self.regions.items()):
            if v == a_uid:
                return k
        assert(False)

    def shift(self, a_amt):
        f_result = {}
        for k, v in self.regions.items():
            f_index = k + a_amt
            if f_index >= 0 and f_index < 300:
                f_result[f_index] = v
        self.regions = f_result

    def insert_region(self, a_index, a_region_uid):
        f_new_dict = {}
        f_old_dict = {}
        for k, v in list(self.regions.items()):
            if k >= a_index:
                if k < 299:
                    f_new_dict[k + 1] = v
            else:
                f_old_dict[k] = v
        print("\n\n\n")
        for k, v in list(f_new_dict.items()):
            f_old_dict[k] = v
        print("\n\n\n")
        self.regions = f_old_dict
        self.regions[a_index] = a_region_uid

    def add_region_ref_by_name(self, a_pos, a_region_name, a_uid_dict):
        self.regions[int(a_pos)] = a_uid_dict.get_uid_by_name(a_region_name)
        #TODO:  Raise an exception if it doesn't exist...

    def add_region_ref_by_uid(self, a_pos, a_region_uid):
        self.regions[int(a_pos)] = int(a_region_uid)
        #TODO:  Raise an exception if it doesn't exist...

    def get_region_names(self, a_uid_dict):
        f_result = {}
        for k, v in list(self.regions.items()):
            f_result[k] = a_uid_dict.get_name_by_uid(v)
        return f_result

    def remove_region_ref(self, a_pos):
        if a_pos in self.regions:
            del self.regions[a_pos]

    def __str__(self):
        f_result = ""
        for k, v in list(self.regions.items()):
            f_result += "{}|{}\n".format(k, v)
        f_result += pydaw_terminating_char
        return f_result
    @staticmethod
    def from_str(a_str):
        f_result = pydaw_song()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == pydaw_terminating_char:
                break
            else:
                f_region = f_line.split("|")
                f_result.add_region_ref_by_uid(f_region[0], f_region[1])
        return f_result

class pydaw_name_uid_dict:
    def gen_file_name_uid(self):
        f_result = random.randint(1000000, 9999999)
        while f_result in self.name_lookup:
            f_result = random.randint(1000000, 9999999)
        return f_result

    def __init__(self):
        self.name_lookup = {}
        self.uid_lookup = {}

    def add_item(self, a_uid, a_name):
        self.name_lookup[a_uid] = str(a_name)
        self.uid_lookup[a_name] = int(a_uid)

    def add_new_item(self, a_name, a_uid=None):
        if a_name in self.uid_lookup:
            raise Exception
        if a_uid is None:
            f_uid = self.gen_file_name_uid()
            while self.uid_exists(f_uid):
                f_uid = self.gen_file_name_uid()
        else:
            f_uid = a_uid
        self.add_item(f_uid, a_name)
        return f_uid

    def get_uid_by_name(self, a_name):
        return self.uid_lookup[str(a_name)]

    def get_name_by_uid(self, a_uid):
        return self.name_lookup[int(a_uid)]

    def rename_item(self, a_old_name, a_new_name):
        f_uid = self.get_uid_by_name(a_old_name)
        f_new_name = str(a_new_name)
        f_old_name = self.name_lookup[f_uid]
        self.uid_lookup.pop(f_old_name)
        self.add_item(f_uid, f_new_name)

    def uid_exists(self, a_uid):
        return int(a_uid) in self.name_lookup

    def name_exists(self, a_name):
        return str(a_name) in self.uid_lookup

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_name_uid_dict()
        f_lines = a_str.split("\n")
        for f_line in f_lines:
            if f_line == pydaw_terminating_char:
                break
            f_arr = f_line.split("|", 1)
            f_uid = int(f_arr[0])
            f_name = f_arr[1]
            f_result.add_item(f_uid, f_name)
        return f_result

    def __str__(self):
        f_result = []
        for k in sorted(self.name_lookup.keys()):
            v = self.name_lookup[k]
            f_result.append("|".join((str(k), v)))
        f_result.append(pydaw_terminating_char)
        return "\n".join(f_result)

class pydaw_region:
    def __init__(self, a_uid):
        self.items = []
        self.uid = a_uid
        self.region_length_bars = 0  #0 == default length for project

    def split(self, a_index, a_new_uid):
        f_region0 = pydaw_region(self.uid)
        f_region1 = pydaw_region(a_new_uid)
        for f_item in self.items:
            if f_item.bar_num >= a_index:
                f_item.bar_num -= a_index
                f_region1.items.append(f_item)
            else:
                f_region0.items.append(f_item)
        if self.region_length_bars == 0:
            f_length = 8
        else:
            f_length = self.region_length_bars
        f_region0.region_length_bars = a_index
        f_region1.region_length_bars = f_length - a_index
        return f_region0, f_region1

    def add_item_ref_by_name(self, a_track_num, a_bar_num, a_item_name, a_uid_dict):
        f_item_uid = a_uid_dict.get_uid_by_name(a_item_name)
        self.add_item_ref_by_uid(a_track_num, a_bar_num, f_item_uid)

    def add_item_ref_by_uid(self, a_track_num, a_bar_num, a_item_uid):
        self.remove_item_ref(a_track_num, a_bar_num)
        self.items.append(pydaw_region.region_item(a_track_num, a_bar_num, int(a_item_uid)))

    def remove_item_ref(self, a_track_num, a_bar_num):
        for f_item in self.items:
            if f_item.bar_num == a_bar_num and f_item.track_num == a_track_num:
                self.items.remove(f_item)
                print("remove_item_ref removed bar: {}, track: {}".format(f_item.bar_num,
                      f_item.track_num))

    def __str__(self):
        f_result = ""
        if self.region_length_bars > 0:
            f_result += "L|{}|0\n".format(self.region_length_bars)
        for f_item in self.items:
            f_result += "{}|{}|{}\n".format(f_item.track_num, f_item.bar_num, f_item.item_uid)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_uid, a_str):
        f_result = pydaw_region(a_uid)
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == pydaw_terminating_char:
                break
            else:
                f_item_arr = f_line.split("|")
                if f_item_arr[0] == "L":
                    f_result.region_length_bars = int(f_item_arr[1])
                    continue
                f_result.add_item_ref_by_uid(int(f_item_arr[0]),
                                             int(f_item_arr[1]), f_item_arr[2])
        return f_result

    class region_item:
        def __init__(self, a_track_num, a_bar_num, a_item_uid):
            self.track_num = a_track_num
            self.bar_num = a_bar_num
            self.item_uid = a_item_uid

def pydaw_smooth_automation_points(a_items_list, a_is_cc, a_plugin_index=0, a_cc_num=-1):
    if a_is_cc:
        f_this_cc_arr = []
        f_beat_offset = 0.0
        f_index = 0
        f_cc_num = int(a_cc_num)
        f_result_arr = []
        for f_item in a_items_list:
            for f_cc in f_item.ccs:
                if f_cc.cc_num == f_cc_num:
                    f_new_cc = pydaw_cc((f_cc.start + f_beat_offset), a_plugin_index,
                                        f_cc_num, f_cc.cc_val)
                    f_new_cc.item_index = f_index
                    f_new_cc.beat_offset = f_beat_offset
                    f_this_cc_arr.append(f_new_cc)
            f_beat_offset += 4.0
            f_index += 1
            f_result_arr.append([])

        f_result_arr_len = len(f_result_arr)
        f_this_cc_arr.sort()
        for i in range(len(f_this_cc_arr) - 1):
            f_val_diff = abs(f_this_cc_arr[i + 1].cc_val - f_this_cc_arr[i].cc_val)
            if f_val_diff == 0:
                continue
            f_time_inc = .0625  #1/16 of a beat
            f_start = f_this_cc_arr[i].start + f_time_inc

            f_start_diff = f_this_cc_arr[i + 1].start - f_this_cc_arr[i].start
            if f_start_diff == 0.0:
                continue

            f_inc = (f_val_diff / (f_start_diff * 16.0))
            if (f_this_cc_arr[i].cc_val) > (f_this_cc_arr[i + 1].cc_val):
                f_inc *= -1.0
            f_new_val = f_this_cc_arr[i].cc_val + f_inc
            while True:
                f_index_offset = 0
                f_adjusted_start = f_start - f_this_cc_arr[i].beat_offset
                while f_adjusted_start >= 4.0:
                    f_index_offset += 1
                    f_adjusted_start -= 4.0
                f_interpolated_cc = pydaw_cc(f_adjusted_start, a_plugin_index,
                                             f_cc_num, f_new_val)
                f_new_val += f_inc
                f_new_index = f_this_cc_arr[i].item_index + f_index_offset
                if f_new_index >= f_result_arr_len:
                    print("Error, {} >= {}".format(f_new_index, f_result_arr_len))
                    break
                f_result_arr[f_new_index].append(f_interpolated_cc)
                f_start += f_time_inc
                if f_start >= (f_this_cc_arr[i + 1].start - 0.0625):
                    break
        for f_i in range(len(a_items_list)):
            a_items_list[f_i].ccs += f_result_arr[f_i]
            a_items_list[f_i].ccs.sort()
    else:
        f_this_pb_arr = []
        f_beat_offset = 0.0
        f_index = 0
        f_result_arr = []
        for f_item in a_items_list:
            for f_pb in f_item.pitchbends:
                f_new_pb = pydaw_pitchbend(f_pb.start + f_beat_offset, f_pb.pb_val)
                f_new_pb.item_index = f_index
                f_new_pb.beat_offset = f_beat_offset
                f_this_pb_arr.append(f_new_pb)
            f_beat_offset += 4.0
            f_index += 1
            f_result_arr.append([])
        f_result_arr_len = len(f_result_arr)
        for i in range(len(f_this_pb_arr) - 1):
            f_val_diff = abs(f_this_pb_arr[i + 1].pb_val - f_this_pb_arr[i].pb_val)
            if f_val_diff == 0.0:
                continue
            f_time_inc = 0.0625
            f_start = f_this_pb_arr[i].start + f_time_inc
            f_start_diff = f_this_pb_arr[i + 1].start - f_this_pb_arr[i].start
            if f_start_diff == 0.0:
                continue
            f_val_inc = f_val_diff / (f_start_diff * 16.0)
            if f_this_pb_arr[i].pb_val > f_this_pb_arr[i + 1].pb_val:
                f_val_inc *= -1.0
            f_new_val = f_this_pb_arr[i].pb_val + f_val_inc

            while True:
                f_index_offset = 0
                f_adjusted_start = f_start - f_this_pb_arr[i].beat_offset
                while f_adjusted_start >= 4.0:
                    f_index_offset += 1
                    f_adjusted_start -= 4.0
                f_interpolated_pb = pydaw_pitchbend(f_adjusted_start, f_new_val)
                f_new_val += f_val_inc
                f_new_index = f_this_pb_arr[i].item_index + f_index_offset
                if f_new_index >= f_result_arr_len:
                    print("Error, {} >= {}".format(f_new_index, f_result_arr_len))
                    break
                f_result_arr[f_new_index].append(f_interpolated_pb)
                f_start += f_time_inc
                if f_start >= (f_this_pb_arr[i + 1].start - 0.0625):
                    break
        for f_i in range(len(a_items_list)):
            a_items_list[f_i].pitchbends += f_result_arr[f_i]
            a_items_list[f_i].pitchbends.sort()

def pydaw_velocity_mod(a_items, a_amt, a_line=False, a_end_amt=127,
                       a_add=False, a_selected_only=False):
    f_start_beat = 0.0
    f_range_beats = 0.0
    f_tmp_index = 0
    f_break = False
    f_result = []

    for f_item in a_items:
        for note in f_item.notes:
            if not a_selected_only or (a_selected_only and note.is_selected):
                f_start_beat = note.start + (f_tmp_index * 4.0)
                f_break = True
                break
        if f_break:
            break
        f_tmp_index += 1
    f_tmp_index = len(a_items) - 1
    f_break = False
    for f_item in reversed(a_items):
        for note in reversed(f_item.notes):
            if not a_selected_only or note.is_selected:
                f_range_beats = note.start + (4.0 * f_tmp_index) - f_start_beat
                f_break = True
                break
        if f_break:
            break
        f_tmp_index -= 1

    f_beat_offset = 0.0
    for f_index, f_item in zip(range(len(a_items)), a_items):
        for note in f_item.notes:
            if a_selected_only and not note.is_selected:
                continue
            if a_line and f_range_beats != 0.0:
                f_frac = ((note.start + f_beat_offset - f_start_beat) / f_range_beats)
                f_value = int(((a_end_amt - a_amt) * f_frac) + a_amt)
            else:
                f_value = int(a_amt)
            if a_add:
                note.velocity += f_value
            else:
                note.velocity = f_value
            if note.velocity > 127:
                note.velocity = 127
            elif note.velocity < 1:
                note.velocity = 1
            f_result.append("{}|{}".format(f_index, note))
        f_beat_offset += 4.0
    return f_result


class pydaw_item:
    def add_note(self, a_note, a_check=True):
        if a_check:
            for note in self.notes:
                if note.overlaps(a_note):
                    #TODO:  return -1 instead of True, and the offending editor_index when False
                    return False
        self.notes.append(a_note)
        self.notes.sort()
        if not a_check:
            self.fix_overlaps()
        return True

    def remove_note(self, a_note):
        try:
            self.notes.remove(a_note)
        except Exception as ex:
            print("\n\n\nException in remove_note:\n{}".format(ex))
            print((repr(traceback.extract_stack())))
            print("\n\n\n")

    def velocity_mod(self, a_amt, a_start_beat=0.0, a_end_beat=4.0, a_line=False,
                     a_end_amt=127, a_add=False, a_notes=None):
        """ velocity_mod
        (self, a_amt, #The amount to add or subtract
         a_start_beat=0.0, #modify values with a start at >= this, and...
         a_end_beat=4.0, # <= to this.
         a_line=False, # draw a line to a_end, otherwise all events are modified by a_amt
         a_end_amt=127, #not used unless a_line=True
         a_add=False, #True to add/subtract from each value, False to assign
         a_notes=None) #Process all notes if None, or selected if a list of notes is provided

         Modify the velocity of a range of notes
         """
        f_notes = []

        if a_notes is None:
            f_notes = self.notes
        else:
            for f_note in a_notes:
                for f_note2 in self.notes:
                    if f_note2 == f_note:
                        f_notes.append(f_note2)
                        break

        f_range_beats = a_end_beat - a_start_beat

        for note in f_notes:
            if note.start >= a_start_beat and note.start <= a_end_beat:
                if a_line:
                    f_frac = ((note.start - a_start_beat)/f_range_beats)
                    f_value = int(((a_end_amt - a_amt) * f_frac) + a_amt)
                else:
                    f_value = int(a_amt)
                if a_add:
                    note.velocity += f_value
                else:
                    note.velocity = f_value
                if note.velocity > 127:
                    note.velocity = 127
                elif note.velocity < 1:
                    note.velocity = 1

    def quantize(self, a_beat_frac, a_events_move_with_item=False,
                 a_notes=None, a_selected_only=False, a_index=0):
        f_notes = []
        f_ccs = []
        f_pbs = []

        f_result = []

        if a_notes is None:
            f_notes = self.notes
            f_ccs = self.ccs
            f_pbs = self.pitchbends
        else:
            for i in range(len(a_notes)):
                for f_note in self.notes:
                    if f_note == a_notes[i]:
                        if a_events_move_with_item:
                            f_start = f_note.start
                            f_end = f_note.start + f_note.length
                            for f_cc in self.ccs:
                                if f_cc.start >= f_start and f_cc.start <= f_end:
                                    f_ccs.append(f_cc)
                            for f_pb in self.pitchbends:
                                if f_pb.start >= f_start and f_pb.start <= f_end:
                                    f_pbs.append(f_pb)
                        f_notes.append(f_note)
                        break

        f_quantized_value = bar_frac_text_to_float(a_beat_frac)
        f_quantize_multiple = 1.0/f_quantized_value

        for note in f_notes:
            if a_selected_only and not note.is_selected:
                continue
            f_new_start = round(note.start * f_quantize_multiple) * f_quantized_value
            note.start = f_new_start
            shift_adjust = note.start - f_new_start
            f_new_length = round(note.length * f_quantize_multiple) * f_quantized_value
            if f_new_length == 0.0:
                f_new_length = f_quantized_value
            note.length = f_new_length
            f_result.append("{}|{}".format(a_index, note))

        self.fix_overlaps()

        if a_events_move_with_item:
            for cc in f_ccs:
                cc.start -= shift_adjust
            for pb in f_pbs:
                pb.start -= shift_adjust

        return f_result

    def transpose(self, a_semitones, a_octave=0, a_notes=None,
                  a_selected_only=False, a_duplicate=False, a_index=0):
        f_total = a_semitones + (a_octave * 12)
        f_notes = []
        f_result = []

        if a_notes is None:
            f_notes = self.notes
        else:
            for i in range(len(a_notes)):
                for f_note in self.notes:
                    if f_note == a_notes[i]:
                        f_notes.append(f_note)
                        break
        if a_duplicate:
            f_duplicates = []
        for note in f_notes:
            if a_selected_only and not note.is_selected:
                continue
            if a_duplicate:
                f_duplicates.append(pydaw_note.from_str(str(note)))
            note.note_num += f_total
            if note.note_num < 0:
                note.note_num = 0
            elif note.note_num > 127:
                note.note_num = 127
            f_result.append("{}|{}".format(a_index, note))
        if a_duplicate:
            self.notes += f_duplicates
            self.notes.sort()
        return f_result

    def fix_overlaps(self):
        """ Truncate the lengths of any notes that overlap the start of another note """
        f_to_delete = []
        for f_note in self.notes:
            if f_note not in f_to_delete:
                for f_note2 in self.notes:
                    if f_note != f_note2 and f_note2 not in f_to_delete:
                        if f_note.note_num == f_note2.note_num:
                            if f_note2.start == f_note.start:
                                if f_note2.length == f_note.length:
                                    f_to_delete.append(f_note2)
                                elif f_note2.length > f_note.length:
                                    f_note2.length = f_note2.length - f_note.length
                                    f_note2.start = f_note.end
                                    f_note2.set_end()
                                else:
                                    f_note.length = f_note.length - f_note2.length
                                    f_note.start = f_note2.end
                                    f_note.set_end()
                            elif f_note2.start > f_note.start:
                                if f_note.end > f_note2.start:
                                    f_note.length = f_note2.start - f_note.start
                                    f_note.set_end()
        for f_note in self.notes:
            if f_note.length < pydaw_min_note_length:
                f_to_delete.append(f_note)
        for f_note in f_to_delete:
            self.notes.remove(f_note)

    def get_next_default_note(self):
        pass

    def add_cc(self, a_cc):
        if a_cc in self.ccs:
            return False
        self.ccs.append(a_cc)
        self.ccs.sort()
        return True

    def remove_cc(self, a_cc):
        self.ccs.remove(a_cc)

    def remove_cc_range(self, a_cc_num, a_start_beat=0.0, a_end_beat=4.0):
        """ Delete all pitchbends greater than a_start_beat and less than a_end_beat """
        f_ccs_to_delete = []
        for cc in self.ccs:
            if cc.cc_num == a_cc_num and cc.start >= a_start_beat and cc.start <= a_end_beat:
                f_ccs_to_delete.append(cc)
        for cc in f_ccs_to_delete:
            self.remove_cc(cc)

    #TODO:  A maximum number of events per line?
    def draw_cc_line(self, a_cc, a_start, a_start_val, a_end, a_end_val, a_curve=0):
        f_cc = int(a_cc)
        f_start = float(a_start)
        f_start_val = int(a_start_val)
        f_end = float(a_end)
        f_end_val = int(a_end_val)
        #Remove any events that would overlap
        self.remove_cc_range(f_cc, f_start, f_end)

        f_start_diff = f_end - f_start
        f_val_diff = abs(f_end_val - f_start_val)
        if f_start_val > f_end_val:
            f_inc = -1
        else:
            f_inc = 1
        f_time_inc = abs(f_start_diff / float(f_val_diff))
        for i in range(0, (f_val_diff + 1)):
            self.ccs.append(pydaw_cc(f_start, f_cc, f_start_val))
            f_start_val += f_inc
            f_start += f_time_inc
        self.ccs.sort()

    def add_pb(self, a_pb):
        if a_pb in self.pitchbends:
            return False
        self.pitchbends.append(a_pb)
        self.pitchbends.sort()
        return True

    def remove_pb(self, a_pb):
        self.pitchbends.remove(a_pb)

    def remove_pb_range(self, a_start_beat=0.0, a_end_beat=4.0):
        """ Delete all pitchbends greater than a_start_beat and less than a_end_beat """
        f_pbs_to_delete = []
        for pb in self.pitchbends:
            if pb.start >= a_start_beat and pb.start <= a_end_beat:
                f_pbs_to_delete.append(pb)
        for pb in f_pbs_to_delete:
            self.remove_pb(pb)

    def draw_pb_line(self, a_start, a_start_val, a_end, a_end_val, a_curve=0):
        f_start = float(a_start)
        f_start_val = float(a_start_val)
        f_end = float(a_end)
        f_end_val = float(a_end_val)
        #Remove any events that would overlap
        self.remove_pb_range(f_start, f_end)

        f_start_diff = f_end - f_start
        f_val_diff = abs(f_end_val - f_start_val)
        if f_start_val > f_end_val:
            f_inc = -0.025
        else:
            f_inc = 0.025
        f_time_inc = abs(f_start_diff/(float(f_val_diff) * 40.0))
        for i in range(0, int((f_val_diff * 40) + 1)):
            self.pitchbends.append(pydaw_pitchbend(f_start, f_start_val))
            f_start_val += f_inc
            f_start += f_time_inc
        #Ensure that the last value is what the user wanted it to be
        self.pitchbends[(len(self.pitchbends) - 1)].pb_val = f_end_val
        self.pitchbends.sort()

    def get_next_default_cc(self):
        pass

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_item()
        f_arr = a_str.split("\n")
        for f_event_str in f_arr:
            if f_event_str == pydaw_terminating_char:
                break
            else:
                f_event_arr = f_event_str.split("|")
                if f_event_arr[0] == "n":
                    f_result.add_note(pydaw_note.from_arr(f_event_arr[1:]))
                elif f_event_arr[0] == "c":
                    f_result.add_cc(pydaw_cc.from_arr(f_event_arr[1:]))
                elif f_event_arr[0] == "p":
                    f_result.add_pb(pydaw_pitchbend.from_arr(f_event_arr[1:]))
        return f_result

    def __init__(self):
        self.notes = []
        self.ccs = []
        self.pitchbends = []

    def __str__(self):
        f_result = ""
        f_list = self.notes + self.ccs + self.pitchbends
        f_list.sort()

        for f_event in f_list:
            f_result += str(f_event)

        f_result += pydaw_terminating_char
        return f_result

class pydaw_abstract_midi_event:
    """ Allows inheriting classes to be sorted by .start variable
    , which is left to the iheriter's to implement"""
    def __lt__(self, other):
        return self.start < other.start

class pydaw_note(pydaw_abstract_midi_event):
    def __eq__(self, other):
        return((self.start == other.start) and (self.note_num == other.note_num) and \
        (self.length == other.length) and (self.velocity == other.velocity))

    def set_start(self, a_start):
        self.start = float(a_start)
        self.set_end()

    def set_length(self, a_length):
        self.length = float(a_length)
        self.set_end()

    def set_end(self):
        self.end = self.length + self.start

    def __init__(self, a_start, a_length, a_note_number, a_velocity):
        self.start = float(a_start)
        self.length = float(a_length)
        self.velocity = int(a_velocity)
        self.note_num = int(a_note_number)
        self.is_selected = False
        self.set_end()

    def overlaps(self, other):
        if self.note_num == other.note_num:
            if other.start >= self.start and other.start < self.end:
                return True
            elif other.start < self.start and other.end > self.start:
                return True
        return False

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_note(*a_arr)
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_note.from_arr(f_arr[1:])

    def __str__(self):
        return "{}\n".format("|".join(map(proj_file_str,
            ("n", self.start, self.length, self.note_num, self.velocity))))

class pydaw_cc(pydaw_abstract_midi_event):
    def __eq__(self, other):
        return ((self.start == other.start) and
        (self.cc_num == other.cc_num) and (self.cc_val == other.cc_val))

    def __init__(self, a_start, a_plugin_index, a_port_num, a_cc_val):
        self.start = float(a_start)
        self.plugin_index = int(a_plugin_index)
        self.cc_num = int(a_port_num) #This is really port_num, I'll rename later...
        self.cc_val = float(a_cc_val)

    def set_val(self, a_val):
        self.cc_val = pydaw_clip_value(float(a_val), 0.0, 127.0, True)

    def __str__(self):
        return "{}\n".format("|".join(map(proj_file_str,
            ("c", self.start, self.plugin_index, self.cc_num, self.cc_val))))

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_cc(*a_arr)
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_cc.from_arr(f_arr[1:])

    def clone(self):
        return pydaw_cc.from_str(str(self))

class pydaw_pitchbend(pydaw_abstract_midi_event):
    def __eq__(self, other):
        #TODO:  get rid of the pb_val comparison?
        return ((self.start == other.start) and (self.pb_val == other.pb_val))

    def __init__(self, a_start, a_pb_val):
        self.start = float(a_start)
        self.pb_val = float(a_pb_val)

    def set_val(self, a_val):
        self.pb_val = pydaw_clip_value(float(a_val), -1.0, 1.0, True)

    def __str__(self):
        return "{}\n".format("|".join(map(proj_file_str, ("p", self.start, self.pb_val))))

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_pitchbend(*a_arr)
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_pitchbend.from_arr(f_arr[1:])

    def clone(self):
        return pydaw_pitchbend.from_str(str(self))

class pydaw_tracks:
    def add_track(self, a_index, a_track):
        self.tracks[int(a_index)] = a_track

    def __init__(self):
        self.tracks = {}

    def __str__(self):
        f_result = ""
        for k, v in list(self.tracks.items()):
            f_result += "{}|{}".format(k, v)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_tracks()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if not f_line == pydaw_terminating_char:
                f_line_arr = f_line.split("|")
                f_result.add_track(f_line_arr[0], pydaw_track(*f_line_arr[1:]))
        return f_result

class pydaw_abstract_track:
    def set_track_pos(self, a_track_pos):
        self.track_pos = int(a_track_pos)
        assert(self.track_pos >= 0)

    def set_vol(self, a_vol):
        self.vol = int(a_vol)


class pydaw_track(pydaw_abstract_track):
    def __init__(self, a_solo=False, a_mute=False, a_vol=0, a_name="track",
                 a_inst=0, a_bus_num=0, a_track_pos=-1):
        self.name = str(a_name)
        self.solo = int_to_bool(a_solo)
        self.mute = int_to_bool(a_mute)
        self.set_vol(a_vol)
        self.inst = int(a_inst)
        self.bus_num = int(a_bus_num)
        self.set_track_pos(a_track_pos)

    def __str__(self):
        return "{}\n".format("|".join(map(proj_file_str,
            (bool_to_int(self.solo), bool_to_int(self.mute), self.vol,
            self.name, self.inst, self.bus_num, self.track_pos))))

class pydaw_busses:
    def add_bus(self, a_index, a_bus):
        self.busses[int(a_index)] = a_bus

    def add_bus_from_str(self, a_str):
        f_arr = a_str.split("|")
        self.add_bus(f_arr[0], pydaw_bus(*f_arr[1:]))

    def __init__(self):
        self.busses = {}

    def __str__(self):
        f_result = ""
        for k, f_bus in list(self.busses.items()):
            f_result += "{}|{}".format(k, f_bus)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_busses()
        f_lines = a_str.split("\n")
        for f_line in f_lines:
            if f_line == pydaw_terminating_char:
                return f_result
            f_result.add_bus_from_str(f_line)
        return f_result

class pydaw_bus(pydaw_abstract_track):
    def __init__(self, a_vol=0, a_track_pos=-1):
        self.vol = int(a_vol)
        self.set_track_pos(a_track_pos)

    def __str__(self):
        return "{}|{}\n".format(self.vol, self.track_pos)

class pydaw_audio_tracks:
    def add_track(self, a_index, a_track):
        self.tracks[int(a_index)] = a_track

    def __init__(self):
        self.tracks = {}

    def __str__(self):
        f_result = ""
        for k, v in list(self.tracks.items()):
            f_result += "{}|{}".format(k, v)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_audio_tracks()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if not f_line == pydaw_terminating_char:
                f_line_arr = f_line.split("|")
                f_result.add_track(f_line_arr[0], pydaw_audio_track(*f_line_arr[1:]))
        return f_result

class pydaw_audio_track(pydaw_abstract_track):
    def __init__(self, a_solo=False, a_mute=False, a_vol=0, a_name="track",
                 a_bus_num=0, a_track_pos=-1):
        self.name = str(a_name)
        self.solo = int_to_bool(a_solo)
        self.mute = int_to_bool(a_mute)
        self.set_vol(a_vol)
        self.bus_num = int(a_bus_num)
        self.set_track_pos(a_track_pos)

    def __str__(self):
        return "{}\n".format("|".join(map(proj_file_str,
            (bool_to_int(self.solo), bool_to_int(self.mute), self.vol,
             self.name, self.bus_num, self.track_pos))))

class pydaw_audio_region:
    def __init__(self):
        self.items = {}

    """ Return the next available index, or -1 if none are available """
    def get_next_index(self):
        for i in range(pydaw_max_audio_item_count):
            if not i in self.items:
                return i
        return -1

    def split(self, a_index):
        f_region0 = pydaw_audio_region()
        f_region1 = pydaw_audio_region()
        for k, v in list(self.items.items()):
            if v.start_bar >= a_index:
                v.start_bar -= a_index
                f_region1.items[k] = v
            else:
                f_region0.items[k] = v
        return f_region0, f_region1

    def add_item(self, a_index, a_item):
        self.items[int(a_index)] = a_item

    def remove_item(self, a_index):
        self.items.pop(int(a_index))

    def deduplicate_items(self):
        f_to_delete = []
        f_values = []
        for k, v in list(self.items.items()):
            f_str = str(v)
            if f_str in f_values:
                f_to_delete.append(k)
            else:
                f_values.append(f_str)
        for f_key in f_to_delete:
            print("Removing duplicate audio item at {}".format(f_key))
            self.items.pop(f_key)

    def set_region_length(self, a_length):
        """ Remove any items not within the new length, or change any end points that are past
        the new end.  Return True if anything changed, otherwise False"""
        f_to_delete = []
        f_length = int(a_length)
        for k, v in list(self.items.items()):
            if v.start_bar >= f_length:
                f_to_delete.append(k)
                print("Item begins after new region length of "
                      "{}, deleting: {}".format(a_length, v))
        for f_key in f_to_delete:
            self.items.pop(f_key)

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_audio_region()
        f_lines = a_str.split("\n")
        for f_line in f_lines:
            if f_line == pydaw_terminating_char:
                return f_result
            f_arr = f_line.split("|")
            f_result.add_item(int(f_arr[0]), pydaw_audio_item.from_arr(f_arr[1:]))
        print("pydaw_audio_region.from_str:  Warning:  no pydaw_terminating_char")
        return f_result

    def __str__(self):
        f_result = ""
        for k, f_item in list(self.items.items()):
            f_result += "{}|{}".format(k, f_item)
        f_result += pydaw_terminating_char
        return f_result

class pydaw_audio_item:
    def __init__(self, a_uid, a_sample_start=0.0, a_sample_end=1000.0,
                 a_start_bar=0, a_start_beat=0.0, a_timestretch_mode=3,
                 a_pitch_shift=0.0, a_output_track=0, a_vol=0, a_timestretch_amt=1.0,
                 a_fade_in=0.0, a_fade_out=999.0, a_lane_num=0, a_pitch_shift_end=0.0,
                 a_timestretch_amt_end=1.0, a_reversed=False, a_crispness=5,
                 a_fadein_vol=-18, a_fadeout_vol=-18, a_paif_automation_uid=0):
        self.uid = int(a_uid)
        self.sample_start = float(a_sample_start)
        self.sample_end = float(a_sample_end)
        self.start_bar = int(a_start_bar)
        self.start_beat = float(a_start_beat)
        self.time_stretch_mode = int(a_timestretch_mode)
        self.pitch_shift = float(a_pitch_shift)
        self.output_track = int(a_output_track)
        self.vol = int(a_vol)
        self.timestretch_amt = float(a_timestretch_amt)
        self.fade_in = float(a_fade_in)
        self.fade_out = float(a_fade_out)
        self.lane_num = int(a_lane_num)
        self.pitch_shift_end = float(a_pitch_shift_end)
        self.timestretch_amt_end = float(a_timestretch_amt_end)
        if isinstance(a_reversed, bool):
            self.reversed = a_reversed
        else:
            self.reversed = int_to_bool(a_reversed)
        self.crispness = int(a_crispness) #This is specific to Rubberband
        self.fadein_vol = int(a_fadein_vol)
        self.fadeout_vol = int(a_fadeout_vol)
        self.paif_automation_uid = int(a_paif_automation_uid)

    def set_pos(self, a_bar, a_beat):
        self.start_bar = int(a_bar)
        self.start_beat = float(a_beat)

    def set_fade_in(self, a_value):
        f_value = pydaw_clip_value(a_value, 0.0, self.fade_out - 1.0)
        self.fade_in = f_value

    def set_fade_out(self, a_value):
        f_value = pydaw_clip_value(a_value, self.fade_in + 1.0, 999.0)
        self.fade_out = f_value

    def clip_at_region_end(self, a_region_length, a_tempo, a_sample_length_seconds,
                           a_truncate=True):
        f_region_length_beats = a_region_length * 4
        f_seconds_per_beat = (60.0 / a_tempo)
        f_region_length_seconds = f_seconds_per_beat * f_region_length_beats
        f_item_start_beats = (self.start_bar * 4.0) + self.start_beat
        f_item_start_seconds = f_item_start_beats * f_seconds_per_beat
        f_sample_start_seconds = (self.sample_start * 0.001 * a_sample_length_seconds)
        f_sample_end_seconds = (self.sample_end * 0.001 * a_sample_length_seconds)
        f_actual_sample_length = f_sample_end_seconds - f_sample_start_seconds
        f_actual_item_end = f_item_start_seconds + f_actual_sample_length

        if a_truncate and f_actual_item_end > f_region_length_seconds:
            f_new_item_end_seconds = (f_region_length_seconds -
                f_item_start_seconds) + f_sample_start_seconds
            f_new_item_end = (f_new_item_end_seconds / a_sample_length_seconds) * 1000.0
            print("clip_at_region_end:  new end: {}".format(f_new_item_end))
            self.sample_end = f_new_item_end
            return True
        elif not a_truncate:
            f_new_start_seconds = f_region_length_seconds - f_actual_sample_length
            f_beats_total = f_new_start_seconds / f_seconds_per_beat
            self.start_bar = int(f_beats_total) // 4
            self.start_beat = f_beats_total % 4.0
            return True
        else:
            return False

    def __eq__(self, other):
        return str(self) == str(other)

    def clone(self):
        """ Using and abusing the functions that are already there... """
        return pydaw_audio_item.from_arr(str(self).strip("\n").split("|"))

    def __str__(self):
        return "{}\n".format("|".join(map(proj_file_str,
        (self.uid, self.sample_start, self.sample_end,
        self.start_bar, self.start_beat,
        self.time_stretch_mode, self.pitch_shift, self.output_track, self.vol,
        self.timestretch_amt,
        self.fade_in, self.fade_out, self.lane_num, self.pitch_shift_end,
        self.timestretch_amt_end, bool_to_int(self.reversed), int(self.crispness),
        int(self.fadein_vol), int(self.fadeout_vol), int(self.paif_automation_uid)))))

    @staticmethod
    def from_str(f_str):
        return pydaw_audio_item.from_arr(f_str.split("|"))

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_audio_item(*a_arr)
        return f_result

class pydaw_audio_item_fx_region:
    def __init__(self):
        self.fx_list = {}

    def __str__(self):
        f_result = ""
        for k, v in list(self.fx_list.items()):
            f_result += "{}\n".format(self.get_row_str(k))
        f_result += pydaw_terminating_char
        return f_result

    def get_row_str(self, a_row_index):
        f_result = str(a_row_index)
        for f_item in self.fx_list[int(a_row_index)]:
            f_result += str(f_item)
        return f_result

    def set_row(self, a_row_index, a_fx_list):
        self.fx_list[int(a_row_index)] = a_fx_list

    def clear_row(self, a_row_index):
        self.fx_list.pop(a_row_index)

    def clear_row_if_exists(self, a_row_index):
        if a_row_index in self.fx_list:
            self.fx_list.pop(a_row_index)

    def get_row(self, a_row_index, a_return_none=False):
        if int(a_row_index) in self.fx_list:
            return self.fx_list[int(a_row_index)]
        else:
            #print("Index {} not found in pydaw_audio_item_fx_region".format(a_row_index))
            if a_return_none:
                return None
            else:
                f_result = []
                for f_i in range(8):
                    f_result.append(pydaw_audio_item_fx(64, 64, 64, 0))
                return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_audio_item_fx_region()
        f_arr = str(a_str).split("\n")
        for f_line in f_arr:
            if f_line == pydaw_terminating_char:
                break
            f_items_arr = []
            f_item_index, f_vals = f_line.split("|", 1)
            f_vals_arr = f_vals.split("|")
            for f_i in range(8):
                f_index = f_i * 4
                f_index_end = f_index + 4
                a_knob0, a_knob1, a_knob2, a_type = f_vals_arr[f_index:f_index_end]
                f_items_arr.append(pydaw_audio_item_fx(a_knob0, a_knob1, a_knob2, a_type))
            f_result.set_row(f_item_index, f_items_arr)
        return f_result

class pydaw_audio_item_fx:
    def __init__(self, a_knob0, a_knob1, a_knob2, a_type):
        self.knobs = []
        self.knobs.append(int(a_knob0))
        self.knobs.append(int(a_knob1))
        self.knobs.append(int(a_knob2))
        self.fx_type = int(a_type)

    def __lt__(self, other):
        if self.index > other.index:
            return False
        else:
            return self.fx_num < other.fx_num

    def __str__(self):
        return "|{}".format("|".join(map(proj_file_str,
            (self.knobs[0], self.knobs[1], self.knobs[2], self.fx_type))))

class pydaw_audio_input_tracks:
    def add_track(self, a_index, a_track):
        self.tracks[a_index] = a_track

    def __init__(self):
        self.tracks = {}

    def __str__(self):
        f_result = ""
        for k, v in list(self.tracks.items()):
            f_result += "{}|{}".format(k, v)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_audio_input_tracks()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == pydaw_terminating_char:
                break
            else:
                f_line_arr = f_line.split("|")
                f_result.add_track(int(f_line_arr[0]),
                                   pydaw_audio_input_track(int_to_bool(f_line_arr[1]),
                                   int(f_line_arr[2]), int(f_line_arr[3])))
        return f_result

class pydaw_audio_input_track:
    def __init__(self, a_vol=0, a_output=0, a_input="None"):
        self.input = str(a_input)
        self.output = int(a_output)
        self.vol = int(a_vol)

    def __str__(self):
        return "{}|{}|{}\n".format(self.vol, self.output, self.input)

class pydaw_transport:
    def __init__(self, a_bpm=128):
        self.bpm = a_bpm

    def __str__(self):
        return "{}\n\\".format(self.bpm)

    @staticmethod
    def from_str(a_str):
        f_str = a_str.split("\n")[0]
        f_arr = f_str.split("|")
        return pydaw_transport(f_arr[0])

class pydaw_cc_map_item:
    def __init__(self, a_effects_only=False, a_rayv_port=0, a_wayv_port=0,
                 a_euphoria_port=0, a_modulex_port=0):
        self.effects_only = bool_to_int(a_effects_only)
        self.rayv_port = int(a_rayv_port)
        self.wayv_port = int(a_wayv_port)
        self.euphoria_port = int(a_euphoria_port)
        self.modulex_port = int(a_modulex_port)

    def __str__(self):
        return "{}\n".format("|".join(map(proj_file_str,
            (self.effects_only, self.rayv_port, self.wayv_port,
            self.euphoria_port, self.modulex_port))))

class pydaw_cc_map:
    def __init__(self):
        self.map = {}

    def add_item(self, a_cc, a_item):
        self.map[int(a_cc)] = a_item

    def __str__(self):
        f_result = ""
        for k, v in list(self.map.items()):
            f_result += "{}|{}".format(k, v)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_cc_map()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == pydaw_terminating_char:
                break
            f_line_arr = f_line.split("|")
            f_result.map[int(f_line_arr[0])] = pydaw_cc_map_item(
                int_to_bool(f_line_arr[1]), f_line_arr[2], f_line_arr[3],
                f_line_arr[4], f_line_arr[5])
        return f_result

#From old sample_graph..py
pydaw_audio_item_scene_height = 1200.0
pydaw_audio_item_scene_width = 6000.0
pydaw_audio_item_scene_rect = QtCore.QRectF(
    0.0, 0.0, pydaw_audio_item_scene_width, pydaw_audio_item_scene_height)

pydaw_audio_item_scene_gradient = QtGui.QLinearGradient(0, 0, 0, 1200)
pydaw_audio_item_scene_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(60, 60, 60, 120))
pydaw_audio_item_scene_gradient.setColorAt(1.0, QtGui.QColor.fromRgb(30, 30, 30, 120))

pydaw_audio_item_editor_gradient = QtGui.QLinearGradient(0, 0, 0, 1200)
pydaw_audio_item_editor_gradient.setColorAt(0.0, QtGui.QColor.fromRgb(190, 192, 123, 120))
pydaw_audio_item_editor_gradient.setColorAt(1.0, QtGui.QColor.fromRgb(130, 130, 100, 120))
#end from sample_graph.py

def pydaw_clear_sample_graph_cache():
    global global_sample_graph_cache
    global_sample_graph_cache = {}

def pydaw_remove_item_from_sg_cache(a_path):
    global global_sample_graph_cache
    if os.path.exists(a_path):
        os.system("rm -f '{}'".format(a_path))
    if a_path in global_sample_graph_cache:
        global_sample_graph_cache.pop(a_path)
    else:
        print("\n\npydaw_remove_item_from_sg_cache: {} not found.\n\n".format(a_path))

global_sample_graph_cache = {}

class pydaw_sample_graph:
    @staticmethod
    def create(a_file_name, a_sample_dir):
        """ Used to instantiate a pydaw_sample_graph, but grabs from the cache
        if it already exists... Prefer this over directly instantiating."""
        f_file_name = str(a_file_name)
        global global_sample_graph_cache
        if f_file_name in global_sample_graph_cache:
            return global_sample_graph_cache[f_file_name]
        else:
            f_result = pydaw_sample_graph(f_file_name, a_sample_dir)
            global_sample_graph_cache[f_file_name] = f_result
            return f_result

    def __init__(self, a_file_name, a_sample_dir):
        """
        a_file_name:  The full path to /.../sample_graphs/uid
        a_sample_dir:  The project's sample dir
        """
        self.sample_graph_cache = None
        f_file_name = str(a_file_name)
        self._file = None
        self.sample_dir = str(a_sample_dir)
        self.sample_dir_file = None
        self.timestamp = None
        self.channels = None
        self.high_peaks = ([],[])
        self.low_peaks = ([],[])
        self.count = None
        self.length_in_seconds = None
        self.sample_rate = None
        self.frame_count = None
        self.peak = 0.0

        if not os.path.isfile(f_file_name):
            return

        try:
            f_file = open(f_file_name, "r")
        except:
            return

        f_line_arr = f_file.readlines()
        f_file.close()
        for f_line in f_line_arr:
            f_line_arr = f_line.split("|")
            if f_line_arr[0] == "\\":
                break
            elif f_line_arr[0] == "meta":
                if f_line_arr[1] == "filename":
                    #Why does this have a newline on the end???
                    self._file = str(f_line_arr[2]).strip("\n")
                    self.sample_dir_file = "{}{}".format(self.sample_dir, self._file)
                elif f_line_arr[1] == "timestamp":
                    self.timestamp = int(f_line_arr[2])
                elif f_line_arr[1] == "channels":
                    self.channels = int(f_line_arr[2])
                elif f_line_arr[1] == "count":
                    self.count = int(f_line_arr[2])
                elif f_line_arr[1] == "length":
                    self.length_in_seconds = float(f_line_arr[2])
                elif f_line_arr[1] == "frame_count":
                    self.frame_count = int(f_line_arr[2])
                elif f_line_arr[1] == "sample_rate":
                    self.sample_rate = int(f_line_arr[2])
            elif f_line_arr[0] == "p":
                f_p_val = float(f_line_arr[3])
                f_abs_p_val = abs(f_p_val)
                if f_abs_p_val > self.peak:
                    self.peak = f_abs_p_val
                if f_p_val > 1.0:
                    f_p_val = 1.0
                elif f_p_val < -1.0:
                    f_p_val = -1.0
                if f_line_arr[2] == "h":
                    self.high_peaks[int(f_line_arr[1])].append(f_p_val)
                elif f_line_arr[2] == "l":
                    self.low_peaks[int(f_line_arr[1])].append(f_p_val)
                else:
                    print(("Invalid sample_graph [2] value " + f_line_arr[2] ))
        for f_list in self.low_peaks:
            f_list.reverse()

    def is_valid(self):
        if (self._file is None):
            print("\n\npydaw_sample_graph.is_valid() self._file is None {}\n".format(
                self._file))
            return False
        if self.timestamp is None:
            print("\n\npydaw_sample_graph.is_valid() self.timestamp is None {}\n".format(
                self._file))
            return False
        if self.channels is None:
            print("\n\npydaw_sample_graph.is_valid() self.channels is None {}\n".format(
                self._file))
            return False
        if self.frame_count is None:
            print("\n\npydaw_sample_graph.is_valid() self.frame_count is None {}\n".format(
                self._file))
            return False
        if self.sample_rate is None:
            print("\n\npydaw_sample_graph.is_valid() self.sample_rate is None {}\n".format(
                self._file))
            return False
        return True

    def normalize(self, a_db=0.0):
        if self.peak == 0.0:
            return 0.0
        f_norm_lin = pydaw_db_to_lin(a_db)
        f_diff = f_norm_lin / self.peak
        f_result = int(pydaw_lin_to_db(f_diff))
        f_result = pydaw_clip_value(f_result, -24, 24)
        return f_result

    def create_sample_graph(self, a_for_scene=False):
        if self.sample_graph_cache is None:
            if self.length_in_seconds > 0.5:
                if a_for_scene:
                    f_width_inc = pydaw_audio_item_scene_width / self.count
                    f_section = pydaw_audio_item_scene_height / float(self.channels)
                else:
                    f_width_inc = 98.0 / self.count
                    f_section = 100.0 / float(self.channels)
                f_section_div2 = f_section * 0.5

                f_paths = []

                for f_i in range(self.channels):
                    f_result = QtGui.QPainterPath()
                    f_width_pos = 1.0
                    f_result.moveTo(f_width_pos, f_section_div2)
                    for f_peak in self.high_peaks[f_i]:
                        f_peak_clipped = pydaw_clip_value(f_peak, 0.01, 0.99)
                        f_result.lineTo(f_width_pos, f_section_div2 -
                            (f_peak_clipped * f_section_div2))
                        f_width_pos += f_width_inc
                    for f_peak in self.low_peaks[f_i]:
                        f_peak_clipped = pydaw_clip_value(f_peak, -0.99, -0.01)
                        f_result.lineTo(f_width_pos, (f_peak_clipped * -1.0 *
                            f_section_div2) + f_section_div2)
                        f_width_pos -= f_width_inc
                    f_result.closeSubpath()
                    f_paths.append(f_result)
                self.sample_graph_cache = f_paths
            else:
                f_width_inc = pydaw_audio_item_scene_width / self.count
                f_section = pydaw_audio_item_scene_height / float(self.channels)
                f_section_div2 = f_section * 0.5
                f_paths = []

                for f_i in range(self.channels):
                    f_result = QtGui.QPainterPath()
                    f_width_pos = 1.0
                    f_result.moveTo(f_width_pos, f_section_div2)
                    for f_i2 in range(len(self.high_peaks[f_i])):
                        f_peak = self.high_peaks[f_i][f_i2]
                        f_result.lineTo(f_width_pos, f_section_div2 - (f_peak * f_section_div2))
                        f_width_pos += f_width_inc
                    f_paths.append(f_result)
                self.sample_graph_cache = f_paths
        return self.sample_graph_cache

    def envelope_to_automation(self, a_is_cc, a_tempo):
        " In the automation viewer clipboard format "
        f_list = [(x if x > y else y) for x, y in
            zip([abs(x) for x in self.high_peaks[0]],
                [abs(x) for x in reversed(self.low_peaks[0])])]
        f_seconds_per_beat = 60.0 / float(a_tempo)
        f_length_beats = self.length_in_seconds / f_seconds_per_beat
        f_point_count = int(f_length_beats * 16.0)
        print("Resampling {} to {}".format(len(f_list), f_point_count))
        f_result = []
        f_arr = numpy.array(f_list)
        f_arr = scipy.signal.resample(f_arr, f_point_count)
        f_max = numpy.amax(f_arr)
        if f_max > 0.0:
            f_arr *= (1.0 / f_max)
        for f_point, f_pos in zip(f_arr, range(f_arr.shape[0])):
            f_start = (float(f_pos) / float(f_arr.shape[0])) * \
                f_length_beats
            f_index = int(f_start / 4.0)
            f_start = f_start % 4.0
            if a_is_cc:
                f_val = pydaw_clip_value(f_point * 127.0, 0.0, 127.0)
                f_result.append((pydaw_cc(f_start, 0, 0, f_val), f_index))
            else:
                f_val = pydaw_clip_value(f_point, 0.0, 1.0)
                f_result.append((pydaw_pitchbend(f_start, f_val), f_index))
        return f_result

    def envelope_to_notes(self, a_tempo):
        " In the piano roll clipboard format "
        f_list = [(x if x > y else y) for x, y in
            zip([abs(x) for x in self.high_peaks[0]],
                [abs(x) for x in reversed(self.low_peaks[0])])]
        f_seconds_per_beat = 60.0 / float(a_tempo)
        f_length_beats = self.length_in_seconds / f_seconds_per_beat
        f_point_count = int(f_length_beats * 16.0)  # 64th note resolution
        print("Resampling {} to {}".format(len(f_list), f_point_count))
        f_result = []
        f_arr = numpy.array(f_list)
        f_arr = scipy.signal.resample(f_arr, f_point_count)
        f_current_note = None
        f_max = numpy.amax(f_arr)
        if f_max > 0.0:
            f_arr *= (1.0 / f_max)
        f_thresh = pydaw_db_to_lin(-24.0)
        for f_point, f_pos in zip(f_arr, range(f_arr.shape[0])):
            f_start = (float(f_pos) / float(f_arr.shape[0])) * \
                f_length_beats
            if f_point > f_thresh:
                if not f_current_note:
                    f_current_note = [f_start, 0.25, f_point]
                else:
                    if f_point > f_current_note[2]:
                        f_current_note[2] = f_point
            else:
                if f_current_note:
                    f_current_note[1] = f_start - f_current_note[0]
                    f_result.append(f_current_note)
                    f_current_note = None
        f_result2 = []
        for f_pair in f_result:
            f_index = int(f_pair[0] / 4.0)
            f_start = f_pair[0] % 4.0
            f_vel = pydaw_clip_value(f_pair[2] * 100.0, 1.0, 127.0)
            f_result2.append(
                (str(pydaw_note(f_start, f_pair[1], 60, f_vel)), f_index))
        return f_result2

    def check_mtime(self):
        """ Returns False if the sample graph is older than the file modified time

            UPDATE:  Now obsolete, will require some fixing if used again...
        """
        try:
            if os.path.isfile(self._file):
                f_timestamp = int(os.path.getmtime(self._file))
            elif os.path.isfile(self.sample_dir_file):
                return True #f_timestamp = int(os.path.getmtime(self.sample_dir_file))
            else:
                raise Exception("Neither original nor cached file exists.")
            return self.timestamp > f_timestamp
        except Exception as f_ex:
            print("\n\nError getting mtime: \n{}\n\n".format(f_ex.message))
            return False

class pydaw_midicomp_event:
    def __init__(self, a_arr):
        self.tick = int(a_arr[0])
        self.type = a_arr[1]
        self.ch = int(a_arr[2].split("ch=")[1]) - 1
        self.pitch = int(a_arr[3].split("n=")[1])
        if self.pitch >= 24:
            self.pitch -= 24
        self.vel = int(a_arr[4].split("v=")[1])
        self.length = -1

    def __lt__(self, other):
        return self.tick < other.tick

class pydaw_midi_file_to_items:
    """ Convert the MIDI file at a_file to a dict of pydaw_item's with keys
        in the format (track#, channel#, bar#)"""
    def __init__(self, a_file):
        f_midi_comp = "{}/midicomp".format(os.path.dirname(os.path.abspath(__file__)))
        f_midi_text_arr = subprocess.check_output([f_midi_comp,
                                                   str(a_file)]).decode("utf-8").split("\n")
        #First fix the lengths of events that have note-off events
        f_note_on_dict = {}
        f_item_list = []
        f_resolution = 96
        for f_line in f_midi_text_arr:
            f_line_arr = f_line.split()
            if len(f_line_arr) <= 1:
                continue
            if f_line_arr[0] == "MFile":
                f_resolution = int(f_line_arr[3])
            elif f_line_arr[1] == "On":
                f_event = pydaw_midicomp_event(f_line_arr)
                if f_event.vel == 0:
                    f_tuple = (f_event.ch, f_event.pitch)
                    if f_tuple in f_note_on_dict:
                        f_note_on_dict[f_tuple].length = \
                            float(f_event.tick - f_note_on_dict[f_tuple].tick) / \
                            float(f_resolution)
                        f_note_on_dict.pop(f_tuple)
                else:
                    f_note_on_dict[(f_event.ch, f_event.pitch)] = f_event
                    f_item_list.append(f_event)
            elif f_line_arr[1] == "Off":
                f_event = pydaw_midicomp_event(f_line_arr)
                f_tuple = (f_event.ch, f_event.pitch)
                if f_tuple in f_note_on_dict:
                    f_note_on_dict[f_tuple].length = \
                        float(f_event.tick - f_note_on_dict[f_tuple].tick) / float(f_resolution)
                    print("{} {}".format(f_note_on_dict[f_tuple].tick,
                          f_note_on_dict[f_tuple].length))
                    f_note_on_dict.pop(f_tuple)
                else:
                    print("Error, note-off event does not correspond to a "
                          "note-on event, ignoring event:\n{}".format(
                        f_event))
            else:
                print("Ignoring event: {}".format(f_line))

        self.result_dict = {}
        f_item_list.sort()

        for f_event in f_item_list:
            if f_event.length > 0.0:
                f_velocity = f_event.vel
                f_beat = (float(f_event.tick) / float(f_resolution)) % 4.0
                f_bar = int((int(f_event.tick) // int(f_resolution)) // 4)
                print("f_beat : {} | f_bar : {}".format(f_beat, f_bar))
                f_pitch = f_event.pitch
                f_length = f_event.length
                f_channel = f_event.ch
                f_key = (f_channel, f_bar)
                if not f_key in self.result_dict:
                    self.result_dict[f_key] = pydaw_item()
                f_note = pydaw_note(f_beat, f_length, f_pitch, f_velocity)
                self.result_dict[f_key].add_note(f_note) #, a_check=False)
            else:
                print("Ignoring note event with <= zero length")

        f_min = 0
        f_max = 0

        for k, v in list(self.result_dict.items()):
            if k[1] < f_min:
                f_min = k[1]
            if k[1] > f_max:
                f_max = k[1]

        print("f_min : {} | f_max : {}".format(f_min, f_max))

        self.bar_count = int(f_max - f_min + 1)
        self.bar_offset = int(f_min)
        self.channel_count = self.get_channel_count()

        #Nested dict in format [channel][bar]
        self.track_map = {}
        for f_i in range(pydaw_midi_track_count):
            self.track_map[f_i] = {}

        for k, v in list(self.result_dict.items()):
            f_channel, f_bar = k
            self.track_map[f_channel][f_bar - self.bar_offset] = v

    def get_channel_count(self):
        f_result = []
        for k in list(self.result_dict.keys()):
            if k[0] not in f_result:
                f_result.append(k[0])
        return len(f_result)

    def populate_region_from_track_map(self, a_project, a_name, a_index):
        f_actual_track_num = 0
        f_song = a_project.get_song()

        f_region_name = a_project.get_next_default_region_name(a_name)
        f_region_uid = a_project.create_empty_region(f_region_name)
        f_result_region = a_project.get_region_by_uid(f_region_uid)
        f_song.add_region_ref_by_uid(a_index, f_region_uid)
        a_project.save_song(f_song)

        if self.bar_count > pydaw_max_region_length:
            f_result_region.region_length_bars = pydaw_max_region_length
        else:
            f_result_region.region_length_bars = self.bar_count
            for f_channel, f_bar_dict in list(self.track_map.items()):
                for f_bar, f_item in list(self.track_map[f_channel].items()):
                    f_this_item_name = "{}-{}-{}".format(a_name, f_channel, f_bar)
                    if a_project.item_exists(f_this_item_name):
                        f_this_item_name = a_project.get_next_default_item_name(f_this_item_name)
                    f_item_uid = a_project.create_empty_item(f_this_item_name)
                    a_project.save_item_by_uid(f_item_uid, f_item)
                    f_result_region.add_item_ref_by_uid(f_actual_track_num, f_bar, f_item_uid)
                    if f_bar >= pydaw_max_region_length:
                        break
                f_actual_track_num += 1
                if f_actual_track_num >= pydaw_midi_track_count:
                    break
        a_project.save_region(f_region_name, f_result_region)
        return True


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

import os, random, traceback, subprocess
from shutil import move
from time import sleep
import midi

#from lms_session import lms_session #deprecated
from dssi_gui import dssi_gui
from math import log, pow

from PyQt4 import QtGui, QtCore
import pydaw_history

global_pydaw_version_string = "pydaw3"
global_pydaw_file_type_string = 'PyDAW3 Project (*.pydaw3)'

pydaw_bus_count = 5
pydaw_audio_track_count = 8
pydaw_audio_input_count = 5
pydaw_midi_track_count = 20
pydaw_max_audio_item_count = 64
pydaw_max_region_length = 32 #bars

pydaw_folder_audio = "audio"
pydaw_folder_audiofx = "audiofx"
pydaw_folder_busfx = "busfx"
pydaw_folder_instruments = "instruments"
pydaw_folder_items = "items"
pydaw_folder_regions = "regions"
pydaw_folder_regions_audio = "regions_audio"
pydaw_folder_samplegraph = "samplegraph"
pydaw_folder_samples = "samples"
pydaw_folder_timestretch = "timestretch"

pydaw_file_pyregions = "default.pyregions"
pydaw_file_pyitems = "default.pyitems"
pydaw_file_pysong = "default.pysong"
pydaw_file_pytransport = "default.pytransport"
pydaw_file_pymididevice = "default.pymididevice"
pydaw_file_pytracks = "default.pytracks"
pydaw_file_pyaudio = "default.pyaudio"
pydaw_file_pyaudioitem = "default.pyaudioitem"
pydaw_file_pybus = "default.pybus"
pydaw_file_pyinput = "default.pyinput"
pydaw_file_pywavs = "default.pywavs"
pydaw_file_pystretch = "default.pystretch"
pydaw_file_pystretch_map = "map.pystretch"

pydaw_min_note_length = 1.0/128.0  #Anything smaller gets deleted when doing a transform

pydaw_terminating_char = "\\"

pydaw_bad_chars = ["|", "\\", "~", "."]

def pydaw_remove_bad_chars(a_str):
    """ Remove any characters that have special meaning to PyDAW """
    f_str = str(a_str)
    for f_char in pydaw_bad_chars:
        f_str = f_str.replace(f_char, "")
    f_str = f_str.replace(' ', '_')
    return f_str

beat_fracs = ['1/16', '1/8', '1/4', '1/3', '1/2', '1/1']

def beat_frac_text_to_float(f_index):
    if f_index == 0:
        return 0.0625
    elif f_index == 1:
        return 0.125
    elif f_index == 2:
        return 0.25
    elif f_index == 3:
        return 0.33333333
    elif f_index == 4:
        return 0.5
    elif f_index == 5:
        return 1.0
    else:
        return 0.25

def pydaw_beats_to_index(a_beat, a_divisor=4.0):
    f_index = int(a_beat / a_divisor)
    f_start = a_beat - (float(f_index) * a_divisor)
    return f_index, f_start

int_to_note_array = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']

def pydaw_read_file_text(a_file):
    f_handle = open(str(a_file))
    f_result = f_handle.read()
    f_handle.close()
    return f_result

def pydaw_write_file_text(a_file, a_text):
    f_handle = open(str(a_file), "w")
    f_handle.write(str(a_text))
    f_handle.close()

def pydaw_gen_uid():
    """Generated an integer uid.  Adding together multiple random numbers gives a far less uniform distribution of
    numbers, more of a natural white noise kind of sample graph than a brick-wall digital white noise... """
    f_result = 5
    for i in range(6):
        f_result += random.randint(6, 50000000)
    return f_result

def note_num_to_string(a_note_num):
    f_note = int(a_note_num) % 12
    f_octave = (int(a_note_num) / 12) - 2
    return int_to_note_array[f_note] + str(f_octave)

def bool_to_int(a_bool):
    """ For sending OSC messages """
    if a_bool:
        return "1"
    else:
        return "0"

def int_to_bool(a_int):
    if int(a_int) == 0:
        return False
    elif int(a_int) == 1:
        return True
    else:
        assert(False)

def time_quantize_round(a_input):
    """Properly quantize time values from QDoubleSpinBoxes that measure beats"""
    if round(a_input) == round(a_input, 2):
        return round(a_input)
    else:
        return round(a_input, 4)

def pydaw_pitch_to_hz(a_pitch):
    return (440.0 * pow(2.0,(a_pitch - 57.0) * 0.0833333))

def pydaw_hz_to_pitch(a_hz):
    return ((12.0 * log(a_hz * (1.0/440.0), 2.0)) + 57.0)

def pydaw_pitch_to_ratio(a_pitch):
    return (1.0/pydaw_pitch_to_hz(0.0)) * pydaw_pitch_to_hz(a_pitch)

def pydaw_db_to_lin(a_value):
    return pow(10.0, (0.05 * a_value))

def pydaw_lin_to_db(a_value):
    return log(a_value, 10.0) * 20.0

class pydaw_project:
    def create_file(self, a_folder, a_file, a_text):
        """  Call save_file only if the file doesn't exist... """
        if not os.path.isfile(self.project_folder + "/" + str(a_folder) + "/" + str(a_file)):
            self.save_file(a_folder, a_file, a_text)

    def save_file(self, a_folder, a_file, a_text, a_force_new=False):
        """ Writes a file to disk and updates the project history to reflect the changes """
        f_full_path = self.project_folder + "/" + str(a_folder) + "/" + str(a_file)
        if not a_force_new and os.path.isfile(f_full_path):
            f_old = pydaw_read_file_text(f_full_path)
            if f_old == a_text:
                return
            f_existed = 1
        else:
            f_old = ""
            f_existed = 0
        pydaw_write_file_text(f_full_path, a_text)
        f_history_file = pydaw_history.pydaw_history_file(a_folder, a_file, a_text, f_old, f_existed)
        self.history_files.append(f_history_file)
        #TODO:  debug/verbose mode this output...
        print(str(f_history_file))

    def commit(self, a_message):
        """ Commit the project history """
        if self.history_undo_cursor > 0:
            self.history_commits = self.history_commits[:self.history_undo_cursor]
            self.history_undo_cursor = 0
        if len(self.history_files) > 0:
            self.history_commits.append(pydaw_history.pydaw_history_commit(self.history_files, a_message))
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
            f_result[f_file] = pydaw_read_file_text(a_folder + "/" + f_file)
        return f_result

    def get_bus_fx_files(self):
        return os.listdir(self.busfx_folder)

    def get_audio_fx_files(self):
        return os.listdir(self.audiofx_folder)

    def save_plugin_state(self, a_old, a_new, a_folder, a_ext=None):
        for k, v in a_new.iteritems():
            if a_old.has_key(k):
                f_existed = 1
                f_old_text = a_old[k]
            else:
                f_existed = 0
                f_old_text = ""
            self.history_files.append(pydaw_history.pydaw_history_file(a_folder, k, v, f_old_text, f_existed))

    def save_project(self):
        f_old_inst = self.get_files_dict(self.instrument_folder, ".pyinst")
        f_old_fx = self.get_files_dict(self.instrument_folder, ".pyfx")
        f_old_audio_fx = self.get_files_dict(self.audiofx_folder)
        f_old_bus_fx = self.get_files_dict(self.busfx_folder)

        self.this_dssi_gui.pydaw_save_tracks()
        sleep(3)

        f_new_inst = self.get_files_dict(self.instrument_folder, ".pyinst")
        f_new_fx = self.get_files_dict(self.instrument_folder, ".pyfx")
        f_new_audio_fx = self.get_files_dict(self.audiofx_folder)
        f_new_bus_fx = self.get_files_dict(self.busfx_folder)

        self.save_plugin_state(f_old_inst, f_new_inst, pydaw_folder_instruments, ".pyinst")
        self.save_plugin_state(f_old_fx, f_new_fx, pydaw_folder_instruments, ".pyfx")
        self.save_plugin_state(f_old_audio_fx, f_new_audio_fx, pydaw_folder_audiofx)
        self.save_plugin_state(f_old_bus_fx, f_new_bus_fx, pydaw_folder_busfx)

        self.commit("Saved plugin state")
        self.flush_history()

    def flush_history(self):
        for f_commit in self.history_commits:
            self.history.commit(f_commit)
        self.history_commits = []
        self.history_undo_cursor = 0

    def save_project_as(self, a_file_name):
        self.save_project()  #This is necessary to capture the plugin states before copying everything over...  Otherwise the instruments and effects may not be what they were at this time...
        f_file_name = str(a_file_name)
        print("Saving project as " + f_file_name + " ...")
        f_new_project_folder = os.path.dirname(f_file_name)
        #The below is safe because we already checked that the folder should be empty before calling this
        f_cmd = 'rm -rf "' + f_new_project_folder + '"'
        os.popen(f_cmd)
        f_cmd = 'cp -r "' + self.project_folder + '" "' + f_new_project_folder + '"'
        os.popen(f_cmd)
        print(f_new_project_folder + "/" + self.project_file + " | " + a_file_name)
        move(f_new_project_folder + "/" + self.project_file + ".pydaw3", a_file_name)
        self.set_project_folders(f_file_name)
        self.this_dssi_gui.pydaw_open_song(self.project_folder)
        self.history = pydaw_history.pydaw_history(self.project_folder)

    def set_project_folders(self, a_project_file):
        #folders
        self.project_folder = os.path.dirname(a_project_file)
        self.project_file = os.path.splitext(os.path.basename(a_project_file))[0]
        self.instrument_folder = self.project_folder + "/" + pydaw_folder_instruments
        self.regions_folder = self.project_folder + "/" + pydaw_folder_regions
        self.regions_audio_folder = self.project_folder + "/" + pydaw_folder_regions_audio
        self.items_folder = self.project_folder + "/" + pydaw_folder_items
        self.audio_folder = self.project_folder + "/" + pydaw_folder_audio
        self.audio_tmp_folder = self.project_folder + "/audio/tmp"
        self.samples_folder = self.project_folder + "/" + pydaw_folder_samples  #Placeholder for future functionality
        self.audiofx_folder = self.project_folder + "/" + pydaw_folder_audiofx
        self.busfx_folder = self.project_folder + "/" + pydaw_folder_busfx
        self.samplegraph_folder = self.project_folder + "/" + pydaw_folder_samplegraph
        self.timestretch_folder = self.project_folder + "/" + pydaw_folder_timestretch
        #files
        self.pyregions_file = self.project_folder + "/default.pyregions"
        self.pyitems_file = self.project_folder + "/default.pyitems"
        self.pywavs_file = self.project_folder + "/default.pywavs"
        self.pystretch_file = self.project_folder + "/" + pydaw_file_pystretch
        self.pystretch_map_file = self.project_folder + "/" + pydaw_file_pystretch_map

        pydaw_clear_sample_graph_cache()

    def open_project(self, a_project_file, a_notify_osc=True):
        self.set_project_folders(a_project_file)
        if not os.path.exists(a_project_file):
            print("project file " + a_project_file + " does not exist, creating as new project")
            self.new_project(a_project_file)
        else:
            self.history = pydaw_history.pydaw_history(self.project_folder)
            self.open_stretch_dicts()
        if a_notify_osc:
            self.this_dssi_gui.pydaw_open_song(self.project_folder)

    def new_project(self, a_project_file, a_notify_osc=True):
        self.set_project_folders(a_project_file)

        project_folders = [
            self.project_folder, self.instrument_folder, self.regions_folder,
            self.items_folder, self.audio_folder, self.samples_folder,
            self.audiofx_folder, self.busfx_folder, self.samplegraph_folder,
            self.audio_tmp_folder, self.regions_audio_folder, self.timestretch_folder]

        for project_dir in project_folders:
            print(project_dir)
            if not os.path.isdir(project_dir):
                os.makedirs(project_dir)
        self.history = pydaw_history.pydaw_history(self.project_folder)

        self.create_file("", "version.txt", "Created with " + global_pydaw_version_string + "-" + \
        pydaw_read_file_text("/usr/lib/" + global_pydaw_version_string + "/" + global_pydaw_version_string + "-version.txt"))
        self.create_file("", os.path.basename(a_project_file), "This file is not supposed to contain any data, it is only a placeholder for saving and opening the project :)")
        self.create_file("", pydaw_file_pyregions, pydaw_terminating_char)
        self.create_file("", pydaw_file_pywavs, pydaw_terminating_char)
        self.create_file("", pydaw_file_pystretch_map, pydaw_terminating_char)
        self.create_file("", pydaw_file_pystretch, pydaw_terminating_char)
        self.create_file("", pydaw_file_pyitems, pydaw_terminating_char)
        self.create_file("", pydaw_file_pysong, pydaw_terminating_char)
        self.create_file("", pydaw_file_pytransport, str(pydaw_transport()))
        self.create_file("", pydaw_file_pymididevice, "")
        f_midi_tracks_instance = pydaw_tracks()
        for i in range(pydaw_midi_track_count):
            f_midi_tracks_instance.add_track(i, pydaw_track(a_name="track" + str(i + 1)))
        self.create_file("", pydaw_file_pytracks, str(f_midi_tracks_instance))
        f_pyaudio_instance = pydaw_audio_tracks()
        for i in range(pydaw_audio_track_count):
            f_pyaudio_instance.add_track(i, pydaw_audio_track(a_name="track" + str(i + 1)))
        self.create_file("", pydaw_file_pyaudio, str(f_pyaudio_instance))
        f_pybus_instance = pydaw_busses()
        for i in range(pydaw_bus_count):
            f_pybus_instance.add_bus(i, pydaw_bus())
        self.create_file("", pydaw_file_pybus, str(f_pybus_instance))
        self.open_stretch_dicts()
        self.commit("Created project")
        if a_notify_osc:
            self.this_dssi_gui.pydaw_open_song(self.project_folder)

    def open_stretch_dicts(self):
        self.timestretch_cache = {}
        self.timestretch_reverse_lookup = {}
        if not os.path.exists(self.pystretch_file):  #TODO:  Remove at PyDAWv4
            self.create_file("", pydaw_file_pystretch_map, pydaw_terminating_char)
            self.create_file("", pydaw_file_pystretch, pydaw_terminating_char)
            self.commit("Create timestretch files")
            return

        f_cache_text = pydaw_read_file_text(self.pystretch_file)
        for f_line in f_cache_text.split("\n"):
            if f_line == pydaw_terminating_char:
                break
            f_line_arr = f_line.split("|", 4)
            self.timestretch_cache[(f_line_arr[0], f_line_arr[1], f_line_arr[2])] = f_line_arr[3]

        f_map_text = pydaw_read_file_text(self.pystretch_map_file)
        for f_line in f_map_text.split("\n"):
            if f_line == pydaw_terminating_char:
                break
            f_line_arr = f_line.split("|||")
            self.timestretch_reverse_lookup[f_line_arr[0]] = f_line_arr[1]

    def save_stretch_dicts(self):
        f_stretch_text = ""
        for k, v in self.timestretch_cache.iteritems():
            f_stretch_text += str(k[0]) + "|"+ str(k[1]) + "|" + str(k[2]) + "|" + str(v) + "\n"
        f_stretch_text += pydaw_terminating_char
        self.save_file("", pydaw_file_pystretch, f_stretch_text)

        f_map_text = ""
        for k, v in self.timestretch_reverse_lookup.iteritems():
            f_map_text += str(k) + "|||" + str(v) + "\n"
        f_map_text += pydaw_terminating_char
        self.save_file("", pydaw_file_pystretch_map, f_map_text)

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
        self.save_file("", pydaw_file_pywavs, str(a_uid_dict))

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
            f_file = open(self.project_folder + "/default.pysong", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_song(self):
        return pydaw_song.from_str(self.get_song_string())

    def get_region_string(self, a_region_uid):
        try:
            f_file = open(self.regions_folder + "/" + str(a_region_uid), "r")
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
            f_new_item_name = str(a_new_item_name) + "-"
            for f_item_name in a_item_names:
                while f_items_dict.name_exists(f_new_item_name + str(f_suffix)):
                    f_suffix += 1
                f_items_dict.rename_item(f_item_name, f_new_item_name + str(f_suffix))
        else:
            f_items_dict.rename_item(a_item_names[0], a_new_item_name)
        self.save_items_dict(f_items_dict)

    def rename_region(self, a_old_name, a_new_name):
        f_regions_dict = self.get_regions_dict()
        if f_regions_dict.name_exists(a_new_name):
            f_suffix = 1
            f_new_name = str(a_new_name) + "-"
            while f_regions_dict.name_exists(f_new_name + str(f_suffix)):
                f_suffix += 1
            f_regions_dict.rename_item(a_old_name, f_new_name)
        else:
            f_regions_dict.rename_item(a_old_name, a_new_name)
        self.save_regions_dict(f_regions_dict)

    def get_item_string(self, a_item_uid):
        try:
            f_file = open(self.items_folder + "/" + str(a_item_uid), "r")
        except:
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_item_by_uid(self, a_item_uid):
        return pydaw_item.from_str(self.get_item_string(a_item_uid))

    def get_item_by_name(self, a_item_name):
        f_items_dict = self.get_items_dict()
        return pydaw_item.from_str(self.get_item_string(f_items_dict.get_uid_by_name(a_item_name)))

    def timestretch_lookup_orig_path(self, a_path):
        if self.timestretch_reverse_lookup.has_key(a_path):
            return self.timestretch_reverse_lookup[a_path]
        else:
            return a_path

    def timestretch_audio_item(self, a_audio_item):
        """ Return path, uid for a time-stretched audio item and update all project files,
        or None if the UID already exists in the cache"""
        if not os.path.isdir(self.timestretch_folder):  #TODO:  remove at PyDAWv4
            os.mkdir(self.timestretch_folder)
        f_src_path = self.get_wav_name_by_uid(a_audio_item.uid)
        if self.timestretch_reverse_lookup.has_key(f_src_path):
            f_src_path = self.timestretch_reverse_lookup[f_src_path]
        else:
            if a_audio_item.timestretch_amt == 1.0 and a_audio_item.pitch_shift == 0.0:
                return None  #Don't run Rubberband if the file is not being timestretched yet
        f_key = (a_audio_item.time_stretch_mode, a_audio_item.timestretch_amt, a_audio_item.pitch_shift, f_src_path)
        if self.timestretch_cache.has_key(f_key):
            a_audio_item.uid = self.timestretch_cache[f_key]
            return None
        else:
            f_uid = pydaw_gen_uid()
            f_dest_path = self.timestretch_folder + "/" + str(f_uid) + ".wav"
            if a_audio_item.time_stretch_mode == 3:
                f_cmd = ["rubberband", "-t",  str(a_audio_item.timestretch_amt), "-p", str(a_audio_item.pitch_shift),
                         "-R", "--pitch-hq", f_src_path, f_dest_path]
            elif a_audio_item.time_stretch_mode == 4:
                f_cmd = ["rubberband", "-F", "-t",  str(a_audio_item.timestretch_amt), "-p", str(a_audio_item.pitch_shift),
                         "-R", "--pitch-hq", f_src_path, f_dest_path]
            elif a_audio_item.time_stretch_mode == 5:
                f_cmd = ["/usr/lib/pydaw3/sbsms/bin/sbsms", f_src_path, f_dest_path,
                         str(1.0 / a_audio_item.timestretch_amt), str(1.0 / a_audio_item.timestretch_amt_end),
                         str(a_audio_item.pitch_shift), str(a_audio_item.pitch_shift_end) ]
            elif a_audio_item.time_stretch_mode == 6:
                f_tmp_file = self.audio_tmp_folder + "/" + str(f_uid) + ".wav"
                self.this_dssi_gui.pydaw_convert_wav_to_32_bit(f_src_path, f_tmp_file)
                f_cmd = ["/usr/lib/pydaw3/pydaw/python/libpydaw/paulstretch_newmethod.py",
                         "-s", str(a_audio_item.timestretch_amt), f_tmp_file, f_dest_path ]
            print("Running " + " ".join(f_cmd))
            f_proc = subprocess.Popen(f_cmd)
            self.timestretch_cache[f_key] = f_uid
            self.timestretch_reverse_lookup[f_dest_path] = f_src_path
            a_audio_item.uid = self.timestretch_cache[f_key]
            return f_dest_path, f_uid, f_proc

    def timestretch_get_orig_file_uid(self, a_uid):
        """ Return the UID of the original file """
        f_new_path = self.get_wav_path_by_uid(a_uid)
        f_old_path = self.timestretch_reverse_lookup[f_new_path]
        return self.get_wav_uid_by_name(f_old_path)

    def check_for_recorded_items(self, a_item_name):
        self.check_for_recorded_regions()
        f_item_name = str(a_item_name) + "-"
        if os.path.isfile(self.project_folder + "/recorded_items"):
            f_str_list = pydaw_read_file_text(self.project_folder + "/recorded_items").split("\n")
            os.remove(self.project_folder + "/recorded_items")
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
            while f_items_dict.uid_lookup.has_key(f_item_name + str(f_suffix)):
                f_suffix += 1
            f_items_dict.add_item(f_int, f_item_name + str(f_suffix))
            f_suffix += 1
        self.save_items_dict(f_items_dict)

    def check_for_recorded_regions(self):
        if os.path.isfile(self.project_folder + "/recorded_regions"):
            f_str_list = pydaw_read_file_text(self.project_folder + "/recorded_regions").split("\n")
            os.remove(self.project_folder + "/recorded_regions")
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
            if not f_regions_dict.name_lookup.has_key(f_int):
                self.save_file(pydaw_folder_regions_audio, f_int, pydaw_terminating_char)
                while f_regions_dict.uid_lookup.has_key("recorded-" + str(f_suffix)):
                    f_suffix += 1
                f_regions_dict.add_item(f_int, "recorded-" + str(f_suffix))
                f_suffix += 1
                f_old_text = ""
                f_existed = 0
            else:
                f_old_text = self.history.get_latest_version_of_file(pydaw_folder_regions, f_int)
                f_existed = 1
            self.history_files.append(pydaw_history.pydaw_history_file(pydaw_folder_regions, str(f_int), \
            pydaw_read_file_text(self.regions_folder + "/" + str(f_int)), f_old_text, f_existed))

        self.save_regions_dict(f_regions_dict)
        f_old_text = self.history.get_latest_version_of_file("", pydaw_file_pysong)
        f_new_text = pydaw_read_file_text(self.project_folder + "/" + pydaw_file_pysong)
        if f_old_text is not None and f_new_text != f_old_text:
            print "Appending history file for pysong"
            self.history_files.append(pydaw_history.pydaw_history_file("", pydaw_file_pysong, f_new_text, f_old_text, 1))
        else:
            print "f_old_text", f_old_text

    def get_tracks_string(self):
        try:
            f_file = open(self.project_folder + "/default.pytracks", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_tracks(self):
        return pydaw_tracks.from_str(self.get_tracks_string())

    def get_bus_tracks_string(self):
        try:
            f_file = open(self.project_folder + "/default.pybus", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_bus_tracks(self):
        return pydaw_busses.from_str(self.get_bus_tracks_string())

    def get_audio_tracks_string(self):
        try:
            f_file = open(self.project_folder + "/default.pyaudio", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_audio_tracks(self):
        return pydaw_audio_tracks.from_str(self.get_audio_tracks_string())

    def get_audio_region_string(self, a_region_uid):
        f_file = open(self.regions_audio_folder + "/" + str(a_region_uid), "r")
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_audio_region(self, a_region_uid):
        return pydaw_audio_region.from_str(self.get_audio_region_string(a_region_uid))

    def get_sample_graph_by_name(self, a_path, a_uid_dict=None):
        f_uid = self.get_wav_uid_by_name(a_path)
        return self.get_sample_graph_by_uid(f_uid)

    def get_sample_graph_by_uid(self, a_uid):
        f_pygraph_file = self.samplegraph_folder + "/" + str(a_uid)
        f_result = pydaw_sample_graph.create(f_pygraph_file)
        if not f_result.is_valid() or not f_result.check_mtime():
            print("Not valid, or else mtime is newer than graph time, deleting sample graph...")
            os.system('rm "' + f_pygraph_file + '"')
            pydaw_remove_item_from_sg_cache(f_pygraph_file)
            self.create_sample_graph(self.get_wav_path_by_uid(a_uid), a_uid)
            return pydaw_sample_graph.create(f_pygraph_file)
        else:
            return f_result

    def get_wav_uid_by_name(self, a_path, a_uid_dict=None, a_uid=None):
        """ Return the UID from the wav pool, or add to the pool if it does not exist """
        if a_uid_dict is None:
            f_uid_dict = self.get_wavs_dict()
        else:
            f_uid_dict = a_uid_dict
        f_path = str(a_path).replace("//", "/")
        if f_uid_dict.name_exists(f_path):
            return f_uid_dict.get_uid_by_name(f_path)
        else:
            f_uid = f_uid_dict.add_new_item(f_path, a_uid)
            self.create_sample_graph(f_path, f_uid)
            self.save_wavs_dict(f_uid_dict)
            self.commit("Add " + str(f_path) + " to pool")
            return f_uid

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
        self.this_dssi_gui.pydaw_generate_sample_graph(a_path, f_uid)
        f_pygraph_file = self.samplegraph_folder + "/" + str(f_uid)
        for i in range(100):
            if os.path.isfile(f_pygraph_file):
                sleep(0.1)
                return
            else:
                sleep(0.1)
        raise Exception

    def get_transport(self):
        try:
            f_file = open(self.project_folder + "/default.pytransport", "r")
        except:
            return pydaw_transport()  #defaults
        f_str = f_file.read()
        f_file.close()
        f_result = pydaw_transport.from_str(f_str)
        f_file_name = self.project_folder + "/default.pymididevice"
        if os.path.isfile(f_file_name):
            f_file = open(f_file_name)
            f_result.midi_keybd = f_file.read()
            f_file.close()
        return f_result

    def save_transport(self, a_transport):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pytransport, str(a_transport))

    def save_midi_device(self, a_device_str):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pymididevice, str(a_device_str))

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
        self.this_dssi_gui.pydaw_save_item(f_uid)
        self.save_items_dict(f_items_dict)
        return f_uid

    def copy_region(self, a_old_region_name, a_new_region_name):
        f_regions_dict = self.get_regions_dict()
        f_uid = f_regions_dict.add_new_item(a_new_region_name)
        f_old_uid = f_regions_dict.get_uid_by_name(a_old_region_name)
        self.save_file(pydaw_folder_regions,  str(f_uid), pydaw_read_file_text(self.regions_folder + "/" + str(f_old_uid)))
        self.save_file(pydaw_folder_regions_audio,  str(f_uid), pydaw_read_file_text(self.regions_audio_folder + "/" + str(f_old_uid)))
        self.save_regions_dict(f_regions_dict)
        return f_uid

    def region_audio_clone(self, a_dest_region_uid, a_src_region_name):
        f_regions_dict = self.get_regions_dict()
        f_uid = f_regions_dict.get_uid_by_name(a_src_region_name)
        self.save_file(pydaw_folder_regions_audio, str(a_dest_region_uid), pydaw_read_file_text(self.regions_audio_folder + "/" + str(f_uid)))
        self.this_dssi_gui.pydaw_reload_audio_items(a_dest_region_uid)

    def copy_item(self, a_old_item, a_new_item):
        f_items_dict = self.get_items_dict()
        f_uid = f_items_dict.add_new_item(a_new_item)
        f_old_uid = f_items_dict.get_uid_by_name(a_old_item)
        self.save_file(pydaw_folder_items,  str(f_uid), pydaw_read_file_text(self.items_folder + "/" + str(f_old_uid)))
        self.this_dssi_gui.pydaw_save_item(f_uid)
        self.save_items_dict(f_items_dict)
        return f_uid

    def save_item(self, a_name, a_item):
        if not self.suppress_updates:
            f_items_dict = self.get_items_dict()
            f_uid = f_items_dict.get_uid_by_name(a_name)
            self.save_file(pydaw_folder_items, str(f_uid), str(a_item))
            self.this_dssi_gui.pydaw_save_item(f_uid)

    def save_item_by_uid(self, a_uid, a_item, a_new_item=False):
        if not self.suppress_updates:
            f_uid = int(a_uid)
            self.save_file(pydaw_folder_items, str(f_uid), str(a_item), a_new_item)
            self.this_dssi_gui.pydaw_save_item(f_uid)

    def save_region(self, a_name, a_region):
        if not self.suppress_updates:
            f_regions_dict = self.get_regions_dict()
            f_uid = f_regions_dict.get_uid_by_name(a_name)
            self.save_file(pydaw_folder_regions, str(f_uid), str(a_region))
            self.this_dssi_gui.pydaw_save_region(f_uid)

    def save_song(self, a_song):
        if not self.suppress_updates:
            self.save_file("", pydaw_file_pysong, str(a_song))
            self.this_dssi_gui.pydaw_save_song()

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
            self.this_dssi_gui.pydaw_reload_audio_items(a_region_uid)

    def item_exists(self, a_item_name, a_name_dict=None):
        if a_name_dict is None:
            f_name_dict = self.get_items_dict()
        else:
            f_name_dict = a_name_dict
        if f_name_dict.uid_lookup.has_key(str(a_item_name)):
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
            if not f_items_dict.uid_lookup.has_key(f_result):
                if f_item_name == "item":
                    self.last_item_number = i
                return f_result

    def get_next_default_region_name(self, a_region_name="region"):
        f_regions_dict = self.get_regions_dict()
        if str(a_region_name) != "region" and not f_regions_dict.uid_lookup.has_key(str(a_region_name)):
            return str(a_region_name)
        for i in range(self.last_region_number, 10000):
            f_result = str(a_region_name) + "-" + str(i)
            if not f_regions_dict.uid_lookup.has_key(f_result):
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
        self.this_dssi_gui.stop_server()
        self.flush_history()

    def __init__(self, a_osc_url=None):
        self.last_item_number = 1
        self.last_region_number = 1
        self.history_files = []
        self.history_commits = []
        self.history_undo_cursor = 0
        self.this_dssi_gui = dssi_gui(a_osc_url)
        self.suppress_updates = False

class pydaw_song:
    def __init__(self):
        self.regions = {}

    def get_next_empty_pos(self):
        for f_i in range(300):
            if not self.regions.has_key(f_i):
                return f_i
        return None

    def get_index_of_region(self, a_uid):
        for k, v in self.regions.iteritems():
            if v == a_uid:
                return k
        assert(False)

    def insert_region(self, a_index, a_region_uid):
        f_new_dict = {}
        f_old_dict = {}
        for k, v in self.regions.iteritems():
            print str(k) + "|" + str(v)
            if k >= a_index:
                if k < 299:
                    f_new_dict[k + 1] = v
            else:
                f_old_dict[k] = v
        print "\n\n\n"
        for k, v in f_new_dict.iteritems():
            print str(k) + "|" + str(v)
            f_old_dict[k] = v
        print "\n\n\n"
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
        for k, v in self.regions.iteritems():
            f_result[k] = a_uid_dict.get_name_by_uid(v)
        return f_result

    def remove_region_ref(self, a_pos):
        if a_pos in self.regions:
            del self.regions[a_pos]

    def __str__(self):
        f_result = ""
        for k, v in self.regions.iteritems():
            f_result += str(k) + "|" + str(v) + "\n"
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
        while self.name_lookup.has_key(f_result):
            f_result = random.randint(1000000, 9999999)
        return f_result

    def __init__(self):
        self.name_lookup = {}
        self.uid_lookup = {}

    def add_item(self, a_uid, a_name):
        self.name_lookup[a_uid] = str(a_name)
        self.uid_lookup[a_name] = int(a_uid)

    def add_new_item(self, a_name, a_uid=None):
        if self.uid_lookup.has_key(a_name):
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
        return self.name_lookup.has_key(int(a_uid))

    def name_exists(self, a_name):
        return self.uid_lookup.has_key(str(a_name))

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
        f_result = ""
        for k, v in self.name_lookup.iteritems():
            f_result += str(k) + "|" + str(v) + "\n"
        return f_result + pydaw_terminating_char

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
                print("remove_item_ref removed bar: " + str(f_item.bar_num) + ", track: " + str(f_item.track_num))

    def __str__(self):
        f_result = ""
        if self.region_length_bars > 0:
            f_result += "L|" + str(self.region_length_bars) + "|0\n"
        for f_item in self.items:
            f_result += str(f_item.track_num) + "|" + str(f_item.bar_num) + "|" + str(f_item.item_uid) + "\n"
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
                f_result.add_item_ref_by_uid(int(f_item_arr[0]), int(f_item_arr[1]), f_item_arr[2])
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
                    f_new_cc = pydaw_cc((f_cc.start + f_beat_offset), a_plugin_index, f_cc_num, f_cc.cc_val)
                    f_new_cc.item_index = f_index
                    f_new_cc.beat_offset = f_beat_offset
                    f_this_cc_arr.append(f_new_cc)
            f_beat_offset += 4.0
            f_index += 1
            f_result_arr.append([])

        f_this_cc_arr.sort()
        for i in range(len(f_this_cc_arr) - 1):
            f_val_diff = abs(f_this_cc_arr[i + 1].cc_val - f_this_cc_arr[i].cc_val)
            if f_val_diff == 0:
                continue
            f_time_inc = .0625  #1/16 of a beat
            f_start = f_this_cc_arr[i].start + f_time_inc
            f_inc = (f_val_diff / ((f_this_cc_arr[i + 1].start - f_this_cc_arr[i].start) * 16.0))
            if (f_this_cc_arr[i].cc_val) > (f_this_cc_arr[i + 1].cc_val):
                f_inc *= -1.0
            f_new_val = f_this_cc_arr[i].cc_val + f_inc
            while True:
                f_index_offset = 0
                f_adjusted_start = f_start - f_this_cc_arr[i].beat_offset
                while f_adjusted_start >= 4.0:
                    f_index_offset += 1
                    f_adjusted_start -= 4.0
                f_interpolated_cc = pydaw_cc(round((f_adjusted_start), 4), a_plugin_index, f_cc_num, round(f_new_val, 4))
                f_new_val += f_inc
                f_new_index = f_this_cc_arr[i].item_index + f_index_offset
                if f_new_index >= len(f_result_arr):
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
        for i in range(len(f_this_pb_arr) - 1):
            f_val_diff = abs(f_this_pb_arr[i + 1].pb_val - f_this_pb_arr[i].pb_val)
            if f_val_diff == 0.0:
                continue
            f_time_inc = 0.0625
            f_start = f_this_pb_arr[i].start + f_time_inc

            f_val_inc = f_val_diff / ((f_this_pb_arr[i + 1].start - f_this_pb_arr[i].start) * 16.0)
            if f_this_pb_arr[i].pb_val > f_this_pb_arr[i + 1].pb_val:
                f_val_inc *= -1.0
            f_new_val = f_this_pb_arr[i].pb_val + f_val_inc

            while True:
                f_index_offset = 0
                f_adjusted_start = f_start - f_this_pb_arr[i].beat_offset
                while f_adjusted_start >= 4.0:
                    f_index_offset += 1
                    f_adjusted_start -= 4.0
                f_interpolated_pb = pydaw_pitchbend(round((f_adjusted_start), 4), round(f_new_val, 4))
                f_new_val += f_val_inc
                f_result_arr[f_this_pb_arr[i].item_index + f_index_offset].append(f_interpolated_pb)
                f_start += f_time_inc
                if f_start >= (f_this_pb_arr[i + 1].start - 0.0625):
                    break
        for f_i in range(len(a_items_list)):
            a_items_list[f_i].pitchbends += f_result_arr[f_i]
            a_items_list[f_i].pitchbends.sort()

def pydaw_velocity_mod(a_items, a_amt, a_line=False, a_end_amt=127, a_add=False, a_selected_only=False):
    f_start_beat = 0.0
    f_range_beats = 0.0
    f_tmp_index = 0
    f_break = False
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
    for f_item in a_items:
        for note in f_item.notes:
            if a_selected_only and not note.is_selected:
                continue
            if a_line:
                f_frac = ((note.start + f_beat_offset - f_start_beat)/f_range_beats)
                f_value = int(((a_end_amt - a_amt) * f_frac) + a_amt)
            else:
                f_value = int(a_amt)
            if a_add:
                note.velocity += f_value
            else:
                note.velocity = f_value
            if note.velocity > 127:
                print note.velocity
                note.velocity = 127
            elif note.velocity < 1:
                note.velocity = 1
        f_beat_offset += 4.0


class pydaw_item:
    def add_note(self, a_note, a_check=True):
        if a_check:
            for note in self.notes:
                if note.overlaps(a_note):
                    return False  #TODO:  return -1 instead of True, and the offending editor_index when False
        self.notes.append(a_note)
        self.notes.sort()
        if not a_check:
            self.fix_overlaps()
        return True

    def remove_note(self, a_note):
        try:
            self.notes.remove(a_note)
        except Exception as ex:
            print("\n\n\nException in remove_note:" + ex.message)
            print(repr(traceback.extract_stack()))
            print("\n\n\n")

    def velocity_mod(self, a_amt, a_start_beat=0.0, a_end_beat=4.0, a_line=False, a_end_amt=127, a_add=False, a_notes=None):
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

    def quantize(self, a_beat_frac, a_events_move_with_item=False, a_notes=None, a_selected_only=False):
        f_notes = []
        f_ccs = []
        f_pbs = []

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

        f_quantized_value = beat_frac_text_to_float(a_beat_frac)
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

        self.fix_overlaps()

        if a_events_move_with_item:
            for cc in f_ccs:
                cc.start -= shift_adjust
            for pb in f_pbs:
                pb.start -= shift_adjust

    def transpose(self, a_semitones, a_octave=0, a_notes=None, a_selected_only=False):
        f_total = a_semitones + (a_octave * 12)
        f_notes = []

        if a_notes is None:
            f_notes = self.notes
        else:
            for i in range(len(a_notes)):
                for f_note in self.notes:
                    if f_note == a_notes[i]:
                        f_notes.append(f_note)
                        break

        for note in f_notes:
            if a_selected_only and not note.is_selected:
                continue
            note.note_num += f_total
            if note.note_num < 0:
                note.note_num = 0
            elif note.note_num > 127:
                note.note_num = 127

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
        f_time_inc = abs(f_start_diff/float(f_val_diff))
        for i in range(0, (f_val_diff + 1)):
            self.ccs.append(pydaw_cc(round(f_start, 4), f_cc, f_start_val))
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
            self.pitchbends.append(pydaw_pitchbend(round(f_start, 4), f_start_val))
            f_start_val += f_inc
            f_start += f_time_inc
        self.pitchbends[(len(self.pitchbends) - 1)].pb_val = f_end_val #Ensure that the last value is what the user wanted it to be
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
                    f_result.add_note(pydaw_note.from_arr(f_event_arr))
                elif f_event_arr[0] == "c":
                    f_result.add_cc(pydaw_cc.from_arr(f_event_arr))
                elif f_event_arr[0] == "p":
                    f_result.add_pb(pydaw_pitchbend.from_arr(f_event_arr))
        return f_result

    def __init__(self):
        self.notes = []
        self.ccs = []
        self.pitchbends = []

    def __str__(self):
        f_result = ""
        self.notes.sort()
        self.ccs.sort()
        self.pitchbends.sort()
        for note in self.notes:
            f_result += note.__str__()
        for cc in self.ccs:
            f_result += cc.__str__()
        for pb in self.pitchbends:
            f_result += pb.__str__()
        f_result += pydaw_terminating_char
        return f_result

class pydaw_note:
    def __eq__(self, other):
        return((self.start == other.start) and (self.note_num == other.note_num) and (self.length == other.length) and \
        (self.velocity == other.velocity))

    def __lt__(self, other):
        return self.start < other.start

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
        f_result = pydaw_note(a_arr[1], a_arr[2], a_arr[3], a_arr[4])
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_note.from_arr(f_arr)

    def __str__(self):
        return "n|" + str(self.start) + "|" + str(self.length) + "|" + str(self.note_num) + "|" + str(self.velocity) + "\n"

class pydaw_cc:
    def __eq__(self, other):
        return ((self.start == other.start) and (self.cc_num == other.cc_num) and (self.cc_val == other.cc_val))

    def __lt__(self, other):
        return self.start < other.start

    def __init__(self, a_start, a_plugin_index, a_port_num, a_cc_val):
        self.start = float(a_start)
        self.plugin_index = int(a_plugin_index)
        self.cc_num = int(a_port_num) #This is really port_num, I'll rename later...
        self.cc_val = float(a_cc_val)

    def set_val(self, a_val):
        f_val = float(a_val)
        if f_val > 127.0:
            f_val = 127.0
        elif f_val < 0.0:
            f_val = 0.0
        self.cc_val = f_val

    def __str__(self):
        return "c|" + str(self.start) + "|" + str(self.plugin_index) + "|" + str(self.cc_num) + "|" + str(self.cc_val) + "\n"

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_cc(a_arr[1], a_arr[2], a_arr[3], a_arr[4])
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_cc.from_arr(f_arr)

    def clone(self):
        return pydaw_cc.from_str(str(self))

class pydaw_pitchbend:
    def __eq__(self, other):
        return ((self.start == other.start) and (self.pb_val == other.pb_val))  #TODO:  get rid of the pb_val comparison?

    def __lt__(self, other):
        return self.start < other.start

    def __init__(self, a_start, a_pb_val):
        self.start = float(a_start)
        self.pb_val = float(a_pb_val)

    def set_val(self, a_val):
        f_val = round(float(a_val), 4)
        if f_val > 1.0:
            f_val = 1.0
        elif f_val < -1.0:
            f_val = -1.0
        self.pb_val = f_val

    def __str__(self):
        return "p|" + str(self.start) + "|" + str(self.pb_val) + "\n"

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_pitchbend(a_arr[1], a_arr[2])
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_pitchbend.from_arr(f_arr)

    def clone(self):
        return pydaw_pitchbend.from_str(str(self))

class pydaw_tracks:
    def add_track(self, a_index, a_track):
        self.tracks[a_index] = a_track

    def __init__(self):
        self.tracks = {}

    def __str__(self):
        f_result = ""
        for k, v in self.tracks.iteritems():
            f_result += str(k) + "|" + str(v)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_tracks()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if not f_line == pydaw_terminating_char:
                f_line_arr = f_line.split("|")
                f_result.add_track(int(f_line_arr[0]), pydaw_track(int_to_bool(f_line_arr[1]), int_to_bool(f_line_arr[2]), \
                f_line_arr[3], f_line_arr[4], f_line_arr[5], f_line_arr[6]))
        return f_result

class pydaw_track:
    def __init__(self, a_solo=False, a_mute=False, a_vol=0, a_name="track", a_inst=0, a_bus_num=0):
        self.name = str(a_name)
        self.solo = bool(a_solo)
        self.mute = bool(a_mute)
        self.vol = int(a_vol)
        self.inst = int(a_inst)
        self.bus_num = int(a_bus_num)

    def __str__(self):
        return bool_to_int(self.solo) + "|" + bool_to_int(self.mute) + "|" + str(self.vol) + "|" + self.name + "|" + \
        str(self.inst) + "|" + str(self.bus_num) + "\n"

class pydaw_busses:
    def add_bus(self, a_index, a_bus):
        self.busses[int(a_index)] = a_bus

    def add_bus_from_str(self, a_str):
        f_arr = a_str.split("|")
        self.add_bus(f_arr[0], pydaw_bus(f_arr[1]))

    def __init__(self):
        self.busses = {}

    def __str__(self):
        f_result = ""
        for k, f_bus in self.busses.iteritems():
            f_result += str(k) + "|" + str(f_bus)
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

class pydaw_bus:
    def __init__(self, a_vol=0):
        self.vol = int(a_vol)

    def __str__(self):
        return str(self.vol) + "\n"

class pydaw_audio_tracks:
    def add_track(self, a_index, a_track):
        self.tracks[a_index] = a_track

    def __init__(self):
        self.tracks = {}

    def __str__(self):
        f_result = ""
        for k, v in self.tracks.iteritems():
            f_result += str(k) + "|" + str(v)
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_audio_tracks()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if not f_line == pydaw_terminating_char:
                f_line_arr = f_line.split("|")
                f_result.add_track(int(f_line_arr[0]), pydaw_audio_track(int_to_bool(f_line_arr[1]), int_to_bool(f_line_arr[2]), int(f_line_arr[3]), f_line_arr[4], int(f_line_arr[5])))
        return f_result

class pydaw_audio_track:
    def __init__(self, a_solo=False, a_mute=False, a_vol=0, a_name="track", a_bus_num=0):
        self.name = str(a_name)
        self.solo = bool(a_solo)
        self.mute = bool(a_mute)
        self.vol = int(a_vol)
        self.bus_num = int(a_bus_num)

    def __str__(self):
        return bool_to_int(self.solo) + "|" + bool_to_int(self.mute) + "|" + str(self.vol) + "|" + self.name + "|" + str(self.bus_num) + "\n"

class pydaw_audio_region:
    def __init__(self):
        self.items = {}

    """ Return the next available index, or -1 if none are available """
    def get_next_index(self):
        for i in range(pydaw_max_audio_item_count):
            if not self.items.has_key(i):
                return i
        return -1

    def split(self, a_index):
        f_region0 = pydaw_audio_region()
        f_region1 = pydaw_audio_region()
        for k, v in self.items.iteritems():
            if v.start_bar >= a_index:
                v.start_bar -= a_index
                if v.end_mode == 1:
                    v.end_bar -= a_index
                f_region1.items[k] = v
            else:
                if v.end_mode == 1 and v.end_bar >= a_index:
                    v.end_bar = a_index - 1
                    v.end_beat = 3.99
                f_region0.items[k] = v
        return f_region0, f_region1

    def add_item(self, a_index, a_item):
        self.items[int(a_index)] = a_item

    def remove_item(self, a_index):
        self.items.pop(int(a_index))

    def deduplicate_items(self):
        f_to_delete = []
        f_values = []
        for k, v in self.items.iteritems():
            f_str = str(v)
            if f_str in f_values:
                f_to_delete.append(k)
            else:
                f_values.append(f_str)
        for f_key in f_to_delete:
            print("Removing duplicate audio item at " + str(f_key))
            self.items.pop(f_key)

    def set_region_length(self, a_length):
        """ Remove any items not within the new length, or change any end points that are past
        the new end.  Return True if anything changed, otherwise False"""
        f_to_delete = []
        f_length = int(a_length)
        f_length_change_count = 0
        for k, v in self.items.iteritems():
            if v.start_bar >= f_length:
                f_to_delete.append(k)
                print("Item begins after new region length of " + str(a_length) + ", deleting: " + str(v))
            elif v.end_bar >= f_length:
                v.end_bar = f_length - 1
                v.end_beat = 3.99
                f_length_change_count += 1
        for f_key in f_to_delete:
            self.items.pop(f_key)
        if len(f_to_delete) > 0 or f_length_change_count > 0:
            return True
        else:
            return False

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
        for k, f_item in self.items.iteritems():
            f_result += str(k) + "|" + str(f_item)
        f_result += pydaw_terminating_char
        return f_result

class pydaw_audio_item:
    def __init__(self, a_uid, a_sample_start=0.0, a_sample_end=1000.0, a_start_bar=0, a_start_beat=0.0, a_end_mode=0, \
    a_end_bar=0, a_end_beat=0, a_timestretch_mode=3, a_pitch_shift=0.0, a_output_track=0, a_vol=0, a_timestretch_amt=1.0, \
    a_fade_in=0.0, a_fade_out=999.0, a_lane_num=0, a_pitch_shift_end=0.0, a_timestretch_amt_end=1.0, a_reversed=False, a_crispness=4):
        self.uid = int(a_uid)
        self.sample_start = float(a_sample_start)
        self.sample_end = float(a_sample_end)
        self.start_bar = int(a_start_bar)
        self.start_beat = round(float(a_start_beat), 6)
        self.end_mode = int(a_end_mode)  #0 == sample length, 1 == musical time
        self.end_bar = int(a_end_bar)
        self.end_beat = round(float(a_end_beat), 6)
        self.time_stretch_mode = int(a_timestretch_mode)
        self.pitch_shift = round(float(a_pitch_shift), 6)
        self.output_track = int(a_output_track)
        self.vol = int(a_vol)
        self.timestretch_amt = round(float(a_timestretch_amt), 6)
        self.fade_in = float(a_fade_in)
        self.fade_out = float(a_fade_out)
        self.lane_num = int(a_lane_num)
        self.pitch_shift_end = round(float(a_pitch_shift_end), 6)
        self.timestretch_amt_end = round(float(a_timestretch_amt_end), 6)
        self.reversed = a_reversed
        self.crispness = int(a_crispness) #This is specific to Rubberband

    def set_pos(self, a_bar, a_beat):
        f_bar = int(a_bar)
        f_beat = float(a_beat)
        if self.end_mode == 0:
            pass
        elif self.end_mode == 1:
            f_bar_diff = f_bar - self.start_bar
            f_beat_diff = f_beat - self.start_beat
            self.end_bar += f_bar_diff
            self.end_beat += f_beat_diff
        self.start_bar = f_bar
        self.start_beat = f_beat

    def clone(self):
        """ Using and abusing the functions that are already there... """
        return pydaw_audio_item.from_arr(str(self).strip("\n").split("|"))

    def __str__(self):
        return str(self.uid) + "|" + str(round(self.sample_start, 6)) + "|" + str(round(self.sample_end, 6)) \
        + "|" + str(self.start_bar) + "|" + str(round(self.start_beat, 6)) + "|" + str(self.end_mode) \
        + "|" + str(self.end_bar) + "|" + str(round(self.end_beat, 6)) + "|" + str(self.time_stretch_mode) \
        + "|" + str(self.pitch_shift) + "|" + str(self.output_track) + "|" + str(self.vol) + "|" + str(round(self.timestretch_amt, 6)) \
        + "|" + str(self.fade_in) + "|" + str(self.fade_out) + "|" + str(self.lane_num) + "|" + str(round(self.pitch_shift_end, 6 )) \
        + "|" + str(round(self.timestretch_amt_end, 6)) + "|" + str(bool_to_int(self.reversed)) + "|" + str(self.crispness) + "\n"

    @staticmethod
    def from_str(f_str):
        return pydaw_audio_item.from_arr(f_str.split("|"))

    @staticmethod
    def from_arr(a_arr):
        if len(a_arr) == 16:
            f_result = pydaw_audio_item(a_arr[0], a_arr[1], a_arr[2], a_arr[3], a_arr[4], a_arr[5], a_arr[6],\
            a_arr[7], a_arr[8], a_arr[9], a_arr[10], a_arr[11], a_arr[12], a_arr[13], a_arr[14], a_arr[15])
        elif len(a_arr) == 20:
            f_result = pydaw_audio_item(a_arr[0], a_arr[1], a_arr[2], a_arr[3], a_arr[4], a_arr[5], a_arr[6],\
            a_arr[7], a_arr[8], a_arr[9], a_arr[10], a_arr[11], a_arr[12], a_arr[13], a_arr[14], a_arr[15], a_arr[16], \
            a_arr[17], int_to_bool(a_arr[18]), a_arr[19])
        return f_result

class pydaw_audio_input_tracks:
    def add_track(self, a_index, a_track):
        self.tracks[a_index] = a_track

    def __init__(self):
        self.tracks = {}

    def __str__(self):
        f_result = ""
        for k, v in self.tracks.iteritems():
            f_result += str(k) + "|" + str(v)
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
                f_result.add_track(int(f_line_arr[0]), pydaw_audio_input_track(int_to_bool(f_line_arr[1]), int(f_line_arr[2]), int(f_line_arr[3])))
        return f_result

class pydaw_audio_input_track:
    def __init__(self, a_vol=0, a_output=0, a_input="None"):
        self.input = str(a_input)
        self.output = int(a_output)
        self.vol = int(a_vol)

    def __str__(self):
        return str(self.vol) + "|" + str(self.output) + "|" + str(self.input) + "\n"

class pydaw_transport:
    def __init__(self, a_bpm=140):
        self.bpm = a_bpm

    def __str__(self):
        return str(self.bpm) + "\n\\"

    @staticmethod
    def from_str(a_str):
        f_str = a_str.split("\n")[0]
        f_arr = f_str.split("|")
        return pydaw_transport(f_arr[0])

class pydaw_cc_map_item:
    def __init__(self, a_effects_only=False, a_rayv_port=0, a_wayv_port=0, a_euphoria_port=0, a_modulex_port=0):
        self.effects_only = bool_to_int(a_effects_only)
        self.rayv_port = int(a_rayv_port)
        self.wayv_port = int(a_wayv_port)
        self.euphoria_port = int(a_euphoria_port)
        self.modulex_port = int(a_modulex_port)

    def __str__(self):
        return str(self.effects_only) + "|" + str(self.rayv_port) + "|" + str(self.wayv_port) + "|" + str(self.euphoria_port) + "|" + str(self.modulex_port) + "\n"

class pydaw_cc_map:
    def __init__(self):
        self.map = {}

    def add_item(self, a_cc, a_item):
        self.map[int(a_cc)] = a_item

    def __str__(self):
        f_result = ""
        for k, v in self.map.iteritems():
            f_result += str(k) + "|" + str(v)
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
            f_result.map[int(f_line_arr[0])] = pydaw_cc_map_item(int_to_bool(f_line_arr[1]), f_line_arr[2], f_line_arr[3], f_line_arr[4], f_line_arr[5])
        return f_result

#From old sample_graph..py
pydaw_audio_item_scene_height = 1200.0
pydaw_audio_item_scene_width = 6000.0
pydaw_audio_item_scene_rect = QtCore.QRectF(0.0, 0.0, pydaw_audio_item_scene_width, pydaw_audio_item_scene_height)

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
    try:
        global_sample_graph_cache.pop(a_path)
    except KeyError:
        print("pydaw_remove_item_from_sg_cache: " + a_path + " not found.")

global_sample_graph_cache = {}

class pydaw_sample_graph:
    @staticmethod
    def create(a_file_name):
        f_file_name = str(a_file_name)
        global global_sample_graph_cache
        if global_sample_graph_cache.has_key(f_file_name):
            return global_sample_graph_cache[f_file_name]
        else:
            f_result = pydaw_sample_graph(f_file_name)
            global_sample_graph_cache[f_file_name] = f_result
            return f_result

    def __init__(self, a_file_name):
        f_file_name = str(a_file_name)
        self.file = None
        self.timestamp = None
        self.channels = None
        self.high_peaks = ([],[])
        self.low_peaks = ([],[])
        self.count = None
        self.length_in_seconds = None

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
                    self.file = str(f_line_arr[2]).strip("\n")  #Why does this have a newline on the end???
                elif f_line_arr[1] == "timestamp":
                    self.timestamp = int(f_line_arr[2])
                elif f_line_arr[1] == "channels":
                    self.channels = int(f_line_arr[2])
                elif f_line_arr[1] == "count":
                    self.count = int(f_line_arr[2])
                elif f_line_arr[1] == "length":
                    self.length_in_seconds = float(f_line_arr[2])
            elif f_line_arr[0] == "p":
                f_p_val = float(f_line_arr[3])
                if f_p_val > 1.0:
                    f_p_val = 1.0
                elif f_p_val < -1.0:
                    f_p_val = -1.0
                if f_line_arr[2] == "h":
                    self.high_peaks[int(f_line_arr[1])].append(f_p_val)
                elif f_line_arr[2] == "l":
                    self.low_peaks[int(f_line_arr[1])].append(f_p_val)
                else:
                    print("Invalid sample_graph [2] value " + f_line_arr[2] )
        for f_list in self.low_peaks:
            f_list.reverse()

    def is_valid(self):
        f_result = (self.file is not None) and (self.timestamp is not None) \
        and (self.channels is not None) and (self.count is not None)
        if not f_result:
            print("pydaw_sample_graph.is_valid() : " + str(self.file) + " failed the validity check...")
        return f_result

    def create_sample_graph(self, a_for_scene=False):
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
                f_result.lineTo(f_width_pos, f_section_div2 - (f_peak * f_section_div2))
                f_width_pos += f_width_inc
            for f_peak in self.low_peaks[f_i]:
                f_result.lineTo(f_width_pos, (f_peak * -1.0 * f_section_div2) + f_section_div2)
                f_width_pos -= f_width_inc
            f_result.closeSubpath()
            f_paths.append(f_result)
        return f_paths

    #def get_sample_graph_widget(self):
    #    f_paths = self.create_sample_graph()
    #    return pydaw_render_widget(f_paths)

    def check_mtime(self):
        """ Returns False if the sample graph is older than the file modified time """
        try:
            return self.timestamp > os.path.getmtime(self.file)
        except Exception as f_ex:
            print("Error getting mtime: " + f_ex.message)
            return False

class pydaw_midi_file_to_items:
    """ Convert the MIDI file at a_file to a dict of pydaw_item's with keys
        in the format (track#, channel#, bar#)"""
    def __init__(self, a_file):
        f_stream = midi.read_midifile(str(a_file))
        f_stream.trackpool.sort()
        #First fix the lengths of events that have note-off events
        f_note_on_dict = {}
        for f_event in f_stream.trackpool:
            if isinstance(f_event, midi.NoteOnEvent):
                f_note_on_dict[f_event.pitch] = f_event
            elif isinstance(f_event, midi.NoteOffEvent):
                if f_note_on_dict.has_key(f_event.pitch):
                    f_note_on_dict[f_event.pitch].length = float(f_event.tick - f_note_on_dict[f_event.pitch].tick) / float(f_stream.resolution)
                    f_note_on_dict.pop(f_event.pitch)
                else:
                    print("Error, note-off event does not correspond to a note-on event, ignoring event:\n" + str(f_event))

        self.result_dict = {}

        for f_event in f_stream.trackpool:
            if isinstance(f_event, midi.NoteOnEvent):
                f_velocity = f_event.velocity
                f_beat = (float(f_event.tick) / float(f_stream.resolution)) % 4.0
                f_bar = (int(f_event.tick) / int(f_stream.resolution)) / 4
                f_pitch = f_event.pitch
                f_length = f_event.length
                f_channel = f_event.channel
                f_track = f_event.track
                f_key = (f_track, f_channel, f_bar)
                if not self.result_dict.has_key(f_key):
                    self.result_dict[f_key] = pydaw_item()
                f_note = pydaw_note(f_beat, f_length, f_pitch, f_velocity)
                self.result_dict[f_key].add_note(f_note) #, a_check=False)

        f_min = 0
        f_max = 0

        for k, v in self.result_dict.iteritems():
            if k[2] < f_min:
                f_min = k[2]
            if k[2] > f_max:
                f_max = k[2]

        self.bar_count = f_max - f_min + 1
        self.bar_offset = f_min
        self.channel_count = self.get_channel_count()
        self.track_count = self.get_track_count()

        #Nested dict in format [track][channel][bar]
        self.track_map = {}
        for f_i in range(pydaw_midi_track_count):
            self.track_map[f_i] = {}

        for k, v in self.result_dict.iteritems():
            f_track, f_channel, f_bar = k
            if f_track < pydaw_midi_track_count:
                if not self.track_map[f_track].has_key(f_channel):
                    self.track_map[f_track][f_channel] = {}
                self.track_map[f_track][f_channel][f_bar - self.bar_offset] = v


    def get_track_count(self):
        f_result = []
        for k, v in self.result_dict.iteritems():
            if k[0] not in f_result:
                f_result.append(k[0])
        return len(f_result)

    def get_channel_count(self):
        f_result = []
        for k, v in self.result_dict.iteritems():
            if k[1] not in f_result:
                f_result.append(k[1])
        return len(f_result)

    def populate_region_from_track_map(self, a_project, a_name):
        f_actual_track_num = 0
        f_song = a_project.get_song()
        f_song_pos = f_song.get_next_empty_pos()
        if f_song_pos is not None:
            f_region_name = a_project.get_next_default_region_name(a_name)
            f_region_uid = a_project.create_empty_region(f_region_name)
            f_result_region = a_project.get_region_by_uid(f_region_uid)
            f_song.add_region_ref_by_uid(f_song_pos, f_region_uid)
            a_project.save_song(f_song)
        else:
            return False
        if self.bar_count > pydaw_max_region_length:
            f_result_region.region_length_bars = pydaw_max_region_length
        else:
            f_result_region.region_length_bars = self.bar_count
        for f_track, f_channel_dict in self.track_map.iteritems():
            for f_channel, f_bar_dict in self.track_map[f_track].iteritems():
                for f_bar, f_item in self.track_map[f_track][f_channel].iteritems():
                    f_this_item_name = str(a_name) + "-" + str(f_track) + "-" + str(f_channel) + "-" + str(f_bar)
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
            if f_actual_track_num >= pydaw_midi_track_count:
                    break
        a_project.save_region(f_region_name, f_result_region)
        return True


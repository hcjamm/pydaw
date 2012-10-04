# -*- coding: utf-8 -*-
"""
A class that contains methods and data for a PyDAW project.
"""

import sys, os
from shutil import copyfile
from os import listdir
from lms_session import lms_session
from dssi_gui import dssi_gui

pydaw_terminating_char = "\\"

""" For sending OSC messages """
def bool_to_int(a_bool):
    if a_bool:
        return "1"
    else:
        return "0"

class pydaw_project:
    def save_project(self):
        self.session_mgr.save_session_file()
    def save_project_as(self):
        pass

    def set_project_folders(self, a_project_file):
        self.project_folder = os.path.dirname(a_project_file)
        self.project_file = os.path.splitext(os.path.basename(a_project_file))[0]
        self.instrument_folder = self.project_folder + "/instruments"
        self.regions_folder = self.project_folder + "/regions"
        self.items_folder = self.project_folder + "/items"

    def instantiate_session_manager(self):
        self.session_mgr = lms_session(self.instrument_folder + '/' + self.project_file + '.pyses')

    def open_project(self, a_project_file):
        self.set_project_folders(a_project_file)
        if not os.path.exists(a_project_file):
            self.new_project(a_project_file)
        self.instantiate_session_manager()
        self.this_dssi_gui.pydaw_open_song(self.project_folder, self.project_file)
        
    def new_project(self, a_project_file):
        self.set_project_folders(a_project_file)

        project_folders = [
            self.project_folder,
            self.instrument_folder,
            self.regions_folder,
            self.items_folder
            ]

        for project_dir in project_folders:
            if not os.path.isdir(project_dir):
                os.makedirs(project_dir)

        self.instantiate_session_manager()

    def get_song_string(self):
        try:
            f_file = open(self.project_folder + "/" + self.project_file + ".pysong", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_song(self):
        return pydaw_song.from_str(self.get_song_string())

    def get_region_string(self, a_region_name):
        try:
            f_file = open(self.regions_folder + "/" + a_region_name + ".pyreg", "r")
        except:
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_region(self, a_region_name):
        return pydaw_region.from_str(a_region_name, self.get_region_string(a_region_name))

    def get_item_string(self, a_item_name):
        try:
            f_file = open(self.items_folder + "/" + a_item_name + ".pyitem", "r")
        except:
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_item(self, a_item_name):
        return pydaw_item.from_str(self.get_item_string(a_item_name))

    def get_tracks_string(self):
        try:
            f_file = open(self.project_folder + "/" + self.project_file + ".pytracks", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_tracks(self):
        return pydaw_tracks.from_str(self.get_tracks_string())

    def create_empty_region(self, a_region_name):
        #TODO:  Check for uniqueness, from a pydaw_project.check_for_uniqueness method...
        f_file = open(self.regions_folder + "/" + a_region_name + ".pyreg", 'w')
        f_file.write(pydaw_terminating_char)
        f_file.close()

    def create_empty_item(self, a_item_name):
        #TODO:  Check for uniqueness, from a pydaw_project.check_for_uniqueness method...
        f_file = open(self.items_folder + "/" + a_item_name + ".pyitem", 'w')
        f_file.write(pydaw_terminating_char)
        f_file.close()

    def copy_region(self, a_old_region, a_new_region):
        copyfile(self.regions_folder + "/" + a_old_region + ".pyreg", self.regions_folder + "/" + a_new_region + ".pyreg")

    def copy_item(self, a_old_item, a_new_item):
        copyfile(self.items_folder + "/" + a_old_item + ".pyitem", self.items_folder + "/" + a_new_item + ".pyitem")

    def save_item(self, a_name, a_item):
        f_file = open(self.items_folder + "/" + a_name + ".pyitem", 'w')
        f_file.write(a_item.__str__())
        f_file.close()
        self.this_dssi_gui.pydaw_save_item(a_name)

    def save_region(self, a_name, a_region):
        f_file_name = self.regions_folder + "/" + a_name + ".pyreg"
        f_file = open(f_file_name, 'w')
        f_file.write(a_region.__str__())
        f_file.close()
        self.this_dssi_gui.pydaw_save_region(a_name)

    def save_song(self, a_song):
        f_file = open(self.project_folder + "/" + self.project_file + ".pysong", 'w')
        f_file.write(a_song.__str__())
        f_file.close()
        self.this_dssi_gui.pydaw_save_song(self.project_file)

    def save_tracks(self, a_tracks):
        f_file = open(self.project_folder + "/" + self.project_file + ".pytracks", 'w')
        f_file.write(a_tracks.__str__())
        f_file.close()
        #Is there a need for a configure message here?

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
        
    def quit_handler(self):
        self.session_mgr.quit_hander()
        self.this_dssi_gui.stop_server()        

    def __init__(self, a_osc_url=None):
        self.last_item_number = 0
        self.last_region_number = 0
        #self.new_project(a_project_file)
        self.this_dssi_gui = dssi_gui(a_osc_url)

#The below classes are used to generate the saved file strings that will properly enforce the standard, rather than relying on developers to do it themselves

class pydaw_song:
    def add_region_ref(self, a_pos, a_region_name):
        self.regions[a_pos] = a_region_name #TODO:  It may be best just to go ahead and make a child class, for when other parameters are added later

    def remove_region_ref(self, a_pos):
        if a_pos in self.regions:
            del self.regions[a_pos]

    def __init__(self):
        self.regions = {}

    def __str__(self):
        f_result = ""
        for k, v in self.regions.iteritems():
            f_result += str(k) + "|" + v + "\n"
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
                f_result.add_region_ref(int(f_region[0]), f_region[1])
        return f_result

class pydaw_region:
    def add_item_ref(self, a_bar_num, a_track_num, a_item_name):
        self.remove_item_ref(a_bar_num, a_track_num)
        self.items.append(pydaw_region.region_item(a_track_num, a_bar_num, a_item_name))

    def remove_item_ref(self, a_bar_num, a_track_num):
        for f_item in self.items:
            if f_item.bar_num == a_bar_num and f_item.track_num == a_track_num:
                self.items.remove(f_item)

    def __init__(self, a_name):
        self.items = []
        self.name = a_name

    def __str__(self):
        f_result = ""
        for f_item in self.items:
            f_result += str(f_item.track_num) + "|" + str(f_item.bar_num) + "|" + f_item.item_name + "\n"
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_name, a_str):
        f_result = pydaw_region(a_name)
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == pydaw_terminating_char:
                break
            else:
                f_item_arr = f_line.split("|")
                f_result.add_item_ref(int(f_item_arr[0]), int(f_item_arr[1]), f_item_arr[2])
        return f_result

    class region_item:
        def __init__(self, a_track_num, a_bar_num, a_item_name):
            self.track_num = a_track_num
            self.bar_num = a_bar_num
            self.item_name = a_item_name

class pydaw_item:
    def add_note(self, a_note):
        for note in self.notes:
            if note.overlaps(a_note):
                return False  #TODO:  return -1 instead of True, and the offending editor_index when False
        self.notes.append(a_note)
        self.notes.sort()
        return True
        
    def remove_note(self, a_index):
        for i in range(0, len(self.notes)):
            if self.notes[i].editor_index == a_index:
                self.notes.pop(i)
                break
            
    def get_next_default_note(self):
        pass

    def add_cc(self, a_cc):
        for cc in self.ccs:
            if a_cc == cc:
                return False #TODO:  return -1 instead of True, and the offending editor_index when False
        self.ccs.append(a_cc)
        self.ccs.sort()
        return True
        
    def remove_cc(self, a_index):
        for i in range(0, len(self.ccs)):
            if self.ccs[i].editor_index == a_index:
                self.ccs.pop(i)
                break
            
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
        return f_result

    def __init__(self):
        self.notes = []
        self.ccs = []

    def __str__(self):
        f_result = ""
        self.notes.sort()
        self.ccs.sort()
        for note in self.notes:
            f_result += note.__str__()
        for cc in self.ccs:
            f_result += cc.__str__()
        f_result += pydaw_terminating_char
        return f_result

class pydaw_note:
    def __lt__(self, other):
        return self.start < other.start
    
    def __init__(self, a_editor_index, a_start, a_length, a_note, a_note_number, a_velocity):
        self.start = float(a_start)
        self.length = float(a_length)
        self.note = a_note
        self.velocity = int(a_velocity)
        self.note_num = int(a_note_number)
        self.editor_index = int(a_editor_index)
        self.end = self.length + self.start
        
    def overlaps(self, other):
        if self.note_num == other.note_num:
            if other.start >= self.start and other.start < self.end:
                return True
            elif other.start < self.start and other.end > self.start:
                return True
        return False

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_note(a_arr[1], a_arr[2], a_arr[3], a_arr[4], a_arr[5], a_arr[6])
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_note.from_arr(f_arr)

    def __str__(self):
        return "n|" + str(self.editor_index) + "|" + str(self.start) + "|" + str(self.length) + "|" + self.note + "|" + str(self.note_num) + "|" + str(self.velocity) + "\n"

class pydaw_cc:
    def __eq__(self, other):
        return ((self.start == other.start) and (self.cc_num == other.cc_num) and (self.cc_val == other.cc_val))
        
    def __lt__(self, other):
        return self.start < other.start
    
    def __init__(self, a_editor_index, a_start, a_cc_num, a_cc_val):
        self.start = float(a_start)
        self.cc_num = int(a_cc_num)
        self.cc_val = int(a_cc_val)
        self.editor_index = int(a_editor_index)

    def __str__(self):
        return "c|" + str(self.editor_index) + "|" + str(self.start) + "|" + str(self.cc_num) + "|" + str(self.cc_val) + "\n"

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_cc(a_arr[1], a_arr[2], a_arr[3], a_arr[4])
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_note.from_arr(f_arr)

class pydaw_tracks:
    def add_track(self, a_index, a_track):
        self.tracks[a_index] = a_track

    def __init__(self):
        self.tracks = {}

    def __str__(self):
        f_result = ""
        for k, v in self.tracks.iteritems():            
            f_result += str(k) + "|" + bool_to_int(v.solo) + "|" + bool_to_int(v.mute) + "|" + bool_to_int(v.rec) + "|" + str(v.vol) + "|" + v.name + "|" + str(v.inst) + "\n"
        f_result += pydaw_terminating_char
        return f_result

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_tracks()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if not f_line == pydaw_terminating_char:
                f_line_arr = f_line.split("|")
                if f_line_arr[1] == "1": f_solo = True
                else: f_solo = False
                if f_line_arr[2] == "1": f_mute = True
                else: f_mute = False
                if f_line_arr[3] == "1": f_rec = True
                else: f_rec = False
                f_result.add_track(int(f_line_arr[0]), pydaw_track(f_solo, f_mute, f_rec, int(f_line_arr[4]), f_line_arr[5], int(f_line_arr[6])))
        return f_result

class pydaw_track:
    def __init__(self, a_solo, a_mute, a_rec, a_vol, a_name, a_inst):
        self.name = a_name
        self.solo = a_solo
        self.mute = a_mute
        self.rec = a_rec
        self.vol = a_vol
        self.inst = a_inst
# -*- coding: utf-8 -*-
"""
A class that contains methods and data for a PyDAW project.
"""

import sys, os
from shutil import copyfile
from os import listdir
from lms_session import lms_session

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

    def open_project(self, a_project_folder, a_project_file):
        self.set_project_folders(a_project_folder, a_project_file)
        self.instantiate_session_manager()
        
        
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
            return ""
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
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result
        
    def get_tracks(self):
        pass
        
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
        
    def save_song(self, a_song):
        f_file = open(self.project_folder + "/" + self.project_file + ".pysong", 'w')
        f_file.write(a_song.__str__())
        f_file.close()
        
    def save_tracks(self, a_string):
        f_file = open(self.project_folder + "/" + self.project_file + ".pytracks", 'w')
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
        
    def __init__(self, a_project_file=None):
        self.last_item_number = 0
        self.last_region_number = 0
        self.new_project(a_project_file)

#The below classes are used to generate the saved file strings that will properly enforce the standard, rather than relying on developers to do it themselves

class pydaw_song:
    def add_region_ref(self, a_pos, a_region_name):
        self.regions[a_pos] = a_region_name #TODO:  It may be best just to go ahead and make a child class, for when other parameters are added later
    
    def __init__(self):
        self.regions = {}
        
    def __str__(self):
        f_result = ""
        for k, v in self.regions.iteritems():
            f_result += str(k) + "|" + v + "\n"
        return f_result
    @staticmethod    
    def from_str(a_str):        
        f_result = pydaw_song()        
        f_arr = a_str.split("\n")     
        for f_line in f_arr:
            if not f_line == "":
                print("pydaw_song f_line :" + f_line)
                f_region = f_line.split("|")
                f_result.add_region_ref(int(f_region[0]), f_region[1])
        return f_result            
                
class pydaw_region:
    def add_item_ref(self, a_track_num, a_bar_num, a_item_name):
        self.items.append(pydaw_region.region_item(a_track_num, a_bar_num, a_item_name))        
    
    def __init__(self, a_name):
        self.items = []
        self.name = a_name
    def __str__(self):
        f_result = ""
        for f_item in self.items:
            f_result += f_item.track_num + "|" + f_item.bar_num + "|" + f_item.item_name + "\n"
        return f_result
        
    def from_str(self, a_name, a_str):
        f_result = pydaw_region(a_name)
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            f_item_arr = f_line.split("|")
            f_result.add_item_ref(f_item_arr[0], f_item_arr[1], f_item_arr[2])
        return f_result
        
    class region_item:
        def __init__(self, a_track_num, a_bar_num, a_item_name):
            self.track_num = a_track_num
            self.bar_num = a_bar_num
            self.item_name = a_item_name
    
class pydaw_item:    
    def add_note(self, a_index, a_note):
        self.notes[a_index] = a_note
        
    def add_cc(self, a_index, a_cc):
        self.ccs[a_index] = a_cc
    
    @staticmethod
    def from_str(a_str):
        f_result = pydaw_item()
        f_arr = a_str.split("\n")
        for f_event_str in f_arr:
            f_event_arr = f_event_str.split("|")
            if f_event_arr[1] == "n":
                f_result.add_note(f_event_arr[0], pydaw_note.from_arr(f_event_arr))
            elif f_event_arr[1] == "c":
                f_result.add_cc(f_event_arr[0], pydaw_cc.from_arr(f_event_arr))
        return f_result                
    
    def __init__(self):
        self.notes = {}
        self.ccs = {}
        
    def __str__(self):
        f_result = ""
        for k, v in self.notes.iteritems():
            f_result += str(k) + "|" + v            
        for k, v in self.ccs.iteritems():
            f_result += str(k) + "|" + v            
        return f_result
    
        
class pydaw_note:
    def __init__(self, a_start, a_length, a_note_number, a_velocity, a_extra_data=""):
        self.start = a_start
        self.length = a_length
        self.note = a_note_number
        self.velocity = a_velocity
        self.extra_data = a_extra_data
        
    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_note(a_arr[2], a_arr[3], a_arr[4], a_arr[5], a_arr[6])
        return f_result
        
    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_note.from_arr(f_arr)
                
    def __str__(self):
        return "n|" + self.start + "|" + self.length + "|" + self.note + "|" + self.velocity + "|" + self.extra_data
    
class pydaw_cc:
    def __init__(self, a_start, a_cc_num, a_cc_val):
        self.start = a_start
        self.cc_num = a_cc_num
        self.cc_val = a_cc_val
        
    def __str__(self):
        return "c|" + self.start + "|" + self.cc_num + "|" + self.cc_val

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_cc(a_arr[1], a_arr[2], a_arr[3], a_arr[4], a_arr[5])        
        return f_result
        
    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_note.from_arr(f_arr)
    
class pydaw_tracks:
    def add_track(self, a_index, a_solo, a_mute, a_rec, a_vol, a_name, a_inst):
        self.tracks[a_index] = pydaw_track(a_solo, a_mute, a_rec, a_vol, a_name, a_inst)        
    def __init__(self):
        self.tracks = {}
    @staticmethod
    def from_str(a_str):
        f_result = pydaw_tracks()
        return f_result
    
class pydaw_track:
    def __init__(self, a_solo, a_mute, a_rec, a_vol, a_name, a_inst):
        self.name = a_name
        self.solo = a_solo
        self.mute = a_mute
        self.rec = a_rec
        self.vol = a_vol
        self.inst = a_inst
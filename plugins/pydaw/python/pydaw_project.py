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
        #this_main_window.setWindowTitle('PyDAW - ' + self.project_file)
        
    def get_song_string(self):
        try:
            f_file = open(self.project_folder + "/" + self.project_file + ".pysong", "r")
        except:
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result
        
    def get_region_string(self, a_region_name):
        try:
            f_file = open(self.regions_folder + "/" + a_region_name + ".pyreg", "r")
        except:
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result
                
    def get_item_string(self, a_item_name):
        try:
            f_file = open(self.items_folder + "/" + a_item_name + ".pyitem", "r")
        except:
            return ""
        f_result = f_file.read()
        f_file.close()
        return f_result
        
    def get_tracks_string(self):
        try:
            f_file = open(self.project_folder + "/" + self.project_file + ".pytracks", "r")
        except:
            return ""
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
        
    def save_song(self, a_string):
        f_file = open(self.project_folder + "/" + self.project_file + ".pysong", 'w')
        f_file.write(a_string)
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


    
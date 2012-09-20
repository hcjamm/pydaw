# -*- coding: utf-8 -*-
#
# Python port of LMS Session Manager
# 
# The C++/Qt version will be deprecated at some point in the future, hence
# I chose not to incorporate the C++ code through any of Python's mechanisms

import os
import os.path

from time import sleep
import subprocess

lms_notify_directory = '/notify/'
lms_delimiter = '|'
lms_instrument_count = 16

class lms_session:
    def __init__(self, a_project_path):
        self.first_quit = True
        self.full_project_file_path = a_project_path
        f_project_path_arr = str.split(a_project_path, '/')
        self.project_directory = ''
        self.project_name = ''
        
        for i in range(1, len(f_project_path_arr)):
            if i == (len(f_project_path_arr) - 1):
                self.project_name = str.split(f_project_path_arr[i], '.pyses')[0]
            else:
                self.project_directory += '/' + f_project_path_arr[i]
                
        self.processes = {}
        self.instance_names = {}
        #This one will probably be deprecated after an initial working prototype is finished
        self.select_instrument = {}
        
        for i in range(0, lms_instrument_count):
            self.processes[i] = None
            self.instance_names[i] = ''
            self.select_instrument[i] = 0
        
        f_notify_dir = self.project_directory + lms_notify_directory
        
        if(os.path.isdir(f_notify_dir) == False):
            os.mkdir(f_notify_dir)
            
        self.supress_instrument_change = False
        
        if(os.path.exists(self.full_project_file_path)):
            f_file = open(self.full_project_file_path)
            
            f_count = 0          
            
            for line in f_file:
                if f_count >= lms_instrument_count:
                    break
                
                f_line_list = str.split(line, lms_delimiter)
                self.instance_names[f_count] = f_line_list[1]
                f_inst_index = int(f_line_list[0])
                self.select_instrument[f_count] = f_inst_index
                f_count += 1                
                #Pause between so that we don't thrash the CPU and hard disk
                #if f_inst_index > 0:
                    #sleep(2)
                            
            f_file.close()
        else:
            pass #TODO:  Create a new file, or something...
                
                
    def instrument_index_changed(self, a_instrument_index, a_index, a_instance_name):
        if self.supress_instrument_change:
            return
            
        #if self.instance_names[a_instrument_index] == '':
        #    print("Error:  The instance name must not be empty") #TODO:  Create a default instance name
        #    return
                
        self.instance_names[a_instrument_index] = a_instance_name.strip() #self.instance_names[a_instrument_index])
        
        #test for uniqueness
        for i in range(0, lms_instrument_count):
            if i != a_instrument_index:
                if self.instance_names[a_instrument_index] == self.instance_names[i]:
                    print('Error:  Instance names must be unique')
                    self.supress_instrument_change = True
                    self.select_instrument[a_instrument_index] = 0
                    self.supress_instrument_change = False
                    return
                    
        #We've passed all of the tests, now run the instrument
        self.select_instrument[a_instrument_index] = a_index

        f_args = ["-a",  "-p", self.project_directory, "-c", self.project_name + "-" + self.instance_names[a_instrument_index]]
        print(f_args)
        
        if a_index == 0: #Send the quit signal
            f_notify_dir = self.project_directory + lms_notify_directory
            f_quit_file_path = f_notify_dir + self.project_name + "-" + self.instance_names[a_instrument_index] + ".quit"
            f_quit_file = open(f_quit_file_path, 'w')
            f_quit_file.write("Created by PyDAW\n")
            f_quit_file.close()
        elif a_index == 1:
            self.processes[a_instrument_index] = subprocess.Popen(['lms-jack-dssi-host', 'euphoria.so'] + f_args)            
        elif a_index == 2:
            self.processes[a_instrument_index] = subprocess.Popen(['lms-jack-dssi-host', 'ray_v.so'] + f_args)
        else:
            print("Invalid index: " + a_index)
        
    
    def save_session_file(self):
        f_file = open(self.full_project_file_path, 'w')
        
        for f_i in range(0, lms_instrument_count):
            f_file.write(str(self.select_instrument[f_i]) + lms_delimiter +
                self.instance_names[f_i] + "\n")
                
        f_file.close()
        
        f_notify_dir = self.project_directory + lms_notify_directory
        
        if os.path.isdir(f_notify_dir) == False:
            os.makedirs(f_notify_dir)
            
        #Now notify the instruments to save their state
        for f_i in range(0, lms_instrument_count):
            if self.select_instrument[f_i] > 0:
                f_save_file = open(f_notify_dir + self.project_name + "-" + self.instance_names[f_i] + ".save", 'w')
                f_save_file.write("Created by PyDAW\n")
                #Does flush need to be called here?
                f_save_file.close()
    
    def quit_hander(self):
        print("Quitting...")
        f_notify_dir = self.project_directory + lms_notify_directory
        print("Sending quit notify messages in " + f_notify_dir)
        for f_i in range(0, lms_instrument_count):
            if self.select_instrument[f_i] > 0:
                f_quit_file_path = f_notify_dir + self.project_name + "-" + self.instance_names[f_i] + ".quit"
                f_quit_file = open(f_quit_file_path, 'w')
                f_quit_file.write("Created by PyDAW\n")
                f_quit_file.close()
                
    def new_project(self):
        pass
    
    def open_project(self):
        pass
                
                

        
        
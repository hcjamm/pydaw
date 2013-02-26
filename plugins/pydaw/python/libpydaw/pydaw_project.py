# -*- coding: utf-8 -*-
"""
A class that contains methods and data for a PyDAW project.
"""

import os, random
from shutil import copyfile, move
from time import sleep

#from lms_session import lms_session #deprecated
from dssi_gui import dssi_gui
from pydaw_git import pydaw_git_repo
from math import log

from sample_graph import pydaw_sample_graphs

pydaw_bus_count = 5
pydaw_audio_track_count = 8
pydaw_audio_input_count = 5
pydaw_midi_track_count = 16
pydaw_max_audio_item_count = 32

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
    return (440.0 * pow(2.0,(a_pitch - 57.0) * 0.0833333));

def pydaw_hz_to_pitch(a_hz):
    return ((12.0 * log(a_hz * (1.0/440.0), 2.0)) + 57.0);

def pydaw_pitch_to_ratio(a_pitch):
    return (1.0/pydaw_pitch_to_hz(0.0)) * pydaw_pitch_to_hz(a_pitch)

class pydaw_project:
    def save_project(self):
        self.this_dssi_gui.pydaw_save_tracks()
        sleep(3)
        self.git_repo.git_add(self.instrument_folder + "/*")
        self.git_repo.git_add(self.audiofx_folder + "/*")
        self.git_repo.git_add(self.busfx_folder + "/*")
        self.git_repo.git_commit("-a", "Saved plugin state")

    def record_stop_git_commit(self):
        """ This should be called once recording has stopped to catch-up Git """
        self.git_repo.git_add(self.regions_folder + "/*")
        self.git_repo.git_add(self.items_folder + "/*")
        self.git_repo.git_commit("-a", "Save recorded items/regions")

    def save_project_as(self, a_file_name):
        self.save_project()  #This is necessary to capture the plugin states before copying everything over...  Otherwise the instruments and effects may not be what they were at this time...
        f_file_name = str(a_file_name)
        print("Saving project as " + f_file_name + " ...")
        f_new_project_folder = os.path.dirname(f_file_name)
        f_cmd = "cp -r " + self.project_folder + "/* " + f_new_project_folder + "/"
        os.popen(f_cmd)
        print(f_new_project_folder + "/" + self.project_file + " | " + a_file_name)
        move(f_new_project_folder + "/" + self.project_file + ".pydaw2", a_file_name)
        self.set_project_folders(f_file_name)
        self.this_dssi_gui.pydaw_open_song(self.project_folder)
        self.git_repo.repo_dir = self.project_folder

    def set_project_folders(self, a_project_file):
        self.project_folder = os.path.dirname(a_project_file)
        self.project_file = os.path.splitext(os.path.basename(a_project_file))[0]
        self.instrument_folder = self.project_folder + "/instruments"
        self.regions_folder = self.project_folder + "/regions"
        self.items_folder = self.project_folder + "/items"
        self.audio_folder = self.project_folder + "/audio"
        self.audio_tmp_folder = self.project_folder + "/audio/tmp"
        self.samples_folder = self.project_folder + "/samples"  #Placeholder for future functionality
        self.audiofx_folder = self.project_folder + "/audiofx"
        self.busfx_folder = self.project_folder + "/busfx"
        self.samplegraph_folder = self.project_folder + "/samplegraph"
        self.audio_automation_folder = self.project_folder + "/audio_automation"
        self.bus_automation_folder = self.project_folder + "/bus_automation"

    def open_project(self, a_project_file, a_notify_osc=True):
        self.set_project_folders(a_project_file)
        if not os.path.exists(a_project_file):
            print("project file " + a_project_file + " does not exist, creating as new project")
            self.new_project(a_project_file)
        else:
            self.git_repo = pydaw_git_repo(self.project_folder)
        if a_notify_osc:
            self.this_dssi_gui.pydaw_open_song(self.project_folder)

    def new_project(self, a_project_file, a_notify_osc=True):
        self.set_project_folders(a_project_file)

        project_folders = [
            self.project_folder, self.instrument_folder, self.regions_folder,
            self.items_folder, self.audio_folder, self.samples_folder,
            self.audiofx_folder, self.busfx_folder, self.samplegraph_folder,
            self.audio_automation_folder, self.bus_automation_folder, self.audio_tmp_folder
            ]

        for project_dir in project_folders:
            print(project_dir)
            if not os.path.isdir(project_dir):
                os.makedirs(project_dir)
        if not os.path.exists(a_project_file):
            f_file = open(a_project_file, 'w')
            f_file.write("This file does is not supposed to contain any data, it is only a placeholder for saving and opening the project :)")
            f_file.close()

        f_pysong_file = self.project_folder + "/default.pysong"
        if not os.path.exists(f_pysong_file):
            f_file = open(f_pysong_file, 'w')
            f_file.write(pydaw_terminating_char)
            f_file.close()
        f_pytransport_file = self.project_folder + "/default.pytransport"
        if not os.path.exists(f_pytransport_file):
            f_transport_instance = pydaw_transport()
            f_file = open(f_pytransport_file, 'w')
            f_file.write(str(f_transport_instance))
            f_file.close()
        f_pytracks_file = self.project_folder + "/default.pytracks"
        if not os.path.exists(f_pytracks_file):
            f_file = open(f_pytracks_file, 'w')
            f_midi_tracks_instance = pydaw_tracks()
            for i in range(16):
                f_midi_tracks_instance.add_track(i, pydaw_track(0,0,0,0,"track"+str(i + 1),0))
            f_file.write(str(f_midi_tracks_instance))
            f_file.close()
        f_pyaudio_file = self.project_folder + "/default.pyaudio"
        if not os.path.exists(f_pyaudio_file):
            f_file = open(f_pyaudio_file, 'w')
            f_pyaudio_instance = pydaw_audio_tracks()
            for i in range(8):
                f_pyaudio_instance.add_track(i, pydaw_audio_track(0,0,0,"track"+str(i + 1)))
            f_file.write(str(f_pyaudio_instance))
            f_file.close()

        f_pyaudio_item_file = self.project_folder + "/default.pyaudioitem"
        if not os.path.exists(f_pyaudio_item_file):
            f_file = open(f_pyaudio_item_file, 'w')
            f_file.write(pydaw_terminating_char)
            f_file.close()
        f_pybus_file = self.project_folder + "/default.pybus"
        if not os.path.exists(f_pybus_file):
            f_file = open(f_pybus_file, 'w')
            f_pybus_instance = pydaw_busses()
            for i in range(5):
                f_pybus_instance.add_bus(i, pydaw_bus())
            f_file.write(str(f_pybus_instance))
            f_file.close()

        f_pyinput_file = self.project_folder + "/default.pyinput"
        if not os.path.exists(f_pyinput_file):
            f_file = open(f_pyinput_file, 'w')
            f_input_instance = pydaw_audio_input_tracks()
            for i in range(5):
                f_input_instance.add_track(i, pydaw_audio_input_track(0,0,0))
            f_file.write(str(f_input_instance))
            f_file.close()

        f_pysamplegraphs_file = self.project_folder + "/default.pygraphs"
        if not os.path.exists(f_pysamplegraphs_file):
            f_file = open(f_pysamplegraphs_file, 'w')
            f_file.write(pydaw_terminating_char)
            f_file.close()

        f_git_ignore_file = self.project_folder + "/.gitignore"
        if not os.path.exists(f_git_ignore_file):
            f_file = open(f_git_ignore_file, 'w')
            f_file.write("default.pygraphs\nsamplegraph/*.pygraph\nsamples/*\naudio/*\n")
            f_file.close()

        self.git_repo = pydaw_git_repo(self.project_folder)
        self.git_repo.git_init()

        for i in range(pydaw_audio_track_count):
            f_automation_file = self.audio_automation_folder + "/" + str(i) + ".pyauto"
            if not os.path.exists(f_automation_file):
                f_file = open(f_automation_file, 'w')
                f_file.write(pydaw_terminating_char)
                f_file.close()
                self.git_repo.git_add(f_automation_file)

        for i in range(pydaw_bus_count):
            f_automation_file = self.bus_automation_folder + "/" + str(i) + ".pyauto"
            if not os.path.exists(f_automation_file):
                f_file = open(f_automation_file, 'w')
                f_file.write(pydaw_terminating_char)
                f_file.close()
                self.git_repo.git_add(f_automation_file)

        self.git_repo.git_add(f_pysong_file)
        self.git_repo.git_add(f_pytracks_file)
        self.git_repo.git_add(f_pytransport_file)
        self.git_repo.git_add(a_project_file)
        self.git_repo.git_add(f_pyaudio_file)
        self.git_repo.git_add(f_pyaudio_item_file)
        self.git_repo.git_add(f_pybus_file)
        self.git_repo.git_add(f_pyinput_file)
        self.git_repo.git_add(f_git_ignore_file)
        self.git_repo.git_commit("-a", "Created new project")
        if a_notify_osc:
            self.this_dssi_gui.pydaw_open_song(self.project_folder)

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

    def get_region_string(self, a_region_name):
        try:
            f_file = open(self.regions_folder + "/" + str(a_region_name) + ".pyreg", "r")
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

    def get_audio_automation_string(self, a_track_num):
        try:
            f_file = open(self.audio_automation_folder + "/" + str(a_track_num) + ".pyauto", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_audio_automation(self, a_track_num):
        return pydaw_song_level_ccs.from_str(self.get_audio_automation_string(a_track_num))

    def get_bus_automation_string(self, a_track_num):
        try:
            f_file = open(self.bus_automation_folder + "/" + str(a_track_num) + ".pyauto", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_bus_automation(self, a_track_num):
        return pydaw_song_level_ccs.from_str(self.get_bus_automation_string(a_track_num))

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

    def get_audio_input_tracks_string(self):
        try:
            f_file = open(self.project_folder + "/default.pyinput", "r")
        except:
            print("get_audio_input_tracks_string() could not open file, returning empty")
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_audio_input_tracks(self):
        return pydaw_audio_input_tracks.from_str(self.get_audio_input_tracks_string())

    def get_audio_items_string(self):
        try:
            f_file = open(self.project_folder + "/default.pyaudioitem", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_audio_items(self):
        return pydaw_audio_items.from_str(self.get_audio_items_string())

    def get_samplegraphs_string(self):
        try:
            f_file = open(self.project_folder + "/default.pygraphs", "r")
        except:
            return pydaw_terminating_char
        f_result = f_file.read()
        f_file.close()
        return f_result

    def get_samplegraphs(self):
        return pydaw_sample_graphs.from_str(self.get_samplegraphs_string(), self.samplegraph_folder)

    def get_transport(self):
        try:
            f_file = open(self.project_folder + "/default.pytransport", "r")
        except:
            return pydaw_transport()  #defaults
        f_str = f_file.read()
        f_file.close()
        return pydaw_transport.from_str(f_str)

    def save_transport(self, a_transport):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pytransport"
            f_file = open(f_file_name, "w")
            f_file.write(a_transport.__str__())
            f_file.close()

    def create_empty_region(self, a_region_name):
        #TODO:  Check for uniqueness, from a pydaw_project.check_for_uniqueness method...
        f_file_name = self.regions_folder + "/" + a_region_name + ".pyreg"
        f_file = open(f_file_name, 'w')
        f_file.write(pydaw_terminating_char)
        f_file.close()
        self.git_repo.git_add(f_file_name)

    def create_empty_item(self, a_item_name):
        f_item_name = str(a_item_name)
        #TODO:  Check for uniqueness, from a pydaw_project.check_for_uniqueness method...
        f_file_name = self.items_folder + "/" + f_item_name + ".pyitem"
        f_file = open(f_file_name, 'w')
        f_file.write(pydaw_terminating_char)
        f_file.close()
        self.git_repo.git_add(f_file_name)
        self.this_dssi_gui.pydaw_save_item(f_item_name)

    def copy_region(self, a_old_region, a_new_region):
        f_new_file = self.regions_folder + "/" + str(a_new_region) + ".pyreg"
        copyfile(self.regions_folder + "/" + str(a_old_region) + ".pyreg", f_new_file)
        self.git_repo.git_add(f_new_file)

    def copy_item(self, a_old_item, a_new_item):
        f_new_item = str(a_new_item)
        f_new_file = self.items_folder + "/" + f_new_item + ".pyitem"
        copyfile(self.items_folder + "/" + str(a_old_item) + ".pyitem", f_new_file)
        self.git_repo.git_add(f_new_file)
        self.this_dssi_gui.pydaw_save_item(f_new_item)

    def save_item(self, a_name, a_item):
        if not self.suppress_updates:
            f_name = str(a_name)
            f_file_name = self.items_folder + "/" + f_name + ".pyitem"
            f_file = open(f_file_name, 'w')
            f_file.write(a_item.__str__())
            f_file.close()
            self.this_dssi_gui.pydaw_save_item(f_name)

    def save_region(self, a_name, a_region):
        if not self.suppress_updates:
            f_name = str(a_name)
            f_file_name = self.regions_folder + "/" + f_name + ".pyreg"
            f_file = open(f_file_name, 'w')
            f_file.write(a_region.__str__())
            f_file.close()
            self.this_dssi_gui.pydaw_save_region(f_name)

    def save_song(self, a_song):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pysong"
            f_file = open(f_file_name, 'w')
            f_file.write(a_song.__str__())
            f_file.close()
            self.this_dssi_gui.pydaw_save_song()

    def save_tracks(self, a_tracks):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pytracks"
            f_file = open(f_file_name, 'w')
            f_file.write(a_tracks.__str__())
            f_file.close()
            #Is there a need for a configure message here?

    def save_busses(self, a_tracks):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pybus"
            f_file = open(f_file_name, 'w')
            f_file.write(a_tracks.__str__())
            f_file.close()
            #Is there a need for a configure message here?

    def save_audio_automation(self, a_track_num, a_ccs):
        if not self.suppress_updates:
            f_file_name = self.audio_automation_folder + "/" + str(a_track_num) + ".pyauto"
            f_file = open(f_file_name, 'w')
            f_file.write(str(a_ccs))
            f_file.close()

    def save_bus_automation(self, a_track_num, a_ccs):
        if not self.suppress_updates:
            f_file_name = self.bus_automation_folder + "/" + str(a_track_num) + ".pyauto"
            f_file = open(f_file_name, 'w')
            f_file.write(str(a_ccs))
            f_file.close()

    def save_audio_tracks(self, a_tracks):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pyaudio"
            f_file = open(f_file_name, 'w')
            f_file.write(a_tracks.__str__())
            f_file.close()
            #Is there a need for a configure message here?

    def save_audio_inputs(self, a_tracks):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pyinput"
            f_file = open(f_file_name, 'w')
            f_file.write(str(a_tracks))
            f_file.close()
            #Is there a need for a configure message here?

    def save_audio_items(self, a_tracks):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pyaudioitem"
            f_file = open(f_file_name, 'w')
            f_file.write(a_tracks.__str__())
            f_file.close()
            #Is there a need for a configure message here?

    def save_samplegraphs(self, a_tracks):
        if not self.suppress_updates:
            f_file_name = self.project_folder + "/default.pygraphs"
            f_file = open(f_file_name, 'w')
            f_file.write(str(a_tracks))
            f_file.close()

    def item_exists(self, a_item_name):
        f_list = self.get_item_list()
        for f_item in f_list:
            if f_item == a_item_name:
                return True
        return False

    def get_next_default_item_name(self, a_item_name="item"):
        f_item_name = str(a_item_name)
        print(f_item_name)
        if f_item_name == "item":
            f_start = self.last_item_number
        else:
            f_start = 1
        for i in range(f_start, 10000):
            f_result = self.items_folder + "/" + f_item_name + "-" + str(i) + ".pyitem"
            print(f_result)
            if not os.path.exists(f_result):
                if f_item_name == "item":
                    self.last_item_number = i
                return f_item_name + "-" + str(i)

    def get_next_default_region_name(self):
        self.last_region_number -= 1
        for i in range(self.last_region_number, 10000):
            f_result = self.regions_folder + "/region-" + str(i) + ".pyreg"
            if not os.path.isfile(f_result):
                self.last_region_number = i + 1
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
        self.this_dssi_gui.stop_server()

    def __init__(self, a_osc_url=None):
        self.last_item_number = 1
        self.last_region_number = 1
        self.this_dssi_gui = dssi_gui(a_osc_url)
        self.suppress_updates = False

class pydaw_song:
    def add_region_ref(self, a_pos, a_region_name):
        self.regions[int(a_pos)] = str(a_region_name)

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
    def add_item_ref(self, a_track_num, a_bar_num, a_item_name):
        self.remove_item_ref(a_track_num, a_bar_num)
        self.items.append(pydaw_region.region_item(a_track_num, a_bar_num, a_item_name))

    def remove_item_ref(self, a_track_num, a_bar_num):
        for f_item in self.items:
            if f_item.bar_num == a_bar_num and f_item.track_num == a_track_num:
                self.items.remove(f_item)
                print("remove_item_ref removed bar: " + str(f_item.bar_num) + ", track: " + str(f_item.track_num))

    def __init__(self, a_name):
        self.items = []
        self.name = a_name
        self.region_length_bars = 0  #0 == default length for project

    def __str__(self):
        f_result = ""
        if self.region_length_bars > 0:
            f_result += "L|" + str(self.region_length_bars) + "|0\n"
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
                if f_item_arr[0] == "L":
                    f_result.region_length_bars = int(f_item_arr[1])
                    continue
                f_result.add_item_ref(int(f_item_arr[0]), int(f_item_arr[1]), f_item_arr[2])
        return f_result

    class region_item:
        def __init__(self, a_track_num, a_bar_num, a_item_name):
            self.track_num = a_track_num
            self.bar_num = a_bar_num
            self.item_name = a_item_name

def pydaw_draw_multi_item_cc_line(a_cc_num, a_start_val, a_end_val, a_items=[]):
    for f_item in a_items:
        f_item.remove_cc_range(a_cc_num)
    if a_start_val > a_end_val:
        f_cc_inc = -1
        f_val_diff = a_start_val - a_end_val
    else:
        f_cc_inc = 1
        f_val_diff = a_end_val - a_start_val

    if f_val_diff == 0:
        return

    f_bar_count = float(len(a_items))
    f_inc = (f_bar_count * 4.0) / float(f_val_diff)

    i2 = 0.0
    f_cc_val = int(a_start_val)

    for f_item in a_items:
        while True:
            if i2 > 4.0:
                i2 -= 4.0
                break
            f_beat_pos = round(i2, 4)
            if f_beat_pos == 4.0:
                f_beat_pos = 3.999
            f_item.add_cc(pydaw_cc(f_beat_pos, a_cc_num, f_cc_val))
            f_cc_val += f_cc_inc
            i2 += f_inc

def pydaw_smooth_automation_points(a_items_list, a_is_cc, a_cc_num=-1):
    if a_is_cc:
        f_this_cc_arr = []
        f_beat_offset = 0.0
        f_index = 0
        f_cc_num = int(a_cc_num)
        f_result_arr = []
        for f_item in a_items_list:
            for f_cc in f_item.ccs:
                if f_cc.cc_num == f_cc_num:
                    f_new_cc = pydaw_cc((f_cc.start + f_beat_offset), f_cc_num, f_cc.cc_val)
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
            f_time_inc = ((f_this_cc_arr[i + 1].start - f_this_cc_arr[i].start) / f_val_diff) * 4.0
            f_start = f_this_cc_arr[i].start + f_time_inc
            if (f_this_cc_arr[i].cc_val) > (f_this_cc_arr[i + 1].cc_val):
                f_start_val = f_this_cc_arr[i].cc_val - 4
                f_inc = -4
            else:
                f_start_val = f_this_cc_arr[i].cc_val + 4
                f_inc = 4

            for f_i2 in range(f_start_val, (f_this_cc_arr[i + 1].cc_val), f_inc):
                f_index_offset = 0
                f_adjusted_start = f_start - f_this_cc_arr[i].beat_offset
                while f_adjusted_start >= 4.0:
                    f_index_offset += 1
                    f_adjusted_start -= 4.0
                f_interpolated_cc = pydaw_cc(round((f_adjusted_start), 4), f_cc_num, f_i2)
                f_result_arr[f_this_cc_arr[i].item_index + f_index_offset].append(f_interpolated_cc)
                f_start += f_time_inc
                if f_start >= (f_this_cc_arr[i + 1].start - 0.05):
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
            f_val_diff = f_this_pb_arr[i + 1].pb_val - f_this_pb_arr[i].pb_val
            if f_val_diff == 0.0:
                continue
            f_steps = (abs(f_val_diff) / 0.05)
            f_time_inc = (f_this_pb_arr[i + 1].start - f_this_pb_arr[i].start) / f_steps
            f_start = f_this_pb_arr[i].start + f_time_inc

            if f_val_diff < 0:
                f_val_inc = -0.05
                f_new_val = f_this_pb_arr[i].pb_val - 0.05
            else:
                f_val_inc = 0.05
                f_new_val = f_this_pb_arr[i].pb_val + 0.05
            for f_i2 in range(int(f_steps)):
                f_index_offset = 0
                f_adjusted_start = f_start - f_this_pb_arr[i].beat_offset
                while f_adjusted_start >= 4.0:
                    f_index_offset += 1
                    f_adjusted_start -= 4.0
                f_interpolated_pb = pydaw_pitchbend(round((f_adjusted_start), 4), round(f_new_val, 4))
                f_result_arr[f_this_pb_arr[i].item_index + f_index_offset].append(f_interpolated_pb)
                f_start += f_time_inc
                if f_start >= (f_this_pb_arr[i + 1].start - 0.05):
                    break
                f_new_val += f_val_inc
        for f_i in range(len(a_items_list)):
            a_items_list[f_i].pitchbends += f_result_arr[f_i]
            a_items_list[f_i].pitchbends.sort()

def pydaw_velocity_mod(a_items, a_amt, a_line=False, a_end_amt=127, a_add=False):
    f_start_beat = a_items[0].notes[0].start
    f_range_beats = a_items[-1].notes[-1].start + (4.0 * (len(a_items) - 1)) - f_start_beat

    f_beat_offset = 0.0
    for f_item in a_items:
        for note in f_item.notes:
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
        self.notes.remove(a_note)

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

    def quantize(self, a_beat_frac, a_events_move_with_item=False, a_notes=None):
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

    def transpose(self, a_semitones, a_octave=0, a_notes=None):
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
            note.note_num += f_total
            if note.note_num < 0:
                note.note_num = 0
            elif note.note_num > 127:
                note.note_num = 127

    def length_shift(self, a_length, a_min_max=None, a_notes=None, a_quantize=None):
        """ Note lengths are clipped at up to a_min_max if a_length > 0, or down to a_min_max if a_length < 0"""
        f_notes = []

        if a_notes is None:
            f_notes = self.notes
        else:
            for i in range(len(a_notes)):
                for f_note in self.notes:
                    if f_note == a_notes[i]:
                        f_notes.append(f_note)
                        break

        f_length = float(a_length)

        if not a_min_max is None:
            f_min_max = float(a_min_max)
        else:
            if a_length < 0:
                f_min_max = 0.01
            else:
                f_min_max = 100

        if a_quantize is None:
            f_is_quantized = False
        else:
            f_is_quantized = True
            f_quantized_value = beat_frac_text_to_float(a_quantize)
            f_quantize_multiple = 1.0/f_quantized_value

        for f_note in f_notes:
            f_note.length += f_length
            if f_is_quantized:
                f_note.length = round(f_note.length * f_quantize_multiple) * f_quantized_value
            if f_length < 0 and f_note.length < f_min_max:
                f_note.length = f_min_max
            elif f_length > 0 and f_note.length > f_min_max:
                f_note.length = f_min_max
            f_note.set_end()

        self.fix_overlaps()

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

    def time_shift(self, a_shift, a_events_move_with_item=False, a_notes=None, a_quantize=None):
        """ Move all items forward or backwards by a_shift number of beats, wrapping if before or after the item"""
        f_shift = float(a_shift)
        if f_shift < -4.0:
            f_shift = -4.0
        elif f_shift > 4.0:
            f_shift = 4.0

        f_notes = []
        f_ccs = []
        f_pbs = []

        if a_quantize is None:
            f_is_quantized = False
        else:
            f_is_quantized = True
            f_quantized_value = beat_frac_text_to_float(a_quantize)
            f_quantize_multiple = 1.0/f_quantized_value

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

        for note in f_notes:
            note.start += f_shift
            if note.start < 0.0:
                note.start += 4.0
            elif note.start >= 4.0:
                note.start -= 4.0

            if f_is_quantized:
                f_new_start = round(note.start * f_quantize_multiple) * f_quantized_value
                f_shift_adjusted = f_shift - (note.start - f_new_start)
                note.start = f_new_start
            else:
                f_shift_adjusted = f_shift

        self.fix_overlaps()

        if a_events_move_with_item:
            for cc in f_ccs:
                cc.start += f_shift_adjusted
                if cc.start < 0.0:
                    cc.start += 4.0
                elif cc.start >= 4.0:
                    cc.start -= 4.0
            for pb in f_pbs:
                pb.start += f_shift_adjusted
                if pb.start < 0.0:
                    pb.start += 4.0
                elif pb.start >= 4.0:
                    pb.start -= 4.0

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

    def __init__(self, a_start, a_cc_num, a_cc_val):
        self.start = float(a_start)
        self.cc_num = int(a_cc_num)
        self.cc_val = int(a_cc_val)

    def set_val(self, a_val):
        f_val = int(a_val)
        if f_val > 127:
            f_val = 127
        elif f_val < 0:
            f_val = 0
        self.cc_val = f_val

    def __str__(self):
        return "c|" + str(self.start) + "|" + str(self.cc_num) + "|" + str(self.cc_val) + "\n"

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_cc(a_arr[1], a_arr[2], a_arr[3])
        return f_result

    @staticmethod
    def from_str(a_str):
        f_arr = a_str.split("|")
        return pydaw_note.from_arr(f_arr)

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
        return pydaw_note.from_arr(f_arr)

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
                f_result.add_track(int(f_line_arr[0]), pydaw_track(int_to_bool(f_line_arr[1]), int_to_bool(f_line_arr[2]), int_to_bool(f_line_arr[3]), int(f_line_arr[4]), f_line_arr[5], int(f_line_arr[6]), int(f_line_arr[7])))
        return f_result

class pydaw_track:
    def __init__(self, a_solo, a_mute, a_rec, a_vol, a_name, a_inst, a_bus_num=0):
        self.name = str(a_name)
        self.solo = bool(a_solo)
        self.mute = bool(a_mute)
        self.rec = bool(a_rec)
        self.vol = int(a_vol)
        self.inst = int(a_inst)
        self.bus_num = int(a_bus_num)

    def __str__(self):
        return bool_to_int(self.solo) + "|" + bool_to_int(self.mute) + "|" + bool_to_int(self.rec) + "|" + str(self.vol) + "|" + self.name + "|" + \
        str(self.inst) + "|" + str(self.bus_num) + "\n"

class pydaw_busses:
    def add_bus(self, a_index, a_bus):
        self.busses[int(a_index)] = a_bus

    def add_bus_from_str(self, a_str):
        f_arr = a_str.split("|")
        self.add_bus(f_arr[0], pydaw_bus(f_arr[1], f_arr[2]))

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
    def __init__(self, a_vol=0, a_rec=False):
        self.vol = int(a_vol)
        self.rec = bool(a_rec)

    def __str__(self):
        return str(self.vol) + "|" + bool_to_int(self.rec) + "\n"

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
    def __init__(self, a_solo, a_mute, a_vol, a_name, a_bus_num=0, a_rec=False):
        self.name = str(a_name)
        self.solo = bool(a_solo)
        self.mute = bool(a_mute)
        self.vol = int(a_vol)
        self.bus_num = int(a_bus_num)
        self.rec = bool(a_rec)

    def __str__(self):
        return bool_to_int(self.solo) + "|" + bool_to_int(self.mute) + "|" + str(self.vol) + "|" + self.name + "|" + \
        str(self.bus_num) + "|" + bool_to_int(self.rec) + "\n"

class pydaw_audio_items:
    """ Return the next available index, or -1 if none are available """
    def get_next_index(self):
        for i in range(32):
            if not self.items.has_key(i):
                return i
        return -1
    def __init__(self):
        self.items = {}

    def add_item(self, a_index, a_item):
        self.items[int(a_index)] = a_item

    def remove_item(self, a_index):
        self.items.pop(int(a_index))

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_audio_items()
        f_lines = a_str.split("\n")
        for f_line in f_lines:
            if f_line == pydaw_terminating_char:
                return f_result
            f_arr = f_line.split("|")
            f_result.add_item(int(f_arr[0]), pydaw_audio_item.from_arr(f_arr[1:]))
        print("pydaw_audio_items.from_str:  Warning:  no pydaw_terminating_char")
        return f_result

    def __str__(self):
        f_result = ""
        for k, f_item in self.items.iteritems():
            f_result += str(k) + "|" + str(f_item)
        f_result += pydaw_terminating_char
        return f_result

class pydaw_audio_item:
    def __init__(self, a_file, a_sample_start, a_sample_end, a_start_region, a_start_bar, a_start_beat, a_end_mode, \
    a_end_region, a_end_bar, a_end_beat, a_timestretch_mode, a_pitch_shift, a_output_track, a_vol, a_timestretch_amt=1.0, \
    a_fade_in=0, a_fade_out=1000):
        self.file = str(a_file)
        self.sample_start = int(a_sample_start)  #TODO:  int() these once I have a way of getting frame count...
        self.sample_end = int(a_sample_end)
        self.start_region = int(a_start_region)
        self.start_bar = int(a_start_bar)
        self.start_beat = float(a_start_beat)
        self.end_mode = int(a_end_mode)
        self.end_region = int(a_end_region)
        self.end_bar = int(a_end_bar)
        self.end_beat = float(a_end_beat)
        self.time_stretch_mode = int(a_timestretch_mode)
        self.pitch_shift = float(a_pitch_shift)
        self.output_track = int(a_output_track)
        self.vol = int(a_vol)
        self.timestretch_amt = float(a_timestretch_amt)
        self.fade_in = int(a_fade_in)
        self.fade_out = int(a_fade_out)

    def __str__(self):
        return self.file + "|" + str(self.sample_start) + "|" + str(self.sample_end) + "|" + str(self.start_region) \
        + "|" + str(self.start_bar) + "|" + str(self.start_beat) + "|" + str(self.end_mode) + "|" + str(self.end_region) \
        + "|" + str(self.end_bar) + "|" + str(self.end_beat) + "|" + str(self.time_stretch_mode) \
        + "|" + str(self.pitch_shift) + "|" + str(self.output_track) + "|" + str(self.vol) + "|" + str(self.timestretch_amt) \
        + "|" + str(self.fade_in) + "|" + str(self.fade_out) + "\n"

    @staticmethod
    def from_arr(a_arr):
        f_result = pydaw_audio_item(a_arr[0], a_arr[1], a_arr[2], a_arr[3], a_arr[4], a_arr[5], a_arr[6],\
        a_arr[7], a_arr[8], a_arr[9], a_arr[10], a_arr[11], a_arr[12], a_arr[13], a_arr[14], a_arr[15], a_arr[16])
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
                f_result.add_track(int(f_line_arr[0]), pydaw_audio_input_track(int_to_bool(f_line_arr[1]), int(f_line_arr[2]), int(f_line_arr[3]), f_line_arr[4]))
        return f_result

class pydaw_audio_input_track:
    def __init__(self, a_rec, a_vol, a_output, a_input="None"):
        self.input = str(a_input)
        self.output = int(a_output)
        self.rec = bool(a_rec)
        self.vol = int(a_vol)

    def __str__(self):
        return bool_to_int(self.rec) + "|" + str(self.vol) + "|" + str(self.output) + "|" + str(self.input) + "\n"

class pydaw_transport:
    def __init__(self, a_bpm=140, a_midi_keybd=None, a_loop_mode=0, a_region=0, a_bar=0):
        self.bpm = a_bpm
        self.midi_keybd = a_midi_keybd
        self.loop_mode = a_loop_mode
        self.region = a_region
        self.bar = a_bar

    def __str__(self):
        return str(self.bpm) + "|" + str(self.midi_keybd) + "|" + str(self.loop_mode) + "|" + str(self.region) + "|" + str(self.bar) + "\n\\"

    @staticmethod
    def from_str(a_str):
        f_str = a_str.split("\n")[0]
        f_arr = f_str.split("|")
        for i in range(len(f_arr)):
            if f_arr[i] == "":
                f_arr[i] = None
        return pydaw_transport(f_arr[0], f_arr[1], f_arr[2], f_arr[3], f_arr[4])


class pydaw_song_level_ccs:
    def remove_cc(self, a_cc):
        self.items.remove(a_cc)

    def remove_cc_range(self, a_cc_num, a_start, a_end):
        """ Delete all CCs between a_start and _end, which should be pydaw_song_level_cc instances """
        f_cc_num = int(a_cc_num)
        f_ccs_to_delete = []
        for cc in self.items:
            if cc.cc == f_cc_num and  cc > a_start and cc < a_end:
                f_ccs_to_delete.append(cc)
        for cc in f_ccs_to_delete:
            self.remove_cc(cc)
        self.items.sort()

    def draw_cc_line(self, a_cc, a_start_val, a_start_region, a_start_bar, a_start_beat, a_end_val, a_end_region, a_end_bar, a_end_beat):
        f_cc = int(a_cc)
        f_start = float(((a_start_region * 8 * 4) + (a_start_bar * 4))) + a_start_beat
        f_start_val = int(a_start_val)
        f_end = float(((a_end_region * 8 * 4) + (a_end_bar * 4))) + a_end_beat
        f_end_val = int(a_end_val)

        f_first_cc = pydaw_song_level_cc(a_start_region, a_start_bar, a_start_beat, a_cc, a_start_val)
        f_last_cc = pydaw_song_level_cc(a_end_region, a_end_bar, a_end_beat, a_cc, a_end_val)
        #Remove any events that would overlap
        self.remove_cc_range(f_cc, f_first_cc, f_last_cc)

        f_start_diff = f_end - f_start
        f_val_diff = abs(f_end_val - f_start_val)
        if f_start_val > f_end_val:
            f_inc = -1
        else:
            f_inc = 1
        f_time_inc = abs(f_start_diff/float(f_val_diff))

        f_new_start_region = a_start_region
        f_new_start_bar = a_start_bar
        f_new_start_beat = a_start_beat

        f_start_val += f_inc

        for i in range(0, (f_val_diff + 1)):
            #self.items.append(pydaw_cc(round(f_start, 4), f_cc, f_start_val))
            self.items.append(pydaw_song_level_cc(f_new_start_region, f_new_start_bar, f_new_start_beat, a_cc, f_start_val))
            f_start_val += f_inc
            if f_start_val > 127 or f_start_val < 0:
                break
            f_new_start_beat += f_time_inc
            f_new_start_beat = round(f_new_start_beat, 4)
            while f_new_start_beat >= 4.0:
                f_new_start_beat -= 4.0
                f_new_start_bar += 1
                if f_new_start_bar >= 8:
                    f_new_start_bar = 0
                    f_new_start_region += 1
        self.items.sort()

    def add_cc(self, a_cc):
        for f_cc in self.items:
            if f_cc == a_cc:
                return False
        self.items.append(a_cc)
        self.items.sort()
        return True

    def __init__(self):
        self.items = []

    @staticmethod
    def from_str(a_str):
        f_result = pydaw_song_level_ccs()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == pydaw_terminating_char:
                break
            f_line_arr = f_line.split("|")
            f_result.add_cc(pydaw_song_level_cc(f_line_arr[0], f_line_arr[1], f_line_arr[2], f_line_arr[3], f_line_arr[4]))
        f_result.items.sort()
        return f_result

    def __str__(self):
        f_result = ""
        self.items.sort()
        for f_item in self.items:
            f_result += str(f_item)
        f_result += pydaw_terminating_char
        return f_result

class pydaw_song_level_cc:
    def __init__(self, a_region, a_bar, a_beat, a_cc, a_val):
        self.region = int(a_region)
        self.bar = int(a_bar)
        self.beat = float(a_beat)
        self.cc = int(a_cc)
        self.value = int(a_val)

    def __lt__(self, other):
        if self.region < other.region:
            return True
        elif self.region == other.region:
            if self.bar < other.bar:
                return True
            elif self.bar == other.bar:
                if self.beat < other.beat:
                    return True
                else:
                    return False
            else:
                return False
        else:
            return False

    def __gt__(self, other):
        return not self < other

    def __eq__(self, other):
        return self.region == other.region and self.bar == other.bar and self.beat == other.beat and self.cc == other.cc and self.value == other.value

    def __str__(self):
        return str(self.region) + "|" + str(self.bar) + "|" + str(self.beat) + "|" + str(self.cc) + "|" + str(self.value) + "\n"

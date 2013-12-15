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

import random, os, re
from time import sleep
from math import log, pow

global_pydaw_version_string = "pydaw4"
global_pydaw_file_type_string = 'PyDAW4 Project (*.pydaw4)'
global_euphoria_file_type_string = 'PyDAW4 Euphoria Sample File (*.u4ia4)'
global_euphoria_file_type_ext = '.u4ia4'

global_pydaw_bin_path = None
global_pydaw_is_sandboxed = False

global_pydaw_with_audio = True

if "src/pydaw/python/" in __file__:
    global_pydaw_install_prefix = "/usr"
else:
    global_pydaw_install_prefix = "{}/../../../../..".format(os.path.abspath( os.path.dirname(__file__)))

def pydaw_set_bin_path():
    global global_pydaw_bin_path
    global_pydaw_bin_path = "{}/bin/{}-engine".format(global_pydaw_install_prefix, global_pydaw_version_string)

def pydaw_escape_stylesheet(a_stylesheet, a_path):
    f_dir = os.path.dirname(str(a_path))
    f_result = a_stylesheet.replace("$STYLE_FOLDER", f_dir)
    return f_result

print(("\n\n\ninstall prefix:  {}\n\n\n".format(global_pydaw_install_prefix,)))

pydaw_bad_chars = ["|", "\\", "~", "."]

def pydaw_which(a_file):
    """ Python equivalent of the UNIX "which" command """
    f_path_arr = os.getenv("PATH").split(":")
    for f_path in f_path_arr:
        f_file_path = "{}/{}".format(f_path, a_file,)
        if os.path.exists(f_file_path) and not os.path.isdir(f_file_path):
            return f_file_path
    return None

#hack for the XFCE live USB/DVD and that panel thing that takes up 2 inches of vertical space at the bottom
if os.path.isdir("/home/liveuser") and pydaw_which("xfconf-query") is not None:
    try:
        os.system("xfconf-query -c xfce4-panel -p /panels/panel-2/autohide -s true --create -t bool")
    except Exception as ex:
        print("Exception while trying to set autohide for XFCE-Panel:\n{}".format(ex,))

def pydaw_remove_bad_chars(a_str):
    """ Remove any characters that have special meaning to PyDAW """
    f_str = str(a_str)
    for f_char in pydaw_bad_chars:
        f_str = f_str.replace(f_char, "")
    f_str = f_str.replace(' ', '_')
    return f_str

def pydaw_str_has_bad_chars(a_str):
    f_str = str(a_str)
    for f_char in pydaw_bad_chars:
        if f_char in f_str:
            return False
    return True

def case_insensitive_path(a_path):
    f_path = os.path.abspath(str(a_path))
    if os.path.exists(f_path):
        return a_path
    else:
        f_path_arr = f_path.split("/")
        f_path = ""
        for f_dir in f_path_arr:
            if os.path.exists("{}/{}".format(f_path, f_dir)):
                f_path = "{}/{}".format(f_path, f_dir)
            else:
                f_found = False
                for f_real_dir in os.listdir(f_path):
                    if f_dir.lower() == f_real_dir.lower():
                        f_found = True
                        f_path = "{}/{}".format(f_path, f_real_dir)
                        break
                if not f_found:
                    assert(False)
        print(f_path)
        return f_path



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

def pydaw_clip_value(a_val, a_min, a_max, a_round=False):
    if a_val < a_min:
        f_result = a_min
    elif a_val > a_max:
        f_result =  a_max
    else:
        f_result = a_val
    if a_round:
        f_result = round(f_result, 4)
    return f_result

def pydaw_clip_min(a_val, a_min):
    if a_val < a_min:
        return a_min
    else:
        return a_val

def pydaw_clip_max(a_val, a_max):
    if a_val > a_max:
        return a_max
    else:
        return a_val

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
    f_octave = (int(a_note_num) // 12) - 2
    return "{}{}".format(int_to_note_array[f_note], f_octave)

def string_to_note_num(a_str):
    f_str = str(a_str).lower()
    if len(f_str) >=2 and len(f_str) < 5 and re.match('[a-g](.*)[0-8]', f_str):
        f_notes = {'c':0, 'd':2, 'e':4, 'f':5, 'g':7, 'a':9, 'b':11}
        for k, v in f_notes.items():
            if f_str.startswith(k):
                f_str = f_str.replace(k, "", 1)
                f_sharp_flat = 0
                if f_str.startswith("#"):
                    f_sharp_flat = 1
                    f_str = f_str[1:]
                elif f_str.startswith("b"):
                    f_sharp_flat = -1
                    f_str = f_str[1:]
                f_result = (int(f_str) * 12) + v + f_sharp_flat
                assert(f_result >= 0 and f_result <= 127)
                return f_result
        return a_str
    else:
        return a_str

def bool_to_int(a_bool):
    if a_bool:
        return "1"
    else:
        return "0"

def int_to_bool(a_int):
    if isinstance(a_int, bool):
        return a_int

    if int(a_int) == 0:
        return False
    elif int(a_int) == 1:
        return True
    else:
        assert(False)

def print_sorted_dict(a_dict):
    """ Mostly intended for printing locals() and globals() """
    for k in sorted(list(a_dict.keys())):
            print("{} : {}".format(k, a_dict[k]))

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

def pydaw_wait_for_finished_file(a_file):
    """ Wait until a_file exists, then delete it and return.  It should
    already have the .finished extension"""
    while True:
        if os.path.isfile(a_file):
            try:
                os.remove(a_file)
                break
            except:
                print("pydaw_wait_for_finished_file:  Exception when deleting {}".format(a_file))
        else:
            sleep(0.1)

def pydaw_get_wait_file_path(a_file):
    f_wait_file = "{}.finished".format(a_file,)
    if os.path.isfile(f_wait_file):
        os.remove(f_wait_file)
    return f_wait_file

global_show_create_folder_error = False

def pydaw_set_live_mode_off():
    global global_is_live_mode, global_home, global_default_project_folder, global_pydaw_home
    global_is_live_mode = False
    global_home = os.path.expanduser("~")
    global_default_project_folder = global_home
    global_pydaw_home = "{}/{}".format(os.path.expanduser("~"), global_pydaw_version_string)
    if not os.path.isdir(global_pydaw_home):
        os.mkdir(global_pydaw_home)

if pydaw_which("gksudo") is not None:
    global_pydaw_sudo_command = "gksudo"
elif pydaw_which("sudo") is not None:
    print("Warning, gksudo not found, falling back to sudo.  If the GUI hangs before opening, this could "
    "be the reason why")
    global_pydaw_sudo_command = "sudo"
else:
    print("Warning, gksudo and sudo not found.  If the GUI hangs before opening, this could be the reason why")
    global_pydaw_sudo_command = None

if (os.path.isdir("/home/ubuntu") or  os.path.isdir("/home/liveuser")) and \
    os.path.islink("/dev/disk/by-label/pydaw_data") and global_pydaw_sudo_command is not None:
    if os.path.isdir("/home/liveuser"): #presumed to be Fedora or Fedora-like
        global_home = "/run/media/liveuser/pydaw_data"
    else: #presumed to be Ubuntu or Ubuntu-like.
        global_home = "/media/pydaw_data"
    if not os.path.isdir(global_home):
        print("Attempting to mount {}.  If this causes the GUI to hang, please try mounting the pydaw_data "
        "partition before starting".format(global_home))
        try:
            os.system("{} mkdir {}".format(global_pydaw_sudo_command, global_home))
            os.system("{} mount /dev/disk/by-label/pydaw_data {}".format(global_pydaw_sudo_command, global_home))
        except:
            print("Could not mount pydaw_data partition, this may indicate a problem with the flash drive "
            "or permissions")

    global_is_live_mode = True
    global_pydaw_home = "{}/{}".format(global_home, global_pydaw_version_string)
    global_default_project_folder = "{}/{}_projects".format(global_home, global_pydaw_version_string)

    try:
        if not os.path.isdir(global_pydaw_home):
            print("{} did not exist, attempting to create.".format(global_pydaw_home))
            os.system("{} mkdir '{}'".format(global_pydaw_sudo_command, global_pydaw_home))
            os.system("{} chmod -R 777 '{}'".format(global_pydaw_sudo_command, global_pydaw_home))
        if not os.path.isdir(global_default_project_folder):
            print("{} did not exist, attempting to create.".format(global_default_project_folder))
            os.system("{} mkdir '{}'".format(global_pydaw_sudo_command, global_default_project_folder))
            os.system("{} chmod -R 777 '{}'".format(global_pydaw_sudo_command, global_default_project_folder))
            pydaw_write_file_text("{}/README.txt".format(global_default_project_folder,),
            "Create subfolders in here and save your live projects to those subfolders.  "
            "Saving in the regular filesystem will not persist between live sessions.")
    except Exception as ex:
        print("pydaw_util :  Exception while creating folder.")
        global_show_create_folder_error = True
        pydaw_set_live_mode_off()
else:
    pydaw_set_live_mode_off()


global_bookmarks_file_path = "{}/file_browser_bookmarks.txt".format(global_pydaw_home)

global_device_val_dict = {}
global_pydaw_device_config = "{}/device.txt".format(global_pydaw_home)

def pydaw_read_device_config():
    global global_pydaw_bin_path, global_device_val_dict, global_pydaw_is_sandboxed, global_pydaw_with_audio

    if os.path.isfile(global_pydaw_device_config):
        f_file_text = pydaw_read_file_text(global_pydaw_device_config)
        for f_line in f_file_text.split("\n"):
            if f_line.strip() == "\\":
                break
            if f_line.strip() != "":
                f_line_arr = f_line.split("|", 1)
                global_device_val_dict[f_line_arr[0].strip()] = f_line_arr[1].strip()

        pydaw_set_bin_path()
        global_pydaw_is_sandboxed = False
        global_pydaw_with_audio = True

        if global_pydaw_bin_path is not None:
            if int(global_device_val_dict["audioEngine"]) == 0:
                global_pydaw_bin_path += "-no-root"
            elif int(global_device_val_dict["audioEngine"]) == 2:
                global_pydaw_bin_path = "{}/bin/{}".format(global_pydaw_install_prefix, global_pydaw_version_string)
                global_pydaw_is_sandboxed = True
            elif int(global_device_val_dict["audioEngine"]) == 3:
                global_pydaw_bin_path += "-dbg"
            elif int(global_device_val_dict["audioEngine"]) == 4 or \
                 int(global_device_val_dict["audioEngine"]) == 5 or \
                 int(global_device_val_dict["audioEngine"]) == 7:
                global_pydaw_bin_path += "-no-hw"
            elif int(global_device_val_dict["audioEngine"]) == 6:
                global_pydaw_with_audio = False
                global_pydaw_bin_path = None
    print("global_pydaw_bin_path == {}".format(global_pydaw_bin_path))

pydaw_read_device_config()

def global_get_file_bookmarks():
    """ Get the bookmarks shared with Euphoria """
    f_result = {}
    if os.path.isfile(global_bookmarks_file_path):
        f_text = pydaw_read_file_text(global_bookmarks_file_path)
        f_arr = f_text.split("\n")
        for f_line in f_arr:
            f_line_arr = f_line.split("|||")
            if len(f_line_arr) < 2:
                break
            f_full_path = "{}/{}".format(f_line_arr[1], f_line_arr[0])
            if os.path.isdir(f_full_path):
                f_result[f_line_arr[0]] = f_line_arr[1]
            else:
                print("Warning:  Not loading bookmark '{}' because the directory '{}'"
                " does not exist.".format(f_line_arr[0], f_full_path))
    return f_result

def global_write_file_bookmarks(a_dict):
    f_result = ""
    for k, v in list(a_dict.items()):
        f_result += "{}|||{}\n".format(k, v)
    pydaw_write_file_text(global_bookmarks_file_path, f_result.strip("\n"))

def global_add_file_bookmark(a_folder):
    f_dict = global_get_file_bookmarks()
    f_folder = str(a_folder)
    f_folder_arr = f_folder.split("/")
    f_dict[f_folder_arr[-1]] = "/".join(f_folder_arr[:-1])
    global_write_file_bookmarks(f_dict)

def global_delete_file_bookmark(a_key):
    f_dict = global_get_file_bookmarks()
    f_dict.pop(str(a_key))
    global_write_file_bookmarks(f_dict)

class sfz_exception(Exception):
    pass

class sfz_sample:
    """ Corresponds to the settings for a single sample """
    def __init__(self):
        self.dict = {}

    def set_from_group(self, a_group_list):
        """ a_group_list: should be in order of least precedence to
        most precedence, ie:  values in the last group can overwrite
        values set by the first group."""
        for f_group in filter(None, a_group_list):
            for k, v in f_group.dict.items():
                if k not in self.dict or v is not None:
                    self.dict[k] = v

    def __str__(self):
        return str(self.dict)


class sfz_file:
    """ Abstracts an .sfz file into a list of sfz_sample whose dicts
    correspond to the attributes of a single sample."""
    def __init__(self, a_file_path):
        self.path = str(a_file_path)
        if not os.path.exists(self.path):
            raise sfz_exception("{} does not exist.".format(self.path))
        f_file_text = pydaw_read_file_text(self.path)
        # In the wild, people can and often do put tags and opcodes on the same
        # line, move all tags and opcodes to their own line
        f_file_text = f_file_text.replace("<", "\n<")
        f_file_text = f_file_text.replace(">", ">\n")
        f_file_text = f_file_text.replace("/*", "\n/*")
        f_file_text = f_file_text.replace("*/", "*/\n")
        f_file_text = f_file_text.replace("\t", " ")
        f_file_text = f_file_text.replace("\r", "")

        f_file_text_new = ""

        for f_line in f_file_text.split("\n"):
            if f_line.strip().startswith("//"):
                continue
            if "=" in f_line:
                f_line_arr = f_line.split("=")
                for f_i in range(1, len(f_line_arr)):
                    f_opcode = f_line_arr[f_i - 1].strip().rsplit(" ")[-1]
                    if f_i == (len(f_line_arr) - 1):
                        f_value = f_line_arr[f_i]
                    else:
                        f_value = f_line_arr[f_i].strip().rsplit(" ", 1)[0]
                    f_file_text_new += "\n{}={}\n".format(f_opcode, f_value)
            else:
                f_file_text_new += "{}\n".format(f_line,)

        f_file_text = f_file_text_new
        self.adjusted_file_text = f_file_text_new

        f_global_settings = None
        f_current_group = None
        f_current_region = None
        f_current_object = None

        f_extended_comment = False

        self.samples = []
        f_samples_list = []

        f_current_mode = None #None = unsupported, 0 = global, 1 = region, 2 = group

        for f_line in f_file_text.split("\n"):
            f_line = f_line.strip()

            if f_line.startswith("/*"):
                f_extended_comment = True
                continue

            if f_extended_comment:
                if "*/" in f_line:
                    f_extended_comment = False
                continue

            if f_line == "" or f_line.startswith("//"):
                continue
            if re.match("<(.*)>", f_line) is not None:
                if f_line.startswith("<global>"):
                    f_current_mode = 0
                    f_global_settings = sfz_sample()
                    f_current_object = f_global_settings
                elif f_line.startswith("<region>"):
                    f_current_mode = 1
                    f_current_region = sfz_sample()
                    f_current_object = f_current_region
                    f_samples_list.append(f_current_region)
                elif f_line.startswith("<group>"):
                    f_current_mode = 2
                    f_current_group = sfz_sample()
                    f_current_object = f_current_group
                else:
                    f_current_mode = None
            else:
                if f_current_mode is None:
                    continue
                try:
                    f_key, f_value = f_line.split("=")
                    f_value = string_to_note_num(f_value)
                except Exception as ex:
                    print("ERROR:  {}".format(f_line))
                    raise sfz_exception("Error parsing key/value pair\n{}\n{}".format(f_line, ex))
                if f_key.lower() == "sample" and not f_value.lower().strip().endswith(".wav"):
                    raise sfz_exception("{} not supported, only .wav supported.".format(f_value))
                f_current_object.dict[f_key.lower()] = f_value
                if f_current_mode == 1:
                    f_current_object.set_from_group([f_global_settings, f_current_group])

        for f_region in f_samples_list:
            if "sample" in f_region.dict:
                self.samples.append(f_region)

    def __str__(self):
        #return self.adjusted_file_text
        f_result = ""
        for f_sample in self.samples:
            f_result += "\n\n{}\n\n".format(f_sample)
        return f_result

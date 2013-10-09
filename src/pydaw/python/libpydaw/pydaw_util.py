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

import random, os
from time import sleep
from math import log, pow

global_pydaw_version_string = "pydaw3"
global_pydaw_file_type_string = 'PyDAW3 Project (*.pydaw3)'

if "src/pydaw/python/" in __file__:
    global_pydaw_install_prefix = "/usr"
    global_pydaw_with_audio = False
else:
    global_pydaw_install_prefix = os.path.abspath( os.path.dirname(__file__) + "/../../../../..")
    global_pydaw_bin_path = global_pydaw_install_prefix + "/bin/" + global_pydaw_version_string + "-engine"
    if os.path.exists(global_pydaw_bin_path):
        global_pydaw_with_audio = True
    else:
        global_pydaw_with_audio = False

def pydaw_escape_stylesheet(a_stylesheet, a_path):
    f_dir = os.path.dirname(str(a_path))
    f_result = a_stylesheet.replace("$STYLE_FOLDER", f_dir)
    f_result = f_result.replace("$PYDAW_PREFIX", global_pydaw_install_prefix) #TODO:  Deprecate at PyDAWv4
    return f_result

print(("\n\n\ninstall prefix:  %s\n\n\n" % (global_pydaw_install_prefix,)))

pydaw_bad_chars = ["|", "\\", "~", "."]

def pydaw_which(a_file):
    """ Python equivalent of the UNIX which command """
    f_path_arr = os.getenv("PATH").split(":")
    for f_path in f_path_arr:
        f_file_path = "%s/%s" % (f_path, a_file,)
        if os.path.exists(f_file_path) and not os.path.isdir(f_file_path):
            return f_file_path
    return None

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

def pydaw_clip_value(a_val, a_min, a_max):
    if a_val < a_min:
        return a_min
    if a_val > a_max:
        return a_max
    return a_val

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
    f_octave = (int(a_note_num) / 12) - 2
    return int_to_note_array[f_note] + str(f_octave)

def bool_to_int(a_bool):
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

def pydaw_wait_for_finished_file(a_file):
    """ Wait until a_file exists, then delete it and return.  It should
    already have the .finished extension"""
    while True:
        if os.path.isfile(a_file):
            try:
                os.remove(a_file)
                break
            except:
                print(("pydaw_wait_for_finished_file:  Exception when deleting " + a_file))
        else:
            sleep(0.1)

def pydaw_get_wait_file_path(a_file):
    f_wait_file = str(a_file) + ".finished"
    if os.path.isfile(f_wait_file):
        os.remove(f_wait_file)
    return f_wait_file

global_show_create_folder_error = False

if pydaw_which("gksudo") is not None:
    global_pydaw_sudo_command = "gksudo"
elif pydaw_which("sudo") is not None:
    print("Warning, gksudo not found, falling back to sudo.  If the GUI hangs before opening, this could be the reason why")
    global_pydaw_sudo_command = "sudo"
else:
    print("Warning, gksudo and sudo not found.  If the GUI hangs before opening, this could be the reason why")
    global_pydaw_sudo_command = None

if os.path.isdir("/home/ubuntu") and os.path.islink("/dev/disk/by-label/pydaw_data") and global_pydaw_sudo_command is not None:
    global_pydaw_bin_path += "-no-root"
    if not os.path.isdir("/media/pydaw_data"):
        print("Attempting to mount /media/pydaw_data.  If this causes the GUI to hang, please try mounting the pydaw_data partition before starting")
        try:
            os.system("%s mkdir /media/pydaw_data" % (global_pydaw_sudo_command,))
            os.system("%s mount /dev/disk/by-label/pydaw_data /media/pydaw_data" % (global_pydaw_sudo_command,))
        except:
            print("Could not mount pydaw_data partition, this may indicate a problem with the flash drive or permissions")
    global_is_live_mode = True
    global_home = "/media/pydaw_data"
    global_pydaw_home = "/media/pydaw_data/" + global_pydaw_version_string
    global_default_project_folder = global_home + "/" + global_pydaw_version_string + "_projects"
    try:
        if not os.path.isdir(global_pydaw_home):
            os.mkdir(global_pydaw_home)
        if not os.path.isdir(global_default_project_folder):
            os.mkdir(global_default_project_folder)
            pydaw_write_file_text(global_default_project_folder + "/README.txt", "Create subfolders in here and save your live projects to those subfolders.  Saving in the regular filesystem will not persist between live sessions.")
    except:
        global_show_create_folder_error = True
        global_is_live_mode = False
        global_home = os.path.expanduser("~")
        global_default_project_folder = global_home
        global_pydaw_home = os.path.expanduser("~") + "/" + global_pydaw_version_string
else:
    global_is_live_mode = False
    global_home = os.path.expanduser("~")
    global_default_project_folder = global_home
    global_pydaw_home = os.path.expanduser("~") + "/" + global_pydaw_version_string
    if not os.path.isdir(global_pydaw_home):
        os.mkdir(global_pydaw_home)

#TODO:  Remove at PyDAWv4
global_pydaw_device_config = global_pydaw_home + "/device.txt"
if os.path.exists(global_pydaw_device_config) and not pydaw_read_file_text(global_pydaw_device_config).endswith("\\"):
    print("Detected bad device.txt config file from an older PyDAW release, deleting...")
    os.system('rm "%s"' % (global_pydaw_device_config,))

global_bookmarks_file_path = global_pydaw_home + "/lms_file_browser_bookmarks.txt"

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
            f_full_path = f_line_arr[1] + "/" + f_line_arr[0]
            if os.path.isdir(f_full_path):
                f_result[f_line_arr[0]] = f_line_arr[1]
            else:
                print(("Warning:  Not loading bookmark '" + f_line_arr[0] + "' because the directory '" + f_full_path + "' does not exist."))
    return f_result

def global_write_file_bookmarks(a_dict):
    f_result = ""
    for k, v in list(a_dict.items()):
        f_result += str(k) + "|||" + str(v) + "\n"
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

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
import liblo
from liblo import *
from libpydaw.pydaw_util import bool_to_int, pydaw_wait_for_finished_file, pydaw_get_wait_file_path


class pydaw_osc:
    def __init__(self, a_with_audio=False):
        if not a_with_audio:
            self.with_osc = False
            return
        else:
            self.with_osc = True
            self.m_suppressHostUpdate = False

            try:
                self.target = liblo.Address(19271)
            except liblo.AddressError as err:
                print((str(err)))
                sys.exit()
            except:
                print(("Unable to start OSC with %s" % (19271,) ))
                self.with_osc = False
                return

            self.base_path = "/dssi/pydaw"
            self.configure_path = self.base_path + "/configure"
            self.exit_path = self.base_path + "/exiting"

    def stop_server(self):
        print("stop_server called")
        if self.with_osc:
            liblo.send(self.target, self.exit_path)

    def send_configure(self, key, value):
        if self.with_osc:
            liblo.send(self.target, self.configure_path, key, value)
        else:
            print(("Running standalone UI without OSC.  Would've sent configure message: key: \"" + str(key) + "\" value: \"" + str(value) + "\""))

    #methods for sending PyDAW OSC messages

    def pydaw_save_song(self):
        self.send_configure("ss", "")

    def pydaw_open_song(self, a_project_folder, a_first_open=True):
        self.send_configure("os",  bool_to_int(a_first_open) + "|" + str(a_project_folder))

    def pydaw_save_item(self, a_uid):
        self.send_configure("si", str(a_uid))

    def pydaw_delete_item(self):
        self.send_configure("di", "TODO")

    def pydaw_save_region(self, a_name):
        self.send_configure("sr", str(a_name))

    def pydaw_delete_region(self):
        self.send_configure("dr", "TODO")

    def pydaw_play(self, a_region_num="0", a_bar="0"):
        self.send_configure("play", str(a_region_num) + "|" + str(a_bar))

    def pydaw_stop(self):
        self.send_configure("stop", "")

    def pydaw_rec(self, a_region_num=0, a_bar=0):
        self.send_configure("rec", str(a_region_num) + "|" + str(a_bar))

    def pydaw_set_loop_mode(self, a_mode):
        self.send_configure("loop", str(a_mode))

    def pydaw_set_tempo(self, a_tempo):
        self.send_configure("tempo", str(a_tempo))

    def pydaw_set_vol(self, a_track_num, a_vol, a_track_type):
        self.send_configure("vol", str(a_track_num) + "|" + str(a_vol) + "|" + str(a_track_type))

    def pydaw_set_solo(self, a_track_num, a_bool, a_track_type):
        self.send_configure("solo", str(a_track_num) + "|" + bool_to_int(a_bool) + "|" + str(a_track_type))

    def pydaw_set_mute(self, a_track_num, a_bool, a_track_type):
        self.send_configure("mute", str(a_track_num) + "|" + bool_to_int(a_bool) + "|" + str(a_track_type))

    def pydaw_set_instrument_index(self, a_track_num, a_index):
        self.send_configure("ci", str(a_track_num) + "|" + str(a_index))

    def pydaw_show_ui(self, a_track_num):
        self.send_configure("su", str(a_track_num))

    def pydaw_save_tracks(self):
        self.send_configure("st", "")

    def pydaw_set_track_rec(self, a_track_type, a_track_num, a_bool):
        self.send_configure("tr", str(a_track_type) + "|" + str(a_track_num) + "|" + bool_to_int(a_bool))

    def pydaw_show_fx(self, a_track_num, a_track_type):
        self.send_configure("fx", str(a_track_num) + "|" + str(a_track_type))

    def pydaw_save_track_name(self, a_track_num, a_new_name, a_type):
        self.send_configure("tn", str(a_track_num) + "|" + str(a_new_name) + "|" + str(a_type))

    def pydaw_offline_render(self, a_start_region, a_start_bar, a_end_region, a_end_bar, a_file_name):
        self.send_configure("or", str(a_start_region) + "|" + str(a_start_bar) + "|" + str(a_end_region) + "|" + str(a_end_bar) + "|" + str(a_file_name))

    def pydaw_set_bus(self, a_track_num, a_bus_num, a_track_type):
        self.send_configure("bs", str(a_track_num) + "|" + str(a_bus_num) + "|" + str(a_track_type))

    def pydaw_reload_audio_items(self, a_region_uid):
        self.send_configure("ai", str(a_region_uid))

    def pydaw_generate_sample_graph(self, a_file, a_uid):
        self.send_configure("sg", str(a_uid) + "|" + str(a_file))

    def pydaw_update_audio_inputs(self):
        self.send_configure("ua", "")

    def pydaw_set_overdub_mode(self, a_is_on):
        """ a_is_on should be a bool """
        self.send_configure("od", bool_to_int(a_is_on))

    def pydaw_load_cc_map(self, a_name):
        self.send_configure("cm", str(a_name))

    def pydaw_ab_open(self, a_file):
        self.send_configure("abo", str(a_file))

    def pydaw_ab_set(self, a_bool):
        self.send_configure("abs", bool_to_int(a_bool))

    def pydaw_ab_pos(self, a_val):
        self.send_configure("abp", str(a_val))

    def pydaw_ab_vol(self, a_val):
        self.send_configure("abv", str(a_val))

    def pydaw_preview_audio(self, a_file):
        self.send_configure("preview", str(a_file))

    def pydaw_panic(self):
        self.send_configure("panic", "")

    def pydaw_convert_wav_to_32_bit(self, a_in_file, a_out_file):
        f_wait_file = pydaw_get_wait_file_path(a_out_file)
        self.send_configure("conv32f", str(a_in_file) + "\n" + str(a_out_file))
        pydaw_wait_for_finished_file(f_wait_file)

    def pydaw_rate_env(self, a_in_file, a_out_file, a_start, a_end):
        f_wait_file = pydaw_get_wait_file_path(a_out_file)
        self.send_configure("renv", str(a_in_file) + "\n" + str(a_out_file) + "\n" + str(a_start) + "|" + str(a_end))
        pydaw_wait_for_finished_file(f_wait_file)

    def pydaw_pitch_env(self, a_in_file, a_out_file, a_start, a_end):
        f_wait_file = pydaw_get_wait_file_path(a_out_file)
        self.send_configure("penv", str(a_in_file) + "\n" + str(a_out_file) + "\n" + str(a_start) + "|" + str(a_end))
        pydaw_wait_for_finished_file(f_wait_file)

    def pydaw_audio_per_item_fx(self, a_region_uid, a_item_index, a_port_num, a_val):
        self.send_configure("paif", str(a_region_uid) + "|" + str(a_item_index) + "|" + str(a_port_num) + "|" + str(a_val))

    def pydaw_audio_per_item_fx_region(self, a_region_uid):
        self.send_configure("par", str(a_region_uid))

    def pydaw_update_plugin_control(self, a_is_instrument, a_track_type, a_track_num, a_port, a_val):
        self.send_configure("pc", bool_to_int(a_is_instrument) + "|" + str(a_track_type) + "|" + str(a_track_num) + "|" + str(a_port) + "|" + str(a_val))

    def pydaw_configure_plugin(self, a_is_instrument, a_track_type, a_track_num, a_key, a_message):
        self.send_configure("co", bool_to_int(a_is_instrument) + "|" + str(a_track_type) + "|" + str(a_track_num) + "|" + str(a_key) + "|" + str(a_message))

    def pydaw_glue_audio(self, a_file_name, a_region_index, a_start_bar_index, a_end_bar_index, a_item_indexes):
        f_index_arr = []
        for f_index in a_item_indexes:
            f_index_arr.append(str(f_index))
        self.send_configure("ga", "%s|%s|%s|%s|%s" % (a_file_name, a_region_index, a_start_bar_index, a_end_bar_index, "|".join(f_index_arr)))
        if self.with_osc:
            f_wait_file = pydaw_get_wait_file_path(a_file_name)
            pydaw_wait_for_finished_file(f_wait_file)

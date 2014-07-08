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
from libpydaw.pydaw_util import bool_to_int, pydaw_wait_for_finished_file, \
    pydaw_get_wait_file_path, global_pydaw_install_prefix, global_stylesheet

try:
    import libpydaw.liblo as liblo
except ImportError:
    try:
        import liblo
    except ImportError:
        from PyQt4 import QtGui
        import locale
        import gettext

        try:
            global_locale, global_encoding = locale.getdefaultlocale()
            global_language = gettext.translation("pydaw4",
                "{}/share/locale".format(global_pydaw_install_prefix),
                [global_locale])
            global_language.install()
        except Exception as ex:
            print("Exception while setting locale, falling back to "
                "English (hopefully)")
            def _(a_string): return a_string

        app = QtGui.QApplication(sys.argv)
        f_error_dialog = QtGui.QDialog()
        f_error_dialog.setStyleSheet(global_stylesheet)
        f_error_layout = QtGui.QVBoxLayout(f_error_dialog)
        f_error_label = QtGui.QLabel(_(
            "Error, cannot import liblo.  This probably means that "
            "you installed the \nwrong "
            "package version.  You must use the version that "
            "corresponds to your version of \n"
            "Ubuntu (or if using Fedora or something else, it must "
            "be compiled against the \n"
            "same version of Python3 that your OS uses).  "
            "If you are unsure, it is probably \n"
            "best to compile PyDAW from the source code "
            "package yourself.\n\nCan't open PyDAW."))
        f_error_layout.addWidget(f_error_label)
        f_error_dialog.show()
        sys.exit(app.exec_())


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
                print("Unable to start OSC with {}".format(19271))
                self.with_osc = False
                return

            self.base_path = "/dssi/pydaw"
            self.configure_path = self.base_path + "/configure"

    def stop_server(self):
        print("stop_server called")
        if self.with_osc:
            self.send_configure("exit", "")

    def send_configure(self, key, value):
        if self.with_osc:
            liblo.send(self.target, self.configure_path, key, value)
        else:
            print("Running standalone UI without OSC.  "
                "Would've sent configure message: key: \""
                "{}\" value: \"{}\"".format(key, value))

    #methods for sending PyDAW OSC messages

    def pydaw_save_song(self):
        self.send_configure("ss", "")

    def pydaw_open_song(self, a_project_folder, a_first_open=True):
        self.send_configure("os",  "{}|{}".format(bool_to_int(a_first_open),
                            a_project_folder))

    def pydaw_save_item(self, a_uid):
        self.send_configure("si", str(a_uid))

    def pydaw_save_region(self, a_name):
        self.send_configure("sr", str(a_name))

    def pydaw_play(self, a_region_num="0", a_bar="0"):
        self.send_configure("play", "{}|{}".format(a_region_num, a_bar))

    def pydaw_stop(self):
        self.send_configure("stop", "")

    def pydaw_rec(self, a_region_num=0, a_bar=0):
        self.send_configure("rec", "{}|{}".format(a_region_num, a_bar))

    def pydaw_set_loop_mode(self, a_mode):
        self.send_configure("loop", str(a_mode))

    def pydaw_set_tempo(self, a_tempo):
        self.send_configure("tempo", str(a_tempo))

    def pydaw_set_vol(self, a_track_num, a_vol, a_track_type):
        self.send_configure(
            "vol", "{}|{}|{}".format(a_track_num, a_vol, a_track_type))

    def pydaw_set_solo(self, a_track_num, a_bool, a_track_type):
        self.send_configure(
            "solo", "{}|{}|{}".format(a_track_num, bool_to_int(a_bool),
            a_track_type))

    def pydaw_set_mute(self, a_track_num, a_bool, a_track_type):
        self.send_configure(
            "mute", "{}|{}|{}".format(a_track_num, bool_to_int(a_bool),
            a_track_type))

    def pydaw_set_instrument_index(self, a_track_num, a_index):
        self.send_configure("ci", "{}|{}".format(a_track_num, a_index))

    def pydaw_show_ui(self, a_track_num):
        self.send_configure("su", str(a_track_num))

    def pydaw_save_tracks(self):
        self.send_configure("st", "")

    def pydaw_set_track_rec(self, a_track_type, a_track_num, a_bool):
        self.send_configure(
            "tr", "{}|{}|{}".format(a_track_type, a_track_num,
            bool_to_int(a_bool)))

    def pydaw_show_fx(self, a_track_num, a_track_type):
        self.send_configure(
            "fx", "{}|{}".format(a_track_num, a_track_type))

    def pydaw_save_track_name(self, a_track_num, a_new_name, a_type):
        self.send_configure(
            "tn", "{}|{}|{}".format(a_track_num, a_new_name, a_type))

    def pydaw_offline_render(self, a_start_region, a_start_bar, a_end_region,
                             a_end_bar, a_file_name):
        self.send_configure(
            "or", "|".join(map(str,
            (a_start_region, a_start_bar, a_end_region, a_end_bar,
             a_file_name))))

    def pydaw_we_export(self, a_file_name):
        self.send_configure("wex", "{}".format(a_file_name))

    def pydaw_set_bus(self, a_track_num, a_bus_num, a_track_type):
        self.send_configure(
            "bs", "|".join(map(str, (a_track_num, a_bus_num, a_track_type))))

    def pydaw_reload_audio_items(self, a_region_uid):
        self.send_configure("ai", str(a_region_uid))

    def pydaw_add_to_wav_pool(self, a_file, a_uid):
        self.send_configure("wp", "{}|{}".format(a_uid, a_file))

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

    def pydaw_we_set(self, a_val):
        self.send_configure("we", str(a_val))

    def pydaw_preview_audio(self, a_file):
        self.send_configure("preview", str(a_file))

    def pydaw_stop_preview(self):
        self.send_configure("spr", "")

    def pydaw_panic(self):
        self.send_configure("panic", "")

    def pydaw_rate_env(self, a_in_file, a_out_file, a_start, a_end):
        f_wait_file = pydaw_get_wait_file_path(a_out_file)
        self.send_configure(
            "renv", "{}\n{}\n{}|{}".format(a_in_file, a_out_file,
            a_start, a_end))
        pydaw_wait_for_finished_file(f_wait_file)

    def pydaw_pitch_env(self, a_in_file, a_out_file, a_start, a_end):
        f_wait_file = pydaw_get_wait_file_path(a_out_file)
        self.send_configure(
            "penv", "{}\n{}\n{}|{}".format(a_in_file, a_out_file,
            a_start, a_end))
        pydaw_wait_for_finished_file(f_wait_file)

    def pydaw_audio_per_item_fx(self, a_region_uid, a_item_index,
                                a_port_num, a_val):
        self.send_configure(
            "paif", "|".join(map(str,
             (a_region_uid, a_item_index, a_port_num, a_val))))

    def pydaw_audio_per_item_fx_region(self, a_region_uid):
        self.send_configure("par", str(a_region_uid))

    def pydaw_update_plugin_control(self, a_is_instrument, a_track_type,
                                    a_track_num, a_port, a_val):
        self.send_configure(
            "pc", "|".join(map(str,
            (bool_to_int(a_is_instrument), a_track_type, a_track_num,
            a_port, a_val))))

    def pydaw_configure_plugin(self, a_is_instrument, a_track_type,
                               a_track_num, a_key, a_message):
        self.send_configure(
            "co", "|".join(map(str,
            (bool_to_int(a_is_instrument), a_track_type, a_track_num,
             a_key, a_message))))

    def pydaw_glue_audio(self, a_file_name, a_region_index, a_start_bar_index,
                         a_end_bar_index, a_item_indexes):
        f_index_arr = []
        for f_index in a_item_indexes:
            f_index_arr.append(str(f_index))
        self.send_configure("ga", "|".join(map(str,
           (a_file_name, a_region_index, a_start_bar_index, a_end_bar_index,
           "|".join(f_index_arr)))))
        if self.with_osc:
            f_wait_file = pydaw_get_wait_file_path(a_file_name)
            pydaw_wait_for_finished_file(f_wait_file)

    def pydaw_midi_learn(self, a_is_on):
        self.send_configure("ml", bool_to_int(a_is_on))

    def pydaw_reload_wavpool_item(self, a_uid):
        self.send_configure("wr", str(a_uid))


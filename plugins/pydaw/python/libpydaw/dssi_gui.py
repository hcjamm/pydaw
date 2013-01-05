#!/usr/bin/env python

import sys
import liblo
from liblo import *
from urlparse import urlparse

#TODO:  Move this to a mutual location and rename to bool_to_int_str
def bool_to_int(a_bool):
    if a_bool:
        return "1"
    else:
        return "0"


class dssi_gui(ServerThread):    
    def __init__(self, a_url=None, a_pc_func=None):
        """    
        a_url:  The OSC URL to send to.  Format will be something like osc.udp://localhostname:12345/dssi/pydaw/...
        a_pc_func:  The function with signature(int, int) to be called when the engine returns a playback cursor configure message
        """
        self.pc_func = a_pc_func
        if a_url is None:
            self.with_osc = False
            return
        else:
            self.with_osc = True
            self.m_suppressHostUpdate = False

            ServerThread.__init__(self, None)
            self.start()

            o = urlparse(a_url)

            try:
                self.target = liblo.Address(o.port)
            except liblo.AddressError, err:
                print str(err)
                sys.exit()
            except:
                print("Unable to start OSC with " + str(o.port))
                self.with_osc = False
                return

            self.base_path = str.split(a_url, str(o.port))[1]
            print("base_path = " + self.base_path)

            self.control_path = self.base_path + "/control"
            self.program_path = self.base_path + "/program"
            self.configure_path = self.base_path + "/configure"
            self.exit_path = self.base_path + "/exiting"
            self.rate_path = self.base_path + "/sample-rate"
            self.show_path = self.base_path + "/show"
            self.hide_path = self.base_path + "/hide"
            self.quit_path = self.base_path + "/quit"
            self.update_path = self.base_path + "/update"

            self.add_method(self.configure_path, 'ss', self.configure_handler)
            self.add_method(self.control_path, 'if', self.control_handler)

            liblo.send(self.target, self.update_path, self.get_url()[:-1] + self.base_path)
            print("Sent " + self.get_url()[:-1] + self.base_path + " to " + self.update_path)

    def stop_server(self):
        print("stop_server called")
        if self.with_osc:
            liblo.send(self.target, self.exit_path)
            self.stop()

    def send_control(self, port_number, port_value):
        if self.with_osc:
            liblo.send(self.target, self.control_path, port_number, port_value)
        else:
            print("Running standalone UI without OSC.  Would've sent control message: key:" + str(port_number) + " value: " + str(port_value))

    def send_configure(self, key, value):
        if self.with_osc:
            liblo.send(self.target, self.configure_path, key, value)
        else:
            print("Running standalone UI without OSC.  Would've sent configure message: key: \"" + str(key) + "\" value: \"" + str(value) + "\"")
    
    def configure_handler(self, path, args):
        s1, s2 = args
        print("PyDAW configure_handler called key: " + s1 + " value: " + s2 + "\n")
        if s1 == "pc":  #playback cursor
            if not self.pc_func is None:                
                f_vals = s2.split("|")
                self.pc_func(int(f_vals[0]), int(f_vals[1]))                

    def control_handler(self, path, args):
        i, f = args
        if self.with_osc:
            liblo.send(target, self.control_path, i, f)

    #def rate_handler(self, path, args):
    #    if self.with_osc:
    #        liblo.send(target, self.rate_path, args[0])

    @make_method(None, None)
    def fallback(path, args, types, src):
        if self.with_osc:
            print "got unknown message '%s' from '%s'" % (path, src.get_url())
            for a, t in zip(args, types):
                print "argument of type '%s': %s" % (t, a)

    #methods for sending PyDAW-protocol OSC messages

    def pydaw_save_song(self):
        self.send_configure("ss", "")

    def pydaw_open_song(self, a_project_folder):
        self.send_configure("os", a_project_folder)

    def pydaw_save_item(self, a_name):
        self.send_configure("si", str(a_name))

    def pydaw_delete_item(self):
        self.send_configure("di", "TODO")

    def pydaw_save_region(self, a_name):
        self.send_configure("sr", str(a_name))

    def pydaw_delete_region(self):
        self.send_configure("dr", "TODO")

    def pydaw_rename_item(self):
        self.send_configure("ri", "TODO")

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

    def pydaw_set_timesig(self):
        self.send_configure("tsig", "TODO")

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
        
    def pydaw_set_track_rec(self, a_track_num, a_bool):
        if a_bool:
            a_value = 1
        else:
            a_value = 0
        self.send_configure("tr", str(a_track_num) + "|" + str(a_value))
        
    def pydaw_show_fx(self, a_track_num, a_track_type):
        self.send_configure("fx", str(a_track_num) + "|" + str(a_track_type))
        
    def pydaw_save_track_name(self, a_track_num, a_new_name, a_type):
        self.send_configure("tn", str(a_track_num) + "|" + str(a_new_name) + "|" + a_type)
        
    def pydaw_offline_render(self, a_start_region, a_start_bar, a_end_region, a_end_bar, a_file_name):
        self.send_configure("or", str(a_start_region) + "|" + str(a_start_bar) + "|" + str(a_end_region) + "|" + str(a_end_bar) + "|" + str(a_file_name))
    
    def pydaw_set_bus(self, a_track_num, a_bus_num):
        self.send_configure("bs", str(a_track_num) + "|" + str(a_bus_num))
    
    def pydaw_reload_audio_items(self):
        self.send_configure("ai", "")
    
    def pydaw_load_single_audio_item(self, a_item):
        """ a_item should be a valid pydaw_audio_item """
        self.send_configure("as", str(a_item))
    
    def pydaw_generate_sample_graph(self, a_file):
        self.send_configure("sg", "")
    
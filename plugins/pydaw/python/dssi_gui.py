#!/usr/bin/env python

import sys
import liblo
from liblo import *
from urlparse import urlparse

"""
Class for implementing one's own DSSI GUI.  Instantiate this class and 
call it's functions to send OSC messages to the DSSI plugin's audio/MIDI
engine
"""
class dssi_gui(ServerThread):
    def __init__(self, a_url=None):
        if a_url is None:
            self.with_osc = False
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
                
    @make_method('/dssi/pydaw/PYDAW/chan00/configure', 'ss')
    def configure_handler(self, path, args):
        s1, s2 = args
        if self.with_osc:
            liblo.send(self.target, self.configure_path, s1, s2)
    
    @make_method('/dssi/pydaw/PYDAW/chan00/control', 'if')
    def control_handler(self, path, args):
        i, f = args
        if self.with_osc:
            liblo.send(target, self.control_path, i, f)
    
    @make_method('/dssi/pydaw/PYDAW/chan00/sample-rate', 'i')
    def rate_handler(self, path, args):
        if self.with_osc:
            liblo.send(target, self.rate_path, args[0])
    
    #@make_method('/foo', 'ifs')
    def debug_handler(self, path, args):
        # send message "/foo/message1" with int, float and string arguments
        #liblo.send(target, "/foo/message1", 123, 456.789, "test")
        pass
    
    @make_method(None, None)
    def fallback(path, args, types, src):
        print "got unknown message '%s' from '%s'" % (path, src.get_url())
        for a, t in zip(args, types):
            print "argument of type '%s': %s" % (t, a)
    
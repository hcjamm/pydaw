#!/usr/bin/env python

import liblo, sys

"""
Base class for implementing one's own DSSI GUI
"""
class dssi_gui:
    def __init__(self, a_url):

        self.url = a_url
        self.host = liblo.Address.get_hostname(self.url)
        self.port = liblo.Address.get_port(self.url)
        self.path = liblo.Address.get_path(self.url)

        # create self.server, listening on port 1234
        try:
            self.server = liblo.self.server(self.port)
        except liblo.self.serverError, err:
            print str(err)
            sys.exit()

        self.server.add_method(self.path + "/control", 'if', self.control_handler)
        self.server.add_method(self.path + "/program", 'ii', self.program_handler)
        self.server.add_method(self.path + "/configure", 'ss', self.configure_handler)
        self.server.add_method(self.path + "/sample-rate", 'i', self.rate_handler)
        self.server.add_method(self.path + "/show", '', self.show_handler)
        self.server.add_method(self.path + "/hide", '', self.hide_handler)
        self.server.add_method(self.path + "/quit", '', self.quit_handler)
        ##    lo_self.server.add_method(osc_self.server, NULL, NULL, debug_handler, &gui);
        # register a fallback for unhandled messages
        self.server.add_method(None, None, self.fallback)
        ##UI stuff goes here???
        # loop and dispatch messages every 100ms
        #while True:
            #self.server.recv(100)
        self.server.start()

    def configure_handler(self, path, args):
        pass
    def control_handler(self, path, args):
        pass
    def rate_handler(self, path, args):
        pass
    def debug_handler(self, path, args):
        pass
    def fallback(path, args, types, src):
        print "got unknown message '%s' from '%s'" % (path, src.get_url())
        for a, t in zip(args, types):
            print "argument of type '%s': %s" % (t, a)
    """Override this function"""
    def init_gui():
        pass
    """Override this function"""
    def v_set_control():
        pass
    """Override this function"""
    def v_control_changed():
        pass
    """Override this function"""
    def i_get_control():
        pass


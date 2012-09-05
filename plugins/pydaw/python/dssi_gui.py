#!/usr/bin/env python

import liblo, sys

"""
Base class for implementing one's own DSSI GUI
"""
class dssi_gui:
    def __init__(self):
        if (argv.count < 5):
            print('test')            
            exit(2)

        self.url = argv[1];
        self.host = liblo.Address.get_hostname(url);
        self.port = liblo.Address.get_port(url);
        self.path = liblo.Address.get_path(url);

        # create self.server, listening on port 1234
        try:
            self.server = liblo.self.server(self.port)
        except liblo.self.serverError, err:
            print str(err)
            sys.exit()

        self.server.add_method(self.path + "/control", 'if', control_handler)
        self.server.add_method(self.path + "/program", 'ii', program_handler)
        self.server.add_method(self.path + "/configure", 'ss', configure_handler)
        self.server.add_method(self.path + "/sample-rate", 'i', rate_handler)
        self.server.add_method(self.path + "/show", '', show_handler)
        self.server.add_method(self.path + "/hide", '', hide_handler)
        self.server.add_method(self.path + "/quit", '', quit_handler)
        ##    lo_self.server.add_method(osc_self.server, NULL, NULL, debug_handler, &gui);
        # register a fallback for unhandled messages
        self.server.add_method(None, None, fallback)
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


test = dssi_gui()

raw_input('press enter to quit')


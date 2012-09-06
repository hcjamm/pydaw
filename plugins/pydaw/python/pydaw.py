# PyDAW - Part of the LibModSynth project
#
# A DAW using Python and OpenGL for the UI, and a high performance audio/MIDI
# backend written in C
#
# Copyright 2012, Jeff Hubbard

import os
import pyglet
# Disable error checking for increased performance
pyglet.options['debug_gl'] = False
from pyglet import gl
from sys import argv
from os.path import expanduser

VERSION = '5.9'

import kytten

from lmssession import lms_session
from dssi_gui import dssi_gui

# Default theme, gold-colored
theme = kytten.Theme(os.path.join(os.getcwd(), 'theme'), override={
    #"gui_color": [64, 128, 255, 255],
    "gui_color": [255, 240, 255, 255],
    "font_size": 12
})

# Default theme, blue-colored
theme2 = kytten.Theme(theme, override={
    "gui_color": [255, 235, 128, 255],
    "font_size": 10
})

# Callback functions for dialogs which may be of interest
def on_escape(dialog):
    dialog.teardown()    

for arg in argv:
    print arg
    
if(len(argv) >= 2):
    this_dssi_gui = dssi_gui(argv[1])
    
user_home_folder = expanduser("~")
print("user_home_folder = " + user_home_folder)
session_mgr = lms_session(user_home_folder + '/default.pss')    
    
class seq_track:
    def on_vol_change(self, value):
        print(value)
    def on_pan_change(self, value):
        pass
    def on_solo(self, value):
        pass
    def on_mute(self, value):
        pass
    def on_rec(self, value):
        pass
    def on_name_changed(self, new_name):
        pass
    def on_instrument_change(self, selected_instrument):
        session_mgr.instrument_index_changed(self.track_number, selected_instrument, self.instrument_label.text)
    
    def __init__(self, a_track_num, a_track_text="track"):
        self.instrument_select = kytten.Dropdown(['None', 'Euphoria', 'Ray-V'], on_select=self.on_instrument_change)
        self.instrument_label = kytten.Input(text=a_track_text)
        
        self.dialog = kytten.Dialog(
        	kytten.Frame(
                 kytten.VerticalLayout([
                         kytten.Slider(value=0.0, min_value=-50.0, max_value=12.0, on_set=self.on_vol_change),
                         kytten.HorizontalLayout([                             
                             kytten.Checkbox(text="Solo",on_click=self.on_solo),
                             kytten.Checkbox(text="Mute",on_click=self.on_mute),    
                             kytten.Checkbox(text="Rec",on_click=self.on_rec)
                            ]),
                         kytten.HorizontalLayout([
                              self.instrument_label,
                              self.instrument_select
                             ])
                     ])
        	),
        	window=window, batch=batch, group=fg_group,
        	anchor=kytten.ANCHOR_CENTER,
        	theme=theme2, on_escape=on_escape, movable=True)
        
        self.track_number = a_track_num

        self.midi_items = []
        self.midi_item_dialogs = []
        
        for i in range(0, 8):
            self.midi_items.append(midi_item())
            self.midi_item_dialogs.append(self.midi_items[i].dialog)        
        
        self.midi_track = kytten.Dialog(
        	kytten.Frame(
                 kytten.HorizontalLayout(self.midi_item_dialogs)
        	),
        	window=window, batch=batch, group=fg_group,
        	anchor=kytten.ANCHOR_CENTER,
        	theme=theme2, on_escape=on_escape, movable=True)
         
        self.hlayout = kytten.HorizontalLayout([self.dialog, self.midi_track])
    

class track_view:
    def on_play(self):
        print("Playing")
    def on_stop(self):
        print("Stopping")
    def on_rec(self):
        print("Recording")
    def on_new(self):
        print("Creating new project")
    def on_open(self):
        print("Opening existing project")
    def on_save(self):
        print("Saving project")
        session_mgr.save_session_file()  #Notify the instruments to save their state
        this_dssi_gui.send_configure("save", "testing") #Send a message to the DSSI engine to save it's state.  Currently, this doesn't do anything...
            
    def __init__(self):
        self.transport_dialog = kytten.Dialog(
        	kytten.Frame(
             kytten.VerticalLayout([
        	    kytten.HorizontalLayout([
                     kytten.Button(text="Play",on_click=self.on_play),
                     kytten.Button(text="Stop",on_click=self.on_stop),
                     kytten.Button(text="Rec",on_click=self.on_rec),
        	    ]),
        	    kytten.HorizontalLayout([
                     kytten.Button(text="New",on_click=self.on_new),
                     kytten.Button(text="Open",on_click=self.on_open),
                     kytten.Button(text="Save",on_click=self.on_save),
        	    ]),
             ])
        	),
        	window=window, batch=batch, group=fg_group,
        	anchor=kytten.ANCHOR_CENTER, theme=theme2)
                
        self.tracks = [
                kytten.HorizontalLayout([self.transport_dialog, kytten.Slider(max_value=16)])
            ]

        for i in range(0, 16):
            f_seq_track = seq_track(i, a_track_text="track" + str(i))
            self.tracks.append(f_seq_track.hlayout)
        
        self.layout = kytten.VerticalLayout(self.tracks)
        self.scrollable = kytten.Scrollable(self.layout, width=1180, height=600, always_show_scrollbars=False)
        
        self.dialog = kytten.Dialog(
            	kytten.Frame(
            	    self.scrollable
        	),
        	window=window, batch=batch, group=fg_group,
        	anchor=kytten.ANCHOR_CENTER,
        	theme=theme2, on_escape=on_escape)
    
    def __del__(self):
        this_dssi_gui.stop_server()

class midi_item:
    def __init__(self):
        self.dialog =  kytten.Dialog(
        	kytten.Frame(
                 kytten.VerticalLayout([
                         kytten.Button(text="Edit"),                         
                         kytten.HorizontalLayout([
                              kytten.Input(text="MIDI Item", length=15, max_length=20),                              
                             ])
                     ])
        	),
        	window=window, batch=batch, group=fg_group,
        	anchor=kytten.ANCHOR_CENTER,
        	theme=theme2, on_escape=on_escape, movable=True)
 

if __name__ == '__main__':
    window = pyglet.window.Window(
	1200, 700, caption="PyDAW - By LibModSynth",
	resizable=True, vsync=False)
    batch = pyglet.graphics.Batch()
    bg_group = pyglet.graphics.OrderedGroup(0)
    fg_group = pyglet.graphics.OrderedGroup(1)
    fps = pyglet.clock.ClockDisplay()
    
    @window.event
    def on_draw():
	window.clear()
	batch.draw()
	fps.draw()

    # Update as often as possible (limited by vsync, if not disabled)
    window.register_event_type('on_update')
    def update(dt):
	window.dispatch_event('on_update', dt)
    pyglet.clock.schedule(update)
    
    track_view_test = track_view()
                         
    # Change this flag to run with profiling and dump top 20 cumulative times
    if True:
	pyglet.app.run()
    else:
	import cProfile
	cProfile.run('pyglet.app.run()', 'kytten.prof')
	import pstats
	p = pstats.Stats('kytten.prof')
	p.strip_dirs()
	p.sort_stats('cumulative')
	p.print_stats(20)

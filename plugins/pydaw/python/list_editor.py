# PyDAW - Part of the LibModSynth project
#
# A generic list editor for MIDI events. Whether or not this is the final
# solution remains to be seen, a piano-roll editor is somewhat intuitive,
# but OTOH, a visualization of a millenia-old musical instrument isn't 
# inline with my vision of a bleeding-edge editor that is both capable
# and intuitive...  Think alternative tunings, algorithimic music, and
# advanced editing and automation.
#
# Copyright 2012, Jeff Hubbard
# See GPLv3 for license

import os
import pyglet
# Disable error checking for increased performance
pyglet.options['debug_gl'] = False
from pyglet import gl
from sys import argv
from os.path import expanduser

import kytten

from list_item import *

# Default theme, gold-colored
theme = kytten.Theme(os.path.join(os.getcwd(), 'theme'), override={
    #"gui_color": [64, 128, 255, 255],
    "gui_color": [255, 240, 255, 255],
    "font_size": 12
})

theme2 = kytten.Theme(theme, override={
    "gui_color": [255, 235, 128, 255],
    "font_size": 10
})

def note_number_to_string(a_note_number):
    pass

def string_to_note_number(a_note_string):
    pass

class le_event_base:
    def __init__(self):
        self.time_in_samples = 0
        self.time_in_bars = 0
    
    #offset is typically -1.0 to 1.0, although more typically much less than that, like < 0.1
    def set_musical_time_in_beats(self, a_beats, a_offset=0.0):
        pass
    
    def set_musical_time_in_notes(self, a_numerator, a_denominator):
        pass

class le_note_event(le_event_base):
    def __init__(self):
        pass
    
class le_controller_event(le_event_base):
    def __init__(self):
        pass

#This may or may not be used...
class le_audio_event(le_event_base):
    def __init__(self):
        self.mode = 0 #0 == one-shot, 1 == loop .  At some point when timestretching is available, 2 == stretch

class midi_list_editor:
    #If a_new_file_name is set, a_file_name will be copied into a new file name with the name a_new_file_name
    def __init__(self, a_file_name, a_new_file_name=None):
        self.events = []        
        self.layout = kytten.VerticalLayout(self.events)
        self.scrollable = kytten.Scrollable(self.layout, width=1180, height=600, always_show_scrollbars=False)
        
        self.dialog = kytten.Dialog(
            	kytten.Frame(
                     kytten.VerticalLayout([
                         self.scrollable
                         ])
        	),
        	window=window, batch=batch, group=fg_group,
        	anchor=kytten.ANCHOR_CENTER,
        	theme=theme2)
        
    
#TODO:  I think this could all be done in the midi_list_editor constructor?
# That would allow the parent window access to functions, properties and
# signals from here, rather than launching as a separate script
if __name__ == '__main__':
    window = pyglet.window.Window(
	900, 700, caption="PyDAW MIDI Item List Editor",
	resizable=False, vsync=False)
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
    
    #Declare the window class here
    
    user_home_folder = expanduser("~")

    midi_list_editor(user_home_folder + '/default.pymid')  
                  
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
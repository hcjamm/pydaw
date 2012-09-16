# PyDAW - Part of the LibModSynth project
#
# THIS FILE IS CURRENTLY DEPRECATED IN FAVOR OF THE QT PORT!
#
# A DAW using Python and OpenGL for the UI, and a high performance audio/MIDI
# backend written in C
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

VERSION = '5.9'

import kytten

from lms_session import lms_session
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



class seq_track:
    def __init__(self):
        self.regions = []
        
    def __str__(self):
        return "" #TODO

class list_region:
    def __init__(self, a_string=None):
        self.seq_items = []
        
        if not a_string is None:
            pass            
    
    def __str__(self):
        return "" #TODO

class list_item_seq:
    def __init__(self, a_position, a_file_name, a_length=1, a_actual_length=1):
        self.position = a_position
        self.length = a_length
        self.name = a_file_name
        self.note_items = []
        self.fq_file_path = os.getcwd() + a_file_name        
        
        file_handle = open(self.fq_file_path, 'r')        
        line_arr = file_handle.readlines()        
        file_handle.close()
        
        for line in line_arr:
            item_arr = line.split('|')
            self.note_items.append(list_item_content(item_arr[0], item_arr[1]))
        
    """
    Rename the underlying file associated with the sequencer item
    """
    def rename(self, a_name):
        pass
    
    def __str__(self):
        f_result = ""
        for content_item in self.note_items:
            f_result += content_item
        
        return f_result
        
class list_item_content:
    """
    Tentatively, for a_type:  0 == MIDI Note, 1 == MIDI CC, 2 == Pitchbend, 3 == Audio
    """
    def __init__(self, a_type, a_start, a_length):
        self.type = a_type        
        self.start = a_start
        self.length = a_length
            
    def __str__(self):
        return str(self.type) + '|' + str(self.start) + '|' + str(self.length) + "\n"
    
    def on_delete(self):
        pass
    
    def on_mute(self):
        pass
    
    """
    Return a widget suitable for displaying in the list editor
    """
    def get_widget(self):
        if self.type == 0:
            return kytten.Dialog(
            	kytten.Frame(
                     kytten.HorizontalLayout([
                         kytten.Input(text='0', length=4), #Start
                         kytten.Input(text='1', length=4), #Length
                         kytten.Dropdown(['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']),                            
                         kytten.Dropdown(['-2', '-1', '0', '1', '2', '3', '4', '5', '6', '7', '8']),
                         kytten.Checkbox(text="Mute",on_click=self.on_mute),
                        ]),
            	),
            	window=window, batch=batch, group=fg_group,
            	anchor=kytten.ANCHOR_CENTER,
            	theme=theme2, movable=True)
        elif self.type == 1:
            return kytten.Dialog(
            	kytten.Frame(
                     kytten.HorizontalLayout([
                         kytten.Input(text='0', length=4), #Start
                         kytten.Input(text='1', length=4), #Length
                         kytten.Input(text='64', length=4), #CC
                         kytten.Slider(max_value=127.0), #Value
                         kytten.Checkbox(text="Mute",on_click=self.on_mute),
                        ]),
            	),
            	window=window, batch=batch, group=fg_group,
            	anchor=kytten.ANCHOR_CENTER,
            	theme=theme2, movable=True)
        elif self.type == 2:
            pass
        elif self.type == 3:
            return kytten.Dialog(
            	kytten.Frame(
                     kytten.HorizontalLayout([
                         kytten.Input(text='0', length=4), #Start
                         kytten.Input(text='1', length=4), #Length
                         kytten.Dropdown(['test1.wav', 'test2.wav', 'test3.wav', 'test4.wav']),                            
                         kytten.Dropdown(['One Shot', 'Loop']),
                         kytten.Checkbox(text="Mute",on_click=self.on_mute),
                        ]),
             
            	),
            	window=window, batch=batch, group=fg_group,
            	anchor=kytten.ANCHOR_CENTER,
            	theme=theme2, movable=True)

def note_number_to_string(a_note_number):
    pass

def string_to_note_number(a_note_string):
    pass

class region_list_editor:
    #If a_new_file_name is set, a_file_name will be copied into a new file name with the name a_new_file_name
    def __init__(self):
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
         
    """
    This should be called whenever the items have been changed, or when 
    switching items
    
    a_items should be an array of 
    """
    def update_items(self, a_items=[]):
         self.layout.delete()
         
         for item in a_items:
             item_dialog = item.get_widget()
             self.layout.add(item_dialog)
    

class item_list_editor:
    #If a_new_file_name is set, a_file_name will be copied into a new file name with the name a_new_file_name
    def __init__(self):
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
         
    """
    This should be called whenever the items have been changed, or when 
    switching items
    
    a_items should be an array of 
    """
    def update_items(self, a_items=[]):
         self.layout.delete()
         
         for item in a_items:
             item_dialog = item.get_widget()
             self.layout.content.append(item_dialog)
    
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
    def on_file_menu_select(self, a_selected):
        pass            
            
    def __init__(self):
        self.transport_dialog = kytten.Dialog(
        	kytten.Frame(
             kytten.VerticalLayout([
               kytten.HorizontalLayout([
                     kytten.Dropdown(['File', 'New', 'Open', 'Save'], on_select=self.on_file_menu_select),                     
        	    ]),
             
        	    kytten.HorizontalLayout([
                     kytten.Button(text="Play",on_click=self.on_play),
                     kytten.Button(text="Stop",on_click=self.on_stop),
                     kytten.Button(text="Rec",on_click=self.on_rec),
        	    ]),
             ])
        	),
        	window=window, batch=batch, group=fg_group,
        	anchor=kytten.ANCHOR_CENTER, theme=theme2)
                
        self.tracks = [
                kytten.HorizontalLayout()
            ]

        for i in range(0, 16):
            f_seq_track = seq_track(i, a_track_text="track" + str(i))
            self.tracks.append(f_seq_track.dialog)
        
        self.tracks_layout = kytten.VerticalLayout(self.tracks)
        
        self.scrollable = kytten.Scrollable(self.tracks_layout, width=1180, height=600, always_show_scrollbars=False)
        
        self.vlayout0 = kytten.VerticalLayout([self.transport_dialog, self.scrollable])

        self.region_editor = region_list_editor()
        self.item_editor = item_list_editor()        
        
        self.dialog = kytten.Dialog(
            	kytten.Frame(
                     kytten.HorizontalLayout([
                        self.vlayout0,
                        self.region_editor.dialog,
                        self.item_editor.dialog
                     ])            	    
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
	1200, 750, caption="PyDAW - By LibModSynth",
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
    
    for arg in argv:
        print arg
    
    if(len(argv) >= 2):
        this_dssi_gui = dssi_gui(argv[1])
        
    user_home_folder = expanduser("~")
    print("user_home_folder = " + user_home_folder)
    session_mgr = lms_session(user_home_folder + '/default.pydaw')    

    
    track_view_test = track_view()
    
    item_list = [        
        list_item_content(0, 1, 1),
        list_item_content(1, 1, 1),
        list_item_content(3, 1, 1)    
    ]
    
    track_view_test.item_editor.update_items(a_items=item_list)
                         
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

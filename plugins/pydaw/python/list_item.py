# Items for use in a list editor
import os
import kytten

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
    
    """
    Return a widget suitable for displaying in the list editor
    """
    def get_widget(self):
        f_result = kytten.Dialog(
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
        return f_result
        

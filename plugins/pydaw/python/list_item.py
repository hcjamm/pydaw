# Items for use in a list editor
import os


class seq_track:
    def __init__(self):
        self.regions = []
        
    def __unicode__(self):
        return "" #TODO

class list_region:
    def __init__(self, a_string=None):
        self.seq_items = []
        
        if not a_string is None:
            pass            
    
    def __unicode__(self):
        return "" #TODO

class list_item_seq:
    def __init__(self, a_position, a_file_name, a_length=1, 
                 a_actual_length=1):        
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
            
    def rename(self):
        pass
    
    def __unicode__(self):
        f_result = ""
        for content_item in self.note_items:
            f_result += content_item.__unicode__()
        
        return f_result
        
class list_item_content:
    def __init__(self, a_type, a_start, a_length):
        self.type = a_type        
        self.start = a_start
        self.length = a_length
            
    def __unicode__(self):
        return str(self.type) + '|' + str(self.start) + '|' + str(self.length) + "\n"
        

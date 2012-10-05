# -*- coding: utf-8 -*-
"""
functions for writing to the UI's log
"""
from datetime import datetime

def pydaw_write_log(a_text):    
    f_file = open("pydaw_ui.log", "a")    
    f_file.write(str(datetime.now()) + " - " + a_text + "\n")
    f_file.close()
    
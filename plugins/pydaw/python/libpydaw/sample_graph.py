from PyQt4 import QtGui, QtCore

class pydaw_sample_graphs:
    def __init__(self, a_file):
        self.graphs = {}
    
    def __str__(self):
        f_result = ""
        for k, v in self.graphs.iteritems():
            f_result += str(v)
        return f_result
    
class pydaw_sample_graph:
    def open_file(self, a_dir):
        f_file_name = a_dir + "/" + str(self.uid) + ".pygraph"
        f_file = open(f_file_name, "r")
        f_line_arr = f_file.readlines()
        f_file.close()
        for f_line in f_line_arr:
            pass  #TODO:  Something with some well-defined file format...
    
    def __init__(self, a_uid, a_file):
        self.uid = int(a_uid)
        self.file = str(a_file)
        self.graph = None
    
    def __str__(self):
        return str(self.uid) + "|" + self.file + "\n"
    
        
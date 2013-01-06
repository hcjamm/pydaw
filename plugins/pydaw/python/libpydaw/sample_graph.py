from PyQt4 import QtGui, QtCore

class pydaw_sample_graphs:
    def __init__(self):        
        self.graphs = {}
    
    def __str__(self):
        f_result = ""
        for k, v in self.graphs.iteritems():
            f_result += str(k) + "|" + str(v) + "\n"
        f_result += "\\"
        return f_result
    
    @staticmethod
    def from_str(a_str):
        f_result = pydaw_sample_graphs()
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == "\\":
                break
            f_line_arr = f_line.split("|")
            f_result.graphs[f_line_arr[0]] = f_line_arr[1]
        return f_result
    
class pydaw_sample_graph:
    def __init__(self, a_file_name):
        self.uid = None
        self.file = None
        self.timestamp = None
        self.channels = None
        self.high_peaks = ([],[])
        self.low_peaks = ([],[])
        self.count = None
        f_file = open(a_file_name, "r")
        f_line_arr = f_file.readlines()
        f_file.close()
        for f_line in f_line_arr:
            f_line_arr = f_line.split("|")
            if f_line_arr[0] == "\\":
                break
            elif f_line_arr[0] == "meta":
                if f_line_arr[1] == "filename":
                    self.file = f_line_arr[2]
                elif f_line_arr[1] == "timestamp":
                    self.timestamp = f_line_arr[2]
                elif f_line_arr[1] == "channels":
                    self.channels = int(f_line_arr[2])
                elif f_line_arr[1] == "count":
                    self.count = int(f_line_arr[2])
            elif f_line_arr[0] == "p":
                if f_line_arr[2] == "h":
                    self.high_peaks[int(f_line_arr[1])].append(float(f_line_arr[3]))
                elif f_line_arr[2] == "l":
                    self.low_peaks[int(f_line_arr[1])].append(float(f_line_arr[3]))
                else:
                    print("Invalid sample_graph [2] value " + f_line_arr[2] )
            
    def is_valid(self):
        return (self.uid is not None) and (self.file is not None) and (self.timestamp is not None) \
        and (self.channels is not None) and (self.count is not None)
    
    def create_sample_graph(self, a_height, a_width):
        f_width_inc = float(a_width) / self.count
        f_width_pos = 0.0
        f_result = QtGui.QPainterPath()
        for f_i in range(self.channels):
            f_result.moveTo(0.0, 50.0)
            for f_peak in self.high_peaks[f_i]:
                f_result.lineTo(f_width_pos, f_peak)  #TODO:  These will need to be multiplied by other values
                f_width_pos += f_width_inc
            for f_peak in self.low_peaks[f_i]:                
                f_result.lineTo(f_width_pos, f_peak)  #TODO:  These will need to be multiplied by other values
                f_width_pos -= f_width_inc
            f_result.closeSubpath()
        return f_result
    
        
from PyQt4 import QtGui, QtCore
import os

class pydaw_sample_graphs:
    def __init__(self, a_sg_dir):        
        self.lookup = {}
        self.sg_dir = a_sg_dir
    
    def __str__(self):
        f_result = ""
        for k, v in self.graphs.iteritems():
            f_result += str(k) + "|" + str(v) + "\n"
        f_result += "\\"
        return f_result
    
    def get_sample_graph(self, a_file_name):
        f_file_name = str(a_file_name)
        if self.lookup.has_key(f_file_name):
            f_pygraph_file = self.sg_dir + "/" + str(self.lookup[f_file_name]) + ".pygraph"
            f_result = pydaw_sample_graph(f_pygraph_file)
            #The below has a corner case of the user could be generating a graph for a huge file, and has
            #triggered this for a 2nd time but the graph isn't finished...  but sample graph generation is lightning quick on my PC, 
            #I need to test it on a slower PC with a mechanical hard drive...
            if not f_result.is_valid() or not f_result.check_mtime():
                self.lookup.pop(f_file_name)
                os.system('rm "' + f_pygraph_file + '"')
                return None
            else:
                return f_result
        else:
            return None
    
    def add_ref(self, a_file_name, a_uid):
        """ Add a reference to a .pygraph that is being created or has been created"""
        self.lookup[str(a_file_name)] = int(a_uid)
    
    @staticmethod
    def from_str(a_str, a_sg_dir):
        f_result = pydaw_sample_graphs()
        f_result.sg_dir = a_sg_dir
        f_arr = a_str.split("\n")
        for f_line in f_arr:
            if f_line == "\\":
                break
            f_line_arr = f_line.split("|")
            f_result.lookup[f_line_arr[0]] = int(f_line_arr[1])
        return f_result
    
class pydaw_sample_graph:
    def __init__(self, a_file_name):
        f_file_name = str(a_file_name)
        self.uid = None
        self.file = None
        self.timestamp = None
        self.channels = None
        self.high_peaks = ([],[])
        self.low_peaks = ([],[])
        self.count = None
        
        if not os.path.isfile(f_file_name):
            return
            
        try:
            f_file = open(f_file_name, "r")
        except:
            return
        
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
                    self.timestamp = int(f_line_arr[2])
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
            f_result.moveTo(0.0, 50.0)  #TODO:  Calculate where to move this to...
            for f_peak in self.high_peaks[f_i]:
                f_result.lineTo(f_width_pos, f_peak)  #TODO:  These will need to be multiplied by other values
                f_width_pos += f_width_inc
            for f_peak in self.low_peaks[f_i]:                
                f_result.lineTo(f_width_pos, f_peak)  #TODO:  These will need to be multiplied by other values
                f_width_pos -= f_width_inc
            f_result.closeSubpath()
        return pydaw_render_widget(f_result)
        
    def check_mtime(self):
        """ Returns False if the sample graph is older than the file modified time """
        f_file_mtime = int(os.path.getmtime(self.file))
        return self.timestamp > f_file_mtime

    
class pydaw_render_widget(QtGui.QWidget):
    def __init__(self, path, parent=None):
        super(pydaw_render_widget, self).__init__(parent)

        self.path = path

        self.penWidth = 1
        self.rotationAngle = 0
        self.setBackgroundRole(QtGui.QPalette.Base)

    def minimumSizeHint(self):
        return QtCore.QSize(50, 50)

    def sizeHint(self):
        return QtCore.QSize(100, 100)

    def setFillRule(self, rule):
        self.path.setFillRule(rule)
        self.update()

    def setFillGradient(self, color1, color2):
        self.fillColor1 = color1
        self.fillColor2 = color2
        self.update()

    def setPenWidth(self, width):
        self.penWidth = width
        self.update()

    def setPenColor(self, color):
        self.penColor = color
        self.update()

    def setRotationAngle(self, degrees):
        self.rotationAngle = degrees
        self.update()

    def paintEvent(self, event):
        painter = QtGui.QPainter(self)
        painter.setRenderHint(QtGui.QPainter.Antialiasing)
        painter.scale(self.width() / 100.0, self.height() / 100.0)
        painter.translate(50.0, 50.0)
        painter.rotate(-self.rotationAngle)
        painter.translate(-50.0, -50.0)

        painter.setPen(QtGui.QPen(self.penColor, self.penWidth,
                QtCore.Qt.SolidLine, QtCore.Qt.RoundCap, QtCore.Qt.RoundJoin))
        gradient = QtGui.QLinearGradient(0, 0, 0, 100)
        gradient.setColorAt(0.0, self.fillColor1)
        gradient.setColorAt(1.0, self.fillColor2)
        painter.setBrush(QtGui.QBrush(gradient))
        painter.drawPath(self.path)
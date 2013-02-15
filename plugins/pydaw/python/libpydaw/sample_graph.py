from PyQt4 import QtGui, QtCore
import os

class pydaw_sample_graphs:
    def __init__(self, a_sg_dir):
        self.lookup = {}
        self.sg_dir = a_sg_dir

    def __str__(self):
        f_result = ""
        for k, v in self.lookup.iteritems():
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
            if not f_result.is_valid() or f_result.check_mtime():
                print("Not valid, or else mtime is newer than graph time, deleting sample graph...")
                self.lookup.pop(f_file_name)
                os.system('rm "' + f_pygraph_file + '"')
                return None
            else:
                return f_result
        else:
            return None

    def add_ref(self, a_file_name, a_uid):
        """ Add a reference to a .pygraph that is being created or has been created"""
        f_file_name = str(a_file_name) #.strip([" ", "\n"])
        print("pydaw_sample_graphs.add_ref() : " + f_file_name)
        self.lookup[f_file_name] = int(a_uid)

    @staticmethod
    def from_str(a_str, a_sg_dir):
        f_result = pydaw_sample_graphs(a_sg_dir)
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
        self.file = None
        self.timestamp = None
        self.channels = None
        self.high_peaks = ([],[])
        self.low_peaks = ([],[])
        self.count = None
        self.length_in_seconds = None

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
                    self.file = str(f_line_arr[2])
                elif f_line_arr[1] == "timestamp":
                    self.timestamp = int(f_line_arr[2])
                elif f_line_arr[1] == "channels":
                    self.channels = int(f_line_arr[2])
                elif f_line_arr[1] == "count":
                    self.count = int(f_line_arr[2])
                elif f_line_arr[1] == "length":
                    self.length_in_seconds = float(f_line_arr[2])
            elif f_line_arr[0] == "p":
                f_p_val = float(f_line_arr[3])
                if f_p_val > 1.0:
                    f_p_val = 1.0
                elif f_p_val < -1.0:
                    f_p_val = -1.0
                if f_line_arr[2] == "h":
                    self.high_peaks[int(f_line_arr[1])].append(f_p_val)
                elif f_line_arr[2] == "l":
                    self.low_peaks[int(f_line_arr[1])].append(f_p_val)
                else:
                    print("Invalid sample_graph [2] value " + f_line_arr[2] )
        for f_list in self.low_peaks:
            f_list.reverse()

    def is_valid(self):
        f_result = (self.file is not None) and (self.timestamp is not None) \
        and (self.channels is not None) and (self.count is not None)
        if not f_result:
            print("pydaw_sample_graph.is_valid() : " + str(self.file) + " failed the validity check...")
        return f_result


    #BIG TODO:  Make path into a list, then pass it to pydaw_render widget and render multiple channels...
    def create_sample_graph(self):
        f_width_inc = 98.0 / self.count
        f_section = 100.0 / float(self.channels)
        f_section_div2 = f_section * 0.5

        f_paths = []

        for f_i in range(self.channels):
            f_result = QtGui.QPainterPath()
            f_width_pos = 1.0
            f_result.moveTo(f_width_pos, (f_section * f_i) + f_section_div2)
            for f_peak in self.high_peaks[f_i]:
                f_result.lineTo(f_width_pos, f_section_div2 - (f_peak * f_section_div2) + (f_section * f_i))
                f_width_pos += f_width_inc
            for f_peak in self.low_peaks[f_i]:
                f_result.lineTo(f_width_pos, (f_peak * -1.0 * f_section_div2) + f_section_div2 + (f_section * f_i))
                f_width_pos -= f_width_inc
            f_result.closeSubpath()
            f_paths.append(f_result)
        return pydaw_render_widget(f_paths)

    def check_mtime(self):
        """ Returns False if the sample graph is older than the file modified time """
        try:
            f_test = os.stat(self.file)
            f_result = self.timestamp > int(f_test.st_mtime)
            return f_result
        except:
            print("Error getting mtime")
            return False


class pydaw_render_widget(QtGui.QWidget):
    def __init__(self, paths, parent=None):
        super(pydaw_render_widget, self).__init__(parent)

        self.paths = paths
        self.setPenColor(QtGui.QColor(QtCore.Qt.lightGray))
        self.setFillGradient(QtGui.QColor.fromRgb(190, 192, 123), QtGui.QColor.fromRgb(132, 132, 93))
        self.penWidth = 1
        self.rotationAngle = 0
        self.setBackgroundRole(QtGui.QPalette.Base)
        self.setPenWidth(0.2)
        self.setStyleSheet("background-color:black;")

    def minimumSizeHint(self):
        return QtCore.QSize(200, 100)

    def sizeHint(self):
        return QtCore.QSize(800, 300)

    def setFillRule(self, rule):
        for path in self.paths:
            path.setFillRule(rule)
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
        gradient = QtGui.QLinearGradient(20, 20, 80, 80)
        gradient.setColorAt(0.0, self.fillColor1)
        gradient.setColorAt(1.0, self.fillColor2)
        painter.setBrush(QtGui.QBrush(gradient))
        for path in self.paths:
            painter.drawPath(path)

if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    f_graph = pydaw_sample_graph("test.pygraph")
    f_test = f_graph.create_sample_graph()
    f_test.show()
    sys.exit(app.exec_())
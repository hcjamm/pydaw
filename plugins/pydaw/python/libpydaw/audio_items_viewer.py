"""
A viewer for all audio items in the song.  This will eventually feature
editing capabilities as well.
"""

import sys
from PyQt4 import QtGui, QtCore

class aiv_audio_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_x, a_y, a_w, a_h, a_color1, a_color2):
        QtGui.QGraphicsRectItem.__init__(self, a_x, a_y, a_w, a_h)
        self.gradient = QtGui.QLinearGradient(a_x, a_y, a_w, a_y)
        self.gradient.setColorAt(0, a_color1)
        self.gradient.setColorAt(1, a_color2)
        self.setBrush(self.gradient)


class aiv_audio_items_viewer(QtGui.QGraphicsView):
    def __init__(self):
        QtGui.QGraphicsView.__init__(self)

    def draw_items(self, a_file):
        self.scene = QtGui.QGraphicsScene(self)
        self.setScene(self.scene)

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    view = aiv_audio_items_viewer()
    view.draw_items("testing123.txt")
    f_item = aiv_audio_item(0, 0, 100, 100, QtCore.Qt.blue, QtCore.Qt.yellow)
    view.scene.addItem(f_item)
    view.show()
    sys.exit(app.exec_())
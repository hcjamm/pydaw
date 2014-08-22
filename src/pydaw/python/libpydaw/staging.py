# -*- coding: utf-8 -*-
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""


from PyQt4 import QtGui, QtCore

class wire_input_port(QtGui.QGraphicsRectItem):
    def __init__(self):
        pass

class wire_output_port(QtGui.QGraphicsRectItem):
    def __init__(self):
        pass

class wire_block(QtGui.QGraphicsRectItem):
    """ Over-ride to add extra controls to the middle-area """
    def __init__(self, a_in_count, a_out_count, a_brush, a_drag=False):
        QtGui.QGraphicsRectItem.__init__(self)

class wire_canvas:
    def __init__(self):
        self.view = QtGui.QGraphicsView()
        self.scene = QtGui.QGraphicsScene()
        self.scene.setBackgroundBrush(QtCore.Qt.darkGray)
        self.view.setScene(self.scene)

    def draw_item(self, a_item):
        """ Over-ride this and use it to draw the various
            blocks and wires
        """
        raise NotImplementedError()

ROUTING_PROTOTYPE_STRUCT = {
    0:{ # level
        1:{ # track
            0:[ #channel
                (0, 0)], # output track, channel
            1:[ #channel
                (0, 1)] # output track, channel
          }
      },
   1:{ # level
       0:{}  # master track, no outs
   }
}

if __name__ == "__main__":

    import sys
    app = QtGui.QApplication(sys.argv)
    f_widget = wire_canvas()
    f_widget.view.show()

    #with open("/usr/lib/pydaw4/themes/default/default.pytheme") as f_file:
    #    f_widget.widget.setStyleSheet(f_file.read().replace("$STYLE_FOLDER",
    #                                  "/usr/lib/pydaw4/themes/default"))
    sys.exit(app.exec_())


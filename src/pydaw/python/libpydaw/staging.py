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

import numpy
from PyQt4 import QtGui, QtCore


class pydaw_abstract_custom_oscillator:
    def __init__(self):
        self.widget = QtGui.QWidget()
        self.layout = QtGui.QVBoxLayout(self.widget)
        self.wavetable_size = 2000
        self.array = numpy.zeros((0, self.wavetable_size))
        self.is_closing = False

    def get_wavetable(self):
        pass

    def open_settings(self, a_settings):
        pass

    def get_settings(self):
        pass



global_additive_osc_height = 310
global_additive_osc_inc = 10
global_additive_max_y_pos = global_additive_osc_height - global_additive_osc_inc
global_additive_osc_harmonic_count = 36
global_additive_osc_width = 720
global_additive_osc_bar_width = 20
#global_additive_osc_height_div2 = global_additive_osc_height * 0.5


class pydaw_additive_osc_amp_bar(QtGui.QGraphicsRectItem):
    def __init__(self, a_x_pos):
        QtGui.QGraphicsRectItem.__init__(self)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setBrush(QtCore.Qt.yellow)
        self.setPen(QtGui.QPen(QtCore.Qt.black, 2.0))
        self.x_pos = a_x_pos
        self.setPos(a_x_pos, global_additive_osc_height - global_additive_osc_inc)
        self.setRect(0.0, 0.0, global_additive_osc_bar_width, global_additive_osc_inc)
        self.value = -30
        self.extend_to_bottom()

    def set_value(self, a_value):
        self.value = a_value

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        self.extend_to_bottom()

    def extend_to_bottom(self):
        f_pos_y = self.pos().y() #TODO: clip
        f_pos_y = round(f_pos_y, -1)
        if f_pos_y < 10:
            f_pos_y = 10
        elif f_pos_y > global_additive_max_y_pos:
            f_pos_y = global_additive_max_y_pos
        self.setPos(self.x_pos, f_pos_y)
        self.setRect(0.0, 0.0, global_additive_osc_bar_width,
                     global_additive_osc_height - f_pos_y - 2.0)


class pydaw_additive_osc_viewer(QtGui.QGraphicsView):
    def __init__(self, a_parent):
        QtGui.QGraphicsView.__init__(self)
        self.last_x_scale = 1.0
        self.last_y_scale = 1.0
        self.setMinimumSize(global_additive_osc_width, global_additive_osc_height)
        self.parent_widget = a_parent
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.scene = QtGui.QGraphicsScene()
        self.setScene(self.scene)
        self.scene.setBackgroundBrush(QtCore.Qt.darkGray)
        self.setSceneRect(0.0, 0.0, global_additive_osc_width, global_additive_osc_height)
        self.bars = []
        for f_i in range(0, global_additive_osc_width, int(global_additive_osc_bar_width)):
            f_bar = pydaw_additive_osc_amp_bar(f_i)
            self.bars.append(f_bar)
            self.scene.addItem(f_bar)

    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / global_additive_osc_width
        self.last_y_scale = f_rect.height() / global_additive_osc_height
        self.scale(self.last_x_scale, self.last_y_scale)

class pydaw_custom_additive_oscillator(pydaw_abstract_custom_oscillator):
    def __init__(self):
        pydaw_abstract_custom_oscillator.__init__(self)
        self.viewer = pydaw_additive_osc_viewer(self)
        self.layout.addWidget(self.viewer)

    def get_wavetable(self):
        pass

    def open_settings(self, a_settings):
        pass

    def get_settings(self):
        pass

if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)

    f_widget = pydaw_custom_additive_oscillator()
    f_widget.widget.show()

    sys.exit(app.exec_())


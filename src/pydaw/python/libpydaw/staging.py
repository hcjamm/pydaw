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



import numpy

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


global_add_osc_fill = QtGui.QLinearGradient(0.0, 0.0, 0.0, global_additive_osc_height)

global_add_osc_fill.setColorAt(0.0, QtGui.QColor(255, 0, 0, 90)) #red
global_add_osc_fill.setColorAt(0.14285, QtGui.QColor(255, 123, 0, 90)) #orange
global_add_osc_fill.setColorAt(0.2857, QtGui.QColor(255, 255, 0, 90)) #yellow
global_add_osc_fill.setColorAt(0.42855, QtGui.QColor(0, 255, 0, 90)) #green
global_add_osc_fill.setColorAt(0.5714, QtGui.QColor(0, 123, 255, 90)) #blue
global_add_osc_fill.setColorAt(0.71425, QtGui.QColor(0, 0, 255, 90)) #indigo
global_add_osc_fill.setColorAt(0.8571, QtGui.QColor(255, 0, 255, 90)) #violet

global_add_osc_background = QtGui.QLinearGradient(0.0, 0.0, 10.0, global_additive_osc_height)
global_add_osc_background.setColorAt(0.0, QtGui.QColor(40, 40, 40))
global_add_osc_background.setColorAt(0.2, QtGui.QColor(20, 20, 20))
global_add_osc_background.setColorAt(0.7, QtGui.QColor(30, 30, 30))
global_add_osc_background.setColorAt(1.0, QtGui.QColor(40, 40, 40))

class pydaw_additive_osc_amp_bar(QtGui.QGraphicsRectItem):
    def __init__(self, a_x_pos):
        QtGui.QGraphicsRectItem.__init__(self)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setBrush(global_add_osc_fill)
        self.setPen(QtGui.QPen(QtCore.Qt.white, 2.0))
        self.x_pos = a_x_pos
        self.setPos(a_x_pos, global_additive_osc_height - global_additive_osc_inc)
        self.setRect(0.0, 0.0, global_additive_osc_bar_width, global_additive_osc_inc)
        self.value = -30
        self.extend_to_bottom()

    def set_value(self, a_value):
        self.value = a_value
        f_y_pos = (a_value * global_additive_osc_inc * -1.0) # + global_additive_osc_inc
        self.setPos(self.x_pos, f_y_pos)
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
        self.is_drawing = False
        self.setMinimumSize(global_additive_osc_width, global_additive_osc_height)
        self.parent_widget = a_parent
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.scene = QtGui.QGraphicsScene()
        self.setScene(self.scene)
        self.scene.mousePressEvent = self.scene_mousePressEvent
        self.scene.mouseReleaseEvent = self.scene_mouseReleaseEvent
        self.scene.mouseMoveEvent = self.scene_mouseMoveEvent
        self.scene.setBackgroundBrush(global_add_osc_background)
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

    def scene_mousePressEvent(self, a_event):
        QtGui.QGraphicsScene.mousePressEvent(self.scene, a_event)
        self.is_drawing = True

    def scene_mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsScene.mouseReleaseEvent(self.scene, a_event)
        self.is_drawing = False

    def scene_mouseMoveEvent(self, a_event):
        if self.is_drawing:
            QtGui.QGraphicsScene.mouseMoveEvent(self.scene, a_event)
            f_pos = a_event.scenePos()
            f_pos_x = f_pos.x()
            f_pos_y = f_pos.y()
            f_db = (f_pos_y / global_additive_osc_height) * -30.0
            f_harmonic = (f_pos_x / global_additive_osc_width) * \
                global_additive_osc_harmonic_count
            if f_harmonic < 0:
                f_harmonic = 0
            elif f_harmonic >= global_additive_osc_harmonic_count:
                f_harmonic = global_additive_osc_harmonic_count - 1
            if f_db > 0:
                f_db = 0
            elif f_db < -30:
                f_db = -30
            self.bars[int(f_harmonic)].set_value(f_db)
            #print("{} {} {} {}".format(f_pos_x, f_pos_y, f_db, f_harmonic))


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


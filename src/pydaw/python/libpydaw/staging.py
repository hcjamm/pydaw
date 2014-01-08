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


def pydaw_db_to_lin(a_value):
    return pow(10.0, (0.05 * a_value))


import numpy



class pydaw_spectrum_analyzer_bar(QtGui.QGraphicsRectItem):
    def __init__(self, a_x_pos):
        QtGui.QGraphicsRectItem.__init__(self)
        #self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setBrush(global_spec_anlzr_fill)
        self.setPen(QtGui.QPen(QtCore.Qt.white))
        self.x_pos = a_x_pos
        self.setPos(a_x_pos, global_spec_anlzr_height - global_spec_anlzr_inc)
        self.setRect(0.0, 0.0, global_spec_anlzr_bar_width, global_spec_anlzr_inc)
        self.value = 0.0
        self.extend_to_bottom()

    def set_value(self, a_value):
        if self.value != a_value:
            self.value = a_value
            f_y_pos = (a_value * global_spec_anlzr_height) - global_spec_anlzr_height
            self.setPos(self.x_pos, f_y_pos)
            self.extend_to_bottom()
            return True
        else:
            return False

    def extend_to_bottom(self):
        f_pos_y = self.pos().y() #TODO: clip
        f_pos_y = round(f_pos_y, -1)
        if f_pos_y < 10:
            f_pos_y = 10
        elif f_pos_y > global_spec_anlzr_max_y_pos:
            f_pos_y = global_spec_anlzr_max_y_pos
        self.setPos(self.x_pos, f_pos_y)
        self.setRect(0.0, 0.0, global_spec_anlzr_bar_width,
                     global_spec_anlzr_height - f_pos_y - 1.0)


global_spec_anlzr_height = 310
global_spec_anlzr_inc = 10
global_spec_anlzr_max_y_pos = global_spec_anlzr_height - global_spec_anlzr_inc
global_spec_anlzr_harmonic_count = 256
global_spec_anlzr_bar_width = 4
global_spec_anlzr_width = global_spec_anlzr_harmonic_count * global_spec_anlzr_bar_width
global_spec_anlzr_sample_size = 4096 #pow(2, global_spec_anlzr_harmonic_count)
#global_spec_anlzr_height_div2 = global_spec_anlzr_height * 0.5


global_spec_anlzr_fill = QtGui.QLinearGradient(0.0, 0.0, 0.0, global_spec_anlzr_height)

global_spec_anlzr_fill.setColorAt(0.0, QtGui.QColor(255, 0, 0, 90)) #red
global_spec_anlzr_fill.setColorAt(0.14285, QtGui.QColor(255, 123, 0, 90)) #orange
global_spec_anlzr_fill.setColorAt(0.2857, QtGui.QColor(255, 255, 0, 90)) #yellow
global_spec_anlzr_fill.setColorAt(0.42855, QtGui.QColor(0, 255, 0, 90)) #green
global_spec_anlzr_fill.setColorAt(0.5714, QtGui.QColor(0, 123, 255, 90)) #blue
global_spec_anlzr_fill.setColorAt(0.71425, QtGui.QColor(0, 0, 255, 90)) #indigo
global_spec_anlzr_fill.setColorAt(0.8571, QtGui.QColor(255, 0, 255, 90)) #violet

global_spec_anlzr_background = QtGui.QLinearGradient(0.0, 0.0, 10.0, global_spec_anlzr_height)
global_spec_anlzr_background.setColorAt(0.0, QtGui.QColor(40, 40, 40))
global_spec_anlzr_background.setColorAt(0.2, QtGui.QColor(20, 20, 20))
global_spec_anlzr_background.setColorAt(0.7, QtGui.QColor(30, 30, 30))
global_spec_anlzr_background.setColorAt(1.0, QtGui.QColor(40, 40, 40))


class pydaw_oscilloscope(QtGui.QGraphicsView):
    def __init__(self):
        QtGui.QGraphicsView.__init__(self)
        self.last_x_scale = 1.0
        self.last_y_scale = 1.0
        self.scene = QtGui.QGraphicsScene()
        self.setScene(self.scene)
        self.scene.setBackgroundBrush(global_spec_anlzr_background)
        self.setSceneRect(0.0, 0.0, global_spec_anlzr_sample_size,
                          global_spec_anlzr_height)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)

    def draw_array(self, a_np_array):
        f_path = QtGui.QPainterPath(QtCore.QPointF(0.0, global_spec_anlzr_height * 0.5))
        f_x = 1.0
        f_half = global_spec_anlzr_height * 0.5
        for f_point in a_np_array:
            f_path.lineTo(f_x, (f_point * f_half) + f_half)
            f_x += 1.0
        self.scene.clear()
        f_path_item = self.scene.addPath(f_path, QtGui.QPen(QtCore.Qt.white, 1.0))
        f_path_item.setBrush(global_spec_anlzr_fill)

    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / global_spec_anlzr_sample_size
        self.last_y_scale = f_rect.height() / global_spec_anlzr_height
        self.scale(self.last_x_scale, self.last_y_scale)


class pydaw_spectrum_analyzer(QtGui.QGraphicsView):
    def __init__(self):
        QtGui.QGraphicsView.__init__(self)
        self.last_x_scale = 1.0
        self.last_y_scale = 1.0
        self.is_drawing = False
        self.edit_mode = 0
        self.setMinimumSize(global_spec_anlzr_width, global_spec_anlzr_height)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.scene = QtGui.QGraphicsScene()
        self.setScene(self.scene)
        self.scene.setBackgroundBrush(global_spec_anlzr_background)
        self.setSceneRect(0.0, 0.0, global_spec_anlzr_width, global_spec_anlzr_height)
        self.bars = []
        for f_i in range(0, global_spec_anlzr_width, int(global_spec_anlzr_bar_width)):
            f_bar = pydaw_spectrum_analyzer_bar(f_i)
            self.bars.append(f_bar)
            self.scene.addItem(f_bar)

    def set_values(self, a_arr):
        ps = numpy.abs(numpy.fft.fft(a_arr)) # **2
        time_step = 1 / 44100
        freqs = numpy.fft.fftfreq(a_arr.size, time_step)
        idx = numpy.argsort(freqs)
        #plt.plot(freqs[idx], ps[idx])
        for f_i in range(len(self.bars)):
            self.bars[f_i].set_value(ps[idx][f_i])

    def set_edit_mode(self, a_mode):
        self.edit_mode = a_mode

    def resizeEvent(self, a_resize_event):
        QtGui.QGraphicsView.resizeEvent(self, a_resize_event)
        self.scale(1.0 / self.last_x_scale, 1.0 / self.last_y_scale)
        f_rect = self.rect()
        self.last_x_scale = f_rect.width() / global_spec_anlzr_width
        self.last_y_scale = f_rect.height() / global_spec_anlzr_height
        self.scale(self.last_x_scale, self.last_y_scale)



if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    f_widget = pydaw_spectrum_analyzer()
    f_widget.show()
    def time_out():
        f_rand = numpy.random.rand(256)
        f_rand -= 0.5
        f_rand *= 0.2
        #f_rand = numpy.linspace(0.0, 32.0 * numpy.pi, 256)
        #f_rand = numpy.sin(f_rand)
        f_widget.set_values(f_rand)

    f_timer = QtCore.QTimer(f_widget)
    f_timer.timeout.connect(time_out)
    f_timer.start(50)
    #with open("/usr/lib/pydaw4/themes/default/default.pytheme") as f_file:
    #    f_widget.widget.setStyleSheet(f_file.read().replace("$STYLE_FOLDER",
    #                                  "/usr/lib/pydaw4/themes/default"))
    sys.exit(app.exec_())


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
from math import log

def pydaw_pitch_to_hz(a_pitch):
    return (440.0 * pow(2.0,(a_pitch - 57.0) * 0.0833333))

def pydaw_hz_to_pitch(a_hz):
    return ((12.0 * log(a_hz * (1.0/440.0), 2.0)) + 57.0)

def pydaw_pitch_to_ratio(a_pitch):
    return (1.0/pydaw_pitch_to_hz(0.0)) * pydaw_pitch_to_hz(a_pitch)

def pydaw_db_to_lin(a_value):
    return pow(10.0, (0.05 * a_value))

def pydaw_lin_to_db(a_value):
    return log(a_value, 10.0) * 20.0



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
            f_y_pos = ((1.0 - a_value) * global_spec_anlzr_height)
            self.setPos(self.x_pos, f_y_pos)
            self.extend_to_bottom()

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
global_spec_anlzr_harmonic_count = 32
global_spec_anlzr_bar_width = 20
global_spec_anlzr_width = global_spec_anlzr_harmonic_count * global_spec_anlzr_bar_width
global_spec_anlzr_sample_size = 4096 #pow(2, global_spec_anlzr_harmonic_count)
#global_spec_anlzr_height_div2 = global_spec_anlzr_height * 0.5

global_spec_anlzr_freqs = []

def global_spec_anlzr_create_freq_table():
    f_spec_anlzr_pitch_inc = \
        (pydaw_hz_to_pitch(20000.0) - pydaw_hz_to_pitch(10.0)) / global_spec_anlzr_harmonic_count
    print("f_spec_anlzr_pitch_inc {}".format(f_spec_anlzr_pitch_inc))
    for f_i in range(global_spec_anlzr_harmonic_count):
        global_spec_anlzr_freqs.append(pydaw_pitch_to_hz(f_i * f_spec_anlzr_pitch_inc))

global_spec_anlzr_create_freq_table()

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
    def __init__(self, a_sample_rate=44100, a_buffer_size=128):
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
        for f_i in range(global_spec_anlzr_harmonic_count):
            f_bar = pydaw_spectrum_analyzer_bar(f_i * global_spec_anlzr_bar_width)
            self.bars.append(f_bar)
            self.scene.addItem(f_bar)
        self.buffer_size = a_buffer_size
        self.time_step = 1.0 / a_sample_rate
        self.freqs = numpy.fft.fftfreq(a_buffer_size // 2, self.time_step)
        self.freqs = self.freqs[a_buffer_size // 2::-1]
        self.idx = numpy.argsort(self.freqs)
#        self.ranges = []
#        for f_i in range(1, global_spec_anlzr_harmonic_count):
#            f_list = []
#            for f_i2 in range(0, len(self.freqs)):
#                if self.freqs[self.idx][f_i2] < global_spec_anlzr_freqs[f_i] and \
#                self.freqs[self.idx][f_i2] >= global_spec_anlzr_freqs[f_i - 1]:
#                    f_list.append(f_i2)
#            f_list.sort()
#            if f_list:
#                self.ranges.append((f_list[0], f_list[-1]))

    def set_values(self, a_arr):
        ps = numpy.abs(numpy.fft.fft(a_arr)) * 0.1 # ** 2
        ps = ps[ps.shape[0] // 2::-1]
        ps = ps[self.idx]
        for f_i in range(global_spec_anlzr_harmonic_count):
            self.bars[f_i].set_value(ps[f_i])
            #self.bars[f_i].set_value(ps[self.idx][self.ranges[f_i][0]:self.ranges[1]])

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
        #f_rand = (numpy.random.rand(64) - 0.5) * 0.1
        f_rand = numpy.linspace(0.0, 21.0 * numpy.pi, 64)
        f_rand = numpy.sin(f_rand) # * 0.1
        f_widget.set_values(f_rand)

    f_timer = QtCore.QTimer(f_widget)
    f_timer.timeout.connect(time_out)
    f_timer.start(100)
    #with open("/usr/lib/pydaw4/themes/default/default.pytheme") as f_file:
    #    f_widget.widget.setStyleSheet(f_file.read().replace("$STYLE_FOLDER",
    #                                  "/usr/lib/pydaw4/themes/default"))
    sys.exit(app.exec_())


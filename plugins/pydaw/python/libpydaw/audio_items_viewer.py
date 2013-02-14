"""
A viewer for all audio items in the song.  This will eventually feature
editing capabilities as well.
"""

import sys
from PyQt4 import QtGui, QtCore
from pydaw_gradients import *

class audio_viewer_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_length, a_height):
        QtGui.QGraphicsRectItem.__init__(0, 0, a_length, a_height)

    def set_length_regions(self):
        pass

    def set_length_seconds(self):
        pass

class audio_items_viewer(QtGui.QGraphicsView):
    def __init__(self, a_item_length=4, a_region_length=8, a_bpm=140, a_px_per_region=200):
        self.item_length = float(a_item_length)
        self.region_length = float(a_region_length)
        self.bps = a_bpm / 60.0
        self.px_per_region = a_px_per_region
        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(90,90,90))
        self.setScene(self.scene)
        self.track = 0
        #make ruler at top
        self.ruler_height = 20
        self.draw_headers()


    def f_seconds_to_regions(self, a_track_seconds):
        '''converts seconds to regions'''
        f_beats_per_region = self.item_length * self.region_length
        f_regions_per_second = self.bps * (1.0/f_beats_per_region)
        f_track_in_regions = a_track_seconds * f_regions_per_second
        return f_track_in_regions

    def draw_headers(self):
        f_total_regions = 300
        f_size = self.px_per_region * f_total_regions
        f_ruler = QtGui.QGraphicsRectItem(0, 0, f_size, self.ruler_height)
        self.scene.addItem(f_ruler)
        for i in range(0, f_total_regions):
            #f_tick = QtGui.QGraphicsLineItem(self.px_per_region*i, 0, self.px_per_region*i, self.ruler_height, f_ruler)
            f_number = QtGui.QGraphicsSimpleTextItem("%d" % i, f_ruler)
            f_number.setPos(45+self.px_per_region*(i), 2)
            f_number.setBrush(QtCore.Qt.white)

    def draw_item_seconds(self, a_start_region, a_start_bar, a_start_beat, a_seconds):
        pass

    def draw_item_musical_time(self, a_start_region, a_start_bar, a_start_beat, a_end_region, a_end_bar, a_end_beat):
        pass

    def clear_drawn_items(self):
        self.track = 0
        self.scene.clear()
        self.draw_headers()

    def v_draw_item(self, a_start, a_length, a_name):
        '''a_start in seconds, a_length in seconds'''
        f_start = self.f_seconds_to_regions(a_start) * self.px_per_region
        f_length = self.f_seconds_to_regions(a_length) * self.px_per_region
        f_height = 65
        f_padding = 2
        f_track_num = self.ruler_height + f_padding + (f_height + f_padding) * self.track
        f_audio_item = QtGui.QGraphicsRectItem(0, 0, f_length, f_height)
        f_audio_item.setPos(f_start,f_track_num)
        f_audio_item.setBrush(pydaw_track_gradients[self.track % len(pydaw_track_gradients)])
        f_label = QtGui.QGraphicsSimpleTextItem(a_name, parent=f_audio_item)
        f_label.setPos(5, 5)
        f_label.setBrush(QtCore.Qt.white)
        self.scene.addItem(f_audio_item)
        self.track += 1


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    view = audio_items_viewer()
    view.v_draw_item(0, 60, "item0")
    view.v_draw_item(100, 40, "item1")
    view.v_draw_item(14, 20, "item2")
    view.v_draw_item(5, 100, "item3")
    view.v_draw_item(30, 40, "item4")
    view.v_draw_item(20, 10, "item5")
    view.show()
    sys.exit(app.exec_())

"""
A viewer for all audio items in the song.  This will eventually feature
editing capabilities as well.
"""

import sys
from PyQt4 import QtGui, QtCore
from pydaw_gradients import *

class audio_viewer_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_length, a_height, a_name, a_track_num, a_y_pos):
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, a_length, a_height)
        self.label = QtGui.QGraphicsSimpleTextItem(a_name, parent=self)
        self.label.setPos(10, 5)
        self.label.setBrush(QtCore.Qt.white)
        #These 2 will allow moving the item, but mouseMoveEvent and others will need to be overridden
        #in order for the items to be snapped into their grid positions...
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.track_num = a_track_num
        self.mouse_y_pos = a_y_pos

    #def mousePressEvent(self, a_event):
    #    QtGui.QGraphicsRectItem.mousePressEvent(self, a_event)

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        f_pos = self.pos().x()
        if f_pos < 0:
            f_pos = 0
        self.setPos(f_pos, self.mouse_y_pos)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        #...and here we would call functionality to change the start/end point...

class audio_items_viewer(QtGui.QGraphicsView):
    def __init__(self, a_item_length=4, a_region_length=8, a_bpm=140.0, a_px_per_region=200):
        self.item_length = float(a_item_length)
        self.region_length = float(a_region_length)
        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(90,90,90))
        self.setScene(self.scene)
        self.track = 0
        self.set_bpm(a_bpm)
        self.ruler_height = 20
        self.set_zoom(a_px_per_region)
        self.draw_headers()

    def set_zoom(self, a_px_per_region):
        """ You must clear and re-draw the items for this to take effect  """
        self.px_per_region = a_px_per_region

    def set_bpm(self, a_bpm):
        self.bps = a_bpm / 60.0
        self.beats_per_region = self.item_length * self.region_length
        self.regions_per_second = self.bps / self.beats_per_region

    def f_seconds_to_regions(self, a_track_seconds):
        '''converts seconds to regions'''
        return a_track_seconds * self.regions_per_second

    def draw_headers(self):
        f_total_regions = 300
        f_size = self.px_per_region * f_total_regions
        f_ruler = QtGui.QGraphicsRectItem(0, 0, f_size, self.ruler_height)
        self.scene.addItem(f_ruler)
        for i in range(0, f_total_regions):
            #f_tick = QtGui.QGraphicsLineItem(self.px_per_region*i, 0, self.px_per_region*i, self.ruler_height, f_ruler)
            f_number = QtGui.QGraphicsSimpleTextItem("%d" % i, f_ruler)
            f_number.setPos(self.px_per_region*(i), 2)
            f_number.setBrush(QtCore.Qt.white)

    def draw_item_seconds(self, a_start_region, a_start_bar, a_start_beat, a_seconds, a_name, a_track_num):
        f_start = (a_start_region + (((a_start_bar * self.item_length) + a_start_beat) / self.beats_per_region)) * self.px_per_region
        f_length = self.f_seconds_to_regions(a_seconds) * self.px_per_region
        self.draw_item(f_start, f_length, a_name, a_track_num)

    def draw_item_musical_time(self, a_start_region, a_start_bar, a_start_beat, a_end_region, a_end_bar, a_end_beat, a_seconds, a_name, a_track_num):
        f_start = (a_start_region + (((a_start_bar * self.item_length) + a_start_beat) / self.beats_per_region)) * self.px_per_region
        f_length = ((a_end_region + (((a_end_bar * self.item_length) + a_end_beat) / self.beats_per_region))  * self.px_per_region) - f_start
        f_length_seconds = self.f_seconds_to_regions(a_seconds) * self.px_per_region
        if f_length_seconds < f_length:
            f_length = f_length_seconds
        self.draw_item(f_start, f_length, a_name, a_track_num)

    def clear_drawn_items(self):
        self.track = 0
        self.scene.clear()
        self.draw_headers()

    def draw_item(self, a_start, a_length, a_name, a_track_num):
        '''a_start in seconds, a_length in seconds'''
        f_height = 65
        f_padding = 2
        f_track_num = self.ruler_height + f_padding + (f_height + f_padding) * self.track
        f_audio_item = audio_viewer_item(a_length, f_height, a_name, a_track_num, f_track_num)
        f_audio_item.setPos(a_start, f_track_num)
        f_audio_item.setBrush(pydaw_track_gradients[self.track % len(pydaw_track_gradients)])
        self.scene.addItem(f_audio_item)
        self.track += 1


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    view = audio_items_viewer()
    for i in range(32):
        view.draw_item_musical_time(0, 0, 0, i + 1, 0, 0, 120, "Item-" + str(i), i)
        #view.draw_item_seconds(0, 0, 0, (i + 1) * 10.0, "Item-" + str(i), i)
    view.show()
    sys.exit(app.exec_())

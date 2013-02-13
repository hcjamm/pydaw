"""
A viewer for all audio items in the song.  This will eventually feature
editing capabilities as well.
"""

import sys
from PyQt4 import QtGui, QtCore
from pydaw_gradients import *

class audio_items_viewer(QtGui.QGraphicsView):
    def __init__(self, a_item_length, a_region_length, a_bpm, a_px_per_region):
        self.item_length = float(a_item_length)
        self.region_length = float(a_region_length)
        self.bps = a_bpm / 60.0
        self.px_per_region = a_px_per_region
        self.track = 0
        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(90,90,90))
        self.setScene(self.scene)
        
        #make ruler at top
        self.ruler_height = 20
        f_total_regions = 20
        f_size = self.px_per_region * f_total_regions
        f_ruler = QtGui.QGraphicsRectItem(0, 0, f_size, self.ruler_height)
        self.scene.addItem(f_ruler)
        for i in range(1, f_total_regions+1):
            f_tick = QtGui.QGraphicsLineItem(self.px_per_region*i, 0, self.px_per_region*i, self.ruler_height, f_ruler)
            f_number = QtGui.QGraphicsSimpleTextItem("%d" % i, f_ruler)
            f_number.setPos(45+100*(i-1), 2)
            f_number.setBrush(QtCore.Qt.white)
    
    def f_seconds_to_regions(self, a_track_seconds):
        '''converts seconds to regions'''
        f_beats_per_region = self.item_length * self.region_length
        f_regions_per_second = self.bps * (1.0/f_beats_per_region)
        f_track_in_regions = a_track_seconds * f_regions_per_second
        return f_track_in_regions

    def v_draw_item(self, a_start, a_length, a_color, a_name):
        '''a_start in seconds, a_length in seconds'''
        f_start = self.f_seconds_to_regions(a_start) * self.px_per_region
        #f_start = round(f_start)
        f_length = self.f_seconds_to_regions(a_length) * self.px_per_region 
        #f_length = round(f_length)
        f_height = 65
        f_padding = 5
        f_track_num = self.ruler_height + f_padding + (f_height + f_padding) * self.track 
        f_audio_item = QtGui.QGraphicsRectItem(0, 0, f_length, f_height)
        f_audio_item.setPos(f_start,f_track_num)
        f_audio_item.setBrush(a_color)
        f_label = QtGui.QGraphicsSimpleTextItem(a_name, parent=f_audio_item)
        f_label.setPos(5, 5)
        f_label.setBrush(QtCore.Qt.white)
        self.scene.addItem(f_audio_item)
        self.track += 1


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    view = audio_items_viewer(4,8,140,100)
    view.v_draw_item(0,60,pydaw_track_gradients[0],"item0")
    view.v_draw_item(100,40,pydaw_track_gradients[1],"item1")
    view.v_draw_item(14,20,pydaw_track_gradients[2],"item2")
    view.v_draw_item(5,100,pydaw_track_gradients[3],"item3")
    view.v_draw_item(30,40,pydaw_track_gradients[4],"item4")
    view.v_draw_item(20,10,pydaw_track_gradients[5],"item5")
    view.show()
    sys.exit(app.exec_())

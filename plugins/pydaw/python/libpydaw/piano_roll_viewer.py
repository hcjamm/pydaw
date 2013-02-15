"""
A piano roll viewer that will eventually become a piano roll editor
"""

import sys
from PyQt4 import QtGui, QtCore
from pydaw_gradients import *

global_piano_roll_snap = True
global_piano_roll_snap_value = 1.0
global_piano_keys_width = 34  #Width of the piano keys in px
global_piano_roll_header_height = 20
global_piano_roll_pixels_per_beat = 100000 #TODO:  the real value

pydaw_note_gradient = QtGui.QLinearGradient(QtCore.QPointF(0, 0), QtCore.QPointF(0, 12))  # I might have the 10 and 0 switched up, LOL...
#pydaw_note_gradient.setColorAt(0, QtGui.QColor(180, 150, 33))
pydaw_note_gradient.setColorAt(0, QtGui.QColor(163, 136, 30))
pydaw_note_gradient.setColorAt(1, QtGui.QColor(230, 221, 45))

class note_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_length, a_note_height):
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, a_length, a_note_height)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setBrush(pydaw_note_gradient)
        self.note_height = a_note_height
        self.setToolTip("Double-click to edit note properties, click and drag to move")

    def mousePressEvent(self, a_event):
        QtGui.QGraphicsRectItem.mousePressEvent(self, a_event)
        self.o_brush = self.brush()
        self.setBrush(QtGui.QColor(255,200,100))
        self.o_pos = self.pos()

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseMoveEvent(self, a_event)
        f_pos_x = self.pos().x()
        f_pos_y = self.pos().y()
        if f_pos_x < global_piano_keys_width:
            f_pos_x = global_piano_keys_width
        if f_pos_y < global_piano_roll_header_height:
            f_pos_y = global_piano_roll_header_height
        f_pos_y = int((f_pos_y - self.o_pos.y())/self.note_height) * self.note_height
        if global_piano_roll_snap:
            f_pos_x = int((f_pos_x - self.o_pos.x())/global_piano_roll_snap_value) * global_piano_roll_snap_value
        else:
            f_pos_x -= self.o_pos.x()
        self.setPos(self.o_pos.x()+f_pos_x, self.o_pos.y()+f_pos_y)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsRectItem.mouseReleaseEvent(self, a_event)
        self.setBrush(self.o_brush)
        f_pos_x = self.pos().x()
        f_pos_y = self.pos().y()
        self.setPos(f_pos_x, f_pos_y)

class piano_key_item(QtGui.QGraphicsRectItem):
    def __init__(self, a_piano_width, a_note_height, a_parent):
        QtGui.QGraphicsRectItem.__init__(self, 0, 0, a_piano_width, a_note_height, a_parent)
        self.setAcceptHoverEvents(True)
        self.hover_brush = QtGui.QColor(200,200,200)

    def hoverEnterEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverEnterEvent(self, a_event)
        self.o_brush = self.brush()
        self.setBrush(self.hover_brush)

    def hoverLeaveEvent(self, a_event):
        QtGui.QGraphicsRectItem.hoverLeaveEvent(self, a_event)
        self.setBrush(self.o_brush)

class piano_roll_viewer(QtGui.QGraphicsView):
    def __init__(self, a_item_length=4, a_grid_div=16):
        self.item_length = float(a_item_length)
        self.viewer_width = 1000
        self.grid_div = a_grid_div

        self.end_octave = 8
        self.start_octave = -2
        self.notes_in_octave = 12
        self.total_notes = (self.end_octave - self.start_octave) * self.notes_in_octave + 1 #for C8
        self.note_height = 15
        self.octave_height = self.notes_in_octave * self.note_height

        self.header_height = global_piano_roll_header_height

        self.piano_height = self.note_height*self.total_notes
        self.piano_width = 32
        self.padding = 2
        self.piano_height = self.note_height * self.total_notes

        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(100,100,100))
        self.setAlignment(QtCore.Qt.AlignLeft)
        self.setScene(self.scene)
        self.draw_header()
        self.draw_piano()
        self.draw_grid()

        self.right_click = False
        self.left_click = False

    def draw_header(self):
        self.header = QtGui.QGraphicsRectItem(0, 0, self.viewer_width, self.header_height)
        self.header.setPos(self.piano_width + self.padding, 0)
        self.scene.addItem(self.header)
        self.beat_width = self.viewer_width / self.item_length
        self.value_width = self.beat_width / self.grid_div

    def draw_piano(self):
        f_labels = ['B', 'Bb', 'A', 'Ab', 'G', 'Gb', 'F', 'E', 'Eb', 'D', 'Db', 'C']
        f_black_notes = [2, 4, 6, 9, 11]
        f_piano_label = QtGui.QFont()
        f_piano_label.setPointSize(8)
        self.piano = QtGui.QGraphicsRectItem(0, 0, self.piano_width, self.piano_height)
        self.piano.setPos(0, self.header_height)
        self.scene.addItem(self.piano)
        f_key = piano_key_item(self.piano_width, self.note_height, self.piano)
        f_label = QtGui.QGraphicsSimpleTextItem("C8", f_key)
        f_label.setPos(4, 0)
        f_label.setFont(f_piano_label)
        f_key.setBrush(QtGui.QColor(255,255,255))
        for i in range(self.end_octave-self.start_octave, self.start_octave-self.start_octave, -1):
            for j in range(self.notes_in_octave, 0, -1):
                f_key = piano_key_item(self.piano_width, self.note_height, self.piano)
                f_key.setPos(0, self.note_height*(j) + self.octave_height*(i-1))
                if j == 12:
                    f_label = QtGui.QGraphicsSimpleTextItem("%s%d" % (f_labels[j-1], self.end_octave-i), f_key)
                    f_label.setPos(4, 0)
                    f_label.setFont(f_piano_label)
                if j in f_black_notes:
                    f_key.setBrush(QtGui.QColor(0,0,0))
                else:
                    f_key.setBrush(QtGui.QColor(255,255,255))

    def draw_grid(self):
        f_black_notes = [2, 4, 6, 9, 11]
        for i in range(self.end_octave-self.start_octave, self.start_octave-self.start_octave, -1):
            for j in range(self.notes_in_octave, 0, -1):
                f_note_bar = QtGui.QGraphicsRectItem(0, 0, self.viewer_width, self.note_height, self.piano)
                f_note_bar.setPos(self.piano_width + self.padding,  self.note_height*(j) + self.octave_height*(i-1))
                #basic implementation of zth's "show scales", there are a few transparency issues
                #if j not in f_black_notes:
                #    f_note_bar.setBrush(QtGui.QColor(230,230,230,100))
        f_beat_pen = QtGui.QPen()
        f_beat_pen.setWidth(2)
        f_line_pen = QtGui.QPen()
        f_line_pen.setColor(QtGui.QColor(0,0,0,40))
        for i in range(0, int(self.item_length)+1):
            f_beat = QtGui.QGraphicsLineItem(0, 0, 0, self.piano_height+self.header_height-f_beat_pen.width(), self.header)
            f_beat.setPos(self.beat_width * i, 0.5*f_beat_pen.width())
            f_beat.setPen(f_beat_pen)
            if i < self.item_length:
                f_number = QtGui.QGraphicsSimpleTextItem("%d" % (i+1), self.header)
                f_number.setPos(self.beat_width * i + 5, 2)
                f_number.setBrush(QtCore.Qt.white)
                for j in range(0, self.grid_div):
                    f_line = QtGui.QGraphicsLineItem(0, 0, 0, self.piano_height, self.header)
                    if float(j) == self.grid_div / 2.0:
                        f_line.setLine(0, 0, 0, self.piano_height)
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.header_height)
                    else:
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.header_height)
                        f_line.setPen(f_line_pen)

    def set_zoom(self, a_scale):
        self.scale(a_scale, 1.0)

    def clear_drawn_items(self):
        self.scene.clear()
        self.draw_header()
        self.draw_piano()
        self.draw_grid()

    def draw_note(self, a_note, a_start, a_length, a_velocity):
        '''a_note is the midi number, a_start is 1-4.99, a_length is in beats, a_velocity is 0-127'''
        f_start = self.piano_width + self.padding + self.beat_width*(a_start - 1)
        f_length = self.beat_width*(a_length*self.item_length)
        f_note = self.header_height + self.note_height * (self.total_notes - a_note - 1)
        f_note_item = note_item(f_length, self.note_height)
        f_note_item.setPos(f_start, f_note)
        f_vel_opacity = QtGui.QGraphicsOpacityEffect()
        f_vel_opacity.setOpacity(a_velocity/127.0)
        f_note_item.setGraphicsEffect(f_vel_opacity)
        self.scene.addItem(f_note_item)

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    view = piano_roll_viewer()
    view.draw_note(60, 2.0625, 0.25, 127)
    view.draw_note(120, 1, 0.5, 100)
    view.show()
    sys.exit(app.exec_())

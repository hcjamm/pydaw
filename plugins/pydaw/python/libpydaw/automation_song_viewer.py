from PyQt4 import QtGui, QtCore
from pydaw_project import *

global_song_automation_point_diameter = 15.0
global_song_automation_point_radius = global_song_automation_point_diameter * 0.5
global_song_automation_ruler_width = 24
global_song_automation_bar_size_px = 20.0
global_song_automation_reg_size_px = global_song_automation_bar_size_px * 8.0

global_region_count = 300
global_bars_per_region = 8
global_song_automation_width = global_region_count * global_bars_per_region
global_song_automation_height = 360

global_song_automation_total_height = global_song_automation_ruler_width +  global_song_automation_height - global_song_automation_point_radius
global_song_automation_total_width = global_song_automation_ruler_width + global_song_automation_width - global_song_automation_point_radius
global_song_automation_min_height = global_song_automation_ruler_width - global_song_automation_point_radius

global_song_automation_gradient = QtGui.QLinearGradient(0, 0, global_song_automation_point_diameter, global_song_automation_point_diameter)
global_song_automation_gradient.setColorAt(0, QtGui.QColor(240, 10, 10))
global_song_automation_gradient.setColorAt(1, QtGui.QColor(250, 90, 90))

def global_song_automation_pos_to_px(a_reg, a_bar, a_beat):
    print "global_song_automation_pos_to_px", a_reg, a_bar, a_beat
    return (((a_reg * 8.0) + (a_bar) + (a_beat * 0.25)) * global_song_automation_bar_size_px) + global_song_automation_ruler_width

def global_song_automation_px_to_pos(a_px):
    f_bar_count = (a_px - global_song_automation_ruler_width + global_song_automation_point_radius) / global_song_automation_bar_size_px
    print "f_bar_count", f_bar_count
    f_reg = 0
    f_bar = 0
    f_beat = 0.0
    while True:
        if f_bar_count >= 8.0:
            f_bar_count -= 8.0
            f_reg += 1
        else:
            f_bar = int(f_bar_count)
            f_beat = float(f_bar_count - f_bar) * 4.0
            break
    return (f_reg, f_bar, f_beat)

class song_automation_point(QtGui.QGraphicsEllipseItem):
    def __init__(self, a_pos_x, a_value, a_cc, a_view):
        QtGui.QGraphicsEllipseItem.__init__(self, 0, 0, global_song_automation_point_diameter, global_song_automation_point_diameter)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.setPos(a_pos_x - global_song_automation_point_radius, a_value - global_song_automation_point_radius)
        self.setBrush(global_song_automation_gradient)
        f_pen = QtGui.QPen()
        f_pen.setWidth(2)
        f_pen.setColor(QtGui.QColor(170,0,0))
        self.setPen(f_pen)
        self.cc_item = a_cc
        self.parent_view = a_view

    def mousePressEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mousePressEvent(self, a_event)
        self.setGraphicsEffect(QtGui.QGraphicsOpacityEffect())

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseMoveEvent(self, a_event)
        for f_point in self.parent_view.automation_points:
            if f_point.isSelected():
                if f_point.pos().x() < global_song_automation_min_height:
                    f_point.setPos(global_song_automation_min_height, f_point.pos().y())
                elif f_point.pos().x() > global_song_automation_total_width:
                    f_point.setPos(global_song_automation_total_width, f_point.pos().y())
                if f_point.pos().y() < global_song_automation_min_height:
                    f_point.setPos(f_point.pos().x(), global_song_automation_min_height)
                elif f_point.pos().y() > global_song_automation_total_height:
                    f_point.setPos(f_point.pos().x(), global_song_automation_total_height)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseReleaseEvent(self, a_event)
        self.setGraphicsEffect(None)
        for f_point in self.parent_view.automation_points:
            if f_point.isSelected():
                f_reg, f_bar, f_beat = global_song_automation_px_to_pos(self.pos().x())
                print f_reg, f_bar, f_beat

class song_automation_viewer(QtGui.QGraphicsView):
    def __init__(self):
        self.automation_points = []
        self.beat_width = global_song_automation_bar_size_px
        self.lines = []
        QtGui.QGraphicsView.__init__(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.scene.setBackgroundBrush(QtGui.QColor(100,100,100))
        self.scene.mouseDoubleClickEvent = self.sceneMouseDoubleClickEvent
        self.setAlignment(QtCore.Qt.AlignLeft)
        self.setScene(self.scene)
        self.draw_axis()
        self.draw_grid()
        self.setDragMode(QtGui.QGraphicsView.RubberBandDrag)
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.setResizeAnchor(QtGui.QGraphicsView.AnchorViewCenter)
        self.cc_num = 1

    def keyPressEvent(self, a_event):
        a_event.setAccepted(True)
        QtGui.QGraphicsScene.keyPressEvent(self.scene, a_event)
        if a_event.key() == QtCore.Qt.Key_Delete:
            f_to_be_deleted = []
            for f_point in self.automation_points:
                if f_point.isSelected():
                    f_to_be_deleted.append(f_point)
            for f_point in f_to_be_deleted:
                self.automation_points.remove(f_point)
                self.scene.removeItem(f_point)
            self.connect_points()

    def sceneMouseDoubleClickEvent(self, a_event):
        QtGui.QGraphicsScene.mouseDoubleClickEvent(self.scene, a_event)
        f_pos_x = a_event.scenePos().x()
        f_pos_y = a_event.scenePos().y()
        f_reg, f_bar, f_beat = global_song_automation_px_to_pos(f_pos_x)
        f_cc_val = 127.0 - (((f_pos_y - global_song_automation_min_height) / global_song_automation_height) * 127.0)
        self.draw_point(pydaw_song_level_cc(f_reg, f_bar, f_beat, self.cc_num, f_cc_val))

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsView.mouseMoveEvent(self, a_event)
        if self.scene.mouseGrabberItem():
            self.connect_points()

    def draw_axis(self):
        self.x_axis = QtGui.QGraphicsRectItem(0, 0, (global_song_automation_width+8)*self.beat_width, global_song_automation_ruler_width)
        self.x_axis.setPos(global_song_automation_ruler_width, 0)
        self.scene.addItem(self.x_axis)
        self.y_axis = QtGui.QGraphicsRectItem(0, 0, global_song_automation_ruler_width, global_song_automation_height)
        self.y_axis.setPos(0, global_song_automation_ruler_width)
        self.scene.addItem(self.y_axis)

    def draw_grid(self):
        f_pen = QtGui.QPen()
        f_pen.setWidth(2)
        f_beat_pen = QtGui.QPen()
        f_beat_pen.setColor(QtGui.QColor(0,0,0,50))
        for i in range(2):
            f_line = QtGui.QGraphicsLineItem(0, 0, (global_song_automation_width+8)*self.beat_width, 0, self.y_axis)
            f_line.setPos(global_song_automation_ruler_width,global_song_automation_height*(i+1)/2.0)
        for i in range(0, global_song_automation_width):
            f_beat = QtGui.QGraphicsLineItem(0, 0, 0, global_song_automation_height + global_song_automation_ruler_width, self.x_axis)
            f_beat.setPos(self.beat_width * i, 0)
            if i % 4.0:
                f_beat.setPen(f_beat_pen)
            elif not i % global_bars_per_region:
                f_number = QtGui.QGraphicsSimpleTextItem(str(int(i/global_bars_per_region)), self.x_axis)
                f_number.setPos(self.beat_width * i + 5, 2)
                f_number.setBrush(QtCore.Qt.white)
                f_beat.setPen(f_pen)

    def set_zoom(self, a_scale):
        self.scale(a_scale, 1.0)

    def clear_drawn_items(self):
        self.scene.clear()
        self.draw_axis()
        self.draw_grid()

    def connect_points(self):
        if self.lines:
            for i in range(len(self.lines)):
                self.scene.removeItem(self.lines[i])
        #sort list based on x
        if len(self.automation_points) > 1:
            self.lines = (len(self.automation_points)-1)*[None]
            self.automation_points.sort(key=lambda point: point.pos().x())
            f_line_pen = QtGui.QPen()
            f_line_pen.setColor(QtGui.QColor(255,60,60))
            f_line_pen.setWidth(2)
            for i in range(1, len(self.automation_points)):
                f_start_x = self.automation_points[i-1].pos().x()
                f_start_y = self.automation_points[i-1].pos().y()
                f_end_x = self.automation_points[i].pos().x()
                f_end_y = self.automation_points[i].pos().y()
                f_pos_x = f_end_x - f_start_x
                f_pos_y = f_end_y - f_start_y
                f_line = QtGui.QGraphicsLineItem(0, 0, f_pos_x, f_pos_y)
                f_line.setPos(global_song_automation_point_radius + f_start_x, global_song_automation_point_radius + f_start_y)
                f_line.setPen(f_line_pen)
                self.scene.addItem(f_line)
                self.lines[i-1] = f_line

    def set_cc_num(self, a_cc_num):
        self.cc_num = a_cc_num
        self.clear_drawn_items()
        #TODO:  call open_item here..

    def draw_item(self, a_item):
        """ a_item is an instance of pydaw_item """
        self.clear_drawn_items()
        for f_cc in a_item.ccs:
            if f_cc.cc_num == self.cc_num:
                self.draw_point(a_cc)

    def draw_point(self, a_cc):
        """ a_cc is an instance of the pydaw_song_level_cc class"""
        f_time = global_song_automation_pos_to_px(a_cc.region, a_cc.bar, a_cc.beat)
        f_value = global_song_automation_ruler_width + (global_song_automation_height/127.0) * (127.0 - a_cc.value)
        f_point = song_automation_point(f_time, f_value, a_cc, self)
        self.automation_points.append(f_point)
        self.scene.addItem(f_point)
        self.connect_points()

if __name__ == '__main__':
    import sys
    app = QtGui.QApplication(sys.argv)
    view = song_automation_viewer()
    test1 = pydaw_song_level_cc(0.0, 0.0, 0.0, 1, 15)
    view.draw_point(test1)
    test2 = pydaw_song_level_cc(1.0, 4.0, 0.0, 1, 30)
    view.draw_point(test2)
    view.show()
    sys.exit(app.exec_())

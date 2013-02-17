"""
A piano roll viewer that will eventually become a piano roll editor
"""

from PyQt4 import QtGui, QtCore
from pydaw_project import *

global_automation_point_diameter = 15.0
global_automation_point_radius = global_automation_point_diameter * 0.5
global_automation_ruler_width = 24
global_automation_width = 800
global_automation_height = 300

global_automation_total_height = global_automation_ruler_width +  global_automation_height - global_automation_point_radius
global_automation_total_width = global_automation_ruler_width + global_automation_width - global_automation_point_radius
global_automation_min_height = global_automation_ruler_width - global_automation_point_radius

global_automation_gradient = QtGui.QLinearGradient(0, 0, global_automation_point_diameter, global_automation_point_diameter)
global_automation_gradient.setColorAt(0, QtGui.QColor(240, 10, 10))
global_automation_gradient.setColorAt(1, QtGui.QColor(250, 90, 90))

class automation_item(QtGui.QGraphicsEllipseItem):
    def __init__(self, a_time, a_value, a_cc, a_view):
        QtGui.QGraphicsEllipseItem.__init__(self, 0, 0, global_automation_point_diameter, global_automation_point_diameter)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable)
        self.setPos(a_time-global_automation_point_radius, a_value - global_automation_point_radius)
        self.setBrush(global_automation_gradient)
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
        if self.pos().x() < global_automation_min_height:
            self.setPos(global_automation_min_height, self.pos().y())
        elif self.pos().x() > global_automation_total_width:
            self.setPos(global_automation_total_width, self.pos().y())

        if self.pos().y() < global_automation_min_height:
            self.setPos(self.pos().x(), global_automation_min_height)
        elif self.pos().y() > global_automation_total_height:
            self.setPos(self.pos().x(), global_automation_total_height)

    def mouseReleaseEvent(self, a_event):
        QtGui.QGraphicsEllipseItem.mouseReleaseEvent(self, a_event)
        self.setGraphicsEffect(None)
        for f_point in self.parent_view.automation_points:
            if f_point.isSelected():
                f_cc_start = ((f_point.pos().x() - global_automation_min_height) / global_automation_width) * 4.0
                f_cc_val = 127.0 - (((f_point.pos().y() - global_automation_min_height) / global_automation_height) * 127.0)
                print(str(f_cc_start) + "|" + str(f_cc_val))

class automation_viewer(QtGui.QGraphicsView):
    def __init__(self, a_item_length=4, a_grid_div=16):
        self.item_length = float(a_item_length)
        self.steps = 127.0
        self.viewer_width = global_automation_width
        self.viewer_height = global_automation_height
        self.grid_div = a_grid_div
        self.automation_points = []

        self.axis_size = global_automation_ruler_width

        self.beat_width = self.viewer_width / self.item_length
        self.value_width = self.beat_width / self.grid_div
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
        QtGui.QGraphicsScene.keyPressEvent(self.scene, a_event)
        if a_event.key() == QtCore.Qt.Key_Delete:
            f_to_be_deleted = []
            for f_point in self.automation_points:
                if f_point.isSelected():
                    f_to_be_deleted.append(f_point)
            for f_point in f_to_be_deleted:
                self.automation_points.remove(f_point)
            self.clear_drawn_items()
            #TODO:  Properly hook into the PyDAW MIDI item and redraw everything after deleting

    def sceneMouseDoubleClickEvent(self, a_event):
        f_pos_x = a_event.scenePos().x()
        f_pos_y = a_event.scenePos().y()
        f_time = (f_pos_x - self.axis_size)/self.beat_width
        f_value = self.steps - ((f_pos_y - self.axis_size) * self.steps / self.viewer_height)
        print f_time, f_value
        f_cc_start = ((f_pos_x - global_automation_min_height) / global_automation_width) * 4.0
        f_cc_val = 127.0 - (((f_pos_y - global_automation_min_height) / global_automation_height) * 127.0)
        print(str(f_cc_start) + "|" + str(f_cc_val))
        self.draw_point(pydaw_cc(f_cc_start, self.cc_num, f_cc_val))
        QtGui.QGraphicsScene.mouseDoubleClickEvent(self.scene, a_event)

    def mouseMoveEvent(self, a_event):
        QtGui.QGraphicsView.mouseMoveEvent(self, a_event)
        if self.scene.mouseGrabberItem():
            self.connect_points()

    def draw_axis(self):
        self.x_axis = QtGui.QGraphicsRectItem(0, 0, self.viewer_width, self.axis_size)
        self.x_axis.setPos(self.axis_size, 0)
        self.scene.addItem(self.x_axis)
        self.y_axis = QtGui.QGraphicsRectItem(0, 0, self.axis_size, self.viewer_height)
        self.y_axis.setPos(0, self.axis_size)
        self.scene.addItem(self.y_axis)

    def draw_grid(self):
        f_beat_pen = QtGui.QPen()
        f_beat_pen.setWidth(2)
        f_line_pen = QtGui.QPen()
        f_line_pen.setColor(QtGui.QColor(0,0,0,40))
        f_labels = [0, '127', 0, '64', 0, '0']
        for i in range(1,6):
            f_line = QtGui.QGraphicsLineItem(0, 0, self.viewer_width, 0, self.y_axis)
            f_line.setPos(self.axis_size,self.viewer_height*(i-1)/4)
            if i % 2:
                f_label = QtGui.QGraphicsSimpleTextItem(f_labels[i], self.y_axis)
                f_label.setPos(1, self.viewer_height*(i-1)/4)
                f_label.setBrush(QtCore.Qt.white)
            if i == 3:
                f_line.setPen(f_beat_pen)

        for i in range(0, int(self.item_length)+1):
            f_beat = QtGui.QGraphicsLineItem(0, 0, 0, self.viewer_height+self.axis_size-f_beat_pen.width(), self.x_axis)
            f_beat.setPos(self.beat_width * i, 0.5*f_beat_pen.width())
            f_beat.setPen(f_beat_pen)
            if i < self.item_length:
                f_number = QtGui.QGraphicsSimpleTextItem(str(i), self.x_axis)
                f_number.setPos(self.beat_width * i + 5, 2)
                f_number.setBrush(QtCore.Qt.white)
                for j in range(0, self.grid_div):
                    f_line = QtGui.QGraphicsLineItem(0, 0, 0, self.viewer_height, self.x_axis)
                    if float(j) == self.grid_div / 2.0:
                        f_line.setLine(0, 0, 0, self.viewer_height)
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.axis_size)
                    else:
                        f_line.setPos((self.beat_width*i)+(self.value_width*j), self.axis_size)
                        f_line.setPen(f_line_pen)

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
                f_line.setPos(7.5+f_start_x, 7.5+f_start_y)
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
        """ a_cc is an instance of the pydaw_cc class"""
        f_time = self.axis_size + self.beat_width * a_cc.start
        f_value = self.axis_size +  self.viewer_height/self.steps * (self.steps - a_cc.cc_val)
        f_point = automation_item(f_time, f_value, a_cc, self)
        self.automation_points.append(f_point)
        self.scene.addItem(f_point)
        self.connect_points()

if __name__ == '__main__':
    import sys
    app = QtGui.QApplication(sys.argv)
    view = automation_viewer()
    view.draw_point(pydaw_cc(0.5, 1, 15))
    view.draw_point(pydaw_cc(1.0, 1, 120))
    view.draw_point(pydaw_cc(2.0, 1, 64))
    view.draw_point(pydaw_cc(3.0, 1, 90))
    view.show()
    sys.exit(app.exec_())

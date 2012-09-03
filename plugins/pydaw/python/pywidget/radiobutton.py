#!/usr/bin/env python
# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright (c) 2009 Nicolas Rougier, Matthieu Kluj, Jessy Cyganczuk
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of pyglet nor the names of its
#    contributors may be used to endorse or promote products
#    derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# -----------------------------------------------------------------------------
import pyglet
from pyglet.gl import *
from shape import Rectangle, Ellipse, Cross, Star
from widget import Widget
from label import Label

# ----------------------------------------------------------------------- Button
class Radiobutton(Widget):
    ''' Radiobutton widget

    This is a basic Radio button.
    '''
    # _________________________________________________________________ __init__
    def __init__(self, x=0, y=0, z=0, width=300, height=300, anchor_x='left',
                 anchor_y='bottom', elements=[]):

        fg = (.5,.5,.5, 1)
        bg = (.5,.5,.5,.5)
        Widget.__init__(self,x,y,z,width,height,anchor_x,anchor_y)

        self.margin = 3
        self.ropen = 0
        length = len(elements)
        for i in range(length):
            elements[i].height = height / length - 2 * self.margin
            elements[i].width = width - 2 * self.margin
            elements[i].x = self.margin
            elements[i].y = (height - self.margin) - (i + 2) * (elements[i].height + self.margin)
            self._elements[i] = elements[i]
            self._elements[i]._hidden = True

        chooselabel = Label(text='...',
                            x=self.margin, y= (height - self.margin) - (
                            elements[i].height + self.margin),
                            height = height / length - 2 * self.margin,
                            width = width - 2 * self.margin)

        frame = Rectangle (x=chooselabel.x, y=chooselabel.y + chooselabel.height, z=z,
                           width=chooselabel.width, height=-chooselabel.height, radius=0,
                           foreground=fg, background=bg,
                           anchor_x=anchor_x, anchor_y=anchor_y)

        self._elements['chooselabel'] = chooselabel
        self._elements['frame'] = frame

    # ____________________________________________________________________ update_width
    def update_width(self):

        for i in range(len(self._elements) - 2):
            self._elements[i].width = self.width - 2 * self.margin
            self._elements[i].x = self.margin

        self._elements['chooselabel'].x = self.margin
        self._elements['chooselabel'].width = self.width - 2 * self.margin

        self._elements['frame'].x = self._elements['chooselabel'].x
        self._elements['frame'].width = self._elements['chooselabel'].width

    # ____________________________________________________________________ update_height
    def update_height(self):
        
        length = len(self._elements) - 2
        for i in range(length):
            #self._elements[i].height = self.height / length - 2* self.margin
            self._elements[i].y = (self.height - self.margin) - (i + 2) * (self._elements[i].height + self.margin)

        self._elements['chooselabel'].y = self.height - self._elements[0].height
        self._elements['chooselabel'].height = self.height - self.margin

        self._elements['frame'].y = self._elements['chooselabel'].y + self._elements['chooselabel'].height
        self._elements['frame'].height = - self._elements['chooselabel'].height + self.margin
        self._elements['frame'].radius= 0

    # ___________________________________________________________ on_mouse_press
    def on_mouse_press(self, x, y, Radiobutton, modifiers):
        if Radiobutton == pyglet.window.mouse.LEFT:

            if self._elements['chooselabel'].hit_test(x - self.x, y - self.y) and self.ropen == 0 :
                self._elements['frame'].background = (0.8, 0.8, 0.8, 0.5)
                self._elements['frame'].height = - self._elements['chooselabel'].height
                for i in range(len(self._elements) - 2):
                    self._elements['frame'].height += - self._elements[i].height - self.margin
                    self._elements[i]._hidden = False
                self.ropen = 1
                return pyglet.event.EVENT_HANDLED

            if self.ropen == 1:
                for i in range(len(self._elements) - 2):
                    if self._elements[i].hit_test(x - self.x, y - self.y):
                        self._elements['chooselabel'].set_text(self._elements[i].text)
                        self.dispatch_event('on_Radiobutton_press', self)
                        self._elements['frame'].background = (0.8, 0.8, 0.8, 0.5)
                        self._elements['frame'].height = - self._elements['chooselabel'].height
                        for i in range(len(self._elements) - 2):
                            self._elements[i]._hidden = True
                        self.ropen = 0
                        return pyglet.event.EVENT_HANDLED

        return pyglet.event.EVENT_UNHANDLED

    # ___________________________________________________________ on_mouse_release
    def on_mouse_release(self, x, y, Radiobutton, modifiers):
        if Radiobutton == pyglet.window.mouse.LEFT:
          self._elements['frame'].background = (0.5, 0.5, 0.5, 0.5)
        return pyglet.event.EVENT_UNHANDLED

Radiobutton.register_event_type('on_Radiobutton_press')

# ------------------------------------------------------------------------------
if __name__ == '__main__':

    window = pyglet.window.Window(resizable=True)
    label1 = Label(text='<font face="Helvetica,Arial" size="2" color=white>First Label</font>',
                    x=50, y=50)
    label2 = Label(text='<font face="Helvetica,Arial" size="2" color=white>second Label</font>',
                    x=50, y=50)
    label3 = Label(text='<font face="Helvetica,Arial" size="2" color=white>third Label</font>',
                    x=50, y=50)
    rbutton = Radiobutton(x=50, y=50, height=90, width=100, elements=[label1, label2, label3])
    window.push_handlers(rbutton)

    @window.event
    def on_draw():
        window.clear()
        rbutton.on_draw()

    @rbutton.event
    def on_Radiobutton_press(rbutton):
        print 'change'
        print rbutton._elements['chooselabel'].text

    pyglet.app.run()
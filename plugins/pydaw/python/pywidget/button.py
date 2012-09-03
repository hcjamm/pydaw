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


# ----------------------------------------------------------------------- Button
class Button(Widget):
    ''' Button widget
    
    This is a basic Button.
    '''
    # _________________________________________________________________ __init__
    def __init__(self, x=0, y=0, z=0, width=0, height=0, pad = (10,2),
                 font_size = 10, anchor_x='left', anchor_y='bottom',
                 text='Button'):
        
        fg = (.5,.5,.5, 1)
        bg = (.5,.5,.5,.5)
        label = pyglet.text.HTMLLabel(text,
                                      anchor_x='center', anchor_y='center')
        xpad,ypad = pad
        if width == 0:
            width = label.content_width + 2*xpad
        if height == 0:
            height = label.content_height/2 +2*ypad
        label.x = width/2
        label.y = height/2+1
        frame = Rectangle (x=0, y=0, z=z,
                           width=width, height=height, radius=height/2-1,
                           foreground=fg, background=bg,
                           anchor_x=anchor_x, anchor_y=anchor_y)

        Widget.__init__(self,x,y,z,width,height,anchor_x,anchor_y)
        self._elements['frame'] = frame
        self._elements['label'] = label
        
    # ____________________________________________________________________ update_width
    def update_width(self):
        self._elements['frame'].width = self.width
        self._elements['label'].width = self.width
        self._elements['label'].x = self.width / 2
        
    # ____________________________________________________________________ update_height
    def update_height(self):
        self._elements['frame'].height = self.height
        self._elements['label'].content_height = self.height
        self._elements['label'].y = self.height / 2 + 1
        self._elements['frame'].radius= self.height / 2 - 1

    # ___________________________________________________________ on_mouse_press
    def on_mouse_press(self, x, y, button, modifiers):
        if button == pyglet.window.mouse.LEFT:
            if self.hit_test(x,y):
                self.dispatch_event('on_button_press', self)
                self._elements['frame'].background = (0.8, 0.8, 0.8, 0.5)
                return pyglet.event.EVENT_HANDLED
        return pyglet.event.EVENT_UNHANDLED
    
    # ___________________________________________________________ on_mouse_release
    def on_mouse_release(self, x, y, button, modifiers):
        if button == pyglet.window.mouse.LEFT:
          self._elements['frame'].background = (0.5, 0.5, 0.5, 0.5)
        return pyglet.event.EVENT_UNHANDLED

Button.register_event_type('on_button_press')

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    window = pyglet.window.Window(resizable=True)
    button = Button(text='<font face="Helvetica,Arial" size="2" color=white>Click me</font>',
                    x=50, y=50)
    window.push_handlers(button)

    @window.event
    def on_draw():
        window.clear()
        button.on_draw()

    @button.event
    def on_button_press(button):
        print 'Click'

    pyglet.app.run()

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


# ----------------------------------------------------------------------- Checkbox
class Checkbox(Widget):
    ''' Checkbox widget
    
    This is a basic Checkbox.
    '''
    # _________________________________________________________________ __init__
    def __init__(self, x=0, y=0, z=0, width=0, height=0, pad = (10,2),
                 font_size = 10, anchor_x='left', anchor_y='bottom', checked=False):
        
        fg = (.5,.5,.5, 1)
        bg = (.5,.5,.5,.5)
        
        fg2 = bg2 = (.8,.8,.8, 1)

        cross = Cross (x=4.2 + width / 2 - height / 2, y=4.2, z=z+.1,
                              width=height-8, height=height-8,
                              branches=4,
                              foreground=fg2, background=bg2,
                              anchor_x='left', anchor_y='bottom',
                              thickness=0.15)

        frame = Rectangle (x=width / 2 - height / 2, y=0, z=z,
                           width=height, height=height, radius=height/4-1,
                           foreground=fg, background=bg,
                           anchor_x=anchor_x, anchor_y=anchor_y)
        Widget.__init__(self,x,y,z,width,height,anchor_x,anchor_y)
        self._elements['frame'] = frame
        self._elements['cross'] = cross
        self.checked = checked
        
    # ____________________________________________________________________ update_width
    def update_width(self):
        self._elements['frame'].x = self.width / 2 - self.height / 2
        self._elements['cross'].x = 4.2 + self.width / 2 - self.height / 2
        
    # ____________________________________________________________________ update_height
    def update_height(self):
        self._elements['frame'].width = self._elements['frame'].height = self.height
        self._elements['frame'].radius= self.height / 4 - 1
        self._elements['frame'].x = self.width / 2 - self.height / 2
        self._elements['cross'].width = self._elements['cross'].height = self.height - 8
        self._elements['cross'].x = 4.2 + self.width / 2 - self.height / 2
    
    def on_draw(self):
        glTranslatef(self._root_x, self._root_y, self._root_z)
        self._elements['frame'].draw()
        if self.checked:
          self._elements['cross'].draw()
        glTranslatef(-self._root_x, -self._root_y, -self._root_z)

    # ___________________________________________________________ on_mouse_press
    def on_mouse_press(self, x, y, button, modifiers):
        if button == pyglet.window.mouse.LEFT:
            if self.hit_test(x,y):
                self.checked = not self.checked
                self.dispatch_event('on_value_change', self)
                self._elements['frame'].background = (0.8, 0.8, 0.8, 0.5)
                return pyglet.event.EVENT_HANDLED
        return pyglet.event.EVENT_UNHANDLED
        
    # ___________________________________________________________ on_mouse_press
    def on_mouse_release(self, x, y, button, modifiers):
        if button == pyglet.window.mouse.LEFT:
          self._elements['frame'].background = (0.5, 0.5, 0.5, 0.5)
        return pyglet.event.EVENT_UNHANDLED

Checkbox.register_event_type('on_value_change')

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    window = pyglet.window.Window(resizable=True)
    checkbox = Checkbox(x=50, y=50, height=20, width=100)
    window.push_handlers(checkbox)

    @window.event
    def on_draw():
        window.clear()
        checkbox.on_draw()

    @checkbox.event
    def on_value_change(checkbox):
        print checkbox.checked

    pyglet.app.run()

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


# ----------------------------------------------------------------------- Slider
class Slider(Widget):
    ''' Slider widget
    
    Basic slider
    '''
    # _________________________________________________________________ __init__
    def __init__(self, x=0, y=0, z=0, width=200, height=10,
                 font_size = 10, anchor_x='left', anchor_y='bottom',
                 value=0.50):

        Widget.__init__(self,x,y,z,width,height,anchor_x,anchor_y)

        fg = (1,1,1,1)
        bg = (1,1,1,.5)

        frame = Rectangle (x=0, y=0, z=z,
                           width=width, height=height, radius=(height-1)/2,
                           foreground=fg, background=bg,
                           anchor_x=anchor_x, anchor_y=anchor_y)
        cursor = Ellipse (x=0, y=-.5+height/2, z=z,
                          width=height-1, height=height-1,
                          foreground=fg, background=fg,
                          anchor_x='center', anchor_y='center')
        label = pyglet.text.Label('0',
                                  font_name='Monaco',
                                  font_size=8,
                                  x=0, y=height+2,
                                  anchor_x='center', anchor_y='bottom')
        self._elements['frame'] = frame
        self._elements['cursor'] = cursor
        self._elements['label'] = label
        self.set_cursor(value)
        self._is_dragging = False
        
    # ____________________________________________________________________ update_width
    def update_width(self):
        self._elements['frame'].width = self.width
        self.set_cursor(self.value)

    # ____________________________________________________________________ update_height
    def update_height(self):
        self._elements['frame'].y = self.height / 2 - self._elements['frame'].height
        self._elements['cursor'].y = self._elements['frame'].y + self._elements['frame'].height / 2 - 0.5
        self._elements['label'].y = self._elements['frame'].y + self._elements['frame'].height + 2

    # ____________________________________________________________________ set_cursor
    def set_cursor(self, v):
      ''' Sets the cursor position
      
      :Parameters:
          `v` : float
              Position of the cursor.
      '''
      v = max(0,min(1,v))
      self.value = v
      self._elements['cursor'].x = int(self.height/2 + (self.width-self.height)*v)
      self._elements['label'].text = '%.2f' % v
      self._elements['label'].x = self._elements['cursor'].x

    # ____________________________________________________________________ on_mouse_press
    def on_mouse_press(self,x,y,button,modifiers):
        if self._elements['cursor'].hit_test(x - self.x, y - self.y) or self.hit_test(x,y):
          self._is_dragging = True
          self._elements['cursor'].background = (0.5, 0.5, 0.5, 1)
          v = (x-self.x) / float(self.width)
          self.set_cursor(v)

    # ____________________________________________________________________ on_mouse_drag
    def on_mouse_drag(self,x,y,dx,dy,button,modifiers):
      if self._is_dragging:
        v = (x+dx-self.x) / float(self.width)
        self.set_cursor(v)
        self.dispatch_event('on_value_change', self)
        return pyglet.event.EVENT_HANDLED
        
    # ____________________________________________________________________ on_mouse_release
    def on_mouse_release(self, x, y, button, modifiers):
        if button == pyglet.window.mouse.LEFT:
          self._is_dragging = False
          self._elements['cursor'].background = (1, 1, 1, 1)
        return pyglet.event.EVENT_UNHANDLED
        
Slider.register_event_type('on_value_change')

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    window = pyglet.window.Window(resizable=True)
    slider = Slider(x=50, y=50)
    window.push_handlers(slider)

    @window.event
    def on_draw():
        window.clear()
        slider.on_draw()

    @slider.event
    def on_value_change(slider):
        print slider.value

    pyglet.app.run()

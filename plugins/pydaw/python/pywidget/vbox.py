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


# ----------------------------------------------------------------------- VBox
class VBox(Widget):
    ''' Slider widget
    
    Used to split content into vertical parts
    '''
    # _________________________________________________________________ __init__
    def __init__(self, x=0, y=0, z=0, width=300, height=300,
                anchor_x='left', anchor_y='bottom', elements=[]):
      Widget.__init__(self,x,y,z,width,height,anchor_x,anchor_y)
      self.margin = 3
      length = len(elements)
      for i in range(length):
        elements[i].height = height / length - 2 * self.margin
        elements[i].width = width - 2 * self.margin
        elements[i].x = self.margin
        elements[i].y = (height - self.margin) - (i + 1) * (elements[i].height + self.margin)
        self._elements[i] = elements[i]
      
    # ____________________________________________________________________ update_width
    def update_width(self):
      for i in range(len(self._elements)):
        self._elements[i].width = self.width - 2 * self.margin

    # ____________________________________________________________________ update_height
    def update_height(self):
      length = len(self._elements)
      for i in range(length):
        self._elements[i].height = self.height / length - self.margin
        self._elements[i].y = (self.height - self.margin / 2) - (i + 1) * self._elements[i].height - i * self.margin

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from button import Button
    window = pyglet.window.Window(resizable=True)
    
    button1 = Button(text='<font face="Helvetica,Arial" size="2" color="white">Click me 1</font>')
    button2 = Button(text='<font face="Helvetica,Arial" size="2" color=white>Click me 2</font>')
    button3 = Button(text='<font face="Helvetica,Arial" size="2" color=white>Click me 3</font>')
    
    vbox = VBox(x=50, y=50, height=90, width=100, elements=[button1, button2, button3])
    window.push_handlers(vbox)
    @window.event
    def on_draw():
        window.clear()
        vbox.on_draw()
    pyglet.app.run()

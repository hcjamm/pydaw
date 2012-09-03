#!/usr/bin/env python
# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright (c) 2009 Nicolas Rougier
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
from pyglet.event import EventDispatcher
from shape import Rectangle, Ellipse, Cross, Star


# ------------------------------------------------------------------------------
class Widget(EventDispatcher):
    ''' Abstract widget.

    This is the common abstract class for all widgets.
    '''

    # _________________________________________________________________ __init__
    def __init__(self, x=0, y=0, z=0, width=100, height=100,
                 anchor_x='left', anchor_y='bottom', *args, **kwargs):
        ''' Create a displayable widget.

        :Parameters:
            `x` : float
                X coordinate of the widget relative to anchor_x.
            `y` : float
                Y coordinate of the widget relative to anchor_y.
            `z` : float
                Z coordinate of the widget plane.
            `width` : int
                Width of the widget.
            `height` : int
                Height of the widget.
            `anchor_x` : str
                Horizontal alignment of the widget.
                See `Widget.anchor_x` for details.
            `anchor_y` : str
                Vertical alignment of the widget.
                See `Widget.anchor_y` for details.
        '''
        EventDispatcher.__init__(self)
        self._x, self._y, self._z = x, y, z
        self._root_x, self._root_y, self._root_z = 0,0,0
        self._width = width
        self._height = height
        self.anchor_x = anchor_x
        self.anchor_y = anchor_y
        self._children = []
        self._elements = {}
        self._moveable = True
        self._focusable = True
        self._sizeable = True
        self._hidden = False

    # ________________________________________________________________________ x
    def _get_x(self):
        return self._x
    def _set_x(self, x):
        self._root_x += (x-self._x)
        self._x = x
        self.update_x()
    x = property(_get_x, _set_x,
        doc='''X coordinate of the widget.

        :type: int
        ''')
    def update_x(self):
      pass

    # ________________________________________________________________________ y
    def _get_y(self):
        return self._y
    def _set_y(self, y):
        self._root_y += (y-self._y)
        self._y = y
        self.update_y()
    y = property(_get_y, _set_y,
        doc='''Y coordinate of the widget.

        :type: int
        ''')
    def update_y(self):
      pass

    # ________________________________________________________________________ z
    def _get_z(self):
        return self._z
    def _set_z(self, z):
        self._z = z
        self.update_z()
    z = property(_get_z, _set_z,
        doc='''Z coordinate of the widget.

        :type: int
        ''')
    def update_z(self):
      pass

    # ____________________________________________________________________ width
    def _get_width(self):
        return self._width
    def _set_width(self, width):
        self._width = width
        self.update_width()
    width = property(_get_width, _set_width, 
        doc='''Width of the widget.

        :type: int
        ''')
    def update_width(self):
      pass 
    
    # ___________________________________________________________________ height
    def _get_height(self):
        return self._height
    def _set_height(self, height):
        self._height = height
        self.update_height()
    height = property(_get_height, _set_height,
        doc='''Height of the widget.
        
        :type: int
        ''')
    def update_height(self):
      pass
      
    # _________________________________________________________________ anchor_x
    def _get_anchor_x(self):
        return self._anchor_x
    def _set_anchor_x(self, anchor_x):
        self._anchor_x = anchor_x
        if self.anchor_x == 'center':
            self._root_x = self.x-self.width/2
        elif self.anchor_x == 'right':
            self._root_x = self.x-self.width
        else:
            self._root_x = self.x

    anchor_x = property(_get_anchor_x, _set_anchor_x,
        doc='''Horizontal alignment of the widget.

        The shape is positioned relative to `x` and `width` according to this
        property, which must be one of the alignment constants `left`,
        `center` or `right`.

        :type: str
        ''')

    # _________________________________________________________________ anchor_y
    def _get_anchor_y(self):
        return self._anchor_y
    def _set_anchor_y(self, anchor_y):
        self._anchor_y = anchor_y
        if self.anchor_y == 'center':
            self._root_y = self.y-self.height/2
        elif self.anchor_y == 'top':
            self._root_y = self.y-self.height
        else:
            self._root_y = self.y

    anchor_y = property(_get_anchor_y, _set_anchor_y,
        doc='''Vertical alignment of the widget.

        The shape is positioned relative to `y` according to this property,
        which must be one of the alignment constants `bottom`, `center`
        or `top`.

        :type: str
        ''')

    # ________________________________________________________________ outer_box
    def outer_box(self):
        ''' Returns the outer bounding box of the widget

        The outer bounding box may be larger than given dimensions because some
        labels or ornaments of the widget may extend actual dimensions.
        '''
        return self._root_x, self._root_y, self._width, self._height

    # ________________________________________________________________ inner_box
    def inner_box(self):
        ''' Returns the inner bounding box of the widget

        The inner bounding box delimitates the available space for children.
        '''
        return self._root_x, self._root_y, self._width, self._height
    
    # ___________________________________________________________ on_mouse_press
    def on_mouse_press(self, x, y, button, modifiers):
        ''' Handles on_mouse_press events
        
        :Parameters:
            `x` : float
                X coordinate.
            `y` : float
                Y coordinate.
            `button` : int
                Button identifier.
            `modifiers` : int
                Button modifiers.
        '''
        for i in self._elements:
          if hasattr(self._elements[i], 'on_mouse_press'):
            self._elements[i].on_mouse_press(x - self.x, y - self.y, button, modifiers)

    # ____________________________________________________________ on_mouse_drag
    def on_mouse_drag(self, x, y, dx, dy, button, modifiers):
      ''' Handles on_mouse_drag events
      
      :Parameters:
          `x` : float
              X coordinate.
          `y` : float
              Y coordinate.
          `dx` : float
              X deplacement.
          `dy` : float
              Y deplacement.
          `button` : int
              Button identifier.
          `modifiers` : int
              Button modifiers.
      '''
      for i in self._elements:
        if hasattr(self._elements[i], 'on_mouse_drag'):
          self._elements[i].on_mouse_drag(x - self.x, y - self.y, dx, dy, button, modifiers)
          
    # __________________________________________________________ on_mouse_motion
    def on_mouse_motion(self, x, y, dx, dy):
      ''' Handles on_mouse_motion events
      
      :Parameters:
          `x` : float
              X coordinate.
          `y` : float
              Y coordinate.
          `dx` : float
              X deplacement.
          `dy` : float
              Y deplacement.
      '''
      for i in self._elements:
        if hasattr(self._elements[i], 'on_mouse_motion'):
          self._elements[i].on_mouse_motion(x - self.x, y - self.y, dx, dy)

    # _________________________________________________________ on_mouse_release
    def on_mouse_release(self, x, y, button, modifiers):
      ''' Handles on_mouse_release events
      
      :Parameters:
          `x` : float
              X coordinate.
          `y` : float
              Y coordinate.
          `button` : int
              Button identifier.
          `modifiers` : int
              Button modifiers.
      '''
      for i in self._elements:
        if hasattr(self._elements[i], 'on_mouse_release'):
          self._elements[i].on_mouse_release(x - self.x, y - self.y, button, modifiers)

    # _________________________________________________________________ hit_test
    def hit_test(self,x,y):
        ''' Return True if x and y are inside the Widget
        
        :Parameters:
            `x` : float
                X coordinate.
            `y` : float
                Y coordinate.
        '''
        return ((self._root_x <= x <= (self._root_x+self._width)) and
                (self._root_y <= y <= (self._root_y+self._height)))

    # _____________________________________________________________________ on_draw
    def on_draw(self):
      ''' Handles on_draw events
      '''
      if not self._hidden:
        glTranslatef(self._root_x, self._root_y, self._root_z)
        for key in self._elements.keys():
            self._elements[key].draw()
        glTranslatef(-self._root_x, -self._root_y, -self._root_z)
        
    # _____________________________________________________________________ on_draw
    def draw(self):
      self.on_draw()
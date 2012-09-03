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
'''Defines a set of basic 2D shapes.

All shapes have:
----------------

- a position in 2D space
- a dimension in 2D space
- a x alignment ('left', 'center' or 'right')
- a y alignment ('top', 'center' or 'bottom')
- background color(s)
- background texture
- foreground color(s) (for the one pixel border)

Display Model:
--------------

Any shape is defined by the x, y, width and height attributes.  Borders are
drawn on the inside of the shape as a single pixel line in the specified border
color(s). Foreground or background color can be specified as a single tuple of 4
floats for uniform color, 2 tuples of 4 floats for radial color patterns (going
from inner to outer) or 4 tuples of 4 floats for an interpolated pattern between
the four corners. Note that the radial pattern does not work for triangle or
rectangle.

Available shapes:
-----------------

- Rectangle (with round corners or not)
- Ellipse (circle if width == height)
- Triangle
- Cross (with any number of branches)
- Star (with any number of branches)


Example usage:
--------------
 
   rectangle = Rectangle(x=100,y=100,width=100,height=100,radius=10)

   @window.event
   def on_draw():
       window.clear()
       rectangle.draw()

   @window.event
   def on_mouse_press(x,y,button,modifiers):
      if rectangle.hit_test(x,y):
         print 'Hit'

:requires: pyglet 1.1
'''
__docformat__ = 'restructuredtext'
__version__ = '1.0'

from rectangle import Rectangle
from triangle import Triangle
from ellipse import Ellipse
from cross import Cross
from star import Star


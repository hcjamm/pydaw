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
import math
import pyglet
from pyglet.gl import *
from shape import Shape

class Triangle(Shape):
    ''' Triangle shape.

    This is a class for rendering a triangle.
    '''

    # _________________________________________________________________ __init__
    def __init__(self, direction='up', *args, **kwargs):
        '''Create an oriented triangle.

        :Parameters:
            `direction` : str
               The triangle is oriented relative to its center to this property,
               which must be one of the alignment constants `left`, `right`,
               `up` or `down`.
        '''

        self._direction = direction
        Shape.__init__(self,*args, **kwargs)
        self._update_position()
        self._update_shape()

    # __________________________________________________________________ __str__
    def __str__(self):
        s='<Triangle %dx%d+%d+%d>' % (self._width,self._height,self._x,self._y)
        return s

    # ________________________________________________________________ direction
    def _get_direction(self):
        return self._direction
    def _set_direction(self, direction):
        self._direction = direction
        self._update_shape()
    direction = property(_get_direction, _set_direction,
        doc='''Direction of triangle.

        The triangle is oriented relative to its center to this property,
        which must be one of the alignment constants `left`, `right`, `up`
        or `down`.

        :type: str
        ''')

    # ________________________________________________________ generate_vertices
    def generate_vertices(self):
        ''' '''

        x,y,w,h = 0,0,self._width-1,self._height-1
        if self._direction == 'left':
            vertices = [(x,y), (x,y+h), (x+w,y+h/2)]
        elif self._direction == 'right':
            vertices = [(x+w,y), (x+w,y+h), (x,y+h/2)]
        elif self._direction == 'down':
            vertices = [(x,y+h), (x+w,y+h), (x+w/2,y)]
        else:
            vertices = [(x,y), (x+w,y), (x+w/2,y+h)]
        return vertices, vertices



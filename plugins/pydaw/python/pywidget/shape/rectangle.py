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
from shape import Shape, arc_circle


class Rectangle(Shape):
    ''' Round rectangle shape.

    This is a class for rendering a round rectangle shape.
    '''

    # _________________________________________________________________ __init__
    def __init__(self, radius=0, *args, **kwargs):
        '''Create an (optionable) round rectangle.

        :Parameters:        
            `radius` : int or tuple of 4 int
                Radius of corners.
        '''
        self._radius = radius
        Shape.__init__(self,*args,**kwargs)
        self._update_position()
        self._update_shape()

    # __________________________________________________________________ __str__
    def __str__(self):
        s='<Rectangle %dx%d+%d+%d>' % (self._width,self._height,self._x,self._y)
        return s

    # ___________________________________________________________________ radius
    def _get_radius(self):
        return self._radius
    def _set_radius(self, radius):
        self._radius = radius
        self._update_shape()
    radius = property(_get_radius, _set_radius,
        doc='''Radius of corner(s)

        :type: int or tuple of 4 int
        ''')

    # ________________________________________________________ generate_vertices
    def generate_vertices(self):
        ''' '''

        x,y,w,h = 0,0,self._width-1,self._height-1
        if type(self._radius) in [tuple,list]:
            r = self._radius
        else:
            r = [self._radius,]*4
        vertices = []
        if r[0] != 0:
            vertices += (x,y+r[0]),
            vertices += arc_circle (x+r[0],y+r[0],r[0],r[0],-180,-270)[1:-1]
            vertices += (x+r[0], y),
        else:
            vertices += (x, y),
        if r[1] > 0:
            vertices += (x+w-r[1], y),
            vertices += arc_circle (x+w-r[1],y+r[1],r[1],r[1],90,0)[1:-1]
            vertices += (x+w, y+r[1]),
        else:
            vertices += (x+w, y),
        if r[2] > 0:
            vertices += (x+w, y+h-r[2]),
            vertices += arc_circle (x+w-r[2],y+h-r[2],r[2],r[2],0,-90)[1:-1]
            vertices += (x+w-r[2], y+h),
        else:
            vertices += (x+w, y+h),
        if r[3] > 0:
            vertices += (x+w-r[2], y+h),
            vertices += arc_circle (x+r[3],y+h-r[3],r[3],r[3],-90,-180)[1:-1]
            vertices += (x, y+h-r[2]),
        else:
            vertices += (x, y+h),
        return vertices, vertices

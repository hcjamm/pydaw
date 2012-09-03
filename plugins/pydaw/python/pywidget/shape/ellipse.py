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


class Ellipse(Shape):
    ''' Ellipse shape.

    This is a class for rendering an ellipse.
    '''

    # _________________________________________________________________ __init__
    def __init__(self, theta1=0, theta2=360, *args, **kwargs):
        '''Create an ellipse '''

        self._theta1 = theta1
        self._theta2 = theta2
        Shape.__init__(self, *args, **kwargs)
        self._fill_mode = GL_TRIANGLES
        self._line_mode = GL_LINE_LOOP
        self._update_position()
        self._update_shape()

    # __________________________________________________________________ __str__
    def __str__(self):
        s = '<Ellipse %dx%d+%d+%d>' % (self._width,self._height,self._x,self._y)
        return s

    # ___________________________________________________________________ theta1
    def _get_theta1(self):
        return self._theta1
    def _set_theta1(self, theta1):
        self._theta1 = theta1
        self._update_shape()
    theta1 = property(_get_theta1, _set_theta1,
        doc='''Starting angle in degrees

        :type: float
        ''')

    # ___________________________________________________________________ theta2
    def _get_theta2(self):
        return self._theta2
    def _set_theta2(self, theta2):
        self._theta2 = theta2
        self._update_shape()
    theta2 = property(_get_theta2, _set_theta2,
        doc='''Ending angle in degrees

        :type: float
        ''')

    # ________________________________________________________ generate_vertices
    def generate_vertices(self):
        ''' '''

        x,y,w,h = 0,0,self._width-1,self._height-1
        vertices = arc_circle (x+w/2,y+h/2,w/2,h/2,self._theta1,self._theta2,5)
        v = []
        for i in range(len(vertices)-1):
            v += [vertices[i],]
            v += [vertices[i+1],]
            v += [(w/2,h/2),]
        if math.fmod(self._theta1,360) != math.fmod(self._theta2,360):
            vertices += [(w/2,h/2),]
        return vertices, v

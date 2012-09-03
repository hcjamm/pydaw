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


class Cross(Shape):
    ''' Cross shape.

    This is a class for rendering a cross.
    '''

    def __init__(self, thickness=0.5, branches=5, *args, **kwargs):
        '''Create a cross.

        :Parameters:
            `thickness` : float
                Thickness of the cross
            `branches` : int
                Number of branches
        '''
        self._thickness = thickness
        self._branches = branches
        Shape.__init__(self, *args, **kwargs)
        self._fill_mode = GL_QUADS
        self._update_position()
        self._update_shape()

    def __str__(self):
        s = '<Cross %dx%d+%d+%d>' % (self._width,self._height,self._x, self._y)
        return s

    def _get_branches(self):
        return self._branches
    def _set_branches(self, branches):
        self._branches = branches
        self._update_shape()
    branches = property(_get_branches, _set_branches,
        doc='''Number of branches of the cross.

        :type: int 
        ''')

    def _get_thickness(self):
        return self._thickness
    def _set_thickness(self, thickness):
        self._thickness = thickness
        self._update_shape()
    thickness = property(_get_thickness, _set_thickness,
        doc='''Thickness of the cross.

        :type: float
        ''')

    def generate_vertices(self):
        ''' '''

        x,y,w,h = 0,0,self._width-1,self._height-1
        b = self._branches
        t = self._thickness
        foreground_vertices = []
        for i in range(b):
            a1 = (i+0.0)*2*math.pi/b-math.pi/2
            a2 = (i+0.5)*2*math.pi/b-math.pi/2
            a3 = (i+1.0)*2*math.pi/b-math.pi/2
            dx = (math.cos(a1)*w/2*t-math.cos(a3)*w/2*t)/2
            dy = (math.sin(a1)*h/2*t-math.sin(a3)*h/2*t)/2
            foreground_vertices += [(w/2+math.cos(a1)*w/2*t,
                                     h/2+math.sin(a1)*h/2*t),
                                    (w/2+math.cos(a2)*w/2+dx,
                                     h/2+math.sin(a2)*h/2+dy),
                                    (w/2+math.cos(a2)*w/2-dx,
                                     h/2+math.sin(a2)*h/2-dy),
                                    (w/2+math.cos(a3)*w/2*t,
                                     h/2+math.sin(a3)*h/2*t)]
        background_vertices = [v for v in foreground_vertices]
        for i in range(b):
            a1 = (i+0.0)*2*math.pi/b-math.pi/2
            a2 = (i+0.5)*2*math.pi/b-math.pi/2
            a3 = (i+1.0)*2*math.pi/b-math.pi/2
            background_vertices += [(w/2+math.cos(a1)*w/2*t,
                                     h/2+math.sin(a1)*h/2*t),
                                    (w/2, h/2),
                                    (w/2, h/2),
                                    (w/2+math.cos(a3)*w/2*t,
                                     h/2+math.sin(a3)*h/2*t)]
        return foreground_vertices, background_vertices


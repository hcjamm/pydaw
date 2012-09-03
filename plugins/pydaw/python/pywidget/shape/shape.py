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
from pyglet import gl


# ------------------------------------------------------------------------------
def arc_circle(x, y, r1, r2, theta1, theta2, dtheta=10):
    ''' Return a list of vertices for the specified arc circle '''

    vertices = []
    if theta2 > theta1:
        while (theta1 <= theta2):
             theta_rad = theta1*math.pi/180
             vertices.append((x+r1*math.cos(theta_rad),
                              y-r2*math.sin(theta_rad)))
             theta1 += abs(dtheta)
    else:
        while (theta1 >= theta2):
             theta_rad = theta1*math.pi/180
             vertices.append((x+r1*math.cos(theta_rad),
                              y-r2*math.sin(theta_rad)))
             theta1 -= abs(dtheta)
    return vertices



# ------------------------------------------------------------------------------
class Shape(object):
    ''' Abstract shape.

    This is the common abstract class for all shape.
    '''

    # _________________________________________________________________ __init__
    def __init__(self, x=0, y=0, z=0, width=100, height=100,
                 foreground=(1.0,1.0,1.0,1.0), background=(1.0,1.0,1.0,0.25),
                 anchor_x='left', anchor_y='bottom', rotation=0,
                 texture = None, *args, **kwargs):
        '''Create a displayable shape.

        :Parameters:
            `x` : float
                X coordinate of the left edge of the shape.
            `y` : float
                Y coordinate of the top edge of the shape.
            `z` : float
                Z coordinate of the shape plane.
            `width` : int
                Width of the shape.
            `height` : int
                Height of the shape.
            `rotation` : float
                Rotation (degrees) of the shape around center.
            `foreground` : 4-tuple of float or 4x4-tuples of float
                Color(s) to render the shape border in. Alpha values can be
                specified in the fourth component.
            `background` : 4-tuple of float or 4x4-tuples of float
                Color(s) to render the shape interior in. Alpha values can be
                specified in the fourth component.
            `anchor_x` : str
                Horizontal alignment of the shape.
                See `Shape.anchor_x` for details.
            `anchor_y` : str
                Vertical alignment of the shape.
                See `Shape.anchor_y` for details.
        '''

        self._background_list = pyglet.graphics.vertex_list(1,'v2f','c4f','t2f')
        self._foreground_list = pyglet.graphics.vertex_list(1,'v2f','c4f')
        self._fill_mode = GL_POLYGON
        self._line_mode = GL_LINE_LOOP
        self._texture = texture
        self._x = x
        self._y = y
        self._z = z
        self._root_x = 0
        self._root_y = 0
        self._root_z = 0
        self._width = width
        self._height = height
        self._rotation = rotation
        self._foreground = foreground
        self._background = background
        self._anchor_x = anchor_x
        self._anchor_y = anchor_y

    # __________________________________________________________________ __str__
    def __str__(self):
        s = '<Shape %dx%d+%d+%d>' % (self._width,self._height,self._x, self._y)
        return s

    # ________________________________________________________________________ x
    def _get_x(self):
        return self._x
    def _set_x(self, x):
        self._x = x
        self._update_position()
    x = property(_get_x, _set_x,
        doc='''Y coordinate of the shape.

        :type: int
        ''')

    # ________________________________________________________________________ y
    def _get_y(self):
        return self._y
    def _set_y(self, y):
        self._y = y
        self._update_position()
    y = property(_get_y, _set_y,
        doc='''Y coordinate of the shape.

        :type: int
        ''')

    # ________________________________________________________________________ z
    def _get_z(self):
        return self._z
    def _set_z(self, z):
        self._z = z
        self._update_position()
    z = property(_get_z, _set_z,
        doc='''Z coordinate of the shape.

        :type: int
        ''')
    
    # ____________________________________________________________________ width
    def _get_width(self):
        return self._width
    def _set_width(self, width):
        self._width = width
        self._update_shape()
    width = property(_get_width, _set_width, 
        doc='''Width of the shape.

        :type: int
        ''')

    # ___________________________________________________________________ height
    def _get_height(self):
        return self._height
    def _set_height(self, height):
        self._height = height
        self._update_shape()
    height = property(_get_height, _set_height,
        doc='''Height of the shape.
        
        :type: int
        ''')

    # _______________________________________________________________ foreground
    def _get_foreground(self):
        return self._foreground
    def _set_foreground(self, color):
        self._foreground = color
        self._update_shape()
    foreground = property(_get_foreground, _set_foreground,
         doc='''Color to render the shape border in.

         Alpha values can be specified in the fourth component.

         :type: tuple
         ''')

    # _______________________________________________________________ background
    def _get_background(self):
        return self._background
    def _set_background(self, color):
        self._background = color
        self._update_shape()
    background = property(_get_background, _set_background,
        doc='''Color to render the shape interior in.

        Alpha values can be specified in the fourth component.

        :type: tuple
        ''')

    # _________________________________________________________________ anchor_x
    def _get_anchor_x(self):
        return self._anchor_x
    def _set_anchor_x(self, anchor_x):
        self._anchor_x = anchor_x
        self._update_position()
    anchor_x = property(_get_anchor_x, _set_anchor_x,
        doc='''Horizontal alignment of the shape.

        The shape is positioned relative to `x` and `width` according to this
        property, which must be one of the alignment constants `LEFT`,
        `CENTER` or `RIGHT`.

        :type: str
        ''')

    # _________________________________________________________________ anchor_y
    def _get_anchor_y(self):
        return self._anchor_y
    def _set_anchor_y(self, anchor_y):
        self._anchor_y = anchor_y
        self._update_position()
    anchor_y = property(_get_anchor_y, _set_anchor_y,
        doc='''Vertical alignment of the shape.

        The shape is positioned relative to `y` according to this property,
        which must be one of the alignment constants `BOTTOM`, `CENTER`
        or `TOP`.

        :type: str
        ''')

    # _________________________________________________________________ rotation
    def _get_rotation(self):
        return self._rotation
    def _set_rotation(self, color):
        self._rotation = color
    rotation = property(_get_rotation, _set_rotation,
         doc='''Rotation (degrees) around center  of the shape.

         :type: float
         ''')

    # __________________________________________________________________ texture
    def _get_texture(self):
        return self._texture
    def _set_texture(self, texture):
        self._texture = texture
    texture = property(_get_texture, _set_texture,
        doc='''Texture to render the shape interior 

        :type: pyglet texture object
        ''')

    # _________________________________________________________ _update_position
    def _update_position(self):
        ''' '''

        if self.anchor_x == 'center':
            self._root_x = self.x-self.width/2
        elif self.anchor_x == 'right':
            self._root_x = self.x-self.width
        else:
            self._root_x = self.x

        if self.anchor_y == 'center':
            self._root_y = self.y-self.height/2
        elif self.anchor_y == 'top':
            self._root_y = self.y-self.height
        else:
            self._root_y = self.y

    # ____________________________________________________________ _update_shape
    def _update_shape(self):
        ''' '''

        def interpolate(x,y,c0,c1,c2=None,c3=None):
            c = [0,0,0,0]
            if not c2:
                for i in range(4):
                    d = math.sqrt((x-0.5)*(x-0.5)+(y-0.5)*(y-0.5))*math.sqrt(2)
                    c[i] = d*c0[i]+(1-d)*c1[i]
            else:
                for i in range(4):
                    c[i] = x*(y*c0[i]+(1-y)*c3[i])+(1-x)*(y*c1[i]+(1-y)*c2[i])
            return c

        foreground_vertices, background_vertices = self.generate_vertices()
        x,y,w,h = 0, 0, float(self._width-1), float(self._height-1)
        if type(self._foreground[0]) in [list,tuple]:
            if len(self._foreground) == 2:
                f0,f1 = self._foreground
                f2,f3 = None,None
            else:
                f0,f1,f2,f3 = self._foreground
        else:
            f0=f1=f2=f3= self._foreground
        if type(self._background[0]) in [list,tuple]:
            if len(self._background) == 2:
                b0,b1 = self._background
                b2,b3 = None,None
            else:
                b0,b1,b2,b3 = self._background
        else:
            b0=b1=b2=b3 = self._background
        self._background_list.resize(len(background_vertices))
        self._foreground_list.resize(len(foreground_vertices))
        c,p = [],[]
        for v in foreground_vertices:
            p += v
            c += interpolate( (v[0]-x)/w, (v[1]-y)/h, f0,f1,f2,f3)
        self._foreground_list.vertices = p
        self._foreground_list.colors = c
        c,p,t = [],[],[]
        for v in background_vertices:
            p += v
            t += [(v[0]-x)/w, (v[1]-y)/h,]
            c += interpolate( (v[0]-x)/w, (v[1]-y)/h, b0,b1,b2,b3)
        self._background_list.vertices = p
        self._background_list.tex_coords = t
        self._background_list.colors = c

    # _________________________________________________________________ hit_test
    def hit_test(self, x, y):
        ''' Return whether (x,y) is inside shape

        The hit test is done using bounding box
        '''

        return ((self._root_x <= x <= (self._root_x+self._width)) and
                (self._root_y <= y <= (self._root_y+self._height)))

    # ____________________________________________________________________ draw
    def draw(self):
        ''' Draw the shape '''

        glDisable(GL_TEXTURE_2D)
        if self._texture is not None:
            glEnable(GL_TEXTURE_2D)
            glBindTexture(GL_TEXTURE_2D, self._texture.id)
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glDepthMask(GL_FALSE)
        glTranslatef(self._root_x+0.5, self._root_y+0.5, self.z)
        glRotatef(self._rotation,0,0,1)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonOffset (1, 1)
        self._background_list.draw(self._fill_mode)
        glDisable(GL_POLYGON_OFFSET_FILL)
        glDisable(GL_TEXTURE_2D)
        glEnable(GL_LINE_SMOOTH)
        glLineWidth (1.0)
        self._foreground_list.draw(self._line_mode)
        glDisable(GL_LINE_SMOOTH)
        glDepthMask(GL_TRUE)
        glRotatef(-self._rotation,0,0,1)
        glTranslatef(-self._root_x-0.5, -self._root_y-0.5, -self.z)



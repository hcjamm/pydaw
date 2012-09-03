#!/usr/bin/env python
#
# PyDAW is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License
#
# PyDAW is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PyDAW.  If not, see <http://www.gnu.org/licenses/>

__author__ = "Jeff Hubbard <jhubbard651@users.sf.net>"
__version__ = "0.1"
__date__ = "Date: 2012/8/27"
__copyright__ = "Copyright (c) 2012 Jeff Hubbard"
__license__ = "GPL"

from pyglet import window
from pyglet import clock
from pyglet import font

import helper

##img = pyglet.resource.image(get_image_dir() + 'midi_item.png')
##img.anchor_x = img.width/2
##img.anchor_y = img.width/2
##self.sprite = pyglet.sprite.Sprite(img, batch = parent.batch, group =
##parent.visualeffects)
##self.sprite.x = pos[0]
##self.sprite.y = pos[1]
##self.sprite.scale = 0.5

x_velocity = 1

class pydaw_main_window(window.Window):

    def __init__(self, *args, **kwargs):
        self.max_midi_items = 240
        #Let all of the standard stuff pass through
        window.Window.__init__(self, 1200, 720, 'PyDAW - LibModSynth', True)
        #self.set_mouse_visible(False)
        self.temp_pos = self.height
        self.midi_item_height = 128
        self.midi_item_width = 128
        self.init_sprites()

    def init_sprites(self):
        self.midi_items = []
        self.midi_item_image = helper.load_image("midi_item.png")
        self.midi_item_image.width = self.midi_item_width
        self.midi_item_image.height = self.midi_item_height

    def main_loop(self):

        #Create a font for our FPS clock
        ft = font.load('Arial', 28)
        #The pyglet.font.Text object to display the FPS
        fps_text = font.Text(ft, y=10)

        #Schedule the midi_item creation
        clock.schedule_interval(self.create_midi_item, 3.5)
        #clock.set_fps_limit(60)

        while not self.has_exit:
            self.dispatch_events()
            self.clear()

            self.update()
            self.draw()

            #Tick the clock
            clock.tick()
            #Gets fps and draw it
            fps_text.text = ("fps: %d") % (clock.get_fps())
            fps_text.draw()
            self.flip()

    def update(self):

        to_remove = []
        for sprite in self.midi_items:
            sprite.update()
            #Is it dead?
            if (sprite.dead):
                to_remove.append(sprite)
        #Remove dead sprites
        for sprite in to_remove:
            self.midi_items.remove(sprite)

    def draw(self):
        for sprite in self.midi_items:
            sprite.draw()

    def create_midi_item(self, interval):
        self.temp_pos = self.height
        while (self.temp_pos >= self.midi_item_height):
            self.midi_items.append(midi_item(self.midi_item_image, x=self.width , y=self.temp_pos))
            self.temp_pos -= self.midi_item_height
    """******************************************
    Event Handlers
    *********************************************"""
    def on_mouse_motion(self, x, y, dx, dy):
        pass

    def on_mouse_drag(self, x, y, dx, dy, buttons, modifiers):
        pass

    def on_mouse_press(self, x, y, button, modifiers):
        if (button == 1):
            pass

class Sprite(object):

    def __get_left(self):
        return self.x
    left = property(__get_left)

    def __get_right(self):
        return self.x + self.image.width
    right = property(__get_right)

    def __get_top(self):
        return self.y + self.image.height
    top = property(__get_top)

    def __get_bottom(self):
        return self.y
    bottom = property(__get_bottom)

    def __init__(self, image_file, image_data=None, **kwargs):

        #init standard variables
        self.image_file = image_file
        if (image_data is None):
            self.image = helper.load_image(image_file)
        else:
            self.image = image_data

        self.image.scale = .1
        self.x = 0
        self.y = 0
        self.dead = False
        #Update the dict if they sent in any keywords
        self.__dict__.update(kwargs)

    def draw(self):
        self.image.blit(self.x, self.y)

    def update(self):
        pass

class midi_item(Sprite):

    def __init__(self, image_data, **kwargs):
        self.x_velocity = 1
        Sprite.__init__(self, "", image_data, **kwargs)

    def update(self):
        self.x -= x_velocity
        #We've gone past the edge of the screen
        if (self.right < 0):
            self.dead = True

if __name__ == "__main__":
    # Someone is launching this directly
    pydaw = pydaw_main_window()
    pydaw.main_loop()


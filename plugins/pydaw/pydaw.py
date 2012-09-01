#!/usr/bin/env python
#
#
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

class pydaw_main_window(window.Window):
    #midi_item_height = 20
    #midi_item_width = 20

    def __init__(self, *args, **kwargs):
        self.max_midi_items = 120
        self.temp_pos = 0
        #Let all of the standard stuff pass through
        window.Window.__init__(self, *args, **kwargs)
        #self.set_mouse_visible(False)

        self.midi_item_height = 75  #16/self.height
        self.midi_item_width = 60
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
        clock.schedule_interval(self.create_midi_item, 0.05)
        clock.set_fps_limit(60)

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
        if (len(self.midi_items) < self.max_midi_items):
            self.temp_pos += self.midi_item_width
            if(self.temp_pos >= self.width):
                self.temp_pos = 0
            self.midi_items.append(midi_item(self.midi_item_image
                , x=self.width , y=self.temp_pos))

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
        self.x = 0
        self.y = 0
        self.dead = False
        #Update the dict if they sent in any keywords
        self.__dict__.update(kwargs)

    def draw(self):
        self.image.blit(self.x, self.y)

    def update(self):
        pass

    def intersect(self, sprite):
        """Do the two sprites intersect?
        @param sprite - Sprite - The Sprite to test
        """
        return not ((self.left > sprite.right)
            or (self.right < sprite.left)
            or (self.top < sprite.bottom)
            or (self.bottom > sprite.top))

    def collide(self, sprite_list):
        """Determing ther are collisions with this
        sprite and the list of sprites
        @param sprite_list - A list of sprites
        @returns list - List of collisions"""

        lst_return = []
        for sprite in sprite_list:
            if (self.intersect(sprite)):
                lst_return.append(sprite)
        return lst_return

    def collide_once(self, sprite_list):
        """Determine if there is at least one
        collision between this sprite and the list
        @param sprite_list - A list of sprites
        @returns - None - No Collision, or the first
        sprite to collide
        """
        for sprite in sprite_list:
            if (self.intersect(sprite)):
                return sprite
        return None

class midi_item(Sprite):

    def __init__(self, image_data, **kwargs):
        self.y_velocity = 1
        self.set_x_velocity()
        self.x_move_count = 0
        self.x_velocity
        Sprite.__init__(self, "", image_data, **kwargs)

    def update(self):
        self.x -= self.y_velocity
        self.y += self.x_velocity#random.randint(-3,3)
        self.x_move_count += 1
        #Have we gone beneath the botton of the screen?
        if (self.x < 0):
            self.dead = True

        if (self.x_move_count >=30):
            self.x_move_count = 0
            self.set_x_velocity()

    def set_x_velocity(self):
        self.x_velocity = 0 #random.randint(-3,3)

if __name__ == "__main__":
    # Someone is launching this directly
    space = pydaw_main_window()
    space.main_loop()


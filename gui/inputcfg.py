#!/usr/bin/env python

import pygtk
pygtk.require('2.0')
import gtk
import cairo
import goocanvas

class InputCfg:
    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def click(self, item, target_item, event, *rest):
        print item, target_item, event, rest

    def __init__(self):
        # create a new window
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)
    
        # self.drawing_area = gtk.DrawingArea()
        # self.drawing_area.set_size_request(512, 512)
        # self.window.add(self.drawing_area)
        # self.drawing_area.show()

        self.canvas = goocanvas.Canvas()
        self.canvas.set_size_request(512, 512)
        self.window.add(self.canvas)
        self.canvas.show()

        self.window.show()


        root = self.canvas.get_root_item()
        ellipse = goocanvas.Ellipse(parent=root,
                                 center_x=300, center_y=300,
                                 radius_x=100, radius_y=100,
                                 stroke_color="black",
                                 fill_color="lightgrey")
        rect = goocanvas.Rect(parent=root,
                                 antialias=cairo.ANTIALIAS_SUBPIXEL,
                              x=100, y=100,
                              radius_x=10, radius_y=10,
                                 width=200, height=100,
                                 stroke_color="black",
                                 fill_color="lightgrey")
        text = goocanvas.Text(parent=root,
                              antialias=cairo.ANTIALIAS_SUBPIXEL,
                              text="FoobarItem Really Long Description",
                              width=180,
                              font="serif bold 10",
                              x=110,y=110)

        rect.connect("button-press-event", self.click)

    def main(self):
        gtk.main()

if __name__ == "__main__":
    inputcfg = InputCfg()
    inputcfg.main()

# EOF #

#!/usr/bin/env python

import pygtk
pygtk.require('2.0')
import gtk
import cairo
import goocanvas
import pango

class Control:
    def __init__(self, root):
        self.title = "EvDev"
        self.in_ports = []
        self.out_ports = []
        
        self.group = goocanvas.Group(parent=root)

        self.mainbox = goocanvas.Rect(parent=self.group,
                                      radius_x=10, radius_y=10,
                                      width=200, 
                                      stroke_color="black",
                                      fill_color="lightgrey",
                                      line_width=1)
        self.titlebox = goocanvas.Rect(parent=self.group,
                                       width=200-10, height=30-10,
                                       radius_x=5, radius_y=5,
                                       stroke_color="black",
                                       fill_color="black")
        self.title = goocanvas.Text(parent=root,
                                    antialias=cairo.ANTIALIAS_SUBPIXEL,
                                    text="EvDev",
                                    width=180,
                                    font="sans bold 12",
                                    x=100,y=5,
                                    anchor=gtk.ANCHOR_NORTH,
                                    alignment=pango.ALIGN_CENTER,
                                    fill_color="white")

        self.add_in_port("btn0")
        self.add_in_port("btn1")

        self.add_out_port("abs1")
        self.add_out_port("abs2")
        self.add_out_port("abs3")
        self.add_out_port("abs4")
        self.add_out_port("abs5")
        self.add_out_port("abs6")
        self.set_pos(100, 100)

    def layout(self):
        height = 40 + max(len(self.in_ports), len(self.out_ports)) * 20
        self.mainbox.set_properties(height=height)

    def add_in_port(self, name):
        in_port = goocanvas.Ellipse(parent=self.group,
                                    center_x=0, center_y=40,
                                    radius_x=5, radius_y=5,
                                    stroke_color="black",
                                    line_width=1,
                                    fill_color="red")
        in_port_title = goocanvas.Text(parent=self.group,
                                    antialias=cairo.ANTIALIAS_SUBPIXEL,
                                    text=name,
                                    font="sans 9",
                                    x=10,y=0,
                                    anchor=gtk.ANCHOR_WEST,
                                    alignment=pango.ALIGN_LEFT,
                                    fill_color="black")
        self.in_ports.append((in_port, in_port_title))
        self.layout()

    def add_out_port(self, name):
        out_port = goocanvas.Ellipse(parent=self.group,
                                    center_x=0, center_y=40,
                                    radius_x=5, radius_y=5,
                                    stroke_color="black",
                                    line_width=1,
                                    fill_color="green")
        out_port_title = goocanvas.Text(parent=self.group,
                                    antialias=cairo.ANTIALIAS_SUBPIXEL,
                                    text=name,
                                    font="sans 9",
                                    x=190,y=0,
                                    anchor=gtk.ANCHOR_EAST,
                                    alignment=pango.ALIGN_RIGHT,
                                    fill_color="black")
        self.out_ports.append((out_port, out_port_title))
        self.layout()

    def set_pos(self, x, y):
        self.mainbox.set_properties(x=x, y=y)
        self.titlebox.set_properties(x=x+5, y=y+5)
        self.title.set_properties(x=x+100, y=y+5)
        for i in range(len(self.in_ports)):
            self.in_ports[i][0].set_properties(center_x=x, center_y=y+40+20*i)
            self.in_ports[i][1].set_properties(x=x+10, y=y+40+20*i)
        for i in range(len(self.out_ports)):
            self.out_ports[i][0].set_properties(center_x=x+200, center_y=y+40+20*i)
            self.out_ports[i][1].set_properties(x=x+190, y=y+40+20*i)


class InputCfg:
    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def button_down(self, item, event):
        self.drag = True
        self.motion(item, event)

    def button_up(self, item, event):
        self.drag = False

    def motion(self, item, event):
        if self.drag:
            self.control.set_pos(event.x, event.y)

    def __init__(self):
        self.drag = False

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

        self.canvas.connect("button-press-event", self.button_down)
        self.canvas.connect("button-release-event", self.button_up)
        self.canvas.connect("motion-notify-event", self.motion)

        self.control = Control(root)

    def main(self):
        gtk.main()

if __name__ == "__main__":
    inputcfg = InputCfg()
    inputcfg.main()

# EOF #

#!/usr/bin/env python

import pygtk
pygtk.require('2.0')
import gtk
import cairo
import goocanvas
import pango

class Port:
    def __init__(self, isInPort, name, control):
        self.control  = control
        self.name     = name
        self.isInPort = isInPort
        self.ellipse = goocanvas.Ellipse(parent=control.get_port_parent(),
                                         center_y=40,
                                         radius_x=5, radius_y=5,
                                         stroke_color="black",
                                         line_width=1)
        
        self.text = goocanvas.Text(parent=control.get_port_parent(),
                                   antialias=cairo.ANTIALIAS_SUBPIXEL,
                                   text=name,
                                   font="sans 9",
                                   x=10,y=0,
                                   fill_color="black")

        if isInPort:
            self.ellipse.set_properties(fill_color="red")
            self.text.set_properties(anchor=gtk.ANCHOR_WEST,
                                     alignment=pango.ALIGN_LEFT)
        else:
            self.ellipse.set_properties(fill_color="green")
            self.text.set_properties(anchor=gtk.ANCHOR_EAST,
                                     alignment=pango.ALIGN_RIGHT)

        self.ellipse.connect("enter-notify-event", self.on_enter)
        self.ellipse.connect("leave-notify-event", self.on_leave)

    def on_enter(self, *rest):
        if self.isInPort:
            self.ellipse.set_properties(fill_color="white")
        else:
            self.ellipse.set_properties(fill_color="white")

    def on_leave(self, *rest):
        pass
        if self.isInPort:
            self.ellipse.set_properties(fill_color="red")
        else:
            self.ellipse.set_properties(fill_color="green")

    def set_pos(self, x, y):
        if self.isInPort:
            self.ellipse.set_properties(center_x=x, center_y=y)
            self.text.set_properties(x=x+10, y=y)
        else:
            self.ellipse.set_properties(center_x=x+self.control.get_width(), center_y=y)
            self.text.set_properties(x=x+self.control.get_width()-10, y=y)

class InPort(Port):
    def __init__(self, name, control):
        Port.__init__(self, True, name, control)

class OutPort(Port):
    def __init__(self, name, control):
        Port.__init__(self, False, name, control)

class Control:
    def __init__(self, name, root):
        self.title = name
        self.in_ports = []
        self.out_ports = []
        
        self.x = 0
        self.y = 0
        self.width  = 200
        self.height = 200

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
                                    text=self.title,
                                    width=180,
                                    font="sans bold 12",
                                    x=100,y=5,
                                    anchor=gtk.ANCHOR_NORTH,
                                    alignment=pango.ALIGN_CENTER,
                                    fill_color="white")
        self.layout()
        self.set_pos(100, 100)

    def get_port_parent(self):
        return self.group

    def get_width(self):
        return self.width

    def layout(self):
        height = 40 + max(len(self.in_ports), len(self.out_ports)) * 20
        self.mainbox.set_properties(height=height)

        self.mainbox.set_properties(x=self.x, y=self.y)
        self.titlebox.set_properties(x=self.x+5, y=self.y+5)
        self.title.set_properties(x=self.x+100, y=self.y+5)

        for i in range(len(self.in_ports)):
            self.in_ports[i].set_pos(self.x, self.y+40+20*i)
        for i in range(len(self.out_ports)):
            self.out_ports[i].set_pos(self.x, self.y+40+20*i)

    def add_in_port(self, name):
        self.in_ports.append(InPort(name, self))
        self.layout()

    def add_out_port(self, name):
        self.out_ports.append(OutPort(name, self))
        self.layout()

    def set_pos(self, x, y):
        self.x = x
        self.y = y
        self.layout()


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
        self.path.set_properties(data="M 100,100 C %d,100 %d,%d %d,%d" % ((100+event.x)/2,
                                                                          (100+event.x)/2,
                                                                          event.y,
                                                                          event.x,
                                                                          event.y))
        if self.drag:
            self.control1.set_pos(event.x, event.y)

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

        self.path = goocanvas.Path(parent=root, data="M 100,100 C 200,100 200,200 300,200")

        self.control1 = Control("EvDev", root)
        self.control1.add_out_port("abs1")
        self.control1.add_out_port("abs2")
        self.control1.add_out_port("abs3")
        self.control1.add_out_port("abs4")
        self.control1.add_out_port("abs5")
        self.control1.add_out_port("abs6")
        self.control1.set_pos(250, 10)

        self.control2 = Control("UInput", root)
        self.control2.add_in_port("btn0")
        self.control2.add_in_port("btn1")
        self.control2.add_out_port("abs5")
        self.control2.add_out_port("abs6")
        self.control2.set_pos(10, 10)

    def main(self):
        gtk.main()

if __name__ == "__main__":
    inputcfg = InputCfg()
    inputcfg.main()

# EOF #

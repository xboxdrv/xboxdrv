#!/usr/bin/env python

import pygtk
pygtk.require('2.0')
import gtk
import cairo
import goocanvas
import pango

menu_xml = """
<ui>
  <menubar name="Menubar">
    <menu name="FileMenu" action="FileMenuAction">
      <menuitem name="New" action="new" />
      <separator/>
      <menuitem name="Quit" action="quit" />
    </menu>

    <menu name="Controls" action="ControlsMenuAction"> 
      <menuitem name="AbsToBtn"  action="AbsToBtn" />
      <menuitem name="BtnToAbs"  action="BtnToAbs" />
    </menu>

    <menu name="Help"  action="HelpMenuAction">
      <menuitem name="Left"   action="about"/>
    </menu>
  </menubar>

  <toolbar action="Toolbar">
      <toolitem name="New" action="new" />
      <separator/>
      <toolitem name="Quit" action="quit" />
      <toolitem name="About" action="about" />
  </toolbar>

  <popup>
      <menuitem name="AbsToBtn"  action="AbsToBtn" />
      <menuitem name="BtnToAbs"  action="BtnToAbs" />      
  </popup>
</ui>
"""

class Port:
    def __init__(self, isInPort, name, control):
        self.control  = control
        self.name     = name
        self.isInPort = isInPort
        self.group = goocanvas.Group(parent=control.get_port_parent())

        self.rect = goocanvas.Rect(parent=self.group,
                                   width=50,
                                   height=18,
                                   radius_x=5, radius_y=5,
                                   line_width=1,
                                   fill_color="darkgrey")

        self.ellipse = goocanvas.Ellipse(parent=self.group,
                                         radius_x=5, radius_y=5,
                                         stroke_color="black",
                                         line_width=1)
        
        self.text = goocanvas.Text(parent=self.group,
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

        self.group.connect("button-press-event", self.on_button_press)
        self.group.connect("enter-notify-event", self.on_enter)
        self.group.connect("leave-notify-event", self.on_leave)

    def on_button_press(self, item, target_item, event):
        inputcfg.drag_start(self)

    def on_enter(self, *rest):
        self.is_active = True
        self.rect.set_properties(fill_color="lightgrey")
        if self.isInPort:
            self.ellipse.set_properties(fill_color="white")
        else:
            self.ellipse.set_properties(fill_color="white")

    def on_leave(self, *rest):
        self.is_active = False
        self.rect.set_properties(fill_color="darkgrey")
        if self.isInPort:
            self.ellipse.set_properties(fill_color="red")
        else:
            self.ellipse.set_properties(fill_color="green")

    def get_pos(self):
        return (self.x, self.y)

    def set_pos(self, x, y):
        self.y = y

        if self.isInPort:
            self.x = x
            self.ellipse.set_properties(center_x=x, center_y=y)
            self.text.set_properties(x=x+10, y=y)
            self.rect.set_properties(x=x, y=y-9)
        else:
            self.x = x+self.control.get_width()
            self.ellipse.set_properties(center_x=x+self.control.get_width(), center_y=y)
            self.text.set_properties(x=x+self.control.get_width()-10, y=y)
            self.rect.set_properties(x=x-50+self.control.get_width(), y=y-9)

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

        self.drag = None

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
                                       stroke_color=None,
                                       line_width = 0,
                                       fill_color="darkgray")
        self.title = goocanvas.Text(parent=self.group,
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

        self.group.connect("button-press-event", self.button_press)
        self.group.connect("button-release-event", self.button_release)
        self.group.connect("motion-notify-event", self.motion)
        # self.group.connect("enter-notify-event", self.on_enter)
        # self.group.connect("leave-notify-event", self.on_leave)

    def button_press(self, item, target_item, event):
        self.drag = (self.x - event.x, self.y - event.y)

    def motion(self, item, target_item, event):
        if self.drag:
            self.set_pos(event.x + self.drag[0], event.y + self.drag[1])
            inputcfg.layout()

    def on_enter(self, *rest):
        self.titlebox.set_properties(fill_color="black")
        
    def on_leave(self, *rest):
        self.titlebox.set_properties(fill_color="darkgrey")

    def button_release(self, *rest):
        self.drag = None

    def get_port_parent(self):
        return self.group.get_parent()

    def get_width(self):
        return self.width

    def layout(self):
        spacing = 26
        height = 45 + max(len(self.in_ports), len(self.out_ports)) * spacing
        self.mainbox.set_properties(height=height)

        self.mainbox.set_properties(x=self.x, y=self.y)
        self.titlebox.set_properties(x=self.x+5, y=self.y+5)
        self.title.set_properties(x=self.x+100, y=self.y+5)

        for i in range(len(self.in_ports)):
            self.in_ports[i].set_pos(self.x, self.y+45+spacing*i)
        for i in range(len(self.out_ports)):
            self.out_ports[i].set_pos(self.x, self.y+45+spacing*i)

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


class Connection:
    def __init__(self, portIn, portOut, root):
        self.portIn  = portIn
        self.portOut = portOut       
        
        self.group = goocanvas.Group(parent=root)
        self.path = goocanvas.Path(parent=self.group,
                                   line_width=2, stroke_color="black")
        self.ellipse_in = goocanvas.Ellipse(parent=self.group,
                                            radius_x=3, radius_y=3,
                                            fill_color="black",
                                            line_width=0)
        self.ellipse_out = goocanvas.Ellipse(parent=self.group,
                                             radius_x=3, radius_y=3,
                                             fill_color="black",
                                             line_width=0)
        # self.group.lower(None)
        self.layout()

    def layout(self):
        str = "M %(x1)d,%(y1)d C %(midx)d,%(y1)d %(midx)d,%(y2)d %(x2)d,%(y2)d" % \
            { 'x1'   : self.portIn.get_pos()[0],
              'y1'   : self.portIn.get_pos()[1],
              'midx' : (self.portIn.get_pos()[0] + self.portOut.get_pos()[0])/2,
              'x2'   : self.portOut.get_pos()[0],
              'y2'   : self.portOut.get_pos()[1] }
        self.path.set_properties(data=str)

        self.ellipse_in.set_properties(center_x = self.portIn.get_pos()[0], 
                                       center_y = self.portIn.get_pos()[1])
        self.ellipse_out.set_properties(center_x = self.portOut.get_pos()[0], 
                                        center_y = self.portOut.get_pos()[1])

class InputCfg:
    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def motion(self, item, event):
        if self.start_port:
            self.layout_with_mouse(event.x, event.y)

    def layout(self):
        for i in self.connections: # FIXME: Fugly, wrong place for this
            i.layout()

    def on_button_press(self, item, event):
        if event.button == 1:
            obj = self.canvas.get_item_at(event.x, event.y, False)
            if obj:
                print "Left Click:", obj
            else:
                print "Left Click: None"

        elif event.button == 3: # right click
            popup = self.uimanager.get_widget("/popup")
            popup.show_all()
            popup.popup(None, None, None, 3, 0)

            # popupMenu = gtk.Menu()
            # menuPopup1 = gtk.ImageMenuItem (gtk.STOCK_OPEN)
            # popupMenu.add(menuPopup1)
            # menuPopup2 = gtk.ImageMenuItem (gtk.STOCK_OK)
            # popupMenu.add(menuPopup2)
            # popupMenu.show_all()
            # popupMenu.popup(None, None, None, 1, 0)

    def on_button_release(self, item, event):
        pass

    def drag_start(self, port):
        if self.start_port:
            if (isinstance(port, InPort) and isinstance(self.start_port, InPort)) or \
                    (isinstance(port, OutPort) and isinstance(self.start_port, OutPort)):
                print "Error: Can't connect ports of the same type"
                self.path.set_properties(data="")
                self.start_port = None
            elif port.control == self.start_port.control:
                print "Error: Can't connect ports of the same control"
                self.path.set_properties(data="")
                self.start_port = None
            else:
                self.connections.append(Connection(self.start_port, port, self.root))
                self.path.set_properties(data="")
                self.start_port = None
        else:
            self.start_port = port

    def layout_with_mouse(self, x, y):
        if self.start_port:
            str = "M %(x1)d,%(y1)d C %(midx)d,%(y1)d %(midx)d,%(y2)d %(x2)d,%(y2)d" % \
                { 'x1'   : self.start_port.get_pos()[0],
                  'y1'   : self.start_port.get_pos()[1],
                  'midx' : (self.start_port.get_pos()[0] + x)/2,
                  'x2'   : x,
                  'y2'   : y }
            self.path.set_properties(data=str)

    def print_hello(self, *rest):
        print "Hello:", rest

    def on_scroll_motion(self, obj, event):
        if self.scrolling:
            print (self.scrolling[0] - event.x), \
                (self.scrolling[1] - event.y)

    def on_scroll_press(self, obj, event):
        if event.button == 2:
            self.scrolling = (event.x, event.y)

    def on_scroll_release(self, obj, event):
        if event.button == 2:
            self.scrolling = None

    def __init__(self):
        self.start_port  = None
        self.scrolling   = False
        self.connections = []

        # create a new window
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)

        self.window.set_title("InputDrv - Event Rerouter")
        self.window.set_size_request(800, 600)

        self.scrolled_win = gtk.ScrolledWindow()
        self.scrolled_win.set_shadow_type(gtk.SHADOW_IN)
        self.scrolled_win.show()

        self.canvas = goocanvas.Canvas()
        # self.canvas.set_size_request(256, 256)
        self.canvas.set_bounds(0, 0, 2048, 2048) # FIXME: Hook this up to resize thing

        
        self.main_vbox = gtk.VBox(False, 1)

        self.main_vbox.set_border_width(1)

        actiongroup = gtk.ActionGroup("name")
        actiongroup.add_actions([('FileMenuAction',     None, '_File',     None, '', None)])
        actiongroup.add_actions([('ControlsMenuAction', None, '_Controls', None, '', None)])
        actiongroup.add_actions([('HelpMenuAction',     None, '_Help',     None, '', None)])
        actiongroup.add_actions([('about',       gtk.STOCK_ABOUT, '_About',    None, 'About this Program', None)])
        actiongroup.add_actions([('new',         gtk.STOCK_NEW,   '_New',      None, 'New Configuration',  None)])
        actiongroup.add_actions([('quit',        gtk.STOCK_QUIT,  '_Quit me!', None, 'Quit the Program',   None)])

        actiongroup.add_actions([('AbsToBtn',       None, 'Insert AbsToBtn',     None, '', None)])
        actiongroup.add_actions([('BtnToAbs',       None, 'insert BtnToAbs',     None, '', None)])
        actiongroup.add_actions([('InvertButton',   None, 'insert InvertButton', None, '', None)])

        self.uimanager = gtk.UIManager()
        self.uimanager.insert_action_group(actiongroup, 0)
        self.uimanager.add_ui_from_string(menu_xml)

        self.menubar = self.uimanager.get_widget("/Menubar")
        self.toolbar = self.uimanager.get_widget("/Toolbar")

        self.statusbar = gtk.Statusbar()

        self.main_vbox.pack_start(self.menubar, False, True, 0)
        self.main_vbox.pack_start(self.toolbar, False, True, 0)
        self.main_vbox.add(self.scrolled_win)
        self.main_vbox.pack_start(self.statusbar, False, True, 0)

        self.scrolled_win.connect("button-press-event",   self.on_scroll_press)
        self.scrolled_win.connect("button-release-event", self.on_scroll_release)
        self.scrolled_win.connect("motion-notify-event",  self.on_scroll_motion)

        self.statusbar.push(0, "Hello World")

        self.window.add(self.main_vbox)

        self.toolbar.show()
        self.menubar.show()
        self.statusbar.show()

        self.scrolled_win.add(self.canvas)
        self.scrolled_win.show()

        vadj = self.scrolled_win.get_vadjustment()
        hadj = self.scrolled_win.get_hadjustment()
        
        self.canvas.show()

        self.statusbar.show()
        self.main_vbox.show()
        self.window.show()

        self.root = self.canvas.get_root_item()

        self.canvas.connect("motion-notify-event", self.motion)
        self.canvas.connect("button-press-event",   self.on_button_press)
        self.canvas.connect("button-release-event", self.on_button_release)

        self.path = goocanvas.Path(parent=self.root,
                                   pointer_events=0,
                                   line_width=2, 
                                   stroke_color_rgba=0x00000060)
        self.init_test_elements()

    def init_test_elements(self):
        self.control3 = Control("Xbox360 Gamepad", self.root)
        self.control3.add_in_port("btn0")
        self.control3.add_in_port("btn1")
        self.control3.add_out_port("abs5")
        self.control3.add_out_port("abs6")
        self.control3.set_pos(10, 300)

        self.control1 = Control("EvDev", self.root)
        self.control1.add_out_port("abs1")
        self.control1.add_out_port("abs2")
        self.control1.add_out_port("abs3")
        self.control1.add_out_port("abs4")
        self.control1.add_out_port("abs5")
        self.control1.add_out_port("abs6")
        self.control1.set_pos(10, 10)

        self.control2 = Control("UInput", self.root)
        self.control2.add_in_port("btn0")
        self.control2.add_in_port("btn1")
        self.control2.add_in_port("btn2")
        self.control2.add_out_port("abs5")
        self.control2.add_out_port("abs6")
        self.control2.set_pos(500, 200)

    def main(self):
        gtk.main()

if __name__ == "__main__":
    inputcfg = InputCfg()
    inputcfg.main()

# EOF #

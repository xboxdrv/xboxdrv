#!/usr/bin/env python
##
##  Xbox360 USB Gamepad Userspace Driver
##  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.

import time
import dbus
from dbus.mainloop.glib import DBusGMainLoop
import gobject
import sys
import os
import signal
from optparse import OptionParser

# This list is a direct copy of src/xboxmsg.hpp
xboxdrv_device_list = [
  ( "xbox",             0x045e, 0x0202, "Microsoft X-Box pad v1 (US)" ),
  ( "xbox",             0x045e, 0x0285, "Microsoft X-Box pad (Japan)" ),
  ( "xbox",             0x045e, 0x0285, "Microsoft Xbox Controller S" ),
  ( "xbox",             0x045e, 0x0287, "Microsoft Xbox Controller S" ),
  ( "xbox",             0x045e, 0x0289, "Microsoft X-Box pad v2 (US)" ),
  ( "xbox",             0x045e, 0x0289, "Microsoft Xbox Controller S" ),
  ( "xbox",             0x046d, 0xca84, "Logitech Xbox Cordless Controller" ),
  ( "xbox",             0x046d, 0xca88, "Logitech Compact Controller for Xbox" ),
  ( "xbox",             0x05fd, 0x1007, "Mad Catz Controller (unverified)" ),
  ( "xbox",             0x05fd, 0x107a, "InterAct 'PowerPad Pro' X-Box pad (Germany)" ),
  ( "xbox",             0x0738, 0x4516, "Mad Catz Control Pad" ),
  ( "xbox",             0x0738, 0x4522, "Mad Catz LumiCON" ),
  ( "xbox",             0x0738, 0x4526, "Mad Catz Control Pad Pro" ),
  ( "xbox",             0x0738, 0x4536, "Mad Catz MicroCON" ),
  ( "xbox",             0x0738, 0x4556, "Mad Catz Lynx Wireless Controller" ),
  ( "xbox",             0x0c12, 0x8802, "Zeroplus Xbox Controller" ),
  ( "xbox",             0x0c12, 0x8810, "Zeroplus Xbox Controller" ),
  ( "xbox",             0x0c12, 0x9902, "HAMA VibraX - *FAULTY HARDWARE*" ),
  ( "xbox",             0x0e4c, 0x1097, "Radica Gamester Controller" ),
  ( "xbox",             0x0e4c, 0x2390, "Radica Games Jtech Controller" ),
  ( "xbox",             0x0e6f, 0x0003, "Logic3 Freebird wireless Controller" ),
  ( "xbox",             0x0e6f, 0x0005, "Eclipse wireless Controller" ),
  ( "xbox",             0x0e6f, 0x0006, "Edge wireless Controller" ),
  ( "xbox",             0x0e8f, 0x0201, "SmartJoy Frag Xpad/PS2 adaptor" ),
  ( "xbox",             0x0f30, 0x0202, "Joytech Advanced Controller" ),
  ( "xbox",             0x0f30, 0x8888, "BigBen XBMiniPad Controller" ),
  ( "xbox",             0x102c, 0xff0c, "Joytech Wireless Advanced Controller" ),
  ( "xbox",             0x044f, 0x0f07, "Thrustmaster, Inc. Controller" ),
  ( "xbox360",          0x045e, 0x028e, "Microsoft Xbox 360 Controller" ),
  ( "xbox360",          0x0738, 0x4716, "Mad Catz Xbox 360 Controller" ),
  ( "xbox360",          0x0738, 0x4726, "Mad Catz Xbox 360 Controller" ),
  ( "xbox360",          0x0f0d, 0x000d, "Hori Fighting Stick Ex2" ),
  ( "xbox360",          0x162e, 0xbeef, "Joytech Neo-Se Take2" ),
  ( "xbox360",          0x046d, 0xc242, "Logitech ChillStream" ),
  ( "xbox360-guitar",   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" ),
  ( "xbox360-guitar",   0x1bad, 0x0002, "Harmonix Guitar for Xbox 360" ),
  ( "xbox360-guitar",   0x1bad, 0x0003, "Harmonix Drum Kit for Xbox 360" ),

  ( "xbox360-wireless", 0x045e, 0x0291, "Microsoft Xbox 360 Wireless Controller" ),
  ( "xbox360-wireless", 0x045e, 0x0719, "Microsoft Xbox 360 Wireless Controller (PC)" ),

  ( "xbox-mat",         0x0738, 0x4540, "Mad Catz Beat Pad" ),
  ( "xbox-mat",         0x0738, 0x6040, "Mad Catz Beat Pad Pro" ),
  ( "xbox-mat",         0x0c12, 0x8809, "RedOctane Xbox Dance Pad" ),
  ( "xbox-mat",         0x12ab, 0x8809, "Xbox DDR dancepad" ),
  ( "xbox-mat",         0x1430, 0x8888, "TX6500+ Dance Pad (first generation)" ),

  ( "xbox360",          0x12ab, 0x0004, "DDR Universe 2 Mat" ), 

  ( "firestorm",        0x044f, 0xb304, "ThrustMaster, Inc. Firestorm Dual Power" ),
]

class DeviceManager:
    def __init__(self, xboxdrv="xboxdrv", attach=None, detach=None, xboxdrv_args=[]):
        self.xboxdrv_bin   = xboxdrv
        self.attach_script = attach
        self.detach_script = detach
        self.xboxdrv_args  = xboxdrv_args

        self.processes = {}
        self.bus = dbus.SystemBus()

        self.bus.add_signal_receiver(self.device_added,
                                     'DeviceAdded',
                                     'org.freedesktop.Hal.Manager',
                                     'org.freedesktop.Hal',
                                     '/org/freedesktop/Hal/Manager')
        
        self.bus.add_signal_receiver(self.device_removed,
                                     'DeviceRemoved',
                                     'org.freedesktop.Hal.Manager',
                                     'org.freedesktop.Hal',
                                     '/org/freedesktop/Hal/Manager')

        hal_manager_obj = self.bus.get_object("org.freedesktop.Hal", "/org/freedesktop/Hal/Manager")
        hal_manager     = dbus.Interface(hal_manager_obj, "org.freedesktop.Hal.Manager")

        # find xboxdrv devices that are already connected
        for udi in hal_manager.FindDeviceStringMatch('info.subsystem', 'usb_device'):
            self.device_added(udi)

    def udi_to_device(self, udi):
        # uid: /org/freedesktop/Hal/devices/usb_device_45e_28e_13FEF2D
        return self.bus.get_object("org.freedesktop.Hal", udi)
    
    def device_added(self, udi):
        # print "add:", udi

        device = self.udi_to_device(udi)
        device_if = dbus.Interface(device, 'org.freedesktop.Hal.Device')

        if device_if.GetPropertyString('info.subsystem') == 'usb_device':
            vendor_id  = device_if.GetPropertyInteger('usb_device.vendor_id')
            product_id = device_if.GetPropertyInteger('usb_device.product_id')

            device_type = self.is_xboxdrv_device(vendor_id, product_id)

            if device_type:           
                bus_number = device_if.GetPropertyInteger('usb_device.bus_number')
                dev_number = device_if.GetPropertyInteger('usb_device.linux.device_number')
                self.xboxdrv_launch(udi, bus_number, dev_number, device_type[0])
            
    def device_removed(self, udi):
	# print "remove:", udi
        self.xboxdrv_kill(udi)

    def xboxdrv_launch(self, udi, bus, dev, type):
        self.processes[udi] = os.spawnvp(os.P_NOWAIT, self.xboxdrv_bin,
                                         ['xboxdrv', 
                                          '--silent', 
                                          '--device-by-path', "%03d:%03d" % (bus, dev), 
                                          '--type', type] +
                                         self.xboxdrv_args)
        print "Launched:", self.processes[udi]

        if self.attach_script:
            time.sleep(2) # give xboxdrv time to start up
            os.system(self.attach_script)

    def xboxdrv_kill(self, udi):
        if self.processes.has_key(udi):
            pid = self.processes[udi]
            print "Killing:  ", pid
            os.kill(pid, signal.SIGINT)
            os.waitpid(pid, 0)
            del self.processes[udi]

            if self.detach_script:
                os.system(self.detach_script)

    def shutdown(self):
        for (udi, pid) in self.processes.iteritems():
            print "Shutdown:", udi, pid
            os.kill(pid, signal.SIGINT)
            os.waitpid(pid, 0)
        self.processes.clear()
            
    def is_xboxdrv_device(self, arg_vendor_id, arg_product_id):
        for (type, vendor_id, product_id, name) in xboxdrv_device_list:
            # print arg_vendor_id,  vendor_id, arg_product_id, product_id
            if arg_vendor_id == vendor_id and arg_product_id == product_id:
                return (type, vendor_id, product_id, name)
        return None
        
if __name__ == '__main__':
    # parse options
    parser = OptionParser(usage = "%prog [OPTIONS] -- [XBOXDRV ARGS]",
                          version = "0.1",
                          description = "A simple daemon that automatically launches xboxdrv.")
    parser.add_option('-a', '--attach', metavar='FILENAME',
                      help="Launch EXE when a new controller is connected")
    parser.add_option('-d', '--detach', metavar='FILENAME',
                      help="Launch EXE when a controller is detached")
    parser.add_option('-x', '--xboxdrv', metavar='FILENAME',
                      help="Set the location of the xboxdrv executable")
    (opts, args) = parser.parse_args()

    DBusGMainLoop(set_as_default=True)

    mgr  = DeviceManager(xboxdrv = opts.xboxdrv, xboxdrv_args = args, 
                         attach = opts.attach, detach = opts.detach)
    loop = gobject.MainLoop()

    try:
        loop.run()

    except KeyboardInterrupt:
        loop.quit()
        mgr.shutdown()
        sys.exit(0)

# EOF #

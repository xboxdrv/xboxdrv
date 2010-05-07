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

import string
import time
import dbus
from dbus.mainloop.glib import DBusGMainLoop
import gobject
import sys
import os
import signal
import subprocess
from optparse import OptionParser

class DeviceManager:
    def __init__(self, xboxdrv_device_list=[], xboxdrv="xboxdrv", attach=None, detach=None, xboxdrv_args=[], verbose = False):
        self.xboxdrv_device_list = xboxdrv_device_list
        self.xboxdrv_bin   = xboxdrv
        self.attach_script = attach
        self.detach_script = detach
        self.xboxdrv_args  = xboxdrv_args
        self.verbose = verbose

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

        if self.verbose:
            print "xboxdrv-daemon: searching for connected controllers"

        # find xboxdrv devices that are already connected
        for udi in hal_manager.FindDeviceStringMatch('info.subsystem', 'usb_device'):
            self.device_added(udi)

    def udi_to_device(self, udi):
        # uid: /org/freedesktop/Hal/devices/usb_device_45e_28e_13FEF2D
        return self.bus.get_object("org.freedesktop.Hal", udi)
    
    def device_added(self, udi):
        # print "xboxdrv-daemon: usb device connected: %s" % udi

        device = self.udi_to_device(udi)
        device_if = dbus.Interface(device, 'org.freedesktop.Hal.Device')

        if device_if.GetPropertyString('info.subsystem') == 'usb_device':
            vendor_id  = device_if.GetPropertyInteger('usb_device.vendor_id')
            product_id = device_if.GetPropertyInteger('usb_device.product_id')

            device_type = self.is_xboxdrv_device(vendor_id, product_id)

            if device_type:
                if self.verbose:
                    print "xboxdrv-daemon: controller connected: %s" % udi

                bus_number = device_if.GetPropertyInteger('usb_device.bus_number')
                dev_number = device_if.GetPropertyInteger('usb_device.linux.device_number')
                self.xboxdrv_launch(udi, bus_number, dev_number, device_type[0])
            
    def device_removed(self, udi):
        # print "xboxdrv-daemon: usb device disconnected: %s" % udi
        if self.verbose and self.processes.has_key(udi):
            print "xboxdrv-daemon: controller disconnected: %s" % udi
        self.xboxdrv_kill(udi)

    def xboxdrv_launch(self, udi, bus, dev, type):
        args = [self.xboxdrv_bin,
                '--silent', 
                '--quiet',
                '--device-by-path', "%03d:%03d" % (bus, dev), 
                '--type', type] + self.xboxdrv_args

        if self.verbose:
            print "xboxdrv-daemon: launching: %s" % string.join(args)

        # self.processes[udi] = os.spawnvp(os.P_NOWAIT, self.xboxdrv_bin, args)
        try: 
            self.processes[udi] = subprocess.Popen(args).pid
        except OSError, err:
            raise Exception("xboxdrv-daemon: couldn't launch '%s', set the location of xboxdrv with '-x'" % self.xboxdrv_bin)

        if self.verbose:
            print "xboxdrv-daemon: launched: pid: %d - %s" % (self.processes[udi], udi)

        if self.attach_script:
            time.sleep(2) # give xboxdrv time to start up
            if self.verbose:
                print "xboxdrv-daemon: executing attach script: %s" % self.attach_script
            os.system(self.attach_script)

    def xboxdrv_kill(self, udi):
        if self.processes.has_key(udi):
            if self.verbose:
                print "xboxdrv-daemon: killing: pid: %d - %s" % (self.processes[udi], udi)

            pid = self.processes[udi]
            os.kill(pid, signal.SIGINT)
            os.waitpid(pid, 0)
            del self.processes[udi]

            if self.detach_script:
                if self.verbose:
                    print "xboxdrv-daemon: executing detach script: %s" % self.detach_script
                os.system(self.detach_script)

    def shutdown(self):
        for (udi, pid) in self.processes.items():
            self.xboxdrv_kill(udi)
            
    def is_xboxdrv_device(self, arg_vendor_id, arg_product_id):
        for (type, vendor_id, product_id, name) in self.xboxdrv_device_list:
            # print arg_vendor_id,  vendor_id, arg_product_id, product_id
            if arg_vendor_id == vendor_id and arg_product_id == product_id:
                return (type, vendor_id, product_id, name)
        return None
    
if __name__ == '__main__':
    # parse options
    parser = OptionParser(usage = "%prog [OPTIONS] -- [XBOXDRV ARGS]",
                          version = "0.4.6",
                          description = "A simple daemon that automatically launches xboxdrv.")
    parser.add_option('-a', '--attach', metavar='FILENAME',
                      help="Launch EXE when a new controller is connected")
    parser.add_option('-d', '--detach', metavar='FILENAME',
                      help="Launch EXE when a controller is detached")
    parser.add_option('-x', '--xboxdrv', metavar='FILENAME', default="xboxdrv",
                      help="Set the location of the xboxdrv executable")
    parser.add_option('-v', '--verbose', action='store_true', default=False,
                      help="Be verbose with printed output")
    (opts, args) = parser.parse_args()

    # Read list of supported devices from xboxdrv
    try:
        xboxdrv_device_list = []
        for i in subprocess.Popen([opts.xboxdrv, '--list-supported-devices'], 
                                  stdout=subprocess.PIPE).stdout:
            lst = i.rstrip().split(' ', 3)
            xboxdrv_device_list.append((lst[0],
                                        int(lst[1], 16),
                                        int(lst[2], 16),
                                        lst[3]))
    except OSError, err:
        raise Exception("xboxdrv-daemon: couldn't launch '%s', set the location of xboxdrv with '-x'" % opts.xboxdrv)

    DBusGMainLoop(set_as_default=True)

    mgr  = DeviceManager(xboxdrv_device_list = xboxdrv_device_list,
                         xboxdrv = opts.xboxdrv, xboxdrv_args = args, 
                         attach = opts.attach, detach = opts.detach,
                         verbose = opts.verbose)
    loop = gobject.MainLoop()

    try:
        loop.run()

    except KeyboardInterrupt:
        if opts.verbose:
            print "xboxdrv-daemon: keyboad interrupt received, shutting down"

        loop.quit()
        mgr.shutdown()
        sys.exit(0)

# EOF #

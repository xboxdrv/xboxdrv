#!/usr/bin/env python

import usb
import sys
import time
 
# find our device
dev = usb.core.find(idVendor=0x045e, idProduct=0x028e)
 
# was it found?
if dev is None:
    raise ValueError('Device not found')

dev.set_configuration()

print dev
print dir(dev)

while True:
    # Get data from brequest 0x32
    # ret = dev.ctrl_transfer(0xC0, 0x32, 0x0, 0x0, 10)
    pass

# EOF #

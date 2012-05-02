/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "xpad_device.hpp"

// FIXME: We shouldn't check device-ids, but device class or so, to
// automatically catch all third party stuff
XPadDevice xpad_devices[] = {
  // Evil?! Anymore info we could use to identify the devices?
  // { GAMEPAD_XBOX,             0x0000, 0x0000, "Generic X-Box pad" },
  // { GAMEPAD_XBOX,             0xffff, 0xffff, "Chinese-made Xbox Controller" },

  // These should work
  { GAMEPAD_XBOX,             0x045e, 0x0202, "Microsoft X-Box pad v1 (US)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft X-Box pad (Japan)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0287, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft Corp. Xbox Controller S" },
  // { GAMEPAD_XBOX,          0x045e, 0x0288, "Microsoft Corp. Xbox Controller S Hub" },  memory card slot
  { GAMEPAD_XBOX,             0x046d, 0xca84, "Logitech Xbox Cordless Controller" },
  { GAMEPAD_XBOX,             0x046d, 0xca88, "Logitech Compact Controller for Xbox" },
  { GAMEPAD_XBOX,             0x05fd, 0x1007, "Mad Catz Controller (unverified)" },
  { GAMEPAD_XBOX,             0x05fd, 0x107a, "InterAct 'PowerPad Pro' X-Box pad (Germany)" },
  { GAMEPAD_XBOX,             0x0738, 0x4516, "Mad Catz Control Pad" },
  { GAMEPAD_XBOX,             0x0738, 0x4522, "Mad Catz LumiCON" },
  { GAMEPAD_XBOX,             0x0738, 0x4526, "Mad Catz Control Pad Pro" },
  { GAMEPAD_XBOX,             0x0738, 0x4536, "Mad Catz MicroCON" },
  { GAMEPAD_XBOX,             0x0738, 0x4556, "Mad Catz Lynx Wireless Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x8802, "Zeroplus Xbox Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x8810, "Zeroplus Xbox Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x9902, "HAMA VibraX - *FAULTY HARDWARE*" },
  { GAMEPAD_XBOX,             0x0e4c, 0x1097, "Radica Gamester Controller" },
  { GAMEPAD_XBOX,             0x0e4c, 0x2390, "Radica Games Jtech Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0003, "Logic3 Freebird wireless Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0005, "Eclipse wireless Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0006, "Edge wireless Controller" },
  { GAMEPAD_XBOX,             0x0e8f, 0x0201, "SmartJoy Frag Xpad/PS2 adaptor" },
  { GAMEPAD_XBOX,             0x0f30, 0x0202, "Joytech Advanced Controller" },
  { GAMEPAD_XBOX,             0x0f30, 0x8888, "BigBen XBMiniPad Controller" },
  { GAMEPAD_XBOX,             0x102c, 0xff0c, "Joytech Wireless Advanced Controller" },
  { GAMEPAD_XBOX,             0x044f, 0x0f07, "Thrustmaster, Inc. Controller" },
  { GAMEPAD_XBOX,             0x0e8f, 0x3008, "Generic xbox control (dealextreme)" },
  { GAMEPAD_XBOX360,          0x045e, 0x028e, "Microsoft Xbox 360 Controller" },
  { GAMEPAD_XBOX360_PLAY_N_CHARGE, 0x045e, 0x028f, "Microsoft Xbox 360 Play&Charge Kit" },
  { GAMEPAD_XBOX360,          0x0738, 0x4716, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4726, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4728, "Mad Catz Street Fighter IV FightPad" },
  { GAMEPAD_XBOX360,          0x0738, 0x4740, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX360,          0x0738, 0xb726, "Mad Catz Xbox controller - MW2" },
  { GAMEPAD_XBOX360,          0x0738, 0xf738, "Super SFIV FightStick TE S" },
  { GAMEPAD_XBOX360,          0x0738, 0x4718, "Mad Catz Street Fighter IV FightStick SE" },
  { GAMEPAD_XBOX360,          0x0738, 0xbeef, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x000a, "Hori Co. DOA4 FightStick" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x000d, "Hori Fighting Stick Ex2" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x0016, "Hori Real Arcade Pro Ex" },
  { GAMEPAD_XBOX360,          0x162e, 0xbeef, "Joytech Neo-Se Take2" },
  { GAMEPAD_XBOX360,          0x046d, 0xc21e, "Logitech F510" },
  { GAMEPAD_XBOX360,          0x046d, 0xc242, "Logitech ChillStream" },
  { GAMEPAD_XBOX360,          0x0738, 0xcb03, "Saitek P3200 Rumble Pad - PC/Xbox 360" },
  { GAMEPAD_XBOX360,          0x0738, 0xcb02, "Saitek Cyborg Rumble Pad - PC/Xbox 360" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0201, "Pelican TSZ360 Pad" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0213, "Afterglow Gamepad for Xbox 360" },
  { GAMEPAD_XBOX360,          0x12ab, 0x0301, "PDP AFTERGLOW AX.1" },
  { GAMEPAD_XBOX360_GUITAR,   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },
  { GAMEPAD_XBOX360_GUITAR,   0x1bad, 0x0002, "Harmonix Guitar for Xbox 360" },
  { GAMEPAD_XBOX360_GUITAR,   0x1bad, 0x0003, "Harmonix Drum Kit for Xbox 360" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf016, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf028, "Street Fighter IV FightPad" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf901, "Gamestop Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf903, "Tron Xbox 360 controller" },
  { GAMEPAD_XBOX360,          0x15e4, 0x3f00, "Power A Mini Pro Elite" },
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0291, "Microsoft Xbox 360 Wireless Controller" }, // RF Module from the Xbox360
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0719, "Microsoft Xbox 360 Wireless Controller (PC)" }, // official Wireless Receiver
  { GAMEPAD_XBOX360,          0x1689, 0xfd00, "Razer Onza" },  
  { GAMEPAD_XBOX360,          0x12ab, 0x0004, "DDR Universe 2 Mat" }, 
  { GAMEPAD_XBOX360,          0x15e4, 0x3f0a, "Xbox Airflo wired controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5300, "Power A Mini Pro Elite Glow" },

  { GAMEPAD_XBOX_MAT,         0x0738, 0x4540, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x6040, "Mad Catz Beat Pad Pro" },
  { GAMEPAD_XBOX_MAT,         0x0c12, 0x8809, "RedOctane Xbox Dance Pad" },
  { GAMEPAD_XBOX_MAT,         0x12ab, 0x8809, "Xbox DDR dancepad" },
  { GAMEPAD_XBOX_MAT,         0x1430, 0x8888, "TX6500+ Dance Pad (first generation)" },
   
  { GAMEPAD_FIRESTORM,        0x044f, 0xb304, "ThrustMaster, Inc. Firestorm Dual Power" },
  { GAMEPAD_FIRESTORM_VSB,    0x044f, 0xb312, "ThrustMaster, Inc. Firestorm Dual Power (vs b)" },

  { GAMEPAD_SAITEK_P2500,     0x06a3, 0xff0c, "Saitek P2500" },
  { GAMEPAD_LOGITECH_F310,    0x046d, 0xc21d, "Logitech Gamepad F310" },

  { GAMEPAD_PLAYSTATION3_USB, 0x054c, 0x0268, "PLAYSTATION(R)3 Controller" },

  { GAMEPAD_HAMA_CRUX,        0x1038, 0x0310, "Hama cruX Gaming Keyboard" }
};

const int xpad_devices_count = sizeof(xpad_devices)/sizeof(XPadDevice);

bool find_xpad_device(uint16_t idVendor, uint16_t idProduct, XPadDevice* dev_type)
{
  for(int i = 0; i < xpad_devices_count; ++i)
  {
    if (idVendor  == xpad_devices[i].idVendor &&
        idProduct == xpad_devices[i].idProduct)
    {
      *dev_type = xpad_devices[i];
      return true;
    }
  }
  return false;
}

/* EOF */

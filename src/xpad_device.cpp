/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

namespace xboxdrv {

// FIXME: We shouldn't check device-ids, but device class or so, to
// automatically catch all third party stuff
XPadDevice xpad_devices[] = {
  // Evil?! Anymore info we could use to identify the devices?
  // { GAMEPAD_XBOX,             0x0000, 0x0000, "Generic X-Box pad" },
  // { GAMEPAD_XBOX,             0xffff, 0xffff, "Chinese-made Xbox Controller" },

  // These should work
  { GAMEPAD_XBOX,             0x0d2f, 0x0002, "Andamiro Pump It Up pad" },
  { GAMEPAD_XBOX,             0x045e, 0x0202, "Microsoft X-Box pad v1 (US)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft X-Box pad (Japan)" },
  { GAMEPAD_XBOX,             0x045e, 0x0287, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft X-Box pad v2 (US)" },
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
  { GAMEPAD_XBOX,             0x0e8f, 0x3008, "Generic Xbox controller (DealExtreme)" },
  { GAMEPAD_XBOX360,          0x045e, 0x028e, "Microsoft X-Box 360 pad" },
  { GAMEPAD_XBOX360_PLAY_N_CHARGE, 0x045e, 0x028f, "Microsoft Xbox 360 Play&Charge Kit" },
  { GAMEPAD_XBOX360,          0x0738, 0x4716, "Mad Catz Wired Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4726, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4728, "Mad Catz Street Fighter IV FightPad" },
  { GAMEPAD_XBOX360,          0x0738, 0x4740, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX360,          0x0738, 0xb726, "Mad Catz Xbox controller - MW2" },
  { GAMEPAD_XBOX360,          0x0738, 0xf738, "Super SFIV FightStick TE S" },
  { GAMEPAD_XBOX360,          0x0738, 0x4718, "Mad Catz Street Fighter IV FightStick SE" },
  { GAMEPAD_XBOX360,          0x0738, 0x4738, "Mad Catz Wired Xbox 360 Controller (SFIV)" },
  { GAMEPAD_XBOX360,          0x0738, 0xbeef, "Mad Catz JOYTECH NEO SE Advanced GamePad" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x000a, "Hori Co. DOA4 FightStick" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x000d, "Hori Fighting Stick EX2" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x0016, "Hori Real Arcade Pro.EX" },
  { GAMEPAD_XBOX360,          0x056e, 0x2004, "Elecom JC-U3613M" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5501, "Hori Real Arcade Pro VX-SA" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5303, "Xbox Airflo wired controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x531a, "PowerA Pro Ex" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5397, "FUS1ON Tournament Controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5503, "Hori Fighting Edge" },
  { GAMEPAD_XBOX360,          0x24c6, 0x550d, "Hori GEM Xbox controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5b03, "Thrustmaster Ferrari 458 Racing Wheel" },
  { GAMEPAD_XBOX360,          0x162e, 0xbeef, "Joytech Neo-Se Take2" },
  { GAMEPAD_XBOX360,          0x044f, 0xb326, "Thrustmaster Gamepad GP XID" },
  { GAMEPAD_XBOX360,          0x046d, 0xc21d, "Logitech Gamepad F310" },
  { GAMEPAD_XBOX360,          0x046d, 0xc21e, "Logitech Gamepad F510" },
  { GAMEPAD_XBOX360,          0x046d, 0xc21f, "Logitech Gamepad F710" },
  { GAMEPAD_XBOX360,          0x046d, 0xc242, "Logitech Chillstream Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0xcb03, "Saitek P3200 Rumble Pad - PC/Xbox 360" },
  { GAMEPAD_XBOX360,          0x0738, 0xcb02, "Saitek Cyborg Rumble Pad - PC/Xbox 360" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0201, "Pelican PL-3601 'TSZ' Wired Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0105, "HSM3 Xbox 360 Dance Pad" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0113, "Afterglow AX.1 Gamepad for Xbox 360" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0413, "Afterglow AX.1 Gamepad for Xbox 360" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0213, "Afterglow Gamepad for Xbox 360" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0401, "Logic3 Controller" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0301, "Logic3 Controller" },
  { GAMEPAD_XBOX360,          0x12ab, 0x0301, "PDP AFTERGLOW AX.1" },
  { GAMEPAD_XBOX360_GUITAR,   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },
  { GAMEPAD_XBOX360,          0x146b, 0x0601, "BigBen Interactive XBOX 360 Controller" },
  { GAMEPAD_XBOX360_GUITAR,   0x1bad, 0x0002, "Harmonix Guitar for Xbox 360" },
  { GAMEPAD_XBOX360_GUITAR,   0x1bad, 0x0003, "Harmonix Drum Kit for Xbox 360" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf016, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf018, "Mad Catz Street Fighter IV SE Fighting Stick" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf021, "Mad Catz Ghost Recon FS GamePad" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf023, "MLG Pro Circuit Controller (Xbox)" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf028, "Street Fighter IV FightPad" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf02e, "Mad Catz Fightpad" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf038, "Street Fighter IV FightStick TE" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf03a, "Mad Catz SFxT Fightstick Pro" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf900, "Harmonix Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf901, "Gamestop Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf903, "Tron Xbox 360 controller" },
  { GAMEPAD_XBOX360,          0x1bad, 0xfa01, "MadCatz GamePad" },
  { GAMEPAD_XBOX360,          0x15e4, 0x3f00, "Power A Mini Pro Elite" },
  { GAMEPAD_XBOX360,          0x15e4, 0x3f10, "Batarang Xbox 360 controller" },
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0291, "Xbox 360 Wireless Receiver (XBOX)" }, // RF Module from the Xbox360
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0719, "Xbox 360 Wireless Receiver" }, // official Wireless Receiver
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x02a9, "Xbox 360 Wireless Receiver (Unofficial)" }, // third-party, MS VID (PR #244)
  { GAMEPAD_XBOX360,          0x24c6, 0x5000, "Razer Atrox Arcade Stick" },
  { GAMEPAD_XBOX360,          0x1689, 0xfd00, "Razer Onza Tournament Edition" },
  { GAMEPAD_XBOX360,          0x1689, 0xfd01, "Razer Onza Classic Edition" },
  { GAMEPAD_XBOX360,          0x12ab, 0x0004, "Honey Bee Xbox 360 Dance Pad" },
  { GAMEPAD_XBOX360,          0x15e4, 0x3f0a, "Xbox Airflo wired controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5300, "PowerA MINI PROEX Controller" },
  { GAMEPAD_XBOXONE_WIRELESS, 0x24c6, 0x543a, "PowerA Xbox One wired controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5500, "Hori XBOX 360 EX 2 with Turbo" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5506, "Hori SOULCALIBUR V Stick" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5b02, "Thrustmaster, Inc. GPX Controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5d04, "Razer Sabertooth Elite" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x011f, "Rock Candy Gamepad Wired Controller" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x021f, "Rock Candy Gamepad for Xbox 360" },


  // Additional Xbox 360-compatible devices from kernel xpad (synced 2026-08)
  // XTYPE_XBOX360 only — Xbox One/Series IDs intentionally omitted until tested.
  { GAMEPAD_XBOX360,          0x0079, 0x18d4, "GPD Win 2" },
  { GAMEPAD_XBOX360,          0x0351, 0x1000, "CRKD LP Blueberry Burst Pro Edition (Xbox)" },
  { GAMEPAD_XBOX360,          0x0351, 0x2000, "CRKD LP Black Tribal Edition (Xbox)" },
  { GAMEPAD_XBOX360,          0x03eb, 0xff01, "Wooting One (Legacy)" },
  { GAMEPAD_XBOX360,          0x03eb, 0xff02, "Wooting Two (Legacy)" },
  { GAMEPAD_XBOX360,          0x03f0, 0x038d, "HyperX Clutch" },
  { GAMEPAD_XBOX360,          0x03f0, 0x048d, "HyperX Clutch" },
  { GAMEPAD_XBOX360,          0x046d, 0xcaa3, "Logitech DriveFx Racing Wheel" },
  { GAMEPAD_XBOX360,          0x0502, 0x1305, "Acer NGR200" },
  { GAMEPAD_XBOX360,          0x0738, 0x4736, "Mad Catz MicroCon Gamepad" },
  { GAMEPAD_XBOX360,          0x0738, 0x4758, "Mad Catz Arcade Game Stick" },
  { GAMEPAD_XBOX360,          0x0738, 0x9871, "Mad Catz Portable Drum" },
  { GAMEPAD_XBOX360,          0x0738, 0xb738, "Mad Catz MVC2TE Stick 2" },
  { GAMEPAD_XBOX360,          0x0738, 0xcb29, "Saitek Aviator Stick AV8R02" },
  { GAMEPAD_XBOX360,          0x07ff, 0xffff, "Mad Catz GamePad" },
  { GAMEPAD_XBOX360,          0x0b05, 0x1c91, "ASUS ROG RAIKIRI II" },
  { GAMEPAD_XBOX360,          0x0b05, 0x1c92, "ASUS ROG RAIKIRI II WIRELESS" },
  { GAMEPAD_XBOX360,          0x0db0, 0x1901, "Micro Star International Xbox360 Controller for Windows" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0131, "PDP EA Sports Controller" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0133, "Xbox 360 Wired Controller" },
  { GAMEPAD_XBOX360,          0x0e6f, 0x0501, "PDP Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0e6f, 0xf900, "PDP Afterglow AX.1" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x000c, "Hori PadEX Turbo" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x001b, "Hori Real Arcade Pro VX" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x00dc, "HORIPAD FPS for Nintendo Switch" },
  { GAMEPAD_XBOX360,          0x1038, 0x1430, "SteelSeries Stratus Duo" },
  { GAMEPAD_XBOX360,          0x1038, 0x1431, "SteelSeries Stratus Duo" },
  { GAMEPAD_XBOX360,          0x11c9, 0x55f0, "Nacon GC-100XF" },
  { GAMEPAD_XBOX360,          0x11ff, 0x0511, "PXN V900" },
  { GAMEPAD_XBOX360,          0x1209, 0x2882, "Ardwiino Controller" },
  { GAMEPAD_XBOX360,          0x12ab, 0x0303, "Mortal Kombat Klassic FightStick" },
  { GAMEPAD_XBOX360,          0x1430, 0xf801, "RedOctane Controller" },
  { GAMEPAD_XBOX360,          0x146b, 0x0604, "Bigben Interactive DAIJA Arcade Stick" },
  { GAMEPAD_XBOX360,          0x1532, 0x0a57, "Razer Wolverine V3 Pro (Wired)" },
  { GAMEPAD_XBOX360,          0x1532, 0x0a59, "Razer Wolverine V3 Pro (2.4 GHz Dongle)" },
  { GAMEPAD_XBOX360,          0x1689, 0xfe00, "Razer Sabertooth" },
  { GAMEPAD_XBOX360,          0x17ef, 0x6182, "Lenovo Legion Controller for Windows" },
  { GAMEPAD_XBOX360,          0x1949, 0x041a, "Amazon Game Controller" },
  { GAMEPAD_XBOX360,          0x1a86, 0xe310, "Legion Go S" },
  { GAMEPAD_XBOX360,          0x1bad, 0x0130, "Ion Drum Rocker" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf019, "Mad Catz Brawlstick for Xbox 360" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf025, "Mad Catz Call of Duty" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf027, "Mad Catz FPS Pro" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf030, "Mad Catz Xbox 360 MC2 MicroCon Racing Wheel" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf036, "Mad Catz MicroCon GamePad Pro" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf039, "Mad Catz MvC2 TE" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf03d, "Street Fighter IV Arcade Stick TE - Chun Li" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf03e, "Mad Catz MLG FightStick TE" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf03f, "Mad Catz FightStick Soulcalibur" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf042, "Mad Catz FightStick TES+" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf080, "Mad Catz FightStick TE2" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf501, "HoriPad EX2 Turbo" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf502, "Hori Real Arcade Pro.VX SA" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf503, "Hori Fighting Stick VX" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf504, "Hori Real Arcade Pro. EX" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf505, "Hori Fighting Stick EX2B" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf506, "Hori Real Arcade Pro.EX Premium VLX" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf904, "PDP Versus Fighting Pad" },
  { GAMEPAD_XBOX360,          0x1bad, 0xf906, "Mortal Kombat FightStick" },
  { GAMEPAD_XBOX360,          0x1bad, 0xfd00, "Razer Onza TE" },
  { GAMEPAD_XBOX360,          0x1bad, 0xfd01, "Razer Onza" },
  { GAMEPAD_XBOX360,          0x1ee9, 0x1590, "ZOTAC Gaming Zone" },
  { GAMEPAD_XBOX360,          0x20bc, 0x5134, "BETOP BTP-KP50B Xinput Dongle" },
  { GAMEPAD_XBOX360,          0x20bc, 0x514a, "BETOP BTP-KP50C Xinput Dongle" },
  { GAMEPAD_XBOX360,          0x20d6, 0x281f, "PowerA Wired Controller For Xbox 360" },
  { GAMEPAD_XBOX360,          0x2345, 0xe00b, "Machenike G5 Pro Controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x530a, "Xbox 360 Pro EX Controller" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5502, "Hori Fighting Stick VX Alt" },
  { GAMEPAD_XBOX360,          0x24c6, 0x550e, "Hori Real Arcade Pro V Kai 360" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5510, "Hori Fighting Commander ONE (Xbox 360/PC Mode)" },
  { GAMEPAD_XBOX360,          0x24c6, 0x5b00, "ThrustMaster Ferrari 458 Racing Wheel" },
  { GAMEPAD_XBOX360,          0x24c6, 0xfafe, "Rock Candy Gamepad for Xbox 360" },
  { GAMEPAD_XBOX360,          0x2563, 0x058d, "OneXPlayer Gamepad" },
  { GAMEPAD_XBOX360,          0x2993, 0x2001, "TECNO Pocket Go" },
  { GAMEPAD_XBOX360,          0x2dc8, 0x3106, "8BitDo Ultimate Wireless / Pro 2 Wired Controller" },
  { GAMEPAD_XBOX360,          0x2dc8, 0x3109, "8BitDo Ultimate Wireless Bluetooth" },
  { GAMEPAD_XBOX360,          0x2dc8, 0x310a, "8BitDo Ultimate 2C Wireless Controller" },
  { GAMEPAD_XBOX360,          0x2dc8, 0x310b, "8BitDo Ultimate 2 Wireless Controller" },
  { GAMEPAD_XBOX360,          0x2dc8, 0x6001, "8BitDo SN30 Pro" },
  { GAMEPAD_XBOX360,          0x31e3, 0x1100, "Wooting One" },
  { GAMEPAD_XBOX360,          0x31e3, 0x1200, "Wooting Two" },
  { GAMEPAD_XBOX360,          0x31e3, 0x1210, "Wooting Lekker" },
  { GAMEPAD_XBOX360,          0x31e3, 0x1220, "Wooting Two HE" },
  { GAMEPAD_XBOX360,          0x31e3, 0x1230, "Wooting Two HE (ARM)" },
  { GAMEPAD_XBOX360,          0x31e3, 0x1300, "Wooting 60HE (AVR)" },
  { GAMEPAD_XBOX360,          0x31e3, 0x1310, "Wooting 60HE (ARM)" },
  { GAMEPAD_XBOX360,          0x3285, 0x0607, "Nacon GC-100" },
  { GAMEPAD_XBOX360,          0x3285, 0x0662, "Nacon Revolution5 Pro" },
  { GAMEPAD_XBOX360,          0x3507, 0x000b, "ZENAIM LEVERLESS" },
  { GAMEPAD_XBOX360,          0x3537, 0x1004, "GameSir T4 Kaleid" },
  { GAMEPAD_XBOX360,          0x3537, 0x100f, "GameSir Nova 2 Lite" },
  { GAMEPAD_XBOX360,          0x3651, 0x1000, "CRKD SG" },
  { GAMEPAD_XBOX360,          0x37d7, 0x2501, "Flydigi Apex 5" },
  { GAMEPAD_XBOX360,          0x413d, 0x2104, "Black Shark Green Ghost Gamepad" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x4540, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x6040, "Mad Catz Beat Pad Pro" },
  { GAMEPAD_XBOX_MAT,         0x0c12, 0x8809, "RedOctane Xbox Dance Pad" },
  { GAMEPAD_XBOX_MAT,         0x12ab, 0x8809, "Xbox DDR Dance Pad" },
  // { GAMEPAD_XBOX_MAT,         0x1430, 0x8888, "TX6500+ Dance Pad (first generation)" }, // just a HID device, not Xbox1

  { GAMEPAD_XBOXONE_WIRELESS, 0x045e, 0x02d1, "Microsoft X-Box One pad" },
  { GAMEPAD_XBOXONE_WIRELESS, 0x045e, 0x02dd, "Microsoft X-Box One pad" },
  { GAMEPAD_XBOXONE_WIRELESS, 0x2e24, 0x1688, "Hyperkin X91" },  // issue #247; Xbox One protocol (xpad XTYPE_XBOXONE)
  { GAMEPAD_FIRESTORM,        0x044f, 0xb304, "ThrustMaster, Inc. Firestorm Dual Power" },
  { GAMEPAD_FIRESTORM_VSB,    0x044f, 0xb312, "ThrustMaster, Inc. Firestorm Dual Power (vs b)" },
  { GAMEPAD_T_WIRELESS,       0x044f, 0xd007, "ThrustMaster, Inc. T-Wireless" },

  { GAMEPAD_SAITEK_P2500,     0x06a3, 0xff0c, "Saitek P2500" },
  { GAMEPAD_SAITEK_P3600,     0x06a3, 0xf51a, "Saitek P3600 (Cyborg Rumble)" },
  { GAMEPAD_XEOX,             0x1a34, 0x0802, "Speedlink Xeox USB Gamepad" },
  { GAMEPAD_LOGITECH_F310,    0x046d, 0xc21d, "Logitech Gamepad F310" },

  { GAMEPAD_PLAYSTATION3_USB, 0x054c, 0x0268, "PLAYSTATION(R)3 Controller" },
  { GAMEPAD_STEAM,            0x28de, 0x1102, "Valve Software Steam Controller" },  // experimental PR #222
  { GAMEPAD_STEAM_WIRELESS,   0x28de, 0x1142, "Valve Software Steam Controller (wireless)" },

  { GAMEPAD_HAMA_CRUX,        0x1038, 0x0310, "Hama cruX Gaming Keyboard" }
};

const int xpad_devices_count = static_cast<int>(sizeof(xpad_devices)/sizeof(XPadDevice));

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

} // namespace xboxdrv

/* EOF */

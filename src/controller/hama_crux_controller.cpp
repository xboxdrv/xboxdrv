/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "controller/hama_crux_controller.hpp"

#include "log.hpp"
#include "helper.hpp"

HamaCruxNames::HamaCruxNames(ControllerMessageDescriptor& desc) :
  crouch(desc.key().put("Crouch")),
  run(desc.key().put("Run")),
  talk(desc.key().put("Talk")),
  
  esc(desc.key().put("Escape")),
  pause(desc.key().put("Pause")),
  option(desc.key().put("Option")),
 
  quickload(desc.key().put("Quickload")),
  quicksave(desc.key().put("Quicksave")),
  print(desc.key().put("Print")),

  n1(desc.key().put("N1")),
  n2(desc.key().put("N2")),
  n3(desc.key().put("N3")),
  n4(desc.key().put("N4")),
  n5(desc.key().put("N5")),
  n6(desc.key().put("N6")),

  n7(desc.key().put("N7")),
  n8(desc.key().put("N8")),
  n9(desc.key().put("N9")),
  n10(desc.key().put("N10")),
  n11(desc.key().put("N11")),

  up(desc.key().put("Up")),
  down(desc.key().put("Down")),
  left(desc.key().put("Left")),
  right(desc.key().put("Right")),
  q(desc.key().put("Q")),
  e(desc.key().put("E")),

  c1(desc.key().put("C1")),
  tab(desc.key().put("Tab")),
  c2(desc.key().put("C2")),
  c3(desc.key().put("C3")),
  c4(desc.key().put("C4")),
  reload(desc.key().put("Reload")),
  use(desc.key().put("Use")),
  c8(desc.key().put("C8")),
  p2(desc.key().put("P2")),
  n(desc.key().put("N")),

  c5(desc.key().put("C5")),
  c6(desc.key().put("C6")),
  c7(desc.key().put("C7")),
  p1(desc.key().put("P1")),
  space(desc.key().put("Space"))
{  
}

HamaCruxController::HamaCruxController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  m_interface(0),
  m_endpoint(1),
  m_names(m_message_descriptor)
{  
  usb_claim_interface(m_interface, try_detach);
  usb_submit_read(m_endpoint, 8);
}

HamaCruxController::~HamaCruxController()
{
  usb_cancel_read();
  usb_release_interface(m_interface);
}

void
HamaCruxController::set_rumble_real(uint8_t left, uint8_t right)
{
  // not supported
}

void
HamaCruxController::set_led_real(uint8_t status)
{
  // not implemented
}

bool
HamaCruxController::parse(const uint8_t* data, int len, ControllerMessage* msg)
{
  //std::cout << "HamaCruxController::parse(): " << raw2str(data, len) << std::endl;
  if (len != 0)
  {
    msg->clear();

    // error: 01 01 01 01 01 01

    msg->set_key(m_names.crouch,  data[0] & 0x01);
    msg->set_key(m_names.run, data[0] & 0x02);
    msg->set_key(m_names.talk,   data[0] & 0x04);
      
    for(int i = 2; i < len; ++i)
    {
      uint8_t scancode = data[i];

      switch(scancode)
      {
        // left top
        case 0x29: msg->set_key(m_names.esc, 1); break;
        case 0x48: msg->set_key(m_names.pause, 1); break;
        case 0x43: msg->set_key(m_names.option, 1); break;

          // right top
        case 0x30: msg->set_key(m_names.quickload, 1); break;
        case 0x2f: msg->set_key(m_names.print, 1); break;
        case 0x46: msg->set_key(m_names.quicksave, 1); break;

          // 1 - 6
        case 0x59: msg->set_key(m_names.n1, 1); break;
        case 0x1f: msg->set_key(m_names.n2, 1); break;
        case 0x5b: msg->set_key(m_names.n3, 1); break;
        case 0x21: msg->set_key(m_names.n4, 1); break;
        case 0x22: msg->set_key(m_names.n5, 1); break;
        case 0x23: msg->set_key(m_names.n6, 1); break;

          // 7 - 11
        case 0x24: msg->set_key(m_names.n7,  1); break;
        case 0x25: msg->set_key(m_names.n8,  1); break;
        case 0x26: msg->set_key(m_names.n9,  1); break;
        case 0x27: msg->set_key(m_names.n10, 1); break;
        case 0x40: msg->set_key(m_names.n11, 1); break;

          // up, down, left, right
        case 0x1a: msg->set_key(m_names.up,    1); break;
        case 0x16: msg->set_key(m_names.down,  1); break;
        case 0x04: msg->set_key(m_names.left,  1); break;
        case 0x07: msg->set_key(m_names.right, 1); break;
        case 0x14: msg->set_key(m_names.q,  1); break;
        case 0x08: msg->set_key(m_names.e, 1); break;

          // left middle
        case 0x42: msg->set_key(m_names.c1, 1); break;
        case 0x2b: msg->set_key(m_names.tab, 1); break;
        case 0x5e: msg->set_key(m_names.c2, 1); break;
          //case 0x49: msg->set_key(m_names.talk, 1); break;

          // right middle
        case 0x3e: msg->set_key(m_names.c3, 1); break;
        case 0x10: msg->set_key(m_names.c4, 1); break;
        case 0x15: msg->set_key(m_names.reload, 1); break;
        case 0x09: msg->set_key(m_names.use, 1); break;

          // left bottom
        case 0x38: msg->set_key(m_names.c8, 1); break;
        case 0x4c: msg->set_key(m_names.p2, 1); break;
        case 0x50: msg->set_key(m_names.n, 1); break;
          
          // right bottom
        case 0x19: msg->set_key(m_names.c5, 1); break;
        case 0x05: msg->set_key(m_names.c6, 1); break;
        case 0x06: msg->set_key(m_names.c7, 1); break;
        case 0x28: msg->set_key(m_names.p1, 1); break;
        case 0x2c: msg->set_key(m_names.space, 1); break;

        case 1:
          // error
          break;

        case 0:
          // ignore
          break;

        default:
          log_error("unhandled scancode: " << static_cast<int>(scancode));
          break;
      }
    }

    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */

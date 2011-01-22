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

#include "xbox_controller_factory.hpp"

#include <stdexcept>

#include "xbox360_controller.hpp"
#include "xbox360_wireless_controller.hpp"
#include "saitek_p2500_controller.hpp"
#include "firestorm_dual_controller.hpp"
#include "xbox_controller.hpp"

std::auto_ptr<XboxGenericController>
XboxControllerFactory::create(const XPadDevice& dev_type, libusb_device* dev, const Options& opts)
{
  switch (dev_type.type)
  {
    case GAMEPAD_XBOX360_PLAY_N_CHARGE: 
      throw std::runtime_error("The Xbox360 Play&Charge cable is for recharging only, it does not transmit data, "
                               "thus xboxdrv can't support it. You have to get a wireless receiver:\n"
                               "\n"
                               "  * http://www.xbox.com/en-ca/hardware/x/xbox360wirelessgamingreceiver/");
      break;

    case GAMEPAD_XBOX:
    case GAMEPAD_XBOX_MAT:
      return std::auto_ptr<XboxGenericController>(new XboxController(dev, opts.detach_kernel_driver));

    case GAMEPAD_XBOX360:
    case GAMEPAD_XBOX360_GUITAR:
      return std::auto_ptr<XboxGenericController>(new Xbox360Controller(dev, 
                                                                        opts.chatpad, opts.chatpad_no_init, opts.chatpad_debug,
                                                                        opts.headset, 
                                                                        opts.headset_debug, 
                                                                        opts.headset_dump,
                                                                        opts.headset_play,
                                                                        opts.detach_kernel_driver));
      break;

    case GAMEPAD_XBOX360_WIRELESS:
      return std::auto_ptr<XboxGenericController>(new Xbox360WirelessController(dev, opts.wireless_id, opts.detach_kernel_driver));

    case GAMEPAD_FIRESTORM:
      return std::auto_ptr<XboxGenericController>(new FirestormDualController(dev, false, opts.detach_kernel_driver));

    case GAMEPAD_FIRESTORM_VSB:
      return std::auto_ptr<XboxGenericController>(new FirestormDualController(dev, true, opts.detach_kernel_driver));

    case GAMEPAD_SAITEK_P2500:
      return std::auto_ptr<XboxGenericController>(new SaitekP2500Controller(dev, opts.detach_kernel_driver));

    default:
      assert(!"Unknown gamepad type");
  }
}

/* EOF */

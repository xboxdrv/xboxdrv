/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
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

#include "controller_factory.hpp"

#include <assert.h>
#include <stdexcept>

#include "controller/firestorm_dual_controller.hpp"
#include "controller/generic_usb_controller.hpp"
#include "controller/hama_crux_controller.hpp"
#include "controller/logitech_f310_controller.hpp"
#include "controller/playstation3_usb_controller.hpp"
#include "controller/saitek_p2500_controller.hpp"
#include "controller/t_wireless_controller.hpp"
#include "controller/saitek_p3600_controller.hpp"
#include "controller/steam_controller.hpp"
#include "controller/xeox_controller.hpp"
#include "controller/wiimote_controller.hpp"
#include "controller/xbox360_controller.hpp"
#include "controller/xbox360_wireless_controller.hpp"
#include "controller/xbox_controller.hpp"
#include "controller/xboxone_wireless_controller.hpp"

namespace xboxdrv {

ControllerPtr
ControllerFactory::create(XPadDevice const& dev_type, libusb_device* dev, Options const& opts)
{
  switch (dev_type.type)
  {
    case GAMEPAD_XBOX360_PLAY_N_CHARGE:
      // FIXME: only trigger this error message in single-instance mode, not in daemon mode
      throw std::runtime_error(
        "USB id 045e:028f is the Xbox 360 Play&Charge path: charging only, no gamepad data. "
        "Use a wireless gaming receiver (045e:0719, 0291, or 02a9) for wireless pads, "
        "or a wired Xbox 360 controller (045e:028e). Forcing --type xbox360 will fail with "
        "'couldn't find matching endpoint' because this interface has no input endpoints.");
      break;

    case GAMEPAD_XBOX:
    case GAMEPAD_XBOX_MAT:
      return ControllerPtr(new XboxController(dev, opts.detach_kernel_driver));

    case GAMEPAD_XBOX360:
    case GAMEPAD_XBOX360_GUITAR:
      return ControllerPtr(new Xbox360Controller(dev,
                                                 opts.chatpad, opts.chatpad_no_init, opts.chatpad_debug,
                                                 opts.headset,
                                                 opts.headset_debug,
                                                 opts.headset_dump,
                                                 opts.headset_play,
                                                 opts.headset_pcm,
                                                        opts.headset_wav,
                                                 opts.headset_play_wav,
                                                 opts.headset_play_left_pack,
                                                 opts.headset_pulse,
                                                 opts.headset_pipewire,
                                                 opts.headset_mic_gain,
                                                 opts.detach_kernel_driver));
      break;

    case GAMEPAD_XBOX360_WIRELESS:
      return ControllerPtr(new Xbox360WirelessController(dev, opts.wireless_id,
                                                    opts.chatpad, opts.chatpad_no_init, opts.chatpad_debug,
                                                    opts.headset, opts.headset_debug,
                                                    opts.headset_dump, opts.headset_play,
                                                    opts.headset_pcm, opts.headset_wav,
                                                    opts.headset_play_wav, opts.headset_play_left_pack,
                                                    opts.headset_pulse, opts.headset_pipewire,
                                                    opts.headset_mic_gain,
                                                    opts.detach_kernel_driver,
                                                    opts.wireless_auto_poweroff,
                                                    opts.guide_poweroff_timeout_sec,
                                                    opts.quiet));

    case GAMEPAD_XBOXONE_WIRELESS:
      return ControllerPtr(new XboxOneWirelessController(dev, opts.detach_kernel_driver));

    case GAMEPAD_FIRESTORM:
      return ControllerPtr(new FirestormDualController(dev, false, opts.detach_kernel_driver));

    case GAMEPAD_FIRESTORM_VSB:
      return ControllerPtr(new FirestormDualController(dev, true, opts.detach_kernel_driver));

    case GAMEPAD_T_WIRELESS:
      return ControllerPtr(new TWirelessController(dev, opts.detach_kernel_driver));

    case GAMEPAD_SAITEK_P2500:
      return ControllerPtr(new SaitekP2500Controller(dev, opts.detach_kernel_driver));

    case GAMEPAD_SAITEK_P3600:
      return ControllerPtr(new SaitekP3600Controller(dev, opts.detach_kernel_driver));

    case GAMEPAD_XEOX:
      return ControllerPtr(new XeoxController(dev, opts.detach_kernel_driver));

    case GAMEPAD_LOGITECH_F310:
      return ControllerPtr(new LogitechF310Controller(dev, opts.detach_kernel_driver));

    case GAMEPAD_PLAYSTATION3_USB:
      return ControllerPtr(new Playstation3USBController(dev, opts.detach_kernel_driver));

    case GAMEPAD_STEAM:
      return ControllerPtr(new SteamController(dev, 0, opts.detach_kernel_driver));

    case GAMEPAD_STEAM_WIRELESS:
      return ControllerPtr(new SteamController(dev, static_cast<uint8_t>(opts.wireless_id + 1), opts.detach_kernel_driver));

#ifdef HAVE_CWIID
    case GAMEPAD_WIIMOTE:
      return ControllerPtr(new WiimoteController);
#else
    case GAMEPAD_WIIMOTE:
      throw std::runtime_error("libcwiid not found at compile time, Wiimote support is not available");
#endif

    case GAMEPAD_HAMA_CRUX:
      return ControllerPtr(new HamaCruxController(dev, opts.detach_kernel_driver));

    case GAMEPAD_GENERIC_USB:
      {
        Options::GenericUSBSpec spec = opts.find_generic_usb_spec(dev_type.idVendor, dev_type.idProduct);
        return ControllerPtr(new GenericUSBController(dev, spec.m_interface, spec.m_endpoint,
                                                      opts.detach_kernel_driver));
      }

    default:
      assert(false && "unknown gamepad type");
      return {};
  }
}

std::vector<ControllerPtr>
ControllerFactory::create_multiple(XPadDevice const& dev_type, libusb_device* dev, Options const& opts)
{
  std::vector<ControllerPtr> lst;

  switch (dev_type.type)
  {
    case GAMEPAD_XBOX360_PLAY_N_CHARGE:
      // FIXME: only trigger this error message in single-instance mode, not in daemon mode
      throw std::runtime_error(
        "USB id 045e:028f is the Xbox 360 Play&Charge path: charging only, no gamepad data. "
        "Use a wireless gaming receiver (045e:0719, 0291, or 02a9) for wireless pads, "
        "or a wired Xbox 360 controller (045e:028e). Forcing --type xbox360 will fail with "
        "'couldn't find matching endpoint' because this interface has no input endpoints.");
      break;

    case GAMEPAD_XBOX:
    case GAMEPAD_XBOX_MAT:
      lst.push_back(ControllerPtr(new XboxController(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_XBOX360:
    case GAMEPAD_XBOX360_GUITAR:
      lst.push_back(ControllerPtr(new Xbox360Controller(dev,
                                                        opts.chatpad, opts.chatpad_no_init, opts.chatpad_debug,
                                                        opts.headset,
                                                        opts.headset_debug,
                                                        opts.headset_dump,
                                                        opts.headset_play,
                                                        opts.headset_pcm,
                                                        opts.headset_wav,
                                                 opts.headset_play_wav,
                                                 opts.headset_play_left_pack,
                                                        opts.headset_pulse,
                                                 opts.headset_pipewire,
                                                 opts.headset_mic_gain,
                                                        opts.detach_kernel_driver)));
      break;

    case GAMEPAD_XBOX360_WIRELESS:
      for(int wireless_id = 0; wireless_id < 4; ++wireless_id)
      {
        lst.push_back(ControllerPtr(new Xbox360WirelessController(dev, wireless_id,
                                                                 opts.chatpad, opts.chatpad_no_init, opts.chatpad_debug,
                                                                 opts.headset, opts.headset_debug,
                                                                 opts.headset_dump, opts.headset_play,
                                                                 opts.headset_pcm, opts.headset_wav,
                                                                 opts.headset_play_wav, opts.headset_play_left_pack,
                                                                 opts.headset_pulse, opts.headset_pipewire,
                                                                 opts.headset_mic_gain,
                                                                 opts.detach_kernel_driver,
                                                                 opts.wireless_auto_poweroff,
                                                                 opts.guide_poweroff_timeout_sec,
                                                                 opts.quiet)));
      }
      break;

    case GAMEPAD_XBOXONE_WIRELESS:
      lst.push_back(ControllerPtr(new XboxOneWirelessController(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_FIRESTORM:
      lst.push_back(ControllerPtr(new FirestormDualController(dev, false, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_FIRESTORM_VSB:
      lst.push_back(ControllerPtr(new FirestormDualController(dev, true, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_T_WIRELESS:
      lst.push_back(ControllerPtr(new TWirelessController(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_SAITEK_P2500:
      lst.push_back(ControllerPtr(new SaitekP2500Controller(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_SAITEK_P3600:
      lst.push_back(ControllerPtr(new SaitekP3600Controller(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_XEOX:
      lst.push_back(ControllerPtr(new XeoxController(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_LOGITECH_F310:
      lst.push_back(ControllerPtr(new LogitechF310Controller(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_PLAYSTATION3_USB:
      lst.push_back(ControllerPtr(new Playstation3USBController(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_STEAM:
      lst.push_back(ControllerPtr(new SteamController(dev, 0, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_STEAM_WIRELESS:
      for (int id = 1; id <= 4; ++id)
        lst.push_back(ControllerPtr(new SteamController(dev, static_cast<uint8_t>(id), opts.detach_kernel_driver)));
      break;

#ifdef HAVE_CWIID
    case GAMEPAD_WIIMOTE:
      lst.push_back(ControllerPtr(new WiimoteController));
      break;
#else
    case GAMEPAD_WIIMOTE:
      throw std::runtime_error("libcwiid not found at compile time, Wiimote support is not available");
#endif

    case GAMEPAD_HAMA_CRUX:
      lst.push_back(ControllerPtr(new HamaCruxController(dev, opts.detach_kernel_driver)));
      break;

    case GAMEPAD_GENERIC_USB:
      {
        Options::GenericUSBSpec spec = opts.find_generic_usb_spec(dev_type.idVendor, dev_type.idProduct);
        lst.push_back(ControllerPtr(new GenericUSBController(dev, spec.m_interface, spec.m_endpoint,
                                                             opts.detach_kernel_driver)));
      }
      break;

    default:
      assert(false && "unknown gamepad type");
  }

  return lst;
}

} // namespace xboxdrv

/* EOF */

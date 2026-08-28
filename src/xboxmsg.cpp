/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008-2026 Ingo Ruhnke <grumbel@gmail.com>
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

#include "xboxmsg.hpp"

#include "raise_exception.hpp"
#include "unpack.hpp"

namespace xboxdrv {

std::string gamepadtype_to_string(GamepadType const& type)
{
  switch (type)
  {
    case GAMEPAD_XBOX360:
      return "xbox360";

    case GAMEPAD_XBOX360_WIRELESS:
      return "xbox360-wireless";

    case GAMEPAD_XBOX360_PLAY_N_CHARGE:
      return "xbox360-playncharge";

    case GAMEPAD_XBOX:
      return "xbox";

    case GAMEPAD_XBOX_MAT:
      return "xbox-mat";

    case GAMEPAD_XBOX360_GUITAR:
      return "xbox360-guitar";

    case GAMEPAD_XBOXONE_WIRELESS:
      return "xbox-one-wireless";

    case GAMEPAD_FIRESTORM:
      return "firestorm";

    case GAMEPAD_FIRESTORM_VSB:
      return "firestorm-vsb";

    case GAMEPAD_T_WIRELESS:
      return "t-wireless";

    case GAMEPAD_SAITEK_P2500:
      return "saitek-p2500";

    case GAMEPAD_SAITEK_P3600:
      return "saitek-p3600";

    case GAMEPAD_XEOX:
      return "xeox";

    case GAMEPAD_LOGITECH_F310:
      return "logitech-f310";

    case GAMEPAD_PLAYSTATION3_USB:
      return "playstation3-usb";

    case GAMEPAD_STEAM:
      return "steam";

    case GAMEPAD_STEAM_WIRELESS:
      return "steam-wireless";

    case GAMEPAD_GENERIC_USB:
      return "generic-usb";

    case GAMEPAD_PLAYSTATION3_BLUETOOTH:
      return "playstation3-bluetooth";

    case GAMEPAD_WIIMOTE:
      return "wiimote";

    case GAMEPAD_HAMA_CRUX:
      return "hama-crux";

    case GAMEPAD_UNKNOWN:
      return "unknown";
  }
  assert(false && "Unknown gamepad type supplied");
  return {};
}


std::string gamepadtype_to_macro_string(GamepadType const& type)
{
  switch (type)
  {
    case GAMEPAD_XBOX360: return "GAMEPAD_XBOX360";
    case GAMEPAD_XBOX360_WIRELESS: return "GAMEPAD_XBOX360_WIRELESS";
    case GAMEPAD_XBOX360_PLAY_N_CHARGE: return "GAMEPAD_XBOX360_PLAY_N_CHARGE";
    case GAMEPAD_XBOX: return "GAMEPAD_XBOX";
    case GAMEPAD_XBOX_MAT: return "GAMEPAD_XBOX_MAT";
    case GAMEPAD_XBOX360_GUITAR: return "GAMEPAD_XBOX360_GUITAR";
    case GAMEPAD_XBOXONE_WIRELESS: return "GAMEPAD_XBOXONE_WIRELESS";
    case GAMEPAD_FIRESTORM: return "GAMEPAD_FIRESTORM";
    case GAMEPAD_FIRESTORM_VSB: return "GAMEPAD_FIRESTORM_VSB";
    case GAMEPAD_T_WIRELESS: return "GAMEPAD_T_WIRELESS";
    case GAMEPAD_SAITEK_P2500: return "GAMEPAD_SAITEK_P2500";
    case GAMEPAD_SAITEK_P3600: return "GAMEPAD_SAITEK_P3600";
    case GAMEPAD_XEOX: return "GAMEPAD_XEOX";
    case GAMEPAD_PLAYSTATION3_USB: return "GAMEPAD_PLAYSTATION3_USB";
    case GAMEPAD_STEAM: return "GAMEPAD_STEAM";
    case GAMEPAD_STEAM_WIRELESS: return "GAMEPAD_STEAM_WIRELESS";
    case GAMEPAD_PLAYSTATION3_BLUETOOTH: return "GAMEPAD_PLAYSTATION3_BLUETOOTH";
    case GAMEPAD_LOGITECH_F310: return "GAMEPAD_LOGITECH_F310";
    case GAMEPAD_GENERIC_USB: return "GAMEPAD_GENERIC_USB";
    case GAMEPAD_WIIMOTE: return "GAMEPAD_WIIMOTE";
    case GAMEPAD_HAMA_CRUX: return "GAMEPAD_HAMA_CRUX";
    case GAMEPAD_UNKNOWN: return "GAMEPAD_UNKNOWN";
  }
  assert(false && "Unknown gamepad type supplied");
  return {};
}

std::ostream& operator<<(std::ostream& out, GamepadType const& type)
{
  switch (type)
  {
    case GAMEPAD_XBOX360:
      return out << "Xbox360";

    case GAMEPAD_XBOX360_WIRELESS:
      return out << "Xbox360 (wireless)";

    case GAMEPAD_XBOX360_PLAY_N_CHARGE:
      return out << "Xbox360 Play&Charge";

    case GAMEPAD_XBOX:
      return out << "Xbox Classic";

    case GAMEPAD_XBOX_MAT:
      return out << "Xbox Dancepad";

    case GAMEPAD_XBOX360_GUITAR:
      return out << "Xbox360 Guitar";

    case GAMEPAD_XBOXONE_WIRELESS:
      return out << "Xbox One wireless";

    case GAMEPAD_FIRESTORM:
      return out << "Firestorm Dual Power";

    case GAMEPAD_FIRESTORM_VSB:
      return out << "Firestorm Dual Power (vsb)";

    case GAMEPAD_T_WIRELESS:
      return out << "Thrustmaster T-Wireless";

    case GAMEPAD_SAITEK_P2500:
      return out << "Saitek P2500";

    case GAMEPAD_SAITEK_P3600:
      return out << "Saitek P3600 (Cyborg Rumble)";

    case GAMEPAD_XEOX:
      return out << "Speedlink Xeox USB Gamepad";

    case GAMEPAD_PLAYSTATION3_USB:
      return out << "Playstation 3 USB";
    case GAMEPAD_STEAM:
      return out << "Steam Controller";
    case GAMEPAD_STEAM_WIRELESS:
      return out << "Steam Controller (wireless)";

    case GAMEPAD_PLAYSTATION3_BLUETOOTH:
      return out << "Playstation 3 Bluetooth";

    case GAMEPAD_WIIMOTE:
      return out << "Wiimote";

    case GAMEPAD_GENERIC_USB:
      return out << "Generic USB";

    case GAMEPAD_LOGITECH_F310:
      return out << "Logitech F310";

    case GAMEPAD_HAMA_CRUX:
      return out << "Hama Crux Gaming Keyboard";

    case GAMEPAD_UNKNOWN:
      return out << "unknown";
  }

  return out << "unknown" << std::endl;
}

} // namespace xboxdrv

/* EOF */

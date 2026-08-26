/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "controller/evdev_controller.hpp"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string.h>

#include <strut/split.hpp>
#include <logmich/log.hpp>

#include "controller_message.hpp"
#include "evdev_helper.hpp"
#include "raise_exception.hpp"
#include "util/string.hpp"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

namespace xboxdrv {

namespace {

/** Short aliases used in configs and older examples. */
std::string expand_button_alias(std::string name)
{
  if (name == "du") return "dpad_up";
  if (name == "dd") return "dpad_down";
  if (name == "dl") return "dpad_left";
  if (name == "dr") return "dpad_right";
  if (name == "tl") return "thumb_l";
  if (name == "tr") return "thumb_r";
  if (name == "lb" || name == "white") return name == "white" ? "lb" : name;
  if (name == "rb" || name == "black") return name == "black" ? "rb" : name;
  return name;
}

} // namespace

std::string
EvdevController::normalize_target_name(std::string name)
{
  // trim
  auto not_space = [](unsigned char c) { return !std::isspace(c); };
  name.erase(name.begin(), std::find_if(name.begin(), name.end(), not_space));
  name.erase(std::find_if(name.rbegin(), name.rend(), not_space).base(), name.end());

  // strip common Linux-style prefixes from the RHS
  auto starts_with = [&](char const* p) {
    size_t n = strlen(p);
    return name.size() >= n && name.compare(0, n, p) == 0;
  };
  if (starts_with("BTN_") || starts_with("btn_"))
    name = name.substr(4);
  else if (starts_with("ABS_") || starts_with("abs_"))
    name = name.substr(4);
  else if (starts_with("KEY_") || starts_with("key_"))
    name = name.substr(4);

  std::transform(name.begin(), name.end(), name.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  return expand_button_alias(name);
}

EvdevKeyBinding
EvdevController::parse_key_binding(std::string const& value)
{
  EvdevKeyBinding binding;

  // Axis form: name:press[:release]  (no '+' in the value)
  if (value.find('+') == std::string::npos && value.find(':') != std::string::npos)
  {
    std::vector<std::string> parts = strut::split(value, ':');
    if (parts.size() < 2 || parts.size() > 3)
    {
      raise_exception(std::runtime_error,
                      "evdev-keymap axis binding must be name:press or name:press:release, got \""
                      + value + "\"");
    }

    std::string name = normalize_target_name(parts[0]);
    if (name.empty())
    {
      raise_exception(std::runtime_error, "evdev-keymap: empty axis name in \"" + value + "\"");
    }

    EvdevKeyBinding::Abs abs;
    abs.abs = m_message_descriptor.abs().put(name);
    abs.press = str2int(parts[1]);
    if (parts.size() == 3)
    {
      abs.release = str2int(parts[2]);
      abs.on_release = true;
    }
    else
    {
      abs.release = 0;
      abs.on_release = false;
    }
    binding.abs = abs;
    return binding;
  }

  // Button chord: name or name+name+...
  std::vector<std::string> tokens = strut::split(value, '+');
  if (tokens.empty())
  {
    raise_exception(std::runtime_error, "evdev-keymap: empty binding");
  }

  for (std::string const& token : tokens)
  {
    std::string name = normalize_target_name(token);
    if (name.empty())
    {
      raise_exception(std::runtime_error,
                      "evdev-keymap: empty button name in \"" + value + "\"");
    }
    binding.keys.push_back(m_message_descriptor.key().put(name));
  }

  return binding;
}

EvdevController::EvdevController(std::string const& filename,
                                 const std::map<int, std::string>& absmap,
                                 const std::map<int, std::string>& keymap,
                                 const std::map<int, std::string>& relmap,
                                 bool grab,
                                 bool debug) :
  m_fd(-1),
  m_io_channel(),
  m_name(),
  m_grab(grab),
  m_debug(debug),
  m_absmap(),
  m_relmap(),
  m_keymap(),
  m_absinfo(ABS_MAX),
  m_event_buffer(),
  m_msg()
{
  m_fd = open(filename.c_str(), O_RDONLY | O_NONBLOCK);

  if (m_fd == -1)
  {
    throw std::runtime_error(filename + ": " + std::string(strerror(errno)));
  }

  { // Get the human readable name
    char c_name[1024] = "unknown";
    ioctl(m_fd, EVIOCGNAME(sizeof(c_name)), c_name);
    m_name = c_name;
    log_debug("name: {}", m_name);
  }

  if (m_grab)
  { // grab the device, so it doesn't broadcast events into the wild
    int ret = ioctl(m_fd, EVIOCGRAB, 1);
    if ( ret == -1 )
    {
      close(m_fd);
      throw std::runtime_error(strerror(errno));
    }
  }

  { // Read in how many btn/abs/rel the device has
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
    memset(bit, 0, sizeof(bit));
    ioctl(m_fd, EVIOCGBIT(0, EV_MAX), bit[0]);

    unsigned long abs_bit[NBITS(ABS_MAX)];
    unsigned long rel_bit[NBITS(REL_MAX)];
    unsigned long key_bit[NBITS(KEY_MAX)];

    memset(abs_bit, 0, sizeof(abs_bit));
    memset(rel_bit, 0, sizeof(rel_bit));
    memset(key_bit, 0, sizeof(key_bit));

    ioctl(m_fd, EVIOCGBIT(EV_ABS, ABS_MAX), abs_bit);
    ioctl(m_fd, EVIOCGBIT(EV_REL, REL_MAX), rel_bit);
    ioctl(m_fd, EVIOCGBIT(EV_KEY, KEY_MAX), key_bit);

    for(int i = 0; i < ABS_MAX; ++i)
    {
      if (test_bit(i, abs_bit))
      {
        struct input_absinfo absinfo;
        ioctl(m_fd, EVIOCGABS(i), &absinfo);

        log_debug("abs: {:-20s} min: {:6d} max: {:6d}", abs2str(i), absinfo.minimum, absinfo.maximum);
        m_absinfo[i] = absinfo;

        std::map<int, std::string>::const_iterator it = absmap.find(i);
        if (it == absmap.end())
        {
          // install default mapping
          m_absmap[i] = m_message_descriptor.abs().put("evdev." + abs2str(i));
        }
        else
        {
          // user mapping → canonical name (x1, y1, lt, …)
          m_absmap[i] = m_message_descriptor.abs().put(normalize_target_name(it->second));
        }
      }
    }

    for(int i = 0; i < REL_MAX; ++i)
    {
      if (test_bit(i, rel_bit))
      {
        log_debug("rel: {}", rel2str(i));

        std::map<int, std::string>::const_iterator it = relmap.find(i);
        if (it == relmap.end())
        {
          m_relmap[i] = m_message_descriptor.rel().put("evdev." + rel2str(i));
        }
        else
        {
          m_relmap[i] = m_message_descriptor.rel().put(normalize_target_name(it->second));
        }
      }
    }

    for(int i = 0; i < KEY_MAX; ++i)
    {
      if (test_bit(i, key_bit))
      {
        log_debug("key: {}", key2str(i));

        std::map<int, std::string>::const_iterator it = keymap.find(i);
        if (it == keymap.end())
        {
          // default: one button channel named after the Linux key
          EvdevKeyBinding b;
          b.keys.push_back(m_message_descriptor.key().put("evdev." + key2str(i)));
          m_keymap[i] = b;
        }
        else
        {
          m_keymap[i] = parse_key_binding(it->second);
        }
      }
    }
  }

  { // start g_io_channel
    m_io_channel = g_io_channel_unix_new(m_fd);

    GError* error = NULL;
    if (g_io_channel_set_encoding(m_io_channel, NULL, &error) != G_IO_STATUS_NORMAL)
    {
      log_error(error->message);
      g_error_free(error);
    }

    g_io_channel_set_buffered(m_io_channel, false);

    g_io_add_watch(m_io_channel,
                   static_cast<GIOCondition>(G_IO_IN | G_IO_ERR | G_IO_HUP),
                   &EvdevController::on_read_data_wrap, this);
  }
}

EvdevController::~EvdevController()
{
  g_io_channel_unref(m_io_channel);
  close(m_fd);
}

void
EvdevController::set_rumble_real(uint8_t left, uint8_t right)
{
  // not implemented
}

void
EvdevController::set_led_real(uint8_t status)
{
  // not implemented
}

bool
EvdevController::parse(const struct input_event& ev, ControllerMessage& msg_inout) const
{
  if (m_debug)
  {
    switch(ev.type)
    {
      case EV_KEY:
        std::cout << "EV_KEY " << key2str(ev.code) << " " << ev.value << std::endl;
        break;

      case EV_REL:
        std::cout << "EV_REL " << rel2str(ev.code) << " " << ev.value << std::endl;
        break;

      case EV_ABS:
        std::cout << "EV_ABS " << abs2str(ev.code) << " " << ev.value << std::endl;
        break;

      case EV_SYN:
        std::cout << "------------------- sync -------------------" << std::endl;
        break;

      case EV_MSC:
        break;

      default:
        log_info("unknown: {} {} {}", ev.type, ev.code, ev.value);
        break;
    }
  }

  switch(ev.type)
  {
    case EV_KEY:
      {
        auto it = m_keymap.find(ev.code);
        if (it == m_keymap.end())
        {
          return false;
        }

        EvdevKeyBinding const& b = it->second;
        // Linux: 0 = release, 1 = press, 2 = autorepeat — treat non-zero as down
        bool const pressed = (ev.value != 0);

        for (int key : b.keys)
        {
          msg_inout.set_key(key, pressed);
        }

        if (b.abs)
        {
          EvdevKeyBinding::Abs const& a = *b.abs;
          if (pressed)
          {
            // stick range matches wired 360 reports; triggers often 0..255
            int amin = -32768;
            int amax = 32767;
            if (a.press >= 0 && a.press <= 255 && a.release >= 0 && a.release <= 255)
            {
              amin = 0;
              amax = 255;
            }
            msg_inout.set_abs(a.abs, a.press, amin, amax);
          }
          else if (a.on_release)
          {
            int amin = -32768;
            int amax = 32767;
            if (a.press >= 0 && a.press <= 255 && a.release >= 0 && a.release <= 255)
            {
              amin = 0;
              amax = 255;
            }
            msg_inout.set_abs(a.abs, a.release, amin, amax);
          }
        }

        return true;
      }

    case EV_ABS:
      {
        EvMap::const_iterator it = m_absmap.find(ev.code);
        if (it != m_absmap.end())
        {
          const struct input_absinfo& absinfo = m_absinfo[ev.code];
          msg_inout.set_abs(it->second, ev.value, absinfo.minimum, absinfo.maximum);
          return true;
        }
        return false;
      }

    case EV_REL:
      {
        EvMap::const_iterator it = m_relmap.find(ev.code);
        if (it != m_relmap.end())
        {
          msg_inout.set_rel(it->second, ev.value);
          return true;
        }
        return false;
      }

    default:
      return false;
  }
}

gboolean
EvdevController::on_read_data(GIOChannel* source, GIOCondition condition)
{
  struct input_event ev[128];
  ssize_t rd = 0;
  while((rd = ::read(m_fd, ev, sizeof(struct input_event) * 128)) > 0)
  {
    for (size_t i = 0; i < rd / sizeof(struct input_event); ++i)
    {
      if (ev[i].type == EV_SYN)
      {
        submit_msg(m_msg, m_message_descriptor);
      }
      else
      {
        parse(ev[i], m_msg);
      }
    }
  }

  return TRUE;
}

} // namespace xboxdrv

/* EOF */

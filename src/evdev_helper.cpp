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

#include <X11/Xlib.h>
#include <linux/input.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>

#include "enum_box.hpp"
#include "evdev_helper.hpp"

class EvDevRelEnum : public EnumBox<int>
{
public:
  EvDevRelEnum() 
    : EnumBox<int>("EV_REL")
  {
    // File.new("/usr/include/linux/input.h")
    //  .grep(/^#define REL/)
    //  .each{|i| name = i.split[1]; puts "add(%s,%s\"%s\");" % [name, " " * (20-name.length), name] };
    add(REL_X,               "REL_X");
    add(REL_Y,               "REL_Y");
    add(REL_Z,               "REL_Z");
    add(REL_RX,              "REL_RX");
    add(REL_RY,              "REL_RY");
    add(REL_RZ,              "REL_RZ");
    add(REL_HWHEEL,          "REL_HWHEEL");
    add(REL_DIAL,            "REL_DIAL");
    add(REL_WHEEL,           "REL_WHEEL");
    add(REL_MISC,            "REL_MISC");
  }
} evdev_rel_names;

class EvDevAbsEnum : public EnumBox<int>
{
public:
  EvDevAbsEnum() 
    : EnumBox<int>("EV_ABS")
  {
    // File.new("/usr/include/linux/input.h")
    //  .grep(/^#define ABS/)
    //  .each{|i| name = i.split[1]; puts "add(%s,%s\"%s\");" % [name, " " * (20-name.length), name] };
    add(ABS_X,               "ABS_X");
    add(ABS_Y,               "ABS_Y");
    add(ABS_Z,               "ABS_Z");
    add(ABS_RX,              "ABS_RX");
    add(ABS_RY,              "ABS_RY");
    add(ABS_RZ,              "ABS_RZ");
    add(ABS_THROTTLE,        "ABS_THROTTLE");
    add(ABS_RUDDER,          "ABS_RUDDER");
    add(ABS_WHEEL,           "ABS_WHEEL");
    add(ABS_GAS,             "ABS_GAS");
    add(ABS_BRAKE,           "ABS_BRAKE");
    add(ABS_HAT0X,           "ABS_HAT0X");
    add(ABS_HAT0Y,           "ABS_HAT0Y");
    add(ABS_HAT1X,           "ABS_HAT1X");
    add(ABS_HAT1Y,           "ABS_HAT1Y");
    add(ABS_HAT2X,           "ABS_HAT2X");
    add(ABS_HAT2Y,           "ABS_HAT2Y");
    add(ABS_HAT3X,           "ABS_HAT3X");
    add(ABS_HAT3Y,           "ABS_HAT3Y");
    add(ABS_PRESSURE,        "ABS_PRESSURE");
    add(ABS_DISTANCE,        "ABS_DISTANCE");
    add(ABS_TILT_X,          "ABS_TILT_X");
    add(ABS_TILT_Y,          "ABS_TILT_Y");
    add(ABS_TOOL_WIDTH,      "ABS_TOOL_WIDTH");
    add(ABS_VOLUME,          "ABS_VOLUME");
    add(ABS_MISC,            "ABS_MISC");
  }
} evdev_abs_names;

class EvDevKeyEnum : public EnumBox<int>
{
public:
  EvDevKeyEnum() 
    : EnumBox<int>("EV_KEY")
  {
    // File.new("/usr/include/linux/input.h")
    //  .grep(/^#define (BTN|KEY)/)
    //  .each{|i| name = i.split[1]; puts "add(%s,%s\"%s\");" % [name, " " * (20-name.length), name] };
    add(KEY_RESERVED,        "KEY_RESERVED");
    add(KEY_ESC,             "KEY_ESC");
    add(KEY_1,               "KEY_1");
    add(KEY_2,               "KEY_2");
    add(KEY_3,               "KEY_3");
    add(KEY_4,               "KEY_4");
    add(KEY_5,               "KEY_5");
    add(KEY_6,               "KEY_6");
    add(KEY_7,               "KEY_7");
    add(KEY_8,               "KEY_8");
    add(KEY_9,               "KEY_9");
    add(KEY_0,               "KEY_0");
    add(KEY_MINUS,           "KEY_MINUS");
    add(KEY_EQUAL,           "KEY_EQUAL");
    add(KEY_BACKSPACE,       "KEY_BACKSPACE");
    add(KEY_TAB,             "KEY_TAB");
    add(KEY_Q,               "KEY_Q");
    add(KEY_W,               "KEY_W");
    add(KEY_E,               "KEY_E");
    add(KEY_R,               "KEY_R");
    add(KEY_T,               "KEY_T");
    add(KEY_Y,               "KEY_Y");
    add(KEY_U,               "KEY_U");
    add(KEY_I,               "KEY_I");
    add(KEY_O,               "KEY_O");
    add(KEY_P,               "KEY_P");
    add(KEY_LEFTBRACE,       "KEY_LEFTBRACE");
    add(KEY_RIGHTBRACE,      "KEY_RIGHTBRACE");
    add(KEY_ENTER,           "KEY_ENTER");
    add(KEY_LEFTCTRL,        "KEY_LEFTCTRL");
    add(KEY_A,               "KEY_A");
    add(KEY_S,               "KEY_S");
    add(KEY_D,               "KEY_D");
    add(KEY_F,               "KEY_F");
    add(KEY_G,               "KEY_G");
    add(KEY_H,               "KEY_H");
    add(KEY_J,               "KEY_J");
    add(KEY_K,               "KEY_K");
    add(KEY_L,               "KEY_L");
    add(KEY_SEMICOLON,       "KEY_SEMICOLON");
    add(KEY_APOSTROPHE,      "KEY_APOSTROPHE");
    add(KEY_GRAVE,           "KEY_GRAVE");
    add(KEY_LEFTSHIFT,       "KEY_LEFTSHIFT");
    add(KEY_BACKSLASH,       "KEY_BACKSLASH");
    add(KEY_Z,               "KEY_Z");
    add(KEY_X,               "KEY_X");
    add(KEY_C,               "KEY_C");
    add(KEY_V,               "KEY_V");
    add(KEY_B,               "KEY_B");
    add(KEY_N,               "KEY_N");
    add(KEY_M,               "KEY_M");
    add(KEY_COMMA,           "KEY_COMMA");
    add(KEY_DOT,             "KEY_DOT");
    add(KEY_SLASH,           "KEY_SLASH");
    add(KEY_RIGHTSHIFT,      "KEY_RIGHTSHIFT");
    add(KEY_KPASTERISK,      "KEY_KPASTERISK");
    add(KEY_LEFTALT,         "KEY_LEFTALT");
    add(KEY_SPACE,           "KEY_SPACE");
    add(KEY_CAPSLOCK,        "KEY_CAPSLOCK");
    add(KEY_F1,              "KEY_F1");
    add(KEY_F2,              "KEY_F2");
    add(KEY_F3,              "KEY_F3");
    add(KEY_F4,              "KEY_F4");
    add(KEY_F5,              "KEY_F5");
    add(KEY_F6,              "KEY_F6");
    add(KEY_F7,              "KEY_F7");
    add(KEY_F8,              "KEY_F8");
    add(KEY_F9,              "KEY_F9");
    add(KEY_F10,             "KEY_F10");
    add(KEY_NUMLOCK,         "KEY_NUMLOCK");
    add(KEY_SCROLLLOCK,      "KEY_SCROLLLOCK");
    add(KEY_KP7,             "KEY_KP7");
    add(KEY_KP8,             "KEY_KP8");
    add(KEY_KP9,             "KEY_KP9");
    add(KEY_KPMINUS,         "KEY_KPMINUS");
    add(KEY_KP4,             "KEY_KP4");
    add(KEY_KP5,             "KEY_KP5");
    add(KEY_KP6,             "KEY_KP6");
    add(KEY_KPPLUS,          "KEY_KPPLUS");
    add(KEY_KP1,             "KEY_KP1");
    add(KEY_KP2,             "KEY_KP2");
    add(KEY_KP3,             "KEY_KP3");
    add(KEY_KP0,             "KEY_KP0");
    add(KEY_KPDOT,           "KEY_KPDOT");
    add(KEY_ZENKAKUHANKAKU,  "KEY_ZENKAKUHANKAKU");
    add(KEY_102ND,           "KEY_102ND");
    add(KEY_F11,             "KEY_F11");
    add(KEY_F12,             "KEY_F12");
    add(KEY_RO,              "KEY_RO");
    add(KEY_KATAKANA,        "KEY_KATAKANA");
    add(KEY_HIRAGANA,        "KEY_HIRAGANA");
    add(KEY_HENKAN,          "KEY_HENKAN");
    add(KEY_KATAKANAHIRAGANA,"KEY_KATAKANAHIRAGANA");
    add(KEY_MUHENKAN,        "KEY_MUHENKAN");
    add(KEY_KPJPCOMMA,       "KEY_KPJPCOMMA");
    add(KEY_KPENTER,         "KEY_KPENTER");
    add(KEY_RIGHTCTRL,       "KEY_RIGHTCTRL");
    add(KEY_KPSLASH,         "KEY_KPSLASH");
    add(KEY_SYSRQ,           "KEY_SYSRQ");
    add(KEY_RIGHTALT,        "KEY_RIGHTALT");
    add(KEY_LINEFEED,        "KEY_LINEFEED");
    add(KEY_HOME,            "KEY_HOME");
    add(KEY_UP,              "KEY_UP");
    add(KEY_PAGEUP,          "KEY_PAGEUP");
    add(KEY_LEFT,            "KEY_LEFT");
    add(KEY_RIGHT,           "KEY_RIGHT");
    add(KEY_END,             "KEY_END");
    add(KEY_DOWN,            "KEY_DOWN");
    add(KEY_PAGEDOWN,        "KEY_PAGEDOWN");
    add(KEY_INSERT,          "KEY_INSERT");
    add(KEY_DELETE,          "KEY_DELETE");
    add(KEY_MACRO,           "KEY_MACRO");
    add(KEY_MUTE,            "KEY_MUTE");
    add(KEY_VOLUMEDOWN,      "KEY_VOLUMEDOWN");
    add(KEY_VOLUMEUP,        "KEY_VOLUMEUP");
    add(KEY_POWER,           "KEY_POWER");
    add(KEY_KPEQUAL,         "KEY_KPEQUAL");
    add(KEY_KPPLUSMINUS,     "KEY_KPPLUSMINUS");
    add(KEY_PAUSE,           "KEY_PAUSE");
    add(KEY_KPCOMMA,         "KEY_KPCOMMA");
    add(KEY_HANGEUL,         "KEY_HANGEUL");
    add(KEY_HANGUEL,         "KEY_HANGUEL");
    add(KEY_HANJA,           "KEY_HANJA");
    add(KEY_YEN,             "KEY_YEN");
    add(KEY_LEFTMETA,        "KEY_LEFTMETA");
    add(KEY_RIGHTMETA,       "KEY_RIGHTMETA");
    add(KEY_COMPOSE,         "KEY_COMPOSE");
    add(KEY_STOP,            "KEY_STOP");
    add(KEY_AGAIN,           "KEY_AGAIN");
    add(KEY_PROPS,           "KEY_PROPS");
    add(KEY_UNDO,            "KEY_UNDO");
    add(KEY_FRONT,           "KEY_FRONT");
    add(KEY_COPY,            "KEY_COPY");
    add(KEY_OPEN,            "KEY_OPEN");
    add(KEY_PASTE,           "KEY_PASTE");
    add(KEY_FIND,            "KEY_FIND");
    add(KEY_CUT,             "KEY_CUT");
    add(KEY_HELP,            "KEY_HELP");
    add(KEY_MENU,            "KEY_MENU");
    add(KEY_CALC,            "KEY_CALC");
    add(KEY_SETUP,           "KEY_SETUP");
    add(KEY_SLEEP,           "KEY_SLEEP");
    add(KEY_WAKEUP,          "KEY_WAKEUP");
    add(KEY_FILE,            "KEY_FILE");
    add(KEY_SENDFILE,        "KEY_SENDFILE");
    add(KEY_DELETEFILE,      "KEY_DELETEFILE");
    add(KEY_XFER,            "KEY_XFER");
    add(KEY_PROG1,           "KEY_PROG1");
    add(KEY_PROG2,           "KEY_PROG2");
    add(KEY_WWW,             "KEY_WWW");
    add(KEY_MSDOS,           "KEY_MSDOS");
    add(KEY_COFFEE,          "KEY_COFFEE");
    add(KEY_SCREENLOCK,      "KEY_SCREENLOCK");
    add(KEY_DIRECTION,       "KEY_DIRECTION");
    add(KEY_CYCLEWINDOWS,    "KEY_CYCLEWINDOWS");
    add(KEY_MAIL,            "KEY_MAIL");
    add(KEY_BOOKMARKS,       "KEY_BOOKMARKS");
    add(KEY_COMPUTER,        "KEY_COMPUTER");
    add(KEY_BACK,            "KEY_BACK");
    add(KEY_FORWARD,         "KEY_FORWARD");
    add(KEY_CLOSECD,         "KEY_CLOSECD");
    add(KEY_EJECTCD,         "KEY_EJECTCD");
    add(KEY_EJECTCLOSECD,    "KEY_EJECTCLOSECD");
    add(KEY_NEXTSONG,        "KEY_NEXTSONG");
    add(KEY_PLAYPAUSE,       "KEY_PLAYPAUSE");
    add(KEY_PREVIOUSSONG,    "KEY_PREVIOUSSONG");
    add(KEY_STOPCD,          "KEY_STOPCD");
    add(KEY_RECORD,          "KEY_RECORD");
    add(KEY_REWIND,          "KEY_REWIND");
    add(KEY_PHONE,           "KEY_PHONE");
    add(KEY_ISO,             "KEY_ISO");
    add(KEY_CONFIG,          "KEY_CONFIG");
    add(KEY_HOMEPAGE,        "KEY_HOMEPAGE");
    add(KEY_REFRESH,         "KEY_REFRESH");
    add(KEY_EXIT,            "KEY_EXIT");
    add(KEY_MOVE,            "KEY_MOVE");
    add(KEY_EDIT,            "KEY_EDIT");
    add(KEY_SCROLLUP,        "KEY_SCROLLUP");
    add(KEY_SCROLLDOWN,      "KEY_SCROLLDOWN");
    add(KEY_KPLEFTPAREN,     "KEY_KPLEFTPAREN");
    add(KEY_KPRIGHTPAREN,    "KEY_KPRIGHTPAREN");
    add(KEY_NEW,             "KEY_NEW");
    add(KEY_REDO,            "KEY_REDO");
    add(KEY_F13,             "KEY_F13");
    add(KEY_F14,             "KEY_F14");
    add(KEY_F15,             "KEY_F15");
    add(KEY_F16,             "KEY_F16");
    add(KEY_F17,             "KEY_F17");
    add(KEY_F18,             "KEY_F18");
    add(KEY_F19,             "KEY_F19");
    add(KEY_F20,             "KEY_F20");
    add(KEY_F21,             "KEY_F21");
    add(KEY_F22,             "KEY_F22");
    add(KEY_F23,             "KEY_F23");
    add(KEY_F24,             "KEY_F24");
    add(KEY_PLAYCD,          "KEY_PLAYCD");
    add(KEY_PAUSECD,         "KEY_PAUSECD");
    add(KEY_PROG3,           "KEY_PROG3");
    add(KEY_PROG4,           "KEY_PROG4");
    add(KEY_SUSPEND,         "KEY_SUSPEND");
    add(KEY_CLOSE,           "KEY_CLOSE");
    add(KEY_PLAY,            "KEY_PLAY");
    add(KEY_FASTFORWARD,     "KEY_FASTFORWARD");
    add(KEY_BASSBOOST,       "KEY_BASSBOOST");
    add(KEY_PRINT,           "KEY_PRINT");
    add(KEY_HP,              "KEY_HP");
    add(KEY_CAMERA,          "KEY_CAMERA");
    add(KEY_SOUND,           "KEY_SOUND");
    add(KEY_QUESTION,        "KEY_QUESTION");
    add(KEY_EMAIL,           "KEY_EMAIL");
    add(KEY_CHAT,            "KEY_CHAT");
    add(KEY_SEARCH,          "KEY_SEARCH");
    add(KEY_CONNECT,         "KEY_CONNECT");
    add(KEY_FINANCE,         "KEY_FINANCE");
    add(KEY_SPORT,           "KEY_SPORT");
    add(KEY_SHOP,            "KEY_SHOP");
    add(KEY_ALTERASE,        "KEY_ALTERASE");
    add(KEY_CANCEL,          "KEY_CANCEL");
    add(KEY_BRIGHTNESSDOWN,  "KEY_BRIGHTNESSDOWN");
    add(KEY_BRIGHTNESSUP,    "KEY_BRIGHTNESSUP");
    add(KEY_MEDIA,           "KEY_MEDIA");
    add(KEY_SWITCHVIDEOMODE, "KEY_SWITCHVIDEOMODE");
    add(KEY_KBDILLUMTOGGLE,  "KEY_KBDILLUMTOGGLE");
    add(KEY_KBDILLUMDOWN,    "KEY_KBDILLUMDOWN");
    add(KEY_KBDILLUMUP,      "KEY_KBDILLUMUP");
    add(KEY_SEND,            "KEY_SEND");
    add(KEY_REPLY,           "KEY_REPLY");
    add(KEY_FORWARDMAIL,     "KEY_FORWARDMAIL");
    add(KEY_SAVE,            "KEY_SAVE");
    add(KEY_DOCUMENTS,       "KEY_DOCUMENTS");
    add(KEY_BATTERY,         "KEY_BATTERY");
    add(KEY_BLUETOOTH,       "KEY_BLUETOOTH");
    add(KEY_WLAN,            "KEY_WLAN");
    add(KEY_UWB,             "KEY_UWB");
    add(KEY_UNKNOWN,         "KEY_UNKNOWN");
    add(KEY_VIDEO_NEXT,      "KEY_VIDEO_NEXT");
    add(KEY_VIDEO_PREV,      "KEY_VIDEO_PREV");
    add(KEY_BRIGHTNESS_CYCLE,"KEY_BRIGHTNESS_CYCLE");
    add(KEY_BRIGHTNESS_ZERO, "KEY_BRIGHTNESS_ZERO");
    add(KEY_DISPLAY_OFF,     "KEY_DISPLAY_OFF");
    add(KEY_WIMAX,           "KEY_WIMAX");
    add(BTN_MISC,            "BTN_MISC");
    add(BTN_0,               "BTN_0");
    add(BTN_1,               "BTN_1");
    add(BTN_2,               "BTN_2");
    add(BTN_3,               "BTN_3");
    add(BTN_4,               "BTN_4");
    add(BTN_5,               "BTN_5");
    add(BTN_6,               "BTN_6");
    add(BTN_7,               "BTN_7");
    add(BTN_8,               "BTN_8");
    add(BTN_9,               "BTN_9");
    add(BTN_MOUSE,           "BTN_MOUSE");
    add(BTN_LEFT,            "BTN_LEFT");
    add(BTN_RIGHT,           "BTN_RIGHT");
    add(BTN_MIDDLE,          "BTN_MIDDLE");
    add(BTN_SIDE,            "BTN_SIDE");
    add(BTN_EXTRA,           "BTN_EXTRA");
    add(BTN_FORWARD,         "BTN_FORWARD");
    add(BTN_BACK,            "BTN_BACK");
    add(BTN_TASK,            "BTN_TASK");
    add(BTN_JOYSTICK,        "BTN_JOYSTICK");
    add(BTN_TRIGGER,         "BTN_TRIGGER");
    add(BTN_THUMB,           "BTN_THUMB");
    add(BTN_THUMB2,          "BTN_THUMB2");
    add(BTN_TOP,             "BTN_TOP");
    add(BTN_TOP2,            "BTN_TOP2");
    add(BTN_PINKIE,          "BTN_PINKIE");
    add(BTN_BASE,            "BTN_BASE");
    add(BTN_BASE2,           "BTN_BASE2");
    add(BTN_BASE3,           "BTN_BASE3");
    add(BTN_BASE4,           "BTN_BASE4");
    add(BTN_BASE5,           "BTN_BASE5");
    add(BTN_BASE6,           "BTN_BASE6");
    add(BTN_DEAD,            "BTN_DEAD");
    add(BTN_GAMEPAD,         "BTN_GAMEPAD");
    add(BTN_A,               "BTN_A");
    add(BTN_B,               "BTN_B");
    add(BTN_C,               "BTN_C");
    add(BTN_X,               "BTN_X");
    add(BTN_Y,               "BTN_Y");
    add(BTN_Z,               "BTN_Z");
    add(BTN_TL,              "BTN_TL");
    add(BTN_TR,              "BTN_TR");
    add(BTN_TL2,             "BTN_TL2");
    add(BTN_TR2,             "BTN_TR2");
    add(BTN_SELECT,          "BTN_SELECT");
    add(BTN_START,           "BTN_START");
    add(BTN_MODE,            "BTN_MODE");
    add(BTN_THUMBL,          "BTN_THUMBL");
    add(BTN_THUMBR,          "BTN_THUMBR");
    add(BTN_DIGI,            "BTN_DIGI");
    add(BTN_TOOL_PEN,        "BTN_TOOL_PEN");
    add(BTN_TOOL_RUBBER,     "BTN_TOOL_RUBBER");
    add(BTN_TOOL_BRUSH,      "BTN_TOOL_BRUSH");
    add(BTN_TOOL_PENCIL,     "BTN_TOOL_PENCIL");
    add(BTN_TOOL_AIRBRUSH,   "BTN_TOOL_AIRBRUSH");
    add(BTN_TOOL_FINGER,     "BTN_TOOL_FINGER");
    add(BTN_TOOL_MOUSE,      "BTN_TOOL_MOUSE");
    add(BTN_TOOL_LENS,       "BTN_TOOL_LENS");
    add(BTN_TOUCH,           "BTN_TOUCH");
    add(BTN_STYLUS,          "BTN_STYLUS");
    add(BTN_STYLUS2,         "BTN_STYLUS2");
    add(BTN_TOOL_DOUBLETAP,  "BTN_TOOL_DOUBLETAP");
    add(BTN_TOOL_TRIPLETAP,  "BTN_TOOL_TRIPLETAP");
    add(BTN_WHEEL,           "BTN_WHEEL");
    add(BTN_GEAR_DOWN,       "BTN_GEAR_DOWN");
    add(BTN_GEAR_UP,         "BTN_GEAR_UP");
    add(KEY_OK,              "KEY_OK");
    add(KEY_SELECT,          "KEY_SELECT");
    add(KEY_GOTO,            "KEY_GOTO");
    add(KEY_CLEAR,           "KEY_CLEAR");
    add(KEY_POWER2,          "KEY_POWER2");
    add(KEY_OPTION,          "KEY_OPTION");
    add(KEY_INFO,            "KEY_INFO");
    add(KEY_TIME,            "KEY_TIME");
    add(KEY_VENDOR,          "KEY_VENDOR");
    add(KEY_ARCHIVE,         "KEY_ARCHIVE");
    add(KEY_PROGRAM,         "KEY_PROGRAM");
    add(KEY_CHANNEL,         "KEY_CHANNEL");
    add(KEY_FAVORITES,       "KEY_FAVORITES");
    add(KEY_EPG,             "KEY_EPG");
    add(KEY_PVR,             "KEY_PVR");
    add(KEY_MHP,             "KEY_MHP");
    add(KEY_LANGUAGE,        "KEY_LANGUAGE");
    add(KEY_TITLE,           "KEY_TITLE");
    add(KEY_SUBTITLE,        "KEY_SUBTITLE");
    add(KEY_ANGLE,           "KEY_ANGLE");
    add(KEY_ZOOM,            "KEY_ZOOM");
    add(KEY_MODE,            "KEY_MODE");
    add(KEY_KEYBOARD,        "KEY_KEYBOARD");
    add(KEY_SCREEN,          "KEY_SCREEN");
    add(KEY_PC,              "KEY_PC");
    add(KEY_TV,              "KEY_TV");
    add(KEY_TV2,             "KEY_TV2");
    add(KEY_VCR,             "KEY_VCR");
    add(KEY_VCR2,            "KEY_VCR2");
    add(KEY_SAT,             "KEY_SAT");
    add(KEY_SAT2,            "KEY_SAT2");
    add(KEY_CD,              "KEY_CD");
    add(KEY_TAPE,            "KEY_TAPE");
    add(KEY_RADIO,           "KEY_RADIO");
    add(KEY_TUNER,           "KEY_TUNER");
    add(KEY_PLAYER,          "KEY_PLAYER");
    add(KEY_TEXT,            "KEY_TEXT");
    add(KEY_DVD,             "KEY_DVD");
    add(KEY_AUX,             "KEY_AUX");
    add(KEY_MP3,             "KEY_MP3");
    add(KEY_AUDIO,           "KEY_AUDIO");
    add(KEY_VIDEO,           "KEY_VIDEO");
    add(KEY_DIRECTORY,       "KEY_DIRECTORY");
    add(KEY_LIST,            "KEY_LIST");
    add(KEY_MEMO,            "KEY_MEMO");
    add(KEY_CALENDAR,        "KEY_CALENDAR");
    add(KEY_RED,             "KEY_RED");
    add(KEY_GREEN,           "KEY_GREEN");
    add(KEY_YELLOW,          "KEY_YELLOW");
    add(KEY_BLUE,            "KEY_BLUE");
    add(KEY_CHANNELUP,       "KEY_CHANNELUP");
    add(KEY_CHANNELDOWN,     "KEY_CHANNELDOWN");
    add(KEY_FIRST,           "KEY_FIRST");
    add(KEY_LAST,            "KEY_LAST");
    add(KEY_AB,              "KEY_AB");
    add(KEY_NEXT,            "KEY_NEXT");
    add(KEY_RESTART,         "KEY_RESTART");
    add(KEY_SLOW,            "KEY_SLOW");
    add(KEY_SHUFFLE,         "KEY_SHUFFLE");
    add(KEY_BREAK,           "KEY_BREAK");
    add(KEY_PREVIOUS,        "KEY_PREVIOUS");
    add(KEY_DIGITS,          "KEY_DIGITS");
    add(KEY_TEEN,            "KEY_TEEN");
    add(KEY_TWEN,            "KEY_TWEN");
    add(KEY_VIDEOPHONE,      "KEY_VIDEOPHONE");
    add(KEY_GAMES,           "KEY_GAMES");
    add(KEY_ZOOMIN,          "KEY_ZOOMIN");
    add(KEY_ZOOMOUT,         "KEY_ZOOMOUT");
    add(KEY_ZOOMRESET,       "KEY_ZOOMRESET");
    add(KEY_WORDPROCESSOR,   "KEY_WORDPROCESSOR");
    add(KEY_EDITOR,          "KEY_EDITOR");
    add(KEY_SPREADSHEET,     "KEY_SPREADSHEET");
    add(KEY_GRAPHICSEDITOR,  "KEY_GRAPHICSEDITOR");
    add(KEY_PRESENTATION,    "KEY_PRESENTATION");
    add(KEY_DATABASE,        "KEY_DATABASE");
    add(KEY_NEWS,            "KEY_NEWS");
    add(KEY_VOICEMAIL,       "KEY_VOICEMAIL");
    add(KEY_ADDRESSBOOK,     "KEY_ADDRESSBOOK");
    add(KEY_MESSENGER,       "KEY_MESSENGER");
    add(KEY_DISPLAYTOGGLE,   "KEY_DISPLAYTOGGLE");
    add(KEY_SPELLCHECK,      "KEY_SPELLCHECK");
    add(KEY_LOGOFF,          "KEY_LOGOFF");
    add(KEY_DOLLAR,          "KEY_DOLLAR");
    add(KEY_EURO,            "KEY_EURO");
    add(KEY_FRAMEBACK,       "KEY_FRAMEBACK");
    add(KEY_FRAMEFORWARD,    "KEY_FRAMEFORWARD");
    add(KEY_CONTEXT_MENU,    "KEY_CONTEXT_MENU");
#ifdef KEY_MEDIA_REPEAT
    add(KEY_MEDIA_REPEAT,    "KEY_MEDIA_REPEAT");
#endif
    add(KEY_DEL_EOL,         "KEY_DEL_EOL");
    add(KEY_DEL_EOS,         "KEY_DEL_EOS");
    add(KEY_INS_LINE,        "KEY_INS_LINE");
    add(KEY_DEL_LINE,        "KEY_DEL_LINE");
    add(KEY_FN,              "KEY_FN");
    add(KEY_FN_ESC,          "KEY_FN_ESC");
    add(KEY_FN_F1,           "KEY_FN_F1");
    add(KEY_FN_F2,           "KEY_FN_F2");
    add(KEY_FN_F3,           "KEY_FN_F3");
    add(KEY_FN_F4,           "KEY_FN_F4");
    add(KEY_FN_F5,           "KEY_FN_F5");
    add(KEY_FN_F6,           "KEY_FN_F6");
    add(KEY_FN_F7,           "KEY_FN_F7");
    add(KEY_FN_F8,           "KEY_FN_F8");
    add(KEY_FN_F9,           "KEY_FN_F9");
    add(KEY_FN_F10,          "KEY_FN_F10");
    add(KEY_FN_F11,          "KEY_FN_F11");
    add(KEY_FN_F12,          "KEY_FN_F12");
    add(KEY_FN_1,            "KEY_FN_1");
    add(KEY_FN_2,            "KEY_FN_2");
    add(KEY_FN_D,            "KEY_FN_D");
    add(KEY_FN_E,            "KEY_FN_E");
    add(KEY_FN_F,            "KEY_FN_F");
    add(KEY_FN_S,            "KEY_FN_S");
    add(KEY_FN_B,            "KEY_FN_B");
    add(KEY_BRL_DOT1,        "KEY_BRL_DOT1");
    add(KEY_BRL_DOT2,        "KEY_BRL_DOT2");
    add(KEY_BRL_DOT3,        "KEY_BRL_DOT3");
    add(KEY_BRL_DOT4,        "KEY_BRL_DOT4");
    add(KEY_BRL_DOT5,        "KEY_BRL_DOT5");
    add(KEY_BRL_DOT6,        "KEY_BRL_DOT6");
    add(KEY_BRL_DOT7,        "KEY_BRL_DOT7");
    add(KEY_BRL_DOT8,        "KEY_BRL_DOT8");
    add(KEY_BRL_DOT9,        "KEY_BRL_DOT9");
    add(KEY_BRL_DOT10,       "KEY_BRL_DOT10");
    add(KEY_MIN_INTERESTING, "KEY_MIN_INTERESTING");
  }
} evdev_key_names;

class Keysym2Keycode
{
public:
  // Map KeySym to kernel keycode
  std::map<KeySym, int> mapping;

  Keysym2Keycode() :
    mapping()
  {
    //std::cout << "Initing Keysym2Keycode" << std::endl;

    Display* dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
      throw std::runtime_error("Keysym2Keycode: Couldn't open X11 display");
    }
    else
    {
      process_keymap(dpy);
      XCloseDisplay(dpy);
    }
  }

  void process_keymap(Display* dpy)
  {
    int min_keycode, max_keycode;
    XDisplayKeycodes(dpy, &min_keycode, &max_keycode);

    int num_keycodes = max_keycode - min_keycode + 1;
    int keysyms_per_keycode;
    KeySym* keymap = XGetKeyboardMapping(dpy, static_cast<KeyCode>(min_keycode),
                                         num_keycodes,
                                         &keysyms_per_keycode);

    for(int i = 0; i < num_keycodes; ++i)
    {
      if (keymap[i*keysyms_per_keycode] != NoSymbol)
      {
        KeySym keysym = keymap[i*keysyms_per_keycode];
        // FIXME: Duplicate entries confuse the conversion
        // std::map<KeySym, int>::iterator it = mapping.find(keysym);
        // if (it != mapping.end())
        //   std::cout << "Duplicate keycode: " << i << std::endl;
        mapping[keysym] = i;
      }
    }

    XFree(keymap);
  }
};

int xkeysym2keycode(const std::string& name)
{
  static Keysym2Keycode sym2code;

  KeySym keysym = XStringToKeysym(name.substr(3).c_str());

  if (keysym == NoSymbol)
  {
    throw std::runtime_error("xkeysym2keycode: Couldn't convert name '" + name + "' to xkeysym");
  }

  std::map<KeySym, int>::iterator i = sym2code.mapping.find(keysym);
  if (i == sym2code.mapping.end())
  {
    throw std::runtime_error("xkeysym2keycode: Couldn't convert xkeysym '" + name + "' to evdev keycode");
  }
  else
  {
    if (0)
      std::cout << name << " -> " << keysym << " -> " << XKeysymToString(keysym) 
                << " -> " << key2str(i->second) << "(" << i->second << ")" << std::endl;
    return i->second;
  }
}

void str2event(const std::string& name, int& type, int& code)
{
  if (name == "void" || name == "none")
  {
    type = -1;
    code = -1;
  }
  else if (name.compare(0, 3, "REL") == 0)
  {
    type = EV_REL;
    code = evdev_rel_names[name];
  }
  else if (name.compare(0, 3, "ABS") == 0)
  {
    type = EV_ABS;
    code = evdev_abs_names[name];
  }
  else if (name.compare(0, 2, "XK") == 0)
  {
    type = EV_KEY;
    code = xkeysym2keycode(name);
  }
  else if (name.compare(0, 2, "JS") == 0)
  {
    type = EV_KEY;
    code = BTN_JOYSTICK + boost::lexical_cast<int>(name.substr(3));
  }
  else if (name.compare(0, 3, "KEY") == 0 ||
           name.compare(0, 3, "BTN") == 0)
  {
    type = EV_KEY;
    code = evdev_key_names[name];
  }
  else
  {
    throw std::runtime_error("str2event(): unknown event type prefix: " + name);
  }
}

int get_event_type(const std::string& name)
{
  if (name == "void" || name == "none")
  {
    return -1;
  }
  else if (name.compare(0, 3, "REL") == 0)
  {
    return EV_REL;
  }
  else if (name.compare(0, 3, "ABS") == 0)
  {
    return EV_ABS;
  }
  else if (name.compare(0, 3, "KEY") == 0 ||
           name.compare(0, 3, "BTN") == 0 ||
           name.compare(0, 2, "JS")  == 0 ||
           name.compare(0, 2, "XK")  == 0)
  {
    return EV_KEY;
  }
  else
  {
    throw std::runtime_error("str2event(): unknown event type prefix: " + name);
  }
}

int str2key(const std::string& name)
{
  return evdev_abs_names[name];
}

int str2abs(const std::string& name)
{
  if (name.compare(0, 2, "XK") == 0)
  {
    return xkeysym2keycode(name);
  }
  else if (name.compare(0, 2, "JS") == 0)
  {
    return BTN_JOYSTICK + boost::lexical_cast<int>(name.substr(3));
  }
  else if (name.compare(0, 3, "KEY") == 0 ||
           name.compare(0, 3, "BTN") == 0)
  {
    return evdev_key_names[name];
  }
  else
  {
    throw std::runtime_error("str2abs: couldn't convert string: " + name);
  }
}

int str2rel(const std::string& name)
{
  return evdev_rel_names[name];
}

std::string key2str(int i)
{
  return evdev_key_names[i];
}

std::string abs2str(int i)
{
  return evdev_abs_names[i];
}

std::string rel2str(int i)
{
  return evdev_rel_names[i];
}

UIEvent str2key_event(const std::string& str)
{
  int device_id;
  std::string rest;
  split_event_name(str, &rest, &device_id);
  return UIEvent::create(device_id, EV_KEY, str2key(rest));
}

UIEvent str2rel_event(const std::string& str)
{
  int device_id;
  std::string rest;
  split_event_name(str, &rest, &device_id);
  return UIEvent::create(device_id, EV_REL, str2rel(rest));
}

UIEvent str2abs_event(const std::string& str)
{
  int device_id;
  std::string rest;
  split_event_name(str, &rest, &device_id);
  return UIEvent::create(device_id, EV_ABS, str2abs(rest));
}

/* EOF */

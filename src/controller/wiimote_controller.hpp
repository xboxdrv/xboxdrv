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

#ifndef HEADER_XBOXDRV_CONTROLLER_WIIMOTE_CONTROLLER_HPP
#define HEADER_XBOXDRV_CONTROLLER_WIIMOTE_CONTROLLER_HPP

#ifdef HAVE_CWIID

#include <pthread.h>
#include <cwiid.h>

#include "controller.hpp"
#include "controller_message.hpp"

struct AccCalibration
{
  uint8_t x;
  uint8_t y;
  uint8_t z;
};

class WiimoteController : public Controller
{
private:
  static WiimoteController* s_wiimote;

public:
  static void err_callback(cwiid_wiimote_t*, const char *s, va_list ap);
  static void mesg_callback(cwiid_wiimote_t*, int mesg_count, union cwiid_mesg mesg[], timespec*);

private:
  pthread_mutex_t  m_mutex;
  cwiid_wiimote_t* m_wiimote;

  ControllerMessage m_ctrl_msg;

  AccCalibration m_wiimote_zero;
  AccCalibration m_wiimote_one;

  AccCalibration m_nunchuk_zero;
  AccCalibration m_nunchuk_one;

  AccCalibration m_nunchuk_x;
  AccCalibration m_nunchuk_y;

public:
  WiimoteController();
  ~WiimoteController();

  void set_rumble_real(uint8_t left, uint8_t right);
  void set_led_real(uint8_t status);

private:
  void connect();
  void disconnect();

  void read_nunchuk_calibration();
  void read_wiimote_calibration();

  void on_status (const cwiid_status_mesg& msg);
  void on_error  (const cwiid_error_mesg& msg);
  void on_button (const cwiid_btn_mesg& msg);
  void on_acc    (const cwiid_acc_mesg& msg);
  void on_ir     (const cwiid_ir_mesg& msg);
  void on_nunchuk(const cwiid_nunchuk_mesg& msg);
  void on_classic(const cwiid_classic_mesg& msg);

private:
  WiimoteController(const WiimoteController&);
  WiimoteController& operator=(const WiimoteController&);
};

#endif /* HAVE_CWIID */

#endif

/* EOF */

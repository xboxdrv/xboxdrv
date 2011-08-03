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

#include "controller/wiimote_controller.hpp"

#include <assert.h>
#include <iostream>

#include "unpack.hpp"
#include "log.hpp"

WiimoteController* WiimoteController::s_wiimote = 0;

WiimoteController::WiimoteController() :
  m_mutex(),
  m_wiimote(),
  m_ctrl_msg()
{
  assert(!s_wiimote);
  s_wiimote = this;

  connect();
}

WiimoteController::~WiimoteController()
{
  disconnect();

  s_wiimote = 0;
}

void
WiimoteController::connect()
{
  assert(m_wiimote == 0);

  /* Connect to any wiimote */
  bdaddr_t bdaddr = {{0, 0, 0, 0, 0, 0}}; // BDADDR_ANY

  /* Connect to address in string WIIMOTE_BDADDR */
  /* str2ba(WIIMOTE_BDADDR, &bdaddr); */

  /* Connect to the wiimote */
  printf("Put Wiimote in discoverable mode now (press 1+2)...\n");

  if (!(m_wiimote = cwiid_connect(&bdaddr, CWIID_FLAG_MESG_IFC))) 
  {
    fprintf(stderr, "Unable to connect to wiimote\n");
  }
  else 
  {
    std::cout << "Wiimote connected: " << m_wiimote << std::endl;
    if (cwiid_set_mesg_callback(m_wiimote, &WiimoteController::mesg_callback)) {
      std::cerr << "Unable to set message callback" << std::endl;
    }

    if (cwiid_command(m_wiimote, CWIID_CMD_RPT_MODE, 
                      CWIID_RPT_STATUS  |
                      CWIID_RPT_NUNCHUK |
                      //CWIID_RPT_ACC     |
                      CWIID_RPT_BTN))
    {
      std::cerr << "Wiimote: Error setting report mode" << std::endl;
    }

#if 0
    if (false)
    { // read calibration data
      uint8_t buf[7];

      if (cwiid_read(m_wiimote, CWIID_RW_EEPROM, 0x16, 7, buf))
      {
        std::cout << "Wiimote: Unable to retrieve accelerometer calibration" << std::endl;
      }
      else
      {
        wiimote_zero.x = buf[0];
        wiimote_zero.y = buf[1];
        wiimote_zero.z = buf[2];

        wiimote_one.x  = buf[4];
        wiimote_one.y  = buf[5];
        wiimote_one.z  = buf[6];
      }

      if (false)
      {
        if (cwiid_read(m_wiimote, CWIID_RW_REG | CWIID_RW_DECODE, 0xA40020, 7, buf))
        {
          std::cout << "Wiimote: Unable to retrieve wiimote accelerometer calibration" << std::endl;
        }
        else
        {
          nunchuk_zero.x = buf[0];
          nunchuk_zero.y = buf[1];
          nunchuk_zero.z = buf[2];
            
          nunchuk_one.x  = buf[4];
          nunchuk_one.y  = buf[5];
          nunchuk_one.z  = buf[6];
        }
      }

      std::cout << "Wiimote Calibration: "
                << static_cast<int>(wiimote_zero.x) << ", "
                << static_cast<int>(wiimote_zero.x) << ", "
                << static_cast<int>(wiimote_zero.x) << " - "
                << static_cast<int>(wiimote_one.x) << ", "
                << static_cast<int>(wiimote_one.x) << ", "
                << static_cast<int>(wiimote_one.x) << std::endl;

      std::cout << "Nunchuk Calibration: "
                << static_cast<int>(nunchuk_zero.x) << ", "
                << static_cast<int>(nunchuk_zero.x) << ", "
                << static_cast<int>(nunchuk_zero.x) << " - "
                << static_cast<int>(nunchuk_one.x) << ", "
                << static_cast<int>(nunchuk_one.x) << ", "
                << static_cast<int>(nunchuk_one.x) << std::endl;
    }
#endif         
  }
}

void
WiimoteController::disconnect()
{
  if (m_wiimote)
  {
    cwiid_disconnect(m_wiimote);
    m_wiimote = 0;
  }
}

void
WiimoteController::set_rumble_real(uint8_t left, uint8_t right)
{
}

void
WiimoteController::set_led_real(uint8_t status)
{
}

void
WiimoteController::on_status(const cwiid_status_mesg& msg)
{
  log_tmp_trace();
}

void
WiimoteController::on_error(const cwiid_error_mesg& msg)
{
  log_tmp_trace();
}

void
WiimoteController::on_button(const cwiid_btn_mesg& msg)
{
  log_tmp_trace();

  m_ctrl_msg.set_button(XBOX_BTN_BACK, msg.buttons & CWIID_BTN_MINUS);
  m_ctrl_msg.set_button(XBOX_BTN_GUIDE, msg.buttons & CWIID_BTN_HOME);
  m_ctrl_msg.set_button(XBOX_BTN_START, msg.buttons & CWIID_BTN_PLUS);

  m_ctrl_msg.set_button(XBOX_BTN_Y, msg.buttons & CWIID_BTN_2);
  m_ctrl_msg.set_button(XBOX_BTN_X, msg.buttons & CWIID_BTN_1);
  m_ctrl_msg.set_button(XBOX_BTN_B, msg.buttons & CWIID_BTN_B);
  m_ctrl_msg.set_button(XBOX_BTN_A, msg.buttons & CWIID_BTN_A);

  m_ctrl_msg.set_button(XBOX_DPAD_LEFT,  msg.buttons & CWIID_BTN_LEFT);
  m_ctrl_msg.set_button(XBOX_DPAD_RIGHT, msg.buttons & CWIID_BTN_RIGHT);
  m_ctrl_msg.set_button(XBOX_DPAD_DOWN,  msg.buttons & CWIID_BTN_DOWN);
  m_ctrl_msg.set_button(XBOX_DPAD_UP,    msg.buttons & CWIID_BTN_UP);

  submit_msg(m_ctrl_msg);
}

void
WiimoteController::on_acc(const cwiid_acc_mesg& msg)
{
  log_tmp_trace();
}

void
WiimoteController::on_ir(const cwiid_ir_mesg& msg)
{
  log_tmp_trace();
}

void
WiimoteController::on_nunchuck(const cwiid_nunchuk_mesg& msg)
{
  log_tmp_trace();

  m_ctrl_msg.set_axis(XBOX_AXIS_X1, unpack::u8_to_s16(msg.stick[0]));
  m_ctrl_msg.set_axis(XBOX_AXIS_Y1, unpack::u8_to_s16(msg.stick[1]));

  // unhandled: msg.acc[3];

  m_ctrl_msg.set_button(XBOX_BTN_LT, msg.buttons & CWIID_NUNCHUK_BTN_Z);
  m_ctrl_msg.set_button(XBOX_BTN_LB, msg.buttons & CWIID_NUNCHUK_BTN_C);

  submit_msg(m_ctrl_msg);
}

void
WiimoteController::on_classic (const cwiid_classic_mesg& msg)
{
  log_tmp_trace();
}

void
WiimoteController::err_callback(cwiid_wiimote_t*, const char *s, va_list ap)
{
  log_tmp("err_callback");
}

void
WiimoteController::mesg_callback(cwiid_wiimote_t*, int mesg_count, union cwiid_mesg msg[], timespec*)
{
  // FIXME: the mesg_callback() comes from another thread, so
  // g_idle_add() should be used to perform syncronisation with the
  // other thread
  for (int i=0; i < mesg_count; i++)
  {
    switch (msg[i].type) 
    {
      case CWIID_MESG_STATUS:
        s_wiimote->on_status(msg[i].status_mesg);
        break;

      case CWIID_MESG_BTN:
        s_wiimote->on_button(msg[i].btn_mesg);
        break;

      case CWIID_MESG_ACC:
        s_wiimote->on_acc(msg[i].acc_mesg);
        break;

      case CWIID_MESG_IR:
        s_wiimote->on_ir(msg[i].ir_mesg);
        break;

      case CWIID_MESG_NUNCHUK:
        s_wiimote->on_nunchuck(msg[i].nunchuk_mesg);
        break;

      case CWIID_MESG_CLASSIC:
        s_wiimote->on_classic(msg[i].classic_mesg);
        break;

      case CWIID_MESG_ERROR:
        s_wiimote->on_error(msg[i].error_mesg);
        break;

      default:
        log_error("unknown report");
        break;
    }
  }
}

/* EOF */

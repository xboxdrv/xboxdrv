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

#ifdef HAVE_CWIID

#include "controller/wiimote_controller.hpp"

#include <assert.h>
#include <iostream>

#include "bluetooth.hpp"

#include "unpack.hpp"
#include "log.hpp"

WiimoteController* WiimoteController::s_wiimote = 0;

WiimoteController::WiimoteController() :
  m_mutex(),
  m_wiimote(),
  m_ctrl_msg(),
  m_wiimote_zero(),
  m_wiimote_one(),
  m_nunchuk_zero(),
  m_nunchuk_one(),
  m_nunchuk_x(),
  m_nunchuk_y()
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
  bdaddr_t bdaddr = Bluetooth::addr_any;

  /* Connect to address in string WIIMOTE_BDADDR */
  /* str2ba(WIIMOTE_BDADDR, &bdaddr); */

  /* Connect to the wiimote */
  printf("Put Wiimote in discoverable mode now (press 1+2)...\n");

  // wait forever till a Wiimote is found
  m_wiimote = cwiid_open_timeout(&bdaddr, CWIID_FLAG_MESG_IFC, -1);

  {
    std::cout << "Wiimote connected: " << m_wiimote << std::endl;
    if (cwiid_set_mesg_callback(m_wiimote, &WiimoteController::mesg_callback)) {
      std::cerr << "Unable to set message callback" << std::endl;
    }

    if (cwiid_command(m_wiimote, CWIID_CMD_RPT_MODE, 
                      CWIID_RPT_STATUS  |
                      CWIID_RPT_NUNCHUK |
                      CWIID_RPT_ACC     |
                      CWIID_RPT_IR      |
                      CWIID_RPT_BTN))
    {
      std::cerr << "Wiimote: Error setting report mode" << std::endl;
    }

    read_wiimote_calibration();
    read_nunchuk_calibration();
  }
}

void
WiimoteController::disconnect()
{
  if (m_wiimote)
  {
    cwiid_close(m_wiimote);
    m_wiimote = 0;
  }
}

std::ostream& operator<<(std::ostream& os, const AccCalibration& cal)
{
  return os << "(" 
            << static_cast<int>(cal.x) << " " 
            << static_cast<int>(cal.y) << " " 
            << static_cast<int>(cal.z) << ")";
}

void
WiimoteController::read_wiimote_calibration()
{
  uint8_t buf[7];

  if (cwiid_read(m_wiimote, CWIID_RW_EEPROM, 0x16, 7, buf))
  {
    std::cout << "Wiimote: Unable to retrieve accelerometer calibration" << std::endl;
  }
  else
  {
    m_wiimote_zero.x = buf[0];
    m_wiimote_zero.y = buf[1];
    m_wiimote_zero.z = buf[2];

    m_wiimote_one.x  = buf[4];
    m_wiimote_one.y  = buf[5];
    m_wiimote_one.z  = buf[6];
  }

  std::cout << "Wiimote Calibration: "
            << static_cast<int>(m_wiimote_zero.x) << ", "
            << static_cast<int>(m_wiimote_zero.x) << ", "
            << static_cast<int>(m_wiimote_zero.x) << " - "
            << static_cast<int>(m_wiimote_one.x) << ", "
            << static_cast<int>(m_wiimote_one.x) << ", "
            << static_cast<int>(m_wiimote_one.x) << std::endl;
}

void
WiimoteController::read_nunchuk_calibration()
{
  uint8_t buf[14];

  if (cwiid_read(m_wiimote, CWIID_RW_REG | CWIID_RW_DECODE, 0xA40020, sizeof(buf), buf))
  {
    log_error("unable to retrieve nunchuk calibration data");
  }
  else
  {
    m_nunchuk_zero.x = buf[0];
    m_nunchuk_zero.y = buf[1];
    m_nunchuk_zero.z = buf[2];
            
    m_nunchuk_one.x  = buf[4];
    m_nunchuk_one.y  = buf[5];
    m_nunchuk_one.z  = buf[6];

    m_nunchuk_x.x = buf[8]; // max
    m_nunchuk_x.y = buf[9]; // min
    m_nunchuk_x.z = buf[10]; // center

    m_nunchuk_y.x = buf[11]; // max
    m_nunchuk_y.y = buf[12]; // min
    m_nunchuk_y.z = buf[13]; // center

    log_tmp("X: " << m_nunchuk_x);
    log_tmp("Y: " << m_nunchuk_y);
  }
}

void
WiimoteController::set_rumble_real(uint8_t left, uint8_t right)
{
  if (left > 127 || right > 127)
  {
    cwiid_set_rumble(m_wiimote, 1);
  }
  else
  {
    cwiid_set_rumble(m_wiimote, 0);
  }
}

void
WiimoteController::set_led_real(uint8_t status)
{
  switch(status)
  {
    case 2:
    case 6:
      cwiid_set_led(m_wiimote, CWIID_LED1_ON);
      break;

    case 3:
    case 7:
      cwiid_set_led(m_wiimote, CWIID_LED2_ON);
      break;

    case 4:
    case 8:
      cwiid_set_led(m_wiimote, CWIID_LED3_ON);
      break;

    case 5:
    case 9:
      cwiid_set_led(m_wiimote, CWIID_LED4_ON);
      break;

    default:
      cwiid_set_led(m_wiimote, 0);
      break;
  }
}

void
WiimoteController::on_status(const cwiid_status_mesg& msg)
{
  log_tmp("Battery: " << static_cast<int>(msg.battery) << " Status: " << msg.ext_type);

  switch(msg.ext_type)
  {
    case CWIID_EXT_NONE:
      break;

    case CWIID_EXT_NUNCHUK:
      read_nunchuk_calibration();
      break;

    case CWIID_EXT_CLASSIC:
      break;

    case CWIID_EXT_BALANCE:
      break;

    case CWIID_EXT_MOTIONPLUS:
      break;

    case CWIID_EXT_UNKNOWN:
      break;        
  }
}

void
WiimoteController::on_error(const cwiid_error_mesg& msg)
{
  log_tmp_trace();
}

void
WiimoteController::on_button(const cwiid_btn_mesg& msg)
{
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
  m_ctrl_msg.set_axis(WIIMOTE_ACC_X, msg.acc[0]);
  m_ctrl_msg.set_axis(WIIMOTE_ACC_Y, msg.acc[1]);
  m_ctrl_msg.set_axis(WIIMOTE_ACC_Z, msg.acc[2]);

  submit_msg(m_ctrl_msg);
}

void
WiimoteController::on_ir(const cwiid_ir_mesg& msg)
{
  // size: 1-7
  // valid 0, 1
  // x 0,1024
  // y 0,768
  if (false)
    log_tmp("IR: " << 
            msg.src[0].pos[0] << " " << msg.src[0].pos[1] << " " << static_cast<int>(msg.src[0].size) << " " << static_cast<int>(msg.src[0].valid) << " - " <<
            msg.src[1].pos[0] << " " << msg.src[1].pos[1] << " " << static_cast<int>(msg.src[1].size) << " " << static_cast<int>(msg.src[1].valid) << " - " <<
            msg.src[2].pos[0] << " " << msg.src[2].pos[1] << " " << static_cast<int>(msg.src[2].size) << " " << static_cast<int>(msg.src[2].valid) << " - " <<
            msg.src[3].pos[0] << " " << msg.src[3].pos[1] << " " << static_cast<int>(msg.src[3].size) << " " << static_cast<int>(msg.src[3].valid));

  // size and valid are not handled

  // FIXME: encoding 'valid' in size might not be such a good idea, as
  // it overwrites the last valid value

  m_ctrl_msg.set_axis(WIIMOTE_IR_X, msg.src[0].pos[0]);
  m_ctrl_msg.set_axis(WIIMOTE_IR_Y, msg.src[0].pos[1]);
  if (msg.src[0].valid)
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE, msg.src[0].size);
  else
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE, -1);

  m_ctrl_msg.set_axis(WIIMOTE_IR_X2, msg.src[1].pos[0]);
  m_ctrl_msg.set_axis(WIIMOTE_IR_Y2, msg.src[1].pos[1]);
  if (msg.src[1].valid)
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE2, msg.src[1].size);
  else
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE2, -1);

  m_ctrl_msg.set_axis(WIIMOTE_IR_X3, msg.src[2].pos[0]);
  m_ctrl_msg.set_axis(WIIMOTE_IR_Y3, msg.src[2].pos[1]);
  if (msg.src[2].valid)
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE3, msg.src[2].size);
  else
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE3, -1);

  m_ctrl_msg.set_axis(WIIMOTE_IR_X4, msg.src[3].pos[0]);
  m_ctrl_msg.set_axis(WIIMOTE_IR_Y4, msg.src[3].pos[1]);
  if (msg.src[3].valid)
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE4, msg.src[3].size);
  else
    m_ctrl_msg.set_axis(WIIMOTE_IR_SIZE4, -1);

  submit_msg(m_ctrl_msg);
}

// FIXME: use proper CalibrationAxisFilter instead of this hack, also CalibrationAxisFilter doesn't handle min/max properly
int8_t calibrate(int value, const AccCalibration& cal)
{
  int m_center = cal.z;
  int m_max  = cal.x;
  int m_min  = cal.y;
  
  int min = -128;
  int max =  127;

  if (value < m_center)
    value = -min * (value - m_center) / (m_center - m_min);
  else if (value > m_center)
    value = max * (value - m_center) / (m_max - m_center);
  else
    value = 0;

  return static_cast<int8_t>(Math::clamp(min, value, max));
}

void
WiimoteController::on_nunchuk(const cwiid_nunchuk_mesg& msg)
{
  m_ctrl_msg.set_axis(XBOX_AXIS_X1, unpack::s8_to_s16(calibrate(msg.stick[0], m_nunchuk_x)));
  m_ctrl_msg.set_axis(XBOX_AXIS_Y1, unpack::s16_invert(unpack::s8_to_s16(calibrate(msg.stick[1], m_nunchuk_y))));

  m_ctrl_msg.set_axis(NUNCHUK_ACC_X, msg.acc[0]);
  m_ctrl_msg.set_axis(NUNCHUK_ACC_Y, msg.acc[1]);
  m_ctrl_msg.set_axis(NUNCHUK_ACC_Z, msg.acc[2]);

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
        s_wiimote->on_nunchuk(msg[i].nunchuk_mesg);
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

#endif /* HAVE_CWIID */

/* EOF */

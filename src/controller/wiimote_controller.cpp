/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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
#include "controller_message_descriptor.hpp"
#include <logmich/log.hpp>
#include "unpack.hpp"

namespace xboxdrv {

WiimoteController* WiimoteController::s_wiimote = 0;

WiiNames::WiiNames(ControllerMessageDescriptor& desc) :
  plus(-1),
  home(-1),
  minus(-1),

  a(-1),
  b(-1),
  btn1(-1),
  btn2(-1),

  dpad_up(-1),
  dpad_down(-1),
  dpad_left(-1),
  dpad_right(-1),

  acc_x(-1),
  acc_y(-1),
  acc_z(-1),

  ir1_x(-1),
  ir1_y(-1),
  ir1_size(-1),

  ir2_x(-1),
  ir2_y(-1),
  ir2_size(-1),

  ir3_x(-1),
  ir3_y(-1),
  ir3_size(-1),

  ir4_x(-1),
  ir4_y(-1),
  ir4_size(-1),

  nunchuk_x(-1),
  nunchuk_y(-1),

  nunchuk_c(-1),
  nunchuk_z(-1),

  nunchuk_acc_x(-1),
  nunchuk_acc_y(-1),
  nunchuk_acc_z(-1)
{
  minus = desc.key().put(KeyName("wiimote.minus"));
  home  = desc.key().put(KeyName("wiimote.home"));
  plus  = desc.key().put(KeyName("wiimote.plus"));

  a = desc.key().put(KeyName("wiimote.a"));
  b = desc.key().put(KeyName("wiimote.b"));
  btn1 = desc.key().put(KeyName("wiimote.1"));
  btn2 = desc.key().put(KeyName("wiimote.2"));

  dpad_up    = desc.key().put(KeyName("wiimote.dpad_up"));
  dpad_down  = desc.key().put(KeyName("wiimote.dpad_down"));
  dpad_left  = desc.key().put(KeyName("wiimote.dpad_left"));
  dpad_right = desc.key().put(KeyName("wiimote.dpad_right"));

  acc_x = desc.abs().put(AbsName("wiimote.acc_x"));
  acc_y = desc.abs().put(AbsName("wiimote.acc_y"));
  acc_z = desc.abs().put(AbsName("wiimote.acc_z"));

  ir1_x = desc.abs().put(AbsName("wiimote.ir1_x"));
  ir1_y = desc.abs().put(AbsName("wiimote.ir1_y"));
  ir1_size = desc.abs().put(AbsName("wiimote.ir1_size"));

  ir2_x = desc.abs().put(AbsName("wiimote.ir2_x"));
  ir2_y = desc.abs().put(AbsName("wiimote.ir2_y"));
  ir2_size = desc.abs().put(AbsName("wiimote.ir2_size"));

  ir3_x = desc.abs().put(AbsName("wiimote.ir3_x"));
  ir3_y = desc.abs().put(AbsName("wiimote.ir3_y"));
  ir3_size = desc.abs().put(AbsName("wiimote.ir3_size"));

  ir4_x = desc.abs().put(AbsName("wiimote.ir4_x"));
  ir4_y = desc.abs().put(AbsName("wiimote.ir4_y"));
  ir4_size = desc.abs().put(AbsName("wiimote.ir4_size"));

  nunchuk_x = desc.abs().put(AbsName("nunchuk.x1"));
  nunchuk_y = desc.abs().put(AbsName("nunchuk.y1"));

  nunchuk_c = desc.key().put(KeyName("nunchuk.c"));
  nunchuk_z = desc.key().put(KeyName("nunchuk.z"));

  nunchuk_acc_x = desc.abs().put(AbsName("nunchuk.acc_x"));
  nunchuk_acc_y = desc.abs().put(AbsName("nunchuk.acc_x"));
  nunchuk_acc_z = desc.abs().put(AbsName("nunchuk.acc_y"));
}

WiimoteController::WiimoteController() :
  m_mutex(),
  m_wiimote(),
  m_ctrl_msg(),
  m_wiimote_zero(),
  m_wiimote_one(),
  m_nunchuk_zero(),
  m_nunchuk_one(),
  m_nunchuk_x(),
  m_nunchuk_y(),
  wiimote(m_message_descriptor)
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

std::ostream& operator<<(std::ostream& os, AccCalibration const& cal)
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
WiimoteController::on_status(cwiid_status_mesg const& msg)
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
WiimoteController::on_error(cwiid_error_mesg const& msg)
{
  log_tmp_trace();
}

void
WiimoteController::on_button(cwiid_btn_mesg const& msg)
{
  m_ctrl_msg.set_key(wiimote.minus, msg.buttons & CWIID_BTN_MINUS);
  m_ctrl_msg.set_key(wiimote.home,  msg.buttons & CWIID_BTN_HOME);
  m_ctrl_msg.set_key(wiimote.plus,  msg.buttons & CWIID_BTN_PLUS);

  m_ctrl_msg.set_key(wiimote.btn2, msg.buttons & CWIID_BTN_2);
  m_ctrl_msg.set_key(wiimote.btn1, msg.buttons & CWIID_BTN_1);
  m_ctrl_msg.set_key(wiimote.b, msg.buttons & CWIID_BTN_B);
  m_ctrl_msg.set_key(wiimote.a, msg.buttons & CWIID_BTN_A);

  m_ctrl_msg.set_key(wiimote.dpad_left,  msg.buttons & CWIID_BTN_LEFT);
  m_ctrl_msg.set_key(wiimote.dpad_right, msg.buttons & CWIID_BTN_RIGHT);
  m_ctrl_msg.set_key(wiimote.dpad_down,  msg.buttons & CWIID_BTN_DOWN);
  m_ctrl_msg.set_key(wiimote.dpad_up,    msg.buttons & CWIID_BTN_UP);

  submit_msg(m_ctrl_msg, m_message_descriptor);
}

void
WiimoteController::on_acc(cwiid_acc_mesg const& msg)
{
  m_ctrl_msg.set_abs(wiimote.acc_x, msg.acc[0], 0, 255);
  m_ctrl_msg.set_abs(wiimote.acc_y, msg.acc[1], 0, 255);
  m_ctrl_msg.set_abs(wiimote.acc_z, msg.acc[2], 0, 255);

  submit_msg(m_ctrl_msg, m_message_descriptor);
}

void
WiimoteController::on_ir(cwiid_ir_mesg const& msg)
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
  m_ctrl_msg.set_abs(wiimote.ir1_x, msg.src[0].pos[0], 0, 1024);
  m_ctrl_msg.set_abs(wiimote.ir1_y, msg.src[0].pos[1], 0, 768);
  if (msg.src[0].valid)
    m_ctrl_msg.set_abs(wiimote.ir1_size, msg.src[0].size, -128, 127);
  else
    m_ctrl_msg.set_abs(wiimote.ir1_size, -1, -128, 127);

  m_ctrl_msg.set_abs(wiimote.ir2_x, msg.src[1].pos[0], 0, 1024);
  m_ctrl_msg.set_abs(wiimote.ir2_x, msg.src[1].pos[1], 0, 768);
  if (msg.src[1].valid)
    m_ctrl_msg.set_abs(wiimote.ir2_size, msg.src[1].size, -128, 127);
  else
    m_ctrl_msg.set_abs(wiimote.ir2_size, -1, -128, 127);

  m_ctrl_msg.set_abs(wiimote.ir3_x, msg.src[2].pos[0], 0, 1024);
  m_ctrl_msg.set_abs(wiimote.ir3_y, msg.src[2].pos[1], 0, 768);
  if (msg.src[2].valid)
    m_ctrl_msg.set_abs(wiimote.ir3_size, msg.src[2].size, -128, 127);
  else
    m_ctrl_msg.set_abs(wiimote.ir3_size, -1, -128, 127);

  m_ctrl_msg.set_abs(wiimote.ir4_x, msg.src[3].pos[0], 0, 1024);
  m_ctrl_msg.set_abs(wiimote.ir4_x, msg.src[3].pos[1], 0, 768);
  if (msg.src[3].valid)
    m_ctrl_msg.set_abs(wiimote.ir4_size, msg.src[3].size, -128, 127);
  else
    m_ctrl_msg.set_abs(wiimote.ir4_size, -1, -128, 127);

  submit_msg(m_ctrl_msg, m_message_descriptor);
}

// FIXME: use proper CalibrationAxisFilter instead of this hack, also CalibrationAxisFilter doesn't handle min/max properly
int8_t calibrate(int value, AccCalibration const& cal)
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
WiimoteController::on_nunchuk(cwiid_nunchuk_mesg const& msg)
{
  m_ctrl_msg.set_abs(wiimote.nunchuk_x, unpack::s8_to_s16(calibrate(msg.stick[0], m_nunchuk_x)), -32768, 32767);
  m_ctrl_msg.set_abs(wiimote.nunchuk_y, unpack::s16_invert(unpack::s8_to_s16(calibrate(msg.stick[1], m_nunchuk_y))), -32768, 32767);

  m_ctrl_msg.set_abs(wiimote.nunchuk_acc_x, msg.acc[0], 0, 255);
  m_ctrl_msg.set_abs(wiimote.nunchuk_acc_y, msg.acc[1], 0, 255);
  m_ctrl_msg.set_abs(wiimote.nunchuk_acc_z, msg.acc[2], 0, 255);

  m_ctrl_msg.set_key(wiimote.nunchuk_z, msg.buttons & CWIID_NUNCHUK_BTN_Z);
  m_ctrl_msg.set_key(wiimote.nunchuk_c, msg.buttons & CWIID_NUNCHUK_BTN_C);

  submit_msg(m_ctrl_msg, m_message_descriptor);
}

void
WiimoteController::on_classic (cwiid_classic_mesg const& msg)
{
  log_tmp_trace();
}

void
WiimoteController::err_callback(cwiid_wiimote_t*, char const* s, va_list ap)
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

} // namespace xboxdrv

#endif /* HAVE_CWIID */

/* EOF */

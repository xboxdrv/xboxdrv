#include "xboxone_wired_controller.hpp"

#include <sstream>
#include <boost/format.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

//#include <iostream>

//#define LOG(...) {std::cout << "log: " << __VA_ARGS__ << "\n";}

XboxOneWiredController::XboxOneWiredController(libusb_device* usb, int controller_id, bool try_detach): USBController(usb) {
  m_endpoint  = controller_id*2 + 1;
  m_interface = controller_id*2;

  sent_auth = false;
  guide_button = false;

  usb_claim_interface(m_interface, try_detach);
  usb_submit_read(m_endpoint, 32);
}

XboxOneWiredController::~XboxOneWiredController() {
}

static const char* ascii = "0123456789abcdefXXXX";
bool XboxOneWiredController::parse(uint8_t* data, int len, XboxGenericMsg* omsg) {
//    std::string hexbuf;
//    for(int i = 0; i < len; i++) {
//        hexbuf += ascii[data[i] >> 4];
//        hexbuf += ascii[data[i] & 15];
//        hexbuf += " ";
//    }

//    LOG(hexbuf.c_str());

    if(data[0] == 0x20) {
        return parse_button_status(data, len, omsg);
    } else if(data[0] == 0x07) {
        return parse_ledbutton_status(data, len, omsg);
    } else if(data[0] == 0x03) {
        return parse_init_status(data, len, omsg);
    } else if(data[0] == 0x02) {
        return parse_auth_status(data, len, omsg);
    }

    return false;
}

void XboxOneWiredController::set_rumble_real(uint8_t left, uint8_t right) {
}

void XboxOneWiredController::set_led_real(uint8_t status) {
}

struct XboxOneButtonData {
    uint8_t type;
    uint8_t const_0;
    uint16_t id;

    bool sync : 1;
    bool dummy1 : 1;  // Always 0.
    bool start : 1;
    bool back : 1;

    bool a : 1;
    bool b : 1;
    bool x : 1;
    bool y : 1;

    bool dpad_up : 1;
    bool dpad_down : 1;
    bool dpad_left : 1;
    bool dpad_right : 1;

    bool bumper_left : 1;
    bool bumper_right : 1;
    bool stick_left_click : 1;
    bool stick_right_click : 1;

    uint16_t trigger_left;
    uint16_t trigger_right;

    int16_t stick_left_x;
    int16_t stick_left_y;
    int16_t stick_right_x;
    int16_t stick_right_y;
};

struct XboxOneGuideData {
    uint8_t type;
    uint8_t const_20;
    uint16_t id;

    uint8_t down;
    uint8_t dummy_const_5b;
};

bool XboxOneWiredController::parse_button_status(uint8_t* data, int len, XboxGenericMsg* omsg) {
    omsg->type = XBOX_MSG_XBOX360;
    Xbox360Msg& msg = omsg->xbox360;
    XboxOneButtonData* button_data = (XboxOneButtonData*) data;

    memset((void*) &msg, 0, sizeof(msg));

    msg.a = button_data->a;
    msg.b = button_data->b;
    msg.x = button_data->x;
    msg.y = button_data->y;
    msg.start = button_data->start;
    msg.back = button_data->back;
    msg.dpad_up = button_data->dpad_up;
    msg.dpad_down = button_data->dpad_down;
    msg.dpad_left = button_data->dpad_left;
    msg.dpad_right = button_data->dpad_right;
    msg.lb = button_data->bumper_left;
    msg.rb = button_data->bumper_right;
    msg.thumb_l = button_data->stick_left_click;
    msg.thumb_r = button_data->stick_right_click;
    msg.lt = button_data->trigger_left / 4;
    msg.rt = button_data->trigger_right / 4;
    msg.x1 = button_data->stick_left_x;
    msg.y1 = button_data->stick_left_y;
    msg.x2 = button_data->stick_right_x;
    msg.y2 = button_data->stick_right_y;
    msg.guide = guide_button;

    return true;
}

bool XboxOneWiredController::parse_ledbutton_status(uint8_t* data, int len, XboxGenericMsg* omsg) {
    XboxOneGuideData* gd = (XboxOneGuideData*) data;
    Xbox360Msg& msg = omsg->xbox360;

    memset((void*) &msg, 0, sizeof(msg));

    msg.type = XBOX_MSG_XBOX360;
    msg.guide = gd->down;
    guide_button = gd->down;

    return true;
}

bool XboxOneWiredController::parse_init_status(uint8_t* data, int len, XboxGenericMsg* omsg) {
    return false;
}

bool XboxOneWiredController::parse_auth_status(uint8_t* data, int len, XboxGenericMsg* omsg) {
    if(!sent_auth && data[1] == 0x20) {
        uint8_t authbuf[2] = { 0x05, 0x20 };
        usb_write(m_endpoint, authbuf, 2);
        sent_auth = true;
    }

    return false;
}

# -*- python -*-

env = Environment(CPPFLAGS=["-g", "-O0", "-Wall"], LIBS=["usb"])
env.Program("xboxdrv", ["xboxdrv.cpp", "uinput.cpp"])
env.Program("inputdrv",
            ["inputdrv.cpp",
             "xbox360_driver.cpp",
             "evdev_driver.cpp",
             "xbox360_usb_thread.cpp",
             "control.cpp",
             "abs_to_rel.cpp",
             "abs_to_btn.cpp",
             "btn_to_abs.cpp",
             "autofire_button.cpp",
             "uinput_driver.cpp",
             "join_axis.cpp",
             "throttle.cpp",
             "toggle_button.cpp"],
            LIBS=['boost_signals', 'usb', 'pthread'])

# EOF #

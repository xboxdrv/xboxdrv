# -*- python -*-

env = Environment(CPPFLAGS=["-g", "-O2", "-Wall"], LIBS=["usb"])
env.Program("xboxdrv", ["xboxdrv.cpp", 
                        "xboxmsg.cpp",
                        "uinput.cpp",
                        "helper.cpp",
                        "command_line_options.cpp",
                        "xbox_controller.cpp",
                        "xbox360_controller.cpp",
                        "xbox360_wireless_controller.cpp",
                        ])

if False:
    env.Program("inputdrv",
                ["src/inputdrv/inputdrv.cpp",
                 "src/inputdrv/xbox360_driver.cpp",
                 "src/inputdrv/evdev_driver.cpp",
                 "src/inputdrv/xbox360_usb_thread.cpp",
                 "src/inputdrv/control.cpp",
                 "src/inputdrv/abs_to_rel.cpp",
                 "src/inputdrv/abs_to_btn.cpp",
                 "src/inputdrv/btn_to_abs.cpp",
                 "src/inputdrv/autofire_button.cpp",
                 "src/inputdrv/uinput_driver.cpp",
                 "src/inputdrv/join_axis.cpp",
                 "src/inputdrv/throttle.cpp",
                 "src/inputdrv/toggle_button.cpp"],
                LIBS=['boost_signals', 'usb', 'pthread'])

# EOF #

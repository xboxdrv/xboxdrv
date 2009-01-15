# -*- python -*-

env = Environment(CPPFLAGS=["-g", "-O2", "-Wall", "-ansi", "-pedantic"], LIBS=["usb", "X11"])
# env = Environment(CPPFLAGS=["-g", "-O2"], LIBS=["usb", "X11"])
env.Program("xboxdrv", ["src/xboxdrv.cpp", 
                        "src/xboxmsg.cpp",
                        "src/uinput.cpp",
                        "src/helper.cpp",
                        "src/modifier.cpp",
                        "src/command_line_options.cpp",
                        "src/xbox_controller.cpp",
                        "src/xbox360_controller.cpp",
                        "src/xbox360_wireless_controller.cpp",
                        'src/firestorm_dual_controller.cpp',
                        "src/evdev_helper.cpp",
                        "src/linux_uinput.cpp"
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

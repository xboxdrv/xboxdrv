# -*- python -*-

env = Environment(CPPFLAGS=["-g", "-O2", "-Wall"], LIBS=["usb"])
env.Program("xboxdrv", ["xboxdrv.cpp", "uinput.cpp"])
env.Program("inputdrv",
            ["inputdrv.cpp",
             "xbox360_driver.cpp",
             "control.cpp",
             "uinput_driver.cpp",
             "toggle_button.cpp"],
            LIBS=['boost_signals', 'usb'])

# EOF #

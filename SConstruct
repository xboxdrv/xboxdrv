# -*- python -*-

if True:
    env = Environment(CPPFLAGS=['-g', '-O2', '-Wall', '-ansi', '-pedantic'])
else:
    env = Environment(CXXFLAGS= [ "-O3", "-g3",
                                  "-ansi",
                                  "-pedantic",
                                  "-Wall",
                                  "-Wextra",
                                  "-Wnon-virtual-dtor",
                                  "-Weffc++",
                                  #"-Wconversion",
                                  "-Wold-style-cast",
                                  # "-Werror",
                                  "-Wshadow",
                                  "-Wcast-qual",
                                  "-Winit-self", # only works with >= -O1
                                  "-Wno-unused-parameter",
                                  ])

f = open("VERSION")
package_version = f.read()
f.close()
    
env.Append(CPPDEFINES={ 'PACKAGE_VERSION': "'\"%s\"'" % package_version})

conf = Configure(env)

if not conf.env['CXX']:
    print "g++ must be installed!"
    Exit(1)

# X11 checks
if not conf.CheckLibWithHeader('X11', 'X11/Xlib.h', 'C++'):
    print 'libx11-dev must be installed!'
    Exit(1)

# libusb Checks
if not conf.CheckLibWithHeader('usb', 'usb.h', 'C++'):
    print 'libusb must be installed!'
    Exit(1)

# boost-thread checks
if not conf.CheckCXXHeader('boost/thread/thread.hpp'):
    print 'libboost-thread-dev must be installed!'
    Exit(1)

if not conf.CheckLib('boost_thread-mt', language='C++'):
    if not conf.CheckLib('boost_thread', language='C++'):
        print 'libboost-thread-dev must be installed!'
        Exit(1)

env = conf.Finish()

env.Program('xboxdrv', ['src/xboxdrv.cpp', 
                        'src/xboxmsg.cpp',
                        'src/uinput.cpp',
                        'src/uinput_deviceid.cpp',
                        'src/uinput_cfg.cpp',
                        'src/button_event.cpp',
                        'src/axis_event.cpp',
                        'src/arg_parser.cpp',
                        'src/button_map.cpp',
                        'src/pretty_printer.cpp',
                        'src/helper.cpp',
                        'src/modifier.cpp',
                        'src/command_line_options.cpp',
                        'src/xbox_controller.cpp',
                        'src/xpad_device.cpp',
                        'src/xbox360_controller.cpp',
                        'src/xbox360_wireless_controller.cpp',
                        'src/firestorm_dual_controller.cpp',
                        'src/saitek_p2500_controller.cpp',
                        'src/evdev_helper.cpp',
                        'src/linux_uinput.cpp',
                        'src/usb_read_thread.cpp',
                        'src/force_feedback_handler.cpp'                        
                        ])

if False:
    env.Program('inputdrv',
                ['src/inputdrv/inputdrv.cpp',
                 'src/inputdrv/xbox360_driver.cpp',
                 'src/inputdrv/evdev_driver.cpp',
                 'src/inputdrv/xbox360_usb_thread.cpp',
                 'src/inputdrv/control.cpp',
                 'src/inputdrv/abs_to_rel.cpp',
                 'src/inputdrv/abs_to_btn.cpp',
                 'src/inputdrv/btn_to_abs.cpp',
                 'src/inputdrv/autofire_button.cpp',
                 'src/inputdrv/uinput_driver.cpp',
                 'src/inputdrv/join_axis.cpp',
                 'src/inputdrv/throttle.cpp',
                 'src/inputdrv/toggle_button.cpp'],
                LIBS=['boost_signals', 'usb', 'pthread'])

# EOF #

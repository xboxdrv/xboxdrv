# -*- python -*-

env = Environment(CPPFLAGS=['-g', '-O2', '-Wall', '-ansi', '-pedantic'])
conf = Configure(env)

# X11 checks
if not conf.CheckLibWithHeader('X11', 'X11/Xlib.h', 'C++'):
    print 'libx11-dev must be installed!'
    Exit(1)
else:
    conf.env.Append(LIBS = 'X11')

# libusb Checks
if not conf.CheckLibWithHeader('usb', 'usb.h', 'C++'):
    print 'libusb must be installed!'
    Exit(1)
else:
    conf.env.Append(LIBS = 'usb')

# boost-thread checks
if not conf.CheckCXXHeader('boost/thread/thread.hpp'):
    print 'libboost-thread-dev must be installed!'
    Exit(1)

boost_thread = None
for lib in ['boost_thread-mt', 'boost_thread']:
    if conf.CheckLib(lib, language='C++'):
        boost_thread = lib
        break

if not boost_thread:
    print 'libboost-thread-dev must be installed!'
    Exit(1)
else:
    conf.env.Append(LIBS = boost_thread)

env = conf.Finish()

# env = Environment(CPPFLAGS=['-g', '-O2'], LIBS=['usb', 'X11'])
env.Program('xboxdrv', ['src/xboxdrv.cpp', 
                        'src/xboxmsg.cpp',
                        'src/uinput.cpp',
                        'src/arg_parser.cpp',
                        'src/pretty_printer.cpp',
                        'src/helper.cpp',
                        'src/modifier.cpp',
                        'src/command_line_options.cpp',
                        'src/xbox_controller.cpp',
                        'src/xpad_device.cpp',
                        'src/xbox360_controller.cpp',
                        'src/xbox360_wireless_controller.cpp',
                        'src/firestorm_dual_controller.cpp',
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

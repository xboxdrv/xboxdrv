# -*- python -*-

if False:
    env = Environment(CPPFLAGS=['-g', '-O2', '-Wall', '-ansi', '-pedantic'],
                      CPPPATH=["src/"])
else:
    env = Environment(CXX="g++-4.5",
                      CXXFLAGS= [ "-O3", "-g3",
                                  "-ansi",
                                  "-pedantic",
                                  "-Wall",
                                  "-Wextra",
                                  "-Wnon-virtual-dtor",
                                  #"-Weffc++",
                                  #"-Wconversion",
                                  "-Wold-style-cast",
                                  # "-Werror",
                                  "-Wshadow",
                                  "-Wcast-qual",
                                  "-Winit-self", # only works with >= -O1
                                  "-Wno-unused-parameter",
                                  ],
                      CPPPATH=["src/"])

env.ParseConfig("pkg-config --cflags --libs libusb-1.0 | sed 's/-I/-isystem/g'")

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

env.Program('xboxdrv',
            Glob('src/*.cpp') +
            Glob('src/modifier/*.cpp'))

if False:
    env.Program('inputdrv', Glob('src/inputdrv/*.cpp'),
                LIBS=['boost_signals', 'usb', 'pthread'])

# EOF #

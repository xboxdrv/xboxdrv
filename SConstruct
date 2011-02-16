# -*- python -*-

env = Environment()

opts = Variables(['custom.py'], ARGUMENTS)

opts.Add('CPPPATH', 'Additional preprocessor paths')
opts.Add('CPPFLAGS', 'Additional preprocessor flags')
opts.Add('CPPDEFINES', 'defined constants')
opts.Add('LIBPATH', 'Additional library paths')
opts.Add('LIBS', 'Additional libraries')
opts.Add('CCFLAGS', 'C Compiler flags')
opts.Add('CXXFLAGS', 'C++ Compiler flags')
opts.Add('LINKFLAGS', 'Linker Compiler flags')
opts.Add('CC', 'C Compiler')
opts.Add('CXX', 'C++ Compiler')
opts.Add('BUILD', 'Build type: release, custom, development')

opts.Update(env)
Help(opts.GenerateHelpText(env))

env.Append(CPPPATH=["src/"])

if 'BUILD' in env and env['BUILD'] == 'development':
    env.Append(CXXFLAGS = [ "-O3",
                            "-g3",
                            "-ansi",
                            "-pedantic",
                            "-Wall",
                            "-Wextra",
                            "-Werror",
                            "-Wnon-virtual-dtor",
                            "-Weffc++",
                            # "-Wconversion",
                            "-Wold-style-cast",
                            "-Wshadow",
                            "-Wcast-qual",
                            "-Winit-self", # only works with >= -O1
                            "-Wno-unused-parameter"])
elif 'BUILD' in env and env['BUILD'] == 'custom':
    pass
else:
    env.Append(CPPFLAGS = ['-g', '-O3', '-Wall', '-ansi', '-pedantic'])

env.ParseConfig("pkg-config --cflags --libs glib-2.0 | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --cflags --libs gthread-2.0 | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --cflags --libs libusb-1.0 | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --cflags --libs libudev | sed 's/-I/-isystem/g'")

f = open("VERSION")
package_version = f.read()
f.close()
    
env.Append(CPPDEFINES = { 'PACKAGE_VERSION': "'\"%s\"'" % package_version })

conf = Configure(env)

if not conf.env['CXX']:
    print "g++ must be installed!"
    Exit(1)

# X11 checks
if not conf.CheckLibWithHeader('X11', 'X11/Xlib.h', 'C++'):
    print 'libx11-dev must be installed!'
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
            Glob('src/axisfilter/*.cpp') +
            Glob('src/buttonfilter/*.cpp') +
            Glob('src/modifier/*.cpp'))

# EOF #

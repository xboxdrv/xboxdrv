# -*- python -*-

import subprocess

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

env.ParseConfig("pkg-config --cflags --libs dbus-glib-1 | sed 's/-I/-isystem/g'")
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

def build_dbus_glue(target, source, env):
    xml = subprocess.Popen(["dbus-binding-tool", "--mode=glib-server", "--prefix=xboxdrv", source[0].get_path()],
                           stdout=subprocess.PIPE).communicate()[0]

    # converting void to a function pointer is forbidden in C++, thus we use a union instead
    xml = xml.replace("callback = (GMarshalFunc_BOOLEAN__POINTER) (marshal_data ? marshal_data : cc->callback);",
                      "union { GMarshalFunc_BOOLEAN__POINTER fn; void* obj; } conv;\n  "
                      "conv.obj = (marshal_data ? marshal_data : cc->callback);\n  "
                      "callback = conv.fn;")

    with open(target[0].get_path(), "w") as f:
        f.write(xml)
    
env.Command("src/xboxdrv_dbus_glue.hpp", "src/xboxdrv_dbus.xml", build_dbus_glue)
env.Program('xboxdrv',
            Glob('src/*.cpp') +
            Glob('src/axisfilter/*.cpp') +
            Glob('src/buttonfilter/*.cpp') +
            Glob('src/modifier/*.cpp'))

# EOF #

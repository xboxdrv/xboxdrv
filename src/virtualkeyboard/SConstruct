env = Environment()
env.Append(CPPPATH=[".."])
env.ParseConfig("pkg-config --libs --cflags gtk+-2.0")
env.Program("virtual_keyboard", ["virtual_keyboard.cpp", "keyboard_description.cpp", "main.cpp"])

# EOF #

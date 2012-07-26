import config
from SCons.Script import *

Import('env')

inc_path = ['./include']

src = Split("""
src/gtkminicom.c
src/window.c
src/serialconfigdialog.c
src/serial.c
src/logfile.c
src/trayicon.c
""")

env.Append(CPPPATH = inc_path)

objs = env.Object(src)

Return('objs')

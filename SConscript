import config
from SCons.Script import *

Import('env')

inc_path = ['./']

src = Split("""
gtkminicom.c
window.c
serialconfigdialog.c
serial.c
logfile.c
trayicon.c
""")

env.Append(CPPPATH = inc_path)

objs = env.Object(src)

Return('objs')

# RT-Thread building script for bridge

import os
from building import *

cwd = GetCurrentDir()
objs = []
list = os.listdir(cwd)
src = Glob('*.c')
CPPPATH = [cwd]
LOCAL_CCFLAGS = ' -D__RT_THREAD__'

for d in list:
    path = os.path.join(cwd, d)
    if os.path.isfile(os.path.join(path, 'SConscript')):
        objs = objs + SConscript(os.path.join(d, 'SConscript'))

group = DefineGroup('Audio_server', src, depend = ['AUDIO_USING_SOURCE_CODE'], CPPPATH = CPPPATH, LOCAL_CCFLAGS = LOCAL_CCFLAGS)
objs = objs + group

Return('objs')

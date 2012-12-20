import os
from build_support import *
from build_config import *

vars = Variables('custom.py')

env = Environment(variables = vars, TW_LIBS = [])
env.Append(CPPDEFINES=['TW_SMALLINTEGER_AS_OBJECT', 'TW_IS_INITIAL'])
opt = env.Clone()

target_dir = '#' + SelectBuildDir(build_base_dir)
install_dir = '#install'

Export('install_dir')

SConscript(target_dir + os.sep + 'SConscript', exports={'env' : env, 'opt' : opt})

Help(vars.GenerateHelpText(env))

for sub in ['source', 'bootstrap']:
  VariantDir(target_dir + os.sep + sub, sub)
  SConscript(target_dir + os.sep + sub + os.sep + 'SConscript', exports={'env' : env})
  Clean('.', target_dir + os.sep + sub)

env.Alias('install', '#install')

# ex: set filetype=python tabstop=2 shiftwidth=2 expandtab: 

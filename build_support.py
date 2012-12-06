import os
import sys

def SelectBuildDir(build_dir, platform=None):
  if not(platform):
    platform = sys.platform

  print "Looking for build directory for platform '%s'" % platform

  test_dir = build_dir + os.sep + platform
  default_dir = build_dir + os.sep + 'default'

  if os.path.exists(test_dir):
    target_dir = test_dir
  else:
    print "Exact match not found, finding closest guess"

    dirs = os.listdir(build_dir)
    found_match = 0
    for dir in dirs:
      if platform.find(dir) != -1:
        target_dir = build_dir + os.sep + dir
        found_match = 1
        break
  
    if not(found_match):
      print "No match found, looking for 'default' directory"
      if os.path.exists(default_dir):
        target_dir = default_dir
      else:
        print "No build directories found for your platform '%s'" % platform
        return None

  print "Found directory %s, will build there" % target_dir
  return target_dir


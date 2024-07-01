import os
import glob
import shutil

pro_dir = os.path.dirname(os.path.realpath(__file__))
build_dir = os.getcwd()
# print("//// PRO_DIR =", pro_dir, " BUILD_DIR =", build_dir)

is_release = build_dir.endswith('-Release')
if not is_release:
    print('Not a Release build, do not copy libraries')
    exit(0)

# Check what platform the build directory is
if glob.glob(build_dir + '/release/*.exe'):
    platform = 'win'
    print('Windows build detected. Proceeding to copy Windows libraries')
elif glob.glob(build_dir + '/release/*.app'):
    platform = 'mac'
    print('MacOS build detected. Proceeding to copy MacOS libraries')
else:
    platform = 'unknown'
    print('Unable to detect platform of build binary.')
    exit(1)

# Copies all the DLLs over
if platform == 'win':
    for file in glob.glob(pro_dir + '/3rd/win32/*.dll'):
        print('Copying', file)
        shutil.copy2(file, build_dir + '/release/')
    print('Copying plugins directory')
    shutil.copytree(pro_dir + '/3rd/win32/plugins', build_dir + '/release/plugins', dirs_exist_ok=True)
elif platform == 'mac':
    for file in glob.glob(pro_dir + '/3rd/macx64/*.dylib'):
        print('Copying', file)
        shutil.copy2(file, build_dir + '/release/')
    for file in glob.glob(pro_dir + '/3rd/macx64/*.framework'):
        print('Copying', file)
        shutil.copytree(file, build_dir + '/release/', dirs_exist_ok=True)

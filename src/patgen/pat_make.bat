@echo off
set old_parent_ver=2.1
set old_ver=2.1
set new_parent_ver=2.2
set new_ver=2.2

set CP=xcopy /H /C /Y
if exist set_ver.bat call set_ver.bat

set patch_platform=windows
set patch_basename=moai-v%old_parent_ver%-patch-v%new_ver%-%patch_platform%
set patch_basename_old=moai-v%old_parent_ver%-patch-v%old_ver%
mkdir %patch_basename%
mkdir %patch_basename%\moai-common\
mkdir %patch_basename%\moai-win32\
mkdir %patch_basename%\moai-win64\
win64\patgen make diff_win32.pmk               %patch_basename%\moai-common %patch_basename%\moai-win32               win32
win64\patgen make diff_win64.pmk               %patch_basename%\moai-common %patch_basename%\moai-win64               win64
REM if exist apply_this_scripts\windows\apply_this_patch.bat %CP% apply_this_scripts\windows\apply_this_patch.bat %patch_basename%\
if exist set_ver.bat %CP% set_ver.bat %patch_basename%\
if exist apply_list.myf %CP% apply_list.myf %patch_basename%\
if not exist %patch_basename%\moai-common\birdman.myf %CP% apply_this_scripts\birdman.myf %patch_basename%\

set patch_platform=android
set patch_basename=moai-v%old_parent_ver%-patch-v%new_ver%-%patch_platform%
set patch_basename_old=moai-v%old_parent_ver%-patch-v%old_ver%
mkdir %patch_basename%
mkdir %patch_basename%\moai-common\
mkdir %patch_basename%\moai-android-armeabi\
REM mkdir %patch_basename%\moai-android-armeabi-v7a\
mkdir %patch_basename%\moai-android-x86\
win64\patgen make diff_android-armeabi.pmk     %patch_basename%\moai-common %patch_basename%\moai-android-armeabi     android-armeabi
REM win64\patgen make diff_android-armeabi-v7a.pmk %patch_basename%\moai-common %patch_basename%\moai-android-armeabi-v7a android-armeabi-v7a
win64\patgen make diff_android-x86.pmk         %patch_basename%\moai-common %patch_basename%\moai-android-x86         android-x86
REM if exist apply_this_scripts\android\apply_this_patch.sh %CP% apply_this_scripts\android\apply_this_patch.sh %patch_basename%\
if exist set_ver.sh %CP% set_ver.sh %patch_basename%\
if exist apply_list.myf %CP% apply_list.myf %patch_basename%\
if not exist %patch_basename%\moai-common\birdman.myf %CP% apply_this_scripts\birdman.myf %patch_basename%\

pause

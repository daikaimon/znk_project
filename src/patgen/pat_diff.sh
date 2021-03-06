#!/bin/sh

old_parent_ver=2.1
old_ver=2.1
new_parent_ver=2.2
new_ver=2.2

if test -e set_ver.sh ; then source ./set_ver.sh ; fi

if test "x$MK_OS" = "x"; then
	MK_OS=`uname | cut -d _ -f 1`
fi

if test "$MK_OS" = "CYGWIN"; then
	cygwin32/patgen diff ../releases_dir/v$old_ver/moai-v$old_parent_ver-cygwin32 ../moai-v$new_parent_ver-cygwin32 > diff_cygwin32.pmk
	if test -e ../moai-v$new_parent_ver-cygwin32/birdman/birdman.myf ; then
		cp ../moai-v$new_parent_ver-cygwin32/birdman/birdman.myf apply_this_scripts/
	fi
else
	linux32/patgen  diff ../releases_dir/v$old_ver/moai-v$old_parent_ver-linux32  ../moai-v$new_parent_ver-linux32  > diff_linux32.pmk
	linux64/patgen  diff ../releases_dir/v$old_ver/moai-v$old_parent_ver-linux64  ../moai-v$new_parent_ver-linux64  > diff_linux64.pmk
	if test -e ../moai-v$new_parent_ver-linux32/birdman/birdman.myf ; then
		cp ../moai-v$new_parent_ver-linux32/birdman/birdman.myf apply_this_scripts/
	fi
	if test -e ../moai-v$new_parent_ver-linux64/birdman/birdman.myf ; then
		cp ../moai-v$new_parent_ver-linux64/birdman/birdman.myf apply_this_scripts/
	fi
fi

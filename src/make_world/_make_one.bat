@echo off
if not exist ..\%3 goto Error

REM 以下のものをcleanする場合は例外的にskipする.
if "%4" == "clean" (
	set SKIP_CLEAN_LIST=
	if exist ..\proj_list_skip_clean.bat call ..\proj_list_skip_clean.bat
	for %%a in ( %SKIP_CLEAN_LIST% ) do if "%3" == "%%a" goto Skip
)

setlocal
echo ======
echo === Entering [%3] ===
cd ..\%3
%1 -f Makefile_%2.mak %4
if errorlevel 1 goto Error_Make
echo === Leaving [%3] ===
echo ======
echo:
endlocal
REM endlocalによりdirectoryも元に戻る.
goto End

:Skip
echo Skip : directory [%3] is skipped.
goto End

:Error
echo Error : directory [%3] does not exist.
pause
goto End

:Error_Make
echo Error : %1 -f Makefile_%2.mak %4 abort.
pause
:End

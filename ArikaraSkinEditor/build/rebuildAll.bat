@echo off

:: Set MAYA_DEV in your environment variable
CALL :NORMALIZEPATH "%~dp0.."
set ARI_LOC=%RETVAL%

CALL %~dp0setpath.bat

@echo Rebuilding All arikara %ARI_VERSION% %ARI_CONFIG%

@echo Generate Make files:
@echo ----------------------------------------

copy %ARI_LOC%\build\build%ARI_VERSION%\qtconfig %ARI_LOC%\qtconfig > NUL

call %DEVKIT_LOCATION%\devkit\bin\qmake.exe -o %ARI_LOC%\makefile %ARI_LOC%\ArikaraSkinEditor.pro

call python %ARI_LOC%\build\replaceLibs.py %ARI_LOC%\makefile.Debug
call python %ARI_LOC%\build\replaceOutput.py %ARI_LOC%\makefile.Debug %ARI_VERSION%
call python %ARI_LOC%\build\replaceOutput.py %ARI_LOC%\makefile.Release %ARI_VERSION%

IF NOT DEFINED MAKE_FILE (
	echo Cleaning files ---------------------------
	call %~dp0clean.bat
	IF "%ARI_CONFIG%"=="Release" (
		if not exist "%ARI_LOC%\release" mkdir "%ARI_LOC%\release"
	) ELSE (
		if not exist "%ARI_LOC%\debug" mkdir "%ARI_LOC%\debug"
	)

	cd %ARI_LOC%
	@echo Starting build
	@echo ----------------------------------------
	call %VS_PATH%\VC\vcvarsall.bat x64
	call %VS_PATH%\VC\bin\nmake.exe /f %ARI_LOC%\makefile.%ARI_CONFIG%
)

@echo Copy file and clean folder
@echo ----------------------------------------

copy %ARI_LOC%\makefile %ARI_LOC%\build\build%ARI_VERSION%\makefile > NUL
copy %ARI_LOC%\makefile.Debug %ARI_LOC%\build\build%ARI_VERSION%\makefile.Debug > NUL
copy %ARI_LOC%\makefile.Release %ARI_LOC%\build\build%ARI_VERSION%\makefile.Release > NUL
del %ARI_LOC%\makefile > NUL
del %ARI_LOC%\makefile.Debug > NUL
del %ARI_LOC%\makefile.Release > NUL
del %ARI_LOC%\qtconfig > NUL


:: ========== FUNCTIONS ==========
EXIT /B

:NORMALIZEPATH
  SET RETVAL=%~dpfn1
  EXIT /B
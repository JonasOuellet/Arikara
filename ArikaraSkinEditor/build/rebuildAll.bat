:: Set MAYA_DEV in your environment variable
CALL :NORMALIZEPATH "%~dp0.."
set ARI_LOC=%RETVAL%

@echo Rebuilding All arikara %ARI_VERSION% %ARI_CONFIG%

@echo Generate Make files:
@echo ----------------------------------------

copy %ARI_LOC%\build%ARI_VERSION%\qtconfig %ARI_LOC%\qtconfig"

set DEVKIT_LOCATION=C:\Program Files\Autodesk\Maya%ARI_VERSION%
set MAYA_LOCATION=C:\Program Files\Autodesk\Maya%ARI_VERSION%
call "C:\Program Files\Autodesk\Maya%ARI_VERSION%\devkit\bin\qmake.exe" -o %ARI_LOC%\makefile %ARI_LOC%\ArikaraSkinEditor.pro

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
	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\nmake.exe" /f %ARI_LOC%\makefile.%ARI_CONFIG%
)

@echo Copy file and clean folder
@echo ----------------------------------------

copy %ARI_LOC%\makefile %ARI_LOC%\build%ARI_VERSION%\makefile
copy %ARI_LOC%\makefile.Debug %ARI_LOC%\build%ARI_VERSION%\makefile.Debug
copy %ARI_LOC%\makefile.Release %ARI_LOC%\build%ARI_VERSION%\makefile.Release
del %ARI_LOC%\makefile
del %ARI_LOC%\makefile.Debug
del %ARI_LOC%\makefile.Release
del %ARI_LOC%\qtconfig


:: ========== FUNCTIONS ==========
EXIT /B

:NORMALIZEPATH
  SET RETVAL=%~dpfn1
  EXIT /B
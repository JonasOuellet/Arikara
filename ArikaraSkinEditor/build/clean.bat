:: Set MAYA_DEV in your environment variable
CALL :NORMALIZEPATH "%~dp0.."
set ARI_LOC=%RETVAL%

@echo Cleaning arikara %ARI_VERSION% %ARI_CONFIG%

echo %ARI_LOC%\build%ARI_VERSION%\makefile.%ARI_CONFIG%

if EXIST %ARI_LOC%\build%ARI_VERSION%\makefile.%ARI_CONFIG% (
	copy %ARI_LOC%\build%ARI_VERSION%\makefile.%ARI_CONFIG% %ARI_LOC%\makefile.%ARI_CONFIG%
	cd %ARI_LOC%
	"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\nmake.exe" /f %ARI_LOC%\Makefile.%ARI_CONFIG% clean
) ELSE (
	@ECHO Make file not found
)

del /S /F /Q %ARI_LOC%\..\MayaModule\arikara\plug-ins\%ARI_VERSION%\*

:: ========== FUNCTIONS ==========
EXIT /B

:NORMALIZEPATH
  SET RETVAL=%~dpfn1
  EXIT /B

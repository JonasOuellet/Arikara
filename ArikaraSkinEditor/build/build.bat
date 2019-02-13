CALL :NORMALIZEPATH "%~dp0.."
set ARI_LOC=%RETVAL%

set CURDIR=%~dp0

@echo Bulding arikara %ARI_VERSION% %ARI_CONFIG% in %ARI_LOC%

IF "%ARI_CONFIG%"=="Release" (
    if not exist "%ARI_LOC%\release" mkdir "%ARI_LOC%\release"
) ELSE (
	if not exist "%ARI_LOC%\debug" mkdir "%ARI_LOC%\debug"
)
cd %ARI_LOC%

copy %ARI_LOC%\build\build%ARI_VERSION%\makefile %ARI_LOC%\makefile
copy %ARI_LOC%\build\build%ARI_VERSION%\makefile.%ARI_CONFIG% %ARI_LOC%\makefile.%ARI_CONFIG%

"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\nmake.exe" /f %ARI_LOC%\Makefile.%ARI_CONFIG%

del %ARI_LOC%\makefile
del %ARI_LOC%\makefile.%ARI_CONFIG%

cd %CURDIR%


:: ========== FUNCTIONS ==========
EXIT /B

:NORMALIZEPATH
  SET RETVAL=%~dpfn1
  EXIT /B
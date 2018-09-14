:: Set MAYA_DEV in your environment variable
set ARI_LOC=E:\OneDrive\maya\ArikaraMaya\ArikaraSkinEditor
@echo Cleaning arikara %ARI_VERSION% %ARI_CONFIG%

if EXIST %ARI_LOC%\build%ARI_VERSION%\makefile (copy %ARI_LOC%\build%ARI_VERSION%\makefile %ARI_LOC%\makefile)
if EXIST %ARI_LOC%\build%ARI_VERSION%\makefile.%ARI_CONFIG% (
copy %ARI_LOC%\build%ARI_VERSION%\makefile.%ARI_CONFIG% %ARI_LOC%\makefile.%ARI_CONFIG%
cd %ARI_LOC%
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\nmake.exe" /f %ARI_LOC%\Makefile.%ARI_CONFIG% clean
) ELSE (
@ECHO Make file not found
)
IF EXIST %ARI_LOC%\makefile (del %ARI_LOC%\makefile)
IF EXIST %ARI_LOC%\makefile.Debug (del %ARI_LOC%\makefile.Debug)
IF EXIST %ARI_LOC%\makefile.Release (del %ARI_LOC%\makefile.Release)

IF EXIST %ARI_LOC%\build%ARI_VERSION%\makefile (del %ARI_LOC%\build%ARI_VERSION%\makefile)
IF EXIST %ARI_LOC%\build%ARI_VERSION%\makefile.Debug (del %ARI_LOC%\build%ARI_VERSION%\makefile.Debug)
IF EXIST %ARI_LOC%\build%ARI_VERSION%\makefile.Release (del %ARI_LOC%\build%ARI_VERSION%\makefile.Release)

IF EXIST %ARI_LOC%\qtconfig (del %ARI_LOC%\qtconfig)

del /S /F /Q %MAYA_DEV%\MayaModule\arikara\plug-ins\%ARI_VERSION%\*
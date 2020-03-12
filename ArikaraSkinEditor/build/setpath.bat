@echo off

if %ARI_VERSION%==2017 (
    set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio 14.0"
    GOTO EXIT0
)

if %ARI_VERSION%==2018 (
    set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio 14.0"
    GOTO EXIT0
)

if %ARI_VERSION%==2019 (
    set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio 14.0"
    GOTO EXIT0
)

if %ARI_VERSION%==2020 (
    set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio 14.0"
    GOTO EXIT0
)

:EXIT0

set DEVKIT_LOCATION="C:\Program Files\Autodesk\Maya%ARI_VERSION%"
set MAYA_LOCATION="C:\Program Files\Autodesk\Maya%ARI_VERSION%"
set PATH=%PATH%;%MAYA_LOCATION:"=%\bin
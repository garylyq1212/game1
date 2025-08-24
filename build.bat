@echo off
REM Path to the Visual Studio environment setup script
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Change to the folder where you keep your C++ files
REM cd /d D:\Projects\Cpp

REM Open cmd with environment ready
REM cmd

mkdir build
pushd build

cl -FC -Zi ..\src\win32_handmade.cpp user32.lib gdi32.lib ole32.lib

popd
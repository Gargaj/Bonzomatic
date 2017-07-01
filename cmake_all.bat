set BNZ_X64=ON
set BNZ_FLAVOR=GLFW
call :build
set BNZ_X64=ON
set BNZ_FLAVOR=DX9
call :build
set BNZ_X64=ON
set BNZ_FLAVOR=DX11
call :build

copy LICENSE _package.x64
copy README.md _package.x64

set BNZ_X64=OFF
set BNZ_FLAVOR=GLFW
call :build
set BNZ_X64=OFF
set BNZ_FLAVOR=DX9
call :build
set BNZ_X64=OFF
set BNZ_FLAVOR=DX11
call :build

copy LICENSE _package.x86
copy README.md _package.x86

goto :eof

REM --------------------- BUILD TIME -------------------------------

:build

set BNZ_COMPILER=Visual Studio 10 2010
if not "%BNZ_X64%"=="ON" goto skipme
set BNZ_COMPILER=Visual Studio 10 2010 Win64
:skipme

set BNZ_OUT_DIR=x86
set BNZ_PLATFORM=W32
if not "%BNZ_X64%"=="ON" goto skipme
set BNZ_OUT_DIR=x64
set BNZ_PLATFORM=W64
:skipme

mkdir build.%BNZ_OUT_DIR%.%BNZ_FLAVOR%
cd build.%BNZ_OUT_DIR%.%BNZ_FLAVOR%
cmake -DBONZOMATIC_NDI="ON" -DBONZOMATIC_64BIT="%BNZ_X64%" -DBONZOMATIC_WINDOWS_FLAVOR:STRING="%BNZ_FLAVOR%" -G "%BNZ_COMPILER%" ../
cmake --build . --config Release 
mkdir ..\_package.%BNZ_OUT_DIR%\
del ..\_package.%BNZ_OUT_DIR%\Bonzomatic_%BNZ_PLATFORM%_%BNZ_FLAVOR%.exe
copy .\Release\Bonzomatic.exe ..\_package.%BNZ_OUT_DIR%\Bonzomatic_%BNZ_PLATFORM%_%BNZ_FLAVOR%.exe
copy .\Release\bass.dll ..\_package.%BNZ_OUT_DIR%\
copy .\Release\Processing.NDI.Lib.%BNZ_OUT_DIR%.dll ..\_package.%BNZ_OUT_DIR%\
cd ..
goto :eof

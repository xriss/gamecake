@echo off
REM
REM To use Visual C++ 2008 Express, set the following environment variables,
REM and execute 'make.cmd'. You can use the shortcut made by installer at:
REM
REM >> All Programs >> Visual C++ 9.0 Express Edition >> Visual Studio Tools
REM 	>> Visual Studio 2008 Command Prompt
REM
REM Visual C++ 2005:
REM 	VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio 8\VC
REM 	VS80COMNTOOLS=C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\
REM 	VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 8
REM
REM Visual C++ 2008:
REM 	VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio 9.0\VC
REM 	VS90COMNTOOLS=C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\
REM 	VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 9.0
REM

set VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 9.0
if exist "%VSINSTALLDIR%\VC\vcvarsall.bat" goto FOUND_VC

set VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 8
if exist "%VSINSTALLDIR%\VC\vcvarsall.bat" goto FOUND_VC

goto ERR_NOVC

:FOUND_VC
set VCINSTALLDIR=%VSINSTALLDIR%\vc

REM vcvars.bat sets the following values right:
REM
REM PATH=...
REM INCLUDE=%VCINSTALLDIR%\ATLMFC\INCLUDE;%VCINSTALLDIR%\INCLUDE;%VCINSTALLDIR%\PlatformSDK\include;%FrameworkSDKDir%\include;%INCLUDE%
REM LIB=%VCINSTALLDIR%\ATLMFC\LIB;%VCINSTALLDIR%\LIB;%VCINSTALLDIR%\PlatformSDK\lib;%FrameworkSDKDir%\lib;%LIB%
REM LIBPATH=%FrameworkDir%\%FrameworkVersion%;%VCINSTALLDIR%\ATLMFC\LIB
REM
call "%VSINSTALLDIR%\VC\vcvarsall.bat"

REM Win32 headers are made available by:
REM
set _SDK=C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\SetEnv.cmd
if not exist "%_SDK%" goto ERR_NOSDK
call "%_SDK%"

REM 'timeit.exe' is part of the MS Server Res Kit Tools (needed for "make perftest")
REM
set _RESKIT=C:\Program Files\Windows Resource Kits\Tools\
if not exist "%_RESKIT%\timeit.exe" goto WARN_NOTIMEIT
PATH=%PATH%;%_RESKIT%
goto EXIT

:WARN_NOTIMEIT
echo.
echo ** WARNING: Windows Server 2003 Resource Kit Tools - not detected
echo             You will need the 'timeit' utility to run 'make perftest'
echo             http://www.microsoft.com/downloads/details.aspx?familyid=9D467A69-57FF-4AE7-96EE-B18C4790CFFD
echo.
goto EXIT

REM ---
:ERR_NOVC
echo.
echo ** ERROR: Visual C++ 2005/08 Express - not detected
echo           You can set the environment variables separately, and run 'make.cmd'
echo           or download the compiler from:
echo           http://msdn.microsoft.com/vstudio/express/downloads/
echo.
goto EXIT

:ERR_NOSDK
echo.
echo ** ERROR: Windows Server 2003 Platform SDK - not detected
echo           You will need the core API's of it to compile Win32 applications.
echo           http://www.microsoft.com/downloads/details.aspx?familyid=0BAF2B35-C656-4969-ACE8-E4C0C0716ADB
echo.
goto EXIT

:EXIT
set _SDK=
set _RESKIT=

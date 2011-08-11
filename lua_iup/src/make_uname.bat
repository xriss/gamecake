@echo off
REM This builds all the libraries of the folder for 1 uname

call tecmake %1 %2 %3 %4 %5 %6
call tecmake %1 "USE_GTK=Yes" %2 %3 %4 %5 %6

if "%1"=="dll" goto stub_dll6
if "%1"=="dll7" goto stub_dll7
if "%1"=="dll8" goto stub_dll8
if "%1"=="dll8_64" goto stub_dll8_64
if "%1"=="dll9" goto stub_dll9
if "%1"=="dll9_64" goto stub_dll9_64
if "%1"=="dll10" goto stub_dll10
if "%1"=="dll10_64" goto stub_dll10_64
if "%1"=="dllg4" goto stub_dllg4
if "%1"=="dllw4" goto stub_dllw4
if "%1"=="all" goto all_dll
goto fim

:stub_dll6
call tecmake vc6 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc6\iupstub.lib ..\lib\dll
goto fim

:stub_dll7
call tecmake vc7 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc7\iupstub.lib ..\lib\dll7
goto fim

:stub_dll8
call tecmake vc8 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc8\iupstub.lib ..\lib\dll8
goto fim

:stub_dll8_64
call tecmake vc8_64 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc8_64\iupstub.lib ..\lib\dll8_64
goto fim

:stub_dll9
call tecmake vc9 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc9\iupstub.lib ..\lib\dll9
goto fim

:stub_dll9_64
call tecmake vc9_64 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc9_64\iupstub.lib ..\lib\dll9_64
goto fim

:stub_dll10
call tecmake vc10 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc10\iupstub.lib ..\lib\dll10
goto fim

:stub_dll10_64
call tecmake vc10_64 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\vc10_64\iupstub.lib ..\lib\dll10_64
goto fim

:stub_dllw4
call tecmake mingw4 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\mingw4\libiupstub.a ..\lib\dllw4
goto fim

:stub_dllg4
call tecmake gcc4 "MF=iupstub" %2 %3 %4 %5 %6 %7
move /y ..\lib\gcc4\libiupstub.a ..\lib\dllg4
goto fim

:all_dll
call make_uname dll %2 %3 %4 %5 %6
call make_uname dll7 %2 %3 %4 %5 %6
call make_uname dll8 %2 %3 %4 %5 %6
call make_uname dll8_64 %2 %3 %4 %5 %6
call make_uname dll9 %2 %3 %4 %5 %6
call make_uname dll9_64 %2 %3 %4 %5 %6
call make_uname dll10 %2 %3 %4 %5 %6
call make_uname dll10_64 %2 %3 %4 %5 %6
call make_uname dllw4 %2 %3 %4 %5 %6
call make_uname dllg4 %2 %3 %4 %5 %6
goto fim

:fim

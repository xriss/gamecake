@echo off  

if "%1"==""     goto iupexe32
if "%1"=="vc8"  goto iupexe32
if "%1"=="vc8_64"  goto iupexe64
if "%1"=="all"  goto iupexe
goto end

REM Must use dll8 so USE_* will use the correct TEC_UNAME

:iupexe32
call tecmake dll8 relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
if not defined TECGRAF_INTERNAL goto end
call tecmake vc8 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
goto end

:iupexe64
call tecmake dll8_64 relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8_64 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
if not defined TECGRAF_INTERNAL goto end
call tecmake vc8_64 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
goto end

:iupexe
call tecmake dll8 relink %2 %3 %4 %5 %6 %7
call tecmake dll8_64 relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8_64 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
if not defined TECGRAF_INTERNAL goto end
call tecmake vc8 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
call tecmake vc8_64 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
goto end

:end

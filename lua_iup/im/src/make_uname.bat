@echo off
REM This builds all the libraries of the folder for 1 uname  

call tecmake %1 %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=im_process" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=im_jp2" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=im_avi" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=im_fftw" %2 %3 %4 %5 %6 %7 %8

call tecmake %1 "MF=imlua5" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=imlua_process5" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=imlua_jp2" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=imlua_avi" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=imlua_fftw5" %2 %3 %4 %5 %6 %7 %8

REM WMV and Capture are NOT available in some compilers,
REM so this may result in errors, just ignore them.
call tecmake %1 "MF=im_wmv" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=im_capture" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=imlua_wmv" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=imlua_capture5" %2 %3 %4 %5 %6 %7 %8

if defined TECGRAF_INTERNAL goto tec_internal
goto end

:tec_internal
call tecmake %1 "MF=imlua3" %2 %3 %4 %5 %6 %7 %8

:end

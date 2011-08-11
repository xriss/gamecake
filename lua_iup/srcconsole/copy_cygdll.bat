@echo off
mkdir ..\bin\cygw17
copy /y ..\lib\cygw17\*.dll ..\bin\cygw17
copy /y ..\..\cd\lib\cygw17\*.dll ..\bin\cygw17
copy /y ..\..\im\lib\cygw17\*.dll ..\bin\cygw17
copy /y ..\..\lua5.1\lib\cygw17\*.dll ..\bin\cygw17
copy /y ..\..\luagl\lib\cygw17\*.dll ..\bin\cygw17
copy /y ..\..\lfs\lib\cygw17\*.dll ..\bin\cygw17
del ..\bin\cygw17\*3.dll

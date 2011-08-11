@echo off  
copy /y ..\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\cd\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\im\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\lua5.1\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\luagl\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\lfs\lib\dll8\*.dll ..\bin\Win32
mkdir ..\bin\Win32\Microsoft.VC80.CRT
copy /y ..\..\lua5.1\bin\Win32\Microsoft.VC80.CRT ..\bin\Win32\Microsoft.VC80.CRT\
del ..\bin\Win32\*3.dll

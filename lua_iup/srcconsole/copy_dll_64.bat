@echo off  
copy /y ..\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\cd\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\im\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\lua5.1\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\luagl\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\lfs\lib\dll8_64\*.dll ..\bin\Win64
mkdir ..\bin\Win64\Microsoft.VC80.CRT
copy /y ..\..\lua5.1\bin\Win64\Microsoft.VC80.CRT ..\bin\Win64\Microsoft.VC80.CRT\
del ..\bin\Win64\*3.dll

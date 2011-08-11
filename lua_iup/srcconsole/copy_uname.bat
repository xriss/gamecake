@echo off  

copy /y ..\lib\%1\*.dylib ..\bin\%1
copy /y ..\..\cd\lib\%1\*.dylib ..\bin\%1
copy /y ..\..\im\lib\%1\*.dylib ..\bin\%1
copy /y ..\..\lua5.1\lib\%1\*.dylib ..\bin\%1
copy /y ..\..\luagl\lib\%1\*.dylib ..\bin\%1
copy /y ..\..\lfs\lib\%1\*.dylib ..\bin\%1

copy /y ..\lib\%1\*.so ..\bin\%1
copy /y ..\..\cd\lib\%1\*.so ..\bin\%1
copy /y ..\..\im\lib\%1\*.so ..\bin\%1
copy /y ..\..\lua5.1\lib\%1\*.so ..\bin\%1
copy /y ..\..\luagl\lib\%1\*.so ..\bin\%1
copy /y ..\..\lfs\lib\%1\*.so ..\bin\%1

copy /y ..\lib\%1\*.dll ..\bin\%1
copy /y ..\..\cd\lib\%1\*.dll ..\bin\%1
copy /y ..\..\im\lib\%1\*.dll ..\bin\%1
copy /y ..\..\lua5.1\lib\%1\*.dll ..\bin\%1
copy /y ..\..\luagl\lib\%1\*.dll ..\bin\%1
copy /y ..\..\lfs\lib\%1\*.dll ..\bin\%1

del ..\bin\%1\*3.dylib ..\bin\%1\*3.so ..\bin\%1\*3.dll

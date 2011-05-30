@set LUA_PATH_OLD=%LUA_PATH%
@set LUA_PATH=src\?.lua;tests\?.lua

@if not "%LUA51%"=="" goto LUADIR_OK
@REM LuaBinaries:
@if exist "C:\Program Files\Lua5.1\lua5.1.exe" set LUA51=C:\Progra~1\Lua5.1

@REM Lua for Windows:
@if exist "C:\Program Files\Lua\5.1\lua.exe" set LUA51=C:\Progra~1\Lua\5.1

@if "%LUA51%"=="" goto ERR_NOLUA
:LUADIR_OK

@if "%1"=="" goto BUILD
@if "%1"=="clean" goto CLEAN
@if "%1"=="test" goto TEST
@if "%1"=="launchtest" goto LAUNCHTEST
@if "%1"=="perftest" goto PERFTEST
@if "%1"=="perftest-plain" goto PERFTEST-PLAIN
@if "%1"=="stress" goto STRESS
@if "%1"=="basic" goto BASIC
@if "%1"=="fifo" goto FIFO
@if "%1"=="keeper" goto KEEPER
@if "%1"=="atomic" goto ATOMIC
@if "%1"=="cyclic" goto CYCLIC
@if "%1"=="timer" goto TIMER
@if "%1"=="recursive" goto RECURSIVE
@if "%1"=="fibonacci" goto FIBONACCI

@echo Unknown target: %1
@echo.
@goto EXIT

:BUILD
@REM
@REM Precompile src/.lua -> .lch
@REM 
@REM Note: we cannot use piping in Windows since we need binary output.
@REM 
"%LUA51%\luac5.1" -o delme src/keeper.lua
"%LUA51%\lua5.1" tools/bin2c.lua -o src/keeper.lch delme
@del delme

@if not "%VCINSTALLDIR%"=="" goto VC

@goto ERR_NOVC
;@REM
;@REM Win32 (MinGW) build commands
;@REM
;gcc -Wall -O2 -I "%LUA51%\include" -L "%LUA51%" -llua5.1 -shared -o lua51-lanes.dll src\*.c
;@goto EXIT

:VC
@REM
@REM Win32 (Visual C++ 2005/08 Express) build commands
@REM
@REM MS itself has warnings in stdlib.h (4255), winbase.h (4668), several (4820, 4826)
@REM 4054: "type cast from function pointer to data pointer"
@REM 4127: "conditional expression is constant"
@REM 4711: ".. selected for automatic inline expansion"
@REM
@set WARN=/Wall /wd4054 /wd4127 /wd4255 /wd4668 /wd4711 /wd4820 /wd4826

@REM /LDd "creates a debug DLL"
@REM
@set NODEF=/link /NODEFAULTLIB:libcmt
cl %WARN% /O2 /I "%LUA51%\include" /LDd /Felua51-lanes.dll src\*.c "%LUA51%\lua5.1.lib" %NODEF%

@del lua51-lanes.lib
@del lua51-lanes.exp
@set NODEF=
@set WARN=
@goto EXIT

:CLEAN
if exist *.dll del *.dll
if exist delme del delme
@goto EXIT

:TEST
@REM "make test" does not automatically build/update the dll. We're NOT a makefile. :!
@REM
"%LUA51%\lua5.1" tests\basic.lua
@IF errorlevel 1 goto EXIT

"%LUA51%\lua5.1" tests\fifo.lua
@IF errorlevel 1 goto EXIT

"%LUA51%\lua5.1" tests\keeper.lua
@IF errorlevel 1 goto EXIT

"%LUA51%\lua5.1" tests\fibonacci.lua
@IF errorlevel 1 goto EXIT

"%LUA51%\lua5.1" tests\timer.lua
@IF errorlevel 1 goto EXIT

"%LUA51%\lua5.1" tests\atomic.lua
@IF errorlevel 1 goto EXIT

"%LUA51%\lua5.1" tests\cyclic.lua
@IF errorlevel 1 goto EXIT

"%LUA51%\lua5.1" tests\recursive.lua
@IF errorlevel 1 goto EXIT

@goto EXIT

:BASIC
"%LUA51%\lua5.1" tests\basic.lua
@goto EXIT

:FIFO
"%LUA51%\lua5.1" tests\fifo.lua
@goto EXIT

:KEEPER
"%LUA51%\lua5.1" tests\keeper.lua
@goto EXIT

:ATOMIC
"%LUA51%\lua5.1" tests\atomic.lua
@goto EXIT

:CYCLIC
"%LUA51%\lua5.1" tests\cyclic.lua
@goto EXIT

:TIMER
"%LUA51%\lua5.1" tests\timer.lua
@goto EXIT

:RECURSIVE
"%LUA51%\lua5.1" tests\recursive.lua
@goto EXIT

:FIBONACCI
"%LUA51%\lua5.1" tests\fibonacci.lua
@goto EXIT

REM 'timeit' does not handle executables in space-containing path. So WHY
REM does MS need to make all their default paths HAVE spaces? :) :) :)
REM (even if using quotes - I tried...)

:LAUNCHTEST
timeit %LUA51%\lua5.1 tests\launchtest.lua %2 %3 %4
@goto EXIT

:PERFTEST
timeit %LUA51%\lua5.1 tests\perftest.lua %2 %3 %4
@goto EXIT

:PERFTEST-PLAIN
timeit %LUA51%\lua5.1 tests\perftest.lua --plain %2 %3 %4
@goto EXIT

:STRESS
"%LUA51%\lua5.1" tests\test.lua
"%LUA51%\lua5.1" tests\perftest.lua 100
"%LUA51%\lua5.1" tests\perftest.lua 50 -prio=-1,0
"%LUA51%\lua5.1" tests\perftest.lua 50 -prio=0,-1
"%LUA51%\lua5.1" tests\perftest.lua 50 -prio=0,2
"%LUA51%\lua5.1" tests\perftest.lua 50 -prio=2,0

@echo All seems okay!
@goto EXIT

REM ---
:ERR_NOLUA
@echo ***
@echo *** Please set LUA51 to point to LuaBinaries directory:
@echo ***
@echo *** http://luabinaries.luaforge.net/download.html
@echo ***	lua5_1_2_Win32_dll8_lib
@echo ***	lua5_1_2_Win32_bin
@echo ***
@echo *** Extracting to C:\Program Files\Lua5.1 will be autodetected.
@echo ***
@echo *** Also Lua for Windows is supported:
@echo ***
@echo *** http://luaforge.net/frs/?group_id=377&release_id=1138
@echo ***
@echo.
@goto EXIT

:ERR_NOVC
@echo ***
@echo *** VCINSTALLDIR not defined; please run 'setup-vc'
@echo ***
@echo.
@goto EXIT

:EXIT
@set LUA_PATH=%LUA_PATH_OLD%
@set LUA_PATH_OLD=

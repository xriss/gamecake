LUA_PKG = 'lua'                     -- lua pkg-config file
PKG     = 'pkg-config'              -- pkg-config program
EXT     = 'so'                      -- shared library extension
CC      = 'gcc'                     -- C compiler
PWD     = 'pwd'                     -- current dir
RM      = 'rm -f'                   -- file removal
SHARED  = '/share/lua/5.1/lgob'     -- Lua shared dir
LIB     = '/lib/lua/5.1/lgob'       -- Lua lib dir
INST    = 'install -Dm644'          -- install a common file
INSTD   = 'install -dm755'          -- create a dir
SED     = 'sed'                     -- sed
CHMOD   = 'chmod +x'                -- give executing permission

-- compiler flags
fpic    = AMD64 and '-fPIC -DAMD64'   or '-fpic'
opt     = DEBUG and '-g -O0 -DIDEBUG' or '-Os'
COMPILE_FLAGS = es'$opt -Wall -shared $fpic'

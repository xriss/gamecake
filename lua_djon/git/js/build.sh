#!/bin/sh
cd `dirname $0`

echo " this will build djon_core.wasm using emscripten "

emcc \
-Wl,--no-entry \
-Wl,--export-table \
\
-Oz --profiling-funcs \
\
-Inode_modules/node-api-headers/include \
\
-s EXPORTED_FUNCTIONS=_napi_register_wasm_v1,_napi_wasm_malloc \
-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
\
-o ./djon_core.wasm \
\
./djon_core.c \


# and base64 it because apparently that is the only safe way to embed wasm
./djon_core.cmd.js


#-s TOTAL_STACK=16MB \
#-s INITIAL_MEMORY=256MB \

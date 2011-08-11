MODULE = 'loader'
init()

mod = {
    name = 'loader',
    src  = 'loader.c'
}

compile(mod)
sh( es'$INST src/loader.lua $DEST/$SHARED/loader.lua' )
sh( es'$INST src/loader.$EXT $DEST/$LIB/_loader.$EXT' )
clean  (mod)

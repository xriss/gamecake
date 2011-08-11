MODULE = 'common'
init()

mod = {
    name = 'common',
    src  = 'interface.c'
}

compile(mod)
install(mod, LIB)
clean  (mod)

sh( es'$INST src/types.h $DEST/include/lgob/common/types.h' )

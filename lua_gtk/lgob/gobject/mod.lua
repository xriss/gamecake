MODULE = 'gobject'
init()

mod = {
    name = 'gobject',
    pkg  = 'gobject-2.0'
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

sh( es'$INST src/types.h $DEST/include/lgob/gobject/types.h' )

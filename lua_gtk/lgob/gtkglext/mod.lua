MODULE = 'gtkglext'
init()

mod = {
    name = 'gtkglext',
    src  = 'gtkglext.c',
    pkg  = 'gtkglext-1.0'
}

compile(mod)
install(mod, LIB)
clean  (mod)

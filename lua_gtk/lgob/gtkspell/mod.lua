MODULE = 'gtkspell'
init()

mod = {
    name = 'gtkspell',
    pkg  = 'gtkspell-2.0',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

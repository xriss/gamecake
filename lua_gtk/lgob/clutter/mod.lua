MODULE = 'clutter'
init()

mod = {
    name = 'clutter',
    pkg  = 'clutter-1.0',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

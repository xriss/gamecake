MODULE = 'gtkextra'
init()

mod = {
    name = 'gtkextra',
    pkg  = 'gtkextra-3.0',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

MODULE = 'pangocairo'
init()

mod = {
    name = 'pangocairo',
    pkg  = 'pangocairo',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

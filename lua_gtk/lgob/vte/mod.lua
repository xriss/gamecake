MODULE = 'vte'
init()

mod = {
    name = 'vte',
    pkg  = 'vte',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

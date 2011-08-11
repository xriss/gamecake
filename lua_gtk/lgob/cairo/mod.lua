MODULE = 'cairo'
init()

mod = {
    name = 'cairo',
    pkg  = 'cairo',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

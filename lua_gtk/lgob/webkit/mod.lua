MODULE = 'webkit'
init()

mod = {
    name = 'webkit',
    pkg  = 'webkit-1.0',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

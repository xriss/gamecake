MODULE = 'atk'
init()

mod = {
    name = 'atk',
    pkg  = 'atk',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

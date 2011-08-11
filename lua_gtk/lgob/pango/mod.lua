MODULE = 'pango'
init()

mod = {
    name = 'pango',
    pkg  = 'pango',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

MODULE = 'gtk'
init()

mod = {
    name = 'gtk',
    pkg  = 'gtk+-2.0',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

MODULE = 'poppler'
init()

mod = {
    name = 'poppler',
    pkg  = 'poppler-glib',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

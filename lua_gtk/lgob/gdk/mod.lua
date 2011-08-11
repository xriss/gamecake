MODULE = 'gdk'
init()

mod = {
    name = 'gdk',
    pkg  = 'gdk-2.0',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

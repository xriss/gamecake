MODULE = 'goocanvas'
init()

mod = {
    name = 'goocanvas',
    pkg  = 'goocanvas',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

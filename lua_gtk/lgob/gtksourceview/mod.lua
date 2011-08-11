MODULE = 'gtksourceview'
init()

mod = {
    name = 'gtksourceview',
    pkg  = 'gtksourceview-2.0',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

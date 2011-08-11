MODULE = 'cluttergtk'
init()

mod = {
    name = 'cluttergtk',
    pkg  = 'clutter-gtk-0.10',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

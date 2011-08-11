MODULE = 'ieembed'
init()

mod = {
    name = 'ieembed',
    pkg  = 'gtkieembed',
}

gen_iface(mod)
compile  (mod)
install  (mod, LIB)
clean    (mod)

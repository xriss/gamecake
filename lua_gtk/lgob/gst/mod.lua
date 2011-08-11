MODULE = 'gst'
init()

mod = {
    name = 'gst',
    src  = 'interface.c',
    pkg  = 'gstreamer-interfaces-0.10'
}

compile(mod)
install(mod, LIB)
clean  (mod)

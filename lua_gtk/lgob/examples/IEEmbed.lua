#! /usr/bin/env lua

-- Until WebKitGtk works on windows, this is as far as we can get!
require("lgob.ieembed")

local window = gtk.Window.new()
local webview = ieembed.WebView.new()

window:add(webview)
window:connect("delete-event", gtk.main_quit)
window:set("window-position", gtk.WIN_POS_CENTER, "title", "GtkIEEmbed example")
window:maximize()
window:show_all()

webview:load_url("http://oproj.tuxfamily.org")
gtk.main()

#! /usr/bin/env lua

-- About dialog

require("lgob.gtk")
require("lgob.gdk")

-- Create the widgets
local window = gtk.Window.new()
local dialog = gtk.AboutDialog.new()
local button = gtk.Button.new_from_stock("gtk-about")
local logo   = gdk.Pixbuf.new_from_file("icon.png")

function showAbout()
	dialog:run()
	dialog:hide()
end

function hook(data, uri)
	gtk.show_uri(nil, (data or "") .. uri, gdk.CURRENT_TIME)
end

-- Set the hooks before the website
gtk.about_dialog_set_email_hook(hook, "mailto:")
gtk.about_dialog_set_url_hook(hook)

dialog:set("program-name", "AboutDialog demo", "authors", {"Lucas Hermann Negri <kkndrox@gmail.com>"},
"comments", "No comments!", "license", "LGPL 3+", "logo", logo, "title", "About...",
"website", "http://oproj.tuxfamily.org")

window:add(button)
window:set("title", "About Dialog", "window-position", gtk.WIN_POS_CENTER)

window:connect("delete-event", gtk.main_quit)
button:connect("clicked", showAbout)

window:show_all()
gtk.main()

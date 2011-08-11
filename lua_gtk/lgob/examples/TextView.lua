#! /usr/bin/env lua

-- TextView example

require("lgob.gtk")
require("lgob.gdk")
require("lgob.pango")

local window 	= gtk.Window.new()
local hbox	 	= gtk.HBox.new(false, 5)
local vbox 		= gtk.VBox.new(true, 5)
local sw		= gtk.ScrolledWindow.new()
local view		= gtk.TextView.new()
local buffer 	= view:get("buffer")
local b1, b2, b3 = gtk.Button.new_with_mnemonic("Insert _Text"),
gtk.Button.new_with_mnemonic("Insert _Pixbuf"), gtk.Button.new_with_mnemonic("Insert _With Tags")
local iter		= gtk.TextIter.new()
local buf		= gdk.Pixbuf.new_from_file("icon.png")

local tag1, tag2 = buffer:create_tag("MyTag1"), buffer:create_tag("MyTag2")

tag1:set("foreground", "red")
tag2:set("font", "Sans Bold", "size", 20 * pango.SCALE)

function insertText()
	buffer:get_end_iter(iter)
	buffer:insert(iter, "Hello World!\n", -1)
end

function insertPixbuf()
	buffer:get_end_iter(iter)
	buffer:insert_pixbuf(iter, buf)
end

function insertTags()
	buffer:get_end_iter(iter)
	buffer:insert_with_tags(iter, "Hello With Tags!\n", -1, tag1, tag2)
end

sw:add(view)
hbox:pack_start(sw, true, true, 0)
hbox:pack_start(vbox, false, false, 0)
vbox:pack_start(b1, true, true, 0)
vbox:pack_start(b2, true, true, 0)
vbox:pack_start(b3, true, true, 0)
window:add(hbox)

window:set("title", "Hello TextView", "window-position", gtk.WIN_POS_CENTER,
"width-request", 500, "height-request", 320)

b1:connect("clicked", insertText)
b2:connect("clicked", insertPixbuf)
b3:connect("clicked", insertTags)

window:connect("delete-event", gtk.main_quit)
window:show_all()

gtk.main()

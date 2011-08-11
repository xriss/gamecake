#! /usr/bin/env lua

-- ComboBox with custom data.

require("lgob.gtk")

local window = gtk.Window.new()
local model  = gtk.ListStore.new("gchararray", "gboolean")
local iter	 = gtk.TreeIter.new()
local combo  = gtk.ComboBox.new_with_model(model)
local r1, r2 = gtk.CellRendererPixbuf.new(), gtk.CellRendererText.new()

local data = {
	"gtk-ok", "gtk-about", "gtk-info", "gtk-cut", "gtk-help"
}

for i, j in ipairs(data) do
	model:append(iter)
	model:seto(iter, j, i % 2 == 1)
end

combo:pack_start(r1, false)
combo:pack_start(r2, false)

-- Magic
function callback(rend, iter)
	local text = rend:get("text")
	rend:set("text", text .. " !! ")
end

combo:set_cell_data_func(r2, callback, r2)

combo:add_attribute(r1, "stock-id", 0, "sensitive", 1)
combo:add_attribute(r2, "text", 0, "sensitive", 1)

combo:set("active", 0)

window:add(combo)
window:set("title", "Hello ComboBox", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()

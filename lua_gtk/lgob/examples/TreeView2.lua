#! /usr/bin/env lua

require("lgob.gtk")

local window = gtk.Window.new()
local view	 = gtk.TreeView.new()
local model	 = gtk.TreeStore.new("gchararray")
local col1	 = gtk.TreeViewColumn.new_with_attributes("Languages", gtk.CellRendererText.new(), "text", 0)
local iter	 = gtk.TreeIter.new()
local dad    = gtk.TreeIter.new()
local selection = view:get_selection()

function selection_changed()
	local res, m = selection:get_selected(iter)

	if res then
		print(m:get(iter, 0))
	end
end

-- Top level
model:append(iter, nil)
model:set(iter, 0, "Low-level")
model:append(iter, nil)
model:set(iter, 0, "High-level")

-- Low level Children
model:get_iter_from_string(dad, "0")
model:append(iter, dad)
model:set(iter, 0, "C")
model:append(iter, dad)
model:set(iter, 0, "C++")

-- High level Children
model:get_iter_from_string(dad, "1")
model:append(iter, dad)
model:set(iter, 0, "Lua")
model:append(iter, dad)
model:set(iter, 0, "Scala")
model:append(iter, dad)
model:set(iter, 0, "Haskell")

view:append_column(col1)
view:set("model", model)

selection:set_mode(gtk.SELECTION_SINGLE)
selection:connect("changed", selection_changed)

window:add(view)
window:set("window-position", gtk.WIN_POS_CENTER, "title", "TreeView demo",
	"resizable", false)
window:connect("delete-event", gtk.main_quit)
window:show_all()

gtk.main()

#! /usr/bin/env lua

--[[
	Another TreeView example, now using set_cell_data_func!
--]]

require("lgob.gdk")
require("lgob.gtk")

local window = gtk.Window.new()
local view	 = gtk.TreeView.new()
local model	 = gtk.ListStore.new("gdouble")
local rend1  = gtk.CellRendererText.new()
local col1	 = gtk.TreeViewColumn.new_with_attributes("Value", rend1, "text", 0)
local iter	 = gtk.TreeIter.new()
local sel	 = view:get_selection()
local i = 0

-- Some magic
function callback(data, iter)
	local val = data[1]:get(iter, 0)
	local back
	
	if val < 5 then
		back = "#CC4444"
	else
		back = "#44CC44"
	end
	
	data[2]:set("background", back)	
end

col1:set_cell_data_func(rend1, callback, {model, rend1})

for i = 1, 10 do
	model:append(iter)
	model:seto(iter, i)
end

view:append_column(col1)
view:set("model", model)

window:add(view)
window:set("window-position", gtk.WIN_POS_CENTER, "title", "TreeView demo")
window:connect("delete-event", gtk.main_quit)
window:show_all()

gtk.main()

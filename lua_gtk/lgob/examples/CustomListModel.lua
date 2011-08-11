#! /usr/bin/env lua

--[[
	Demonstrates the use of ProxyList to implement a custom list GtkTreeModel
	in Lua.
--]]

require("lgob.gtk")

-- Create the "proxy" table
local MyProxy = {
}

function MyProxy.new()
	local self = {}
	self.cols = {[0] = {}, [1] = {}}
	setmetatable(self, {__index = MyProxy})
	
	return self
end

function MyProxy:iter_next(row)
	if row < 100 then
		return true
	end
end

function MyProxy:get_value(row, col)
	if self.cols[col][row] then
		return self.cols[col][row]
	end
	
	if col == 0 then
		return 100 - row
	else
		return "Row " .. row
	end
end

function MyProxy:set_value(row, col, value)
	self.cols[col][row] = value
end

-- Create the GUI

local window = gtk.Window.new()
local scroll = gtk.ScrolledWindow.new()
local proxy = MyProxy.new()
local model = gtk.ProxyList.new(proxy, "gint", "gchararray")
local view = gtk.TreeView.new_with_model(model)
window:connect("delete-event", gtk.main_quit)

local rend1 = gtk.CellRendererProgress.new()
local rend2 = gtk.CellRendererText.new()
local col1	 = gtk.TreeViewColumn.new_with_attributes("Col 1", rend1, "value", 0)
local col2	 = gtk.TreeViewColumn.new_with_attributes("Col 2", rend2, "text", 1)
col1:set("expand", true)
col2:set("expand", true)
view:append_column(col1)
view:append_column(col2)

local iter = gtk.TreeIter.new()

function rend2Edited(data, path, new)
	model:get_iter_from_string(iter, path)
	model:set_value(iter, 1, new)
end

rend2:set("editable", true)
rend2:connect("edited", rend2Edited)

window:set("window-position", gtk.WIN_POS_CENTER, "title", "Custom ListModel demo",
	"width-request", 400, "height-request", 500)
window:connect("delete-event", gtk.main_quit)
scroll:add(view)
window:add(scroll)
window:show_all()

gtk.main()

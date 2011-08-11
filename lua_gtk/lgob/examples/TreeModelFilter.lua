#! /usr/bin/env lua

--[[
	TreeModelFilter example.
	To enhance the performance, its a good idea to only refilter when the user
	stops typing (waiting, for example, one second after the last type).
	
	The filter operation requires N calls to the visible function, where N is
	the number of the rows of the model.
	
	This example shows how GtkEntryCompletion works too.
--]]

require("lgob.gtk")

local window = gtk.Window.new()
local entry  = gtk.Entry.new()
local comp	 = gtk.EntryCompletion.new()
local label  = gtk.Label.new("Name:")
local hbox   = gtk.HBox.new(false, 5)
local vbox   = gtk.VBox.new(false, 5)
local view	 = gtk.TreeView.new()
local model	 = gtk.ListStore.new("gchararray", "gboolean")
local col1	 = gtk.TreeViewColumn.new_with_attributes("Name", gtk.CellRendererText.new(), "text", 0)
local col2   = gtk.TreeViewColumn.new_with_attributes("Rocks?", gtk.CellRendererToggle.new(), "active", 1)
local iter	 = gtk.TreeIter.new()

-- Configure the entry completion
comp:set_model(model)
comp:set_text_column(0)
entry:set_completion(comp)

-- The filter callback. Return true to show, false to hide the row.
function filter(data, iter)
	local str = gtk.TreeModel.get(data, iter, 0)
	local wanted = entry:get("text")
	return str:find(wanted)
end

-- Append the values to the list
local values = {
	{name = "Banana", flag = false},
	{name = "Green Apple", flag = false},
	{name = "Apple",  flag = true},
	{name = "Apple Pie",  flag = true},
	{name = "Apple aliens", flag = false},
	{name = "Apple insects", flag = false},
	{name = "Data structures", flag = true},
	{name = "Data consumers", flag = false},
	{name = "Linux", flag = true}
}

for i, j in ipairs(values) do
	model:append(iter)
	model:set(iter, 0, j.name, 1, j.flag)
end

local filterModel = gtk.TreeModelFilter.new(model)
filterModel:set_visible_func(filter, model)

-- Do the control
local counter = 0

function search()
	counter = counter + 1
	glib.timeout_add(glib.PRIORITY_DEFAULT, 1000, doSearch)
	return false
end

function doSearch()
	counter = counter - 1
	
	if counter == 0 then
		filterModel:refilter()
	end
end

view:append_column(col1)
view:append_column(col2)
view:set("model", filterModel)

hbox:add(label)
hbox:add(entry)
vbox:pack_start(hbox, false, false, 0)
vbox:pack_start(view, true, true, 0)

-- When the content of the entry change, refilter the model
entry:connect("changed", search)
window:add(vbox)
window:set("window-position", gtk.WIN_POS_CENTER, "title", "TreeModelFilter demo",
"width-request", 300)
window:connect("delete-event", gtk.main_quit)
window:show_all()

gtk.main()

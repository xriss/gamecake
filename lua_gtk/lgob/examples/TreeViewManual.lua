#! /usr/bin/env lua

--[[
	TreeView example. Can be edited and have its rows swapped by drag'n'drop!
	
	Note: If you wan't to just reorder the rows, you can set the property
	"reorderable" of the TreeView (see TreeView example).
--]]

require('lgob.gdk')
require('lgob.gtk')

local window = gtk.Window.new()
local view	 = gtk.TreeView.new()
local model	 = gtk.ListStore.new('gchararray', 'gboolean')
local rend1  = gtk.CellRendererText.new()
local rend2  = gtk.CellRendererToggle.new()
local col1	 = gtk.TreeViewColumn.new_with_attributes("Name", rend1, 'text', 0)
local col2   = gtk.TreeViewColumn.new_with_attributes("Is red?", rend2, 'active', 1)
local iter	 = gtk.TreeIter.new()
local iter2	 = gtk.TreeIter.new()
local sel	 = view:get_selection()

-- Called when the user confirms an edit.
function rend1_edited(data, path, new)
	model:get_iter_from_string(iter, path)
	model:set(iter, 0, new)
end

-- Called when the checkbutton is toggled.
function rend2_toggled(data, path)
	model:get_iter_from_string(iter, path)
	local current = model:get(iter, 1)
	model:set(iter, 1, not current)
end

col1:set_reorderable(true)
col1:set_sort_column_id(0)
col2:set_reorderable(true)
col2:set_sort_column_id(1)
rend1:set('editable', true)
rend1:connect('edited', rend1_edited)
rend2:connect('toggled', rend2_toggled)

local values = {
	{name = "Pineapple", flag = false},
	{name = "Grape",     flag = false},
	{name = "Banana",    flag = false},
	{name = "Apple",     flag = true },
	{name = "Orange",    flag = false},
}

for i, j in ipairs(values) do
	model:append(iter)
	model:set(iter, 0, j.name, 1, j.flag)
end

function motion(data, context, x, y, time)
	gdk.drag_status(context, gdk.ACTION_COPY, time)
	return true
end

function drop(data, context, x, y, time)
	gtk.drag_finish(context, true, false, time)
	sel:get_selected(iter)

	local res, path = view:get_path_at_pos(x, y)
	
	if res then
		path = path - 1
		local res, id = model:get_sort_column_id()
		
		if id == gtk.TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID then
			model:get_iter_from_string(iter2, path)
			model:swap(iter, iter2)
		else
			print("Can't do that on a sorted model!")
		end
	end
	
	return true
end

-- Enable internal drag'n'drop
gtk.drag_source_set(view, gdk.BUTTON1_MASK)
gtk.drag_dest_set(view, 0)
gtk.drag_source_set_icon_stock(view, 'gtk-copy')
view:connect('drag-motion', motion)
view:connect('drag-drop', drop)
view:set('reorderable', true)

view:append_column(col1)
view:append_column(col2)
view:set('model', model)

window:add(view)
window:set('window-position', gtk.WIN_POS_CENTER, 'title', "TreeView demo")
window:connect('delete-event', gtk.main_quit)
window:show_all()

gtk.main()

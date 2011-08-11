#! /usr/bin/env lua

-- FileDialog demo

require("lgob.gtk")

function runDialog(dialog)
	dialog:run()
	dialog:hide()
	
	local names = dialog:get_filenames()
	table.foreach(names, print)
end

local window = gtk.Window.new()
local button = gtk.Button.new_with_mnemonic("_Show Dialog")
local dialog = gtk.FileChooserDialog.new("Select the file", window, gtk.FILE_CHOOSER_ACTION_OPEN,
	"gtk-cancel", gtk.RESPONSE_CANCEL, "gtk-ok", gtk.RESPONSE_OK)

filter = gtk.FileFilter.new()
filter:add_pattern("*.lua")
filter:set_name("Lua scripts")
dialog:add_filter(filter)
dialog:set("select-multiple", true)
window:connect("delete-event", gtk.main_quit)
button:connect("clicked", runDialog, dialog)

window:set("title", "FileDialog demo", "window-position", gtk.WIN_POS_CENTER)
window:add(button)
window:show_all()

gtk.main()

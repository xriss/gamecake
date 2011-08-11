#! /usr/bin/env lua

-- Font / Color dialog selection demo. Plus, somo runtime style changes

require("lgob.gtk")
require("lgob.pango")

-- Create the widgets
local window = gtk.Window.new()
local vbox   = gtk.VBox.new(true, 5)
local hbox   = gtk.HBox.new(true, 5)
local label  = gtk.Label.new("This is a sample text! Can the widgets style be changed?")
local bFont  = gtk.Button.new_with_mnemonic("Change _Font")
local bColor = gtk.Button.new_with_mnemonic("Change _Background")
local bReset = gtk.Button.new_with_mnemonic("_Reset")

-- Font dialog
local fontD  = gtk.FontSelectionDialog.new("Font selection")

-- Color dialog
local colorD = gtk.ColorSelectionDialog.new("Color selection")
local colorS = colorD:get_color_selection()

-- Select font callback
function selectFont()
	local res = fontD:run()
	fontD:hide()	
	
	if res == gtk.RESPONSE_OK then
		local desc = fontD:get_font_name()	
		label:modify_font(pango.FontDescription.from_string(desc))
	end
end

-- Select background callback
function selectBackground()
	local res = colorD:run()
	colorD:hide()
	
	if res == gtk.RESPONSE_OK then
		local color = colorS:get("current-color")
		window:modify_bg(gtk.STATE_NORMAL, color)
	end
end

-- Reset callback
function reset()
	label:modify_font()
	window:modify_bg(gtk.STATE_NORMAL)
	window:resize(1, 1)
end

-- Connect the callbacks
bFont:connect("clicked", selectFont)
bColor:connect("clicked", selectBackground)
bReset:connect("clicked", reset)
window:connect("delete-event", gtk.main_quit)

-- Add the widgets
hbox:add(bFont, bColor, bReset)
vbox:add(label, hbox)
window:add(vbox)

-- Configure and show the window
window:set("title", "Style demo", "window-position", gtk.WIN_POS_CENTER)
window:show_all()

gtk.main()

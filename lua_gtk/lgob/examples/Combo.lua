#! /usr/bin/env lua

-- Simple ComboBox

require("lgob.gtk")

local window = gtk.Window.new()
local combo  = gtk.ComboBox.new_text()

local data = {
	"Orange", "Lemon", "Banana", "Grape", "Apple"
}

for i, j in ipairs(data) do
	combo:append_text(j)
end

combo:set("active", 0)

window:add(combo)
window:set("title", "Hello ComboBox", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()

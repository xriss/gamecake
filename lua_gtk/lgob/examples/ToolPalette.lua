#! /usr/bin/env lua
require('lgob.gtk')

local win       = gtk.Window.new()
local palette   = gtk.ToolPalette.new()
local group1    = gtk.ToolItemGroup.new("Group 1")
local group2    = gtk.ToolItemGroup.new("Group 2")

local function callback(i)
    print('Item ' .. i .. ' clicked')
end

for i = 1, 12 do
    local item = gtk.ToolButton.new(nil, "Item " .. i)
    item:connect('clicked', callback, i)
    if i <= 6 then group1:add(item) else group2:add(item) end
end

palette:add(group1, group2)
win:add(palette)

win:set('title', "Tool palette", 'window-position', gtk.WIN_POS_CENTER,
    'default-width', 150, 'default-height', 250)
win:connect('delete-event', gtk.main_quit)

win:show_all()
gtk.main()

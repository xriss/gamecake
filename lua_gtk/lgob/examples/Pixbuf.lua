#! /usr/bin/env lua

require('lgob.gdk')
require('lgob.gtk')

-- just to test serialization
local pixbuf1       = gdk.Pixbuf.new_from_file('tux.png')
local r, buf, size  = pixbuf1:save_to_buffer('jpeg', {quality = 100})
local loader        = gdk.PixbufLoader.new()
loader:write(buf, size)
loader:close()
local pixbuf2       = loader:get_pixbuf()

local window        = gtk.Window.new()
local hbox          = gtk.HBox.new(true, 10)
local image1        = gtk.Image.new_from_pixbuf(pixbuf1)
local image2        = gtk.Image.new_from_pixbuf(pixbuf2)
hbox:add(image1, image2)
window:add(hbox)

window:set('title', "Pixbuf example", 'window-position', gtk.WIN_POS_CENTER)
window:connect('delete-event', gtk.main_quit)
window:show_all()

gtk.main()

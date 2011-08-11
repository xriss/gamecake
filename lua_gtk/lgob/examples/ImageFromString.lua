#! /usr/bin/env lua

require('lgob.gdk')
require('lgob.gtk')

local function load_str(path)
    local file = io.open(path)
    local str  = file:read('*a')
    file:close()
    
    return str
end

local function pixbuf_from_str(str)
    local loader = gdk.PixbufLoader.new()
    loader:write(str, #str) -- do not forget the length
    loader:close()
    
    return loader:get_pixbuf()
end

local window = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
local str    = load_str('tux.png')
local pixbuf = pixbuf_from_str(str)
local image  = gtk.Image.new_from_pixbuf(pixbuf)
window:add(image)
window:connect('delete-event', gtk.main_quit)

window:show_all()
gtk.main()

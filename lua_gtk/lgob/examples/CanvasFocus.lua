#! /usr/bin/env lua

require("lgob.goocanvas")

function on_focus_in(item, target, event)
	item:set("stroke-color", "black")
	return false
end

function on_focus_out(item, target, event)
	item:set("stroke-color", "white")
	return false
end

function on_button_press(item, target, event)
	canvas = item:get_canvas()
	canvas:grab_focus(item)
	return true
end

function create_focus_box(canvas, x, y, width, height, color)
	local root = canvas:get_root_item()
	local item = goocanvas.Rect.new(root, x, y, width, height)
	
	item:set("fill-color", color, "line-width", 5, "can-focus", true,
		"stroke-color", "white")
	item:connect ("focus_in_event", on_focus_in, item)
	item:connect ("focus_out_event", on_focus_out, item)
	item:connect ("button_press_event", on_button_press, item)
end

function setup_canvas(canvas)
	create_focus_box (canvas, 110, 80, 50, 30, "red")
	create_focus_box (canvas, 300, 160, 50, 30, "orange")
	create_focus_box (canvas, 500, 50, 50, 30, "yellow")
	create_focus_box (canvas, 70, 400, 50, 30, "blue")
	create_focus_box (canvas, 130, 200, 50, 30, "magenta")
	create_focus_box (canvas, 200, 160, 50, 30, "green")
	create_focus_box (canvas, 450, 450, 50, 30, "cyan")
	create_focus_box (canvas, 300, 350, 50, 30, "grey")
	create_focus_box (canvas, 900, 900, 50, 30, "gold")
	create_focus_box (canvas, 800, 150, 50, 30, "thistle")
	create_focus_box (canvas, 600, 800, 50, 30, "azure")
	create_focus_box (canvas, 700, 250, 50, 30, "moccasin")
	create_focus_box (canvas, 500, 100, 50, 30, "cornsilk")
	create_focus_box (canvas, 200, 750, 50, 30, "plum")
	create_focus_box (canvas, 400, 800, 50, 30, "orchid")
end

local win = gtk.Window.new()
local scroll = gtk.ScrolledWindow.new()
local canvas = goocanvas.Canvas.new()

setup_canvas(canvas)
scroll:add(canvas)

win:connect("delete-event", gtk.main_quit)
win:add(scroll)
win:set("default-height", 400, "default-width", 600, "window-position",
	gtk.WIN_POS_CENTER)
win:show_all()

gtk.main()	

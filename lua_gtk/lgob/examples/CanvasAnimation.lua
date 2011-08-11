#! /usr/bin/env lua

require("lgob.goocanvas")

local ellipse1, ellipse2
local rect1, rect2, rect3, rect4

function start_animation_clicked(button)
    ellipse1:set_simple_transform (100, 100, 1, 0)
    ellipse1:animate (500, 100, 2, 720, true,
                      2000, 40, goocanvas.ANIMATE_BOUNCE)
    
    rect1:set_simple_transform (100, 200, 1, 0)
    rect1:animate (100, 200, 1, 350, true,
                   40 * 36, 40, goocanvas.ANIMATE_RESTART)

    rect3:set_simple_transform (200, 200, 1, 0)
    rect3:animate (200, 200, 3, 0, true,
                   400, 40, goocanvas.ANIMATE_BOUNCE)

    ellipse2:set_simple_transform (100, 400, 1, 0)
    ellipse2:animate (400, 0, 2, 720, false,
                      2000, 40, goocanvas.ANIMATE_BOUNCE)

    rect2:set_simple_transform (100, 500, 1, 0)
    rect2:animate (0, 0, 1, 350, false,
                   40 * 36, 40, goocanvas.ANIMATE_RESTART)

    rect4:set_simple_transform (200, 500, 1, 0)
    rect4:animate (0, 0, 3, 0, false,
                   400, 40, goocanvas.ANIMATE_BOUNCE)
end

function stop_animation_clicked (button)
    ellipse1:stop_animation ()
    ellipse2:stop_animation ()
    rect1:stop_animation ()
    rect2:stop_animation ()
    rect3:stop_animation ()
    rect4:stop_animation ()
end

function setup_canvas (canvas)
    root = canvas:get_root_item ()

    ellipse1 = goocanvas.Ellipse.new(root, 0, 0, 25, 15)
    ellipse1:set("fill-color", "blue")
    ellipse1:translate (100, 100)

    rect1 = goocanvas.Rect.new(root, -10, -10, 20, 20)
    rect1:set("fill-color", "blue")
    rect1:translate (100, 200)

    rect3 = goocanvas.Rect.new(root, -10, -10, 20, 20)
    rect3:set("fill-color", "blue")
    rect3:translate (200, 200)
    
    ellipse2 = goocanvas.Ellipse.new(root, 0, 0, 25, 15)
    ellipse2:set("fill-color", "red")
    ellipse2:translate (100, 400)

    rect2 = goocanvas.Rect.new(root, -10, -10, 20, 20)
    rect2:set("fill-color", "red")
    rect2:translate (100, 500)

    rect4 = goocanvas.Rect.new(root, -10, -10, 20, 20)
    rect4:set("fill-color", "red")
    rect4:translate (200, 500)
end

function create_animation_page ()
    local vbox = gtk.VBox.new(false, 4)
	local hbox = gtk.HBox.new(false, 4)
    vbox:pack_start(hbox, false, false, 0)

    local w = gtk.Button.new_with_label("Start Animation")
    hbox:pack_start (w, false, false, 0)
    w:connect ("clicked", start_animation_clicked)
    
    w = gtk.Button.new_with_label("Stop Animation")
    hbox:pack_start (w, false, false, 0)
    w:connect ("clicked", stop_animation_clicked)

    local scrolled_win = gtk.ScrolledWindow.new()
    scrolled_win:set_shadow_type (gtk.SHADOW_IN)

    vbox:add (scrolled_win)

    local canvas = goocanvas.Canvas.new ()
	scrolled_win:add (canvas)
	setup_canvas (canvas)

    return vbox
end

local v = create_animation_page ()
w = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
w:connect("destroy", gtk.main_quit)   
w:add(v)
w:set("default-width", 600, "default-height", 600, "window-position",
	gtk.WIN_POS_CENTER)

w:show_all()
gtk.main()

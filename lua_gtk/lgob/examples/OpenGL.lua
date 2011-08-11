#! /usr/bin/env lua

--[[
    lgob.gtk + lgob.gtkglext + luagl : OpenGL demo

    Written by Yuri Kaszubowski Lopes <yurikazuba@gmail.com> based on (translate from)
    the C code written by Davyd Madeley <davyd@madeley.id.au>, originaly made available under
    a BSD license.

    LuaGL project: http://luagl.sourceforge.net/
--]]

require 'lgob.gdk'
require 'lgob.gtk'
require 'lgob.gtkglext'
require 'luagl'
require 'luaglu'

boxv = {
    { -0.5, -0.5, -0.5 },
    {  0.5, -0.5, -0.5 },
    {  0.5,  0.5, -0.5 },
    { -0.5,  0.5, -0.5 },
    { -0.5, -0.5,  0.5 },
    {  0.5, -0.5,  0.5 },
    {  0.5,  0.5,  0.5 },
    { -0.5,  0.5,  0.5 },
}


OpenGl    = {}
OpenGl_MT = { __index = OpenGl }


function OpenGl.new( options )
    local self = {
        angle = 30,
    }
    setmetatable( self, OpenGl_MT )

    --Structs
    self.window                    = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
        self.vbox                  = gtk.VBox.new(false, 0)
            self.menubar           = gtk.MenuBar.new()
                self.menu_1        = gtk.MenuItem.new_with_mnemonic( 'Help' )
                    self.menu_1_sm = gtk.Menu.new()
            self.drawing_area      = gtk.DrawingArea.new( )
            self.statusbar         = gtk.Statusbar.new()

    self.dialog = gtk.AboutDialog.new()

    --Actions
    self.about = gtk.Action.new( 'about', "About", nil, 'gtk-about')
    self.about:connect('activate', self.show_about, self )

    --Link Structs
    self.window:add( self.vbox )
        self.vbox:pack_start(self.menubar, false, false, 0)
            self.menubar:append( self.menu_1 )
                self.menu_1:set_submenu( self.menu_1_sm )
                    self.menu_1_sm:append( self.about:create_menu_item() )
        self.vbox:pack_start(self.drawing_area, true, true, 0)
        self.vbox:pack_start(self.statusbar, false, false, 0)


    --Properties
    self.window:set(
        'title', "Lua OpenGL: lgob.gtk, lgob.gtkglext, luagl",
        'width-request', 800,
        'height-request', 600,
        'window-position', gtk.WIN_POS_CENTER,
        'icon-name', "gtk-about"
    )

    --Objects
    self.glconfig = gtkglext.Config.new_by_mode(
        gtkglext.MODE_RGB +
        gtkglext.MODE_DOUBLE +
        gtkglext.MODE_DEPTH
    )
    gtkglext.Widget.set_gl_capability(self.drawing_area, self.glconfig, true, gtkglext.MODE_RGB)

    --CallBacks
     self.drawing_area:connect('configure-event', self.configure, self )
     self.drawing_area:connect('expose-event', self.expose, self )
     self.window:connect('delete-event', gtk.main_quit)

     --
     self.dialog:set('program-name', "Lua GTK + OpenGL demo", 'authors', {"Yuri Kaszubowski Lopes <yurikazuba@gmail.com>"},
        'comments', "Using lgob.gtk, lgob.gtkglext and luagl", 'license', "LGPL 3+", 'title', "About...",
        'website', "http://oproj.tuxfamily.org")

     self.window:show_all()

    return self
end

function OpenGl:configure( event )
    local context  = gtkglext.Widget.get_gl_context(self.drawing_area)
    local drawable = gtkglext.Widget.get_gl_drawable(self.drawing_area)

    drawable:gl_begin( context )

    gl.LoadIdentity()
    gl.Viewport  (0, 0, 800, 600);
    gl.Ortho     (-10,10,-10,10,-20050,10000);
    gl.Enable    (gl.BLEND)
    gl.BlendFunc (gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

    gl.Scale (10, 10, 10);

    drawable:gl_end()

    return true
end

function OpenGl:expose( event )
    local context  = gtkglext.Widget.get_gl_context(self.drawing_area)
    local drawable = gtkglext.Widget.get_gl_drawable(self.drawing_area)

    drawable:gl_begin( context )

    gl.Clear(gl.COLOR_BUFFER_BIT + gl.DEPTH_BUFFER_BIT)
    gl.PushMatrix()
    gl.Rotate( self.angle, 1, 0, 1)

    gl.ShadeModel(gl.FLAT)

    gl.Begin (gl.LINES)
    gl.Color (1, 0, 0)
    gl.Vertex (0, 0, 0)
    gl.Vertex (1, 0, 0)
    gl.End ()

    gl.Begin (gl.LINES)
    gl.Color (0, 1, 0)
    gl.Vertex (0, 0, 0)
    gl.Vertex (0, 1, 0)
    gl.End ()

    gl.Begin (gl.LINES)
    gl.Color (0, 0, 1)
    gl.Vertex (0, 0, 0)
    gl.Vertex (0, 0, 1)
    gl.End ()

    gl.Begin(gl.LINES)
    gl.Color (1, 1, 1)
    gl.Vertex(boxv[1])
    gl.Vertex(boxv[2])

    gl.Vertex(boxv[2])
    gl.Vertex(boxv[3])

    gl.Vertex(boxv[3])
    gl.Vertex(boxv[4])

    gl.Vertex(boxv[4])
    gl.Vertex(boxv[1])

    gl.Vertex(boxv[5])
    gl.Vertex(boxv[6])

    gl.Vertex(boxv[6])
    gl.Vertex(boxv[7])

    gl.Vertex(boxv[7])
    gl.Vertex(boxv[8])

    gl.Vertex(boxv[8])
    gl.Vertex(boxv[5])

    gl.Vertex(boxv[1])
    gl.Vertex(boxv[5])

    gl.Vertex(boxv[2])
    gl.Vertex(boxv[6])

    gl.Vertex(boxv[3])
    gl.Vertex(boxv[7])

    gl.Vertex(boxv[4])
    gl.Vertex(boxv[8])
    gl.End();

    gl.PopMatrix ()

    drawable:swap_buffers()
    drawable:gl_end()

    return true
end

function OpenGl:rotate( )
    self.angle = self.angle + 1
    if self.angle >= 360 then self.angle = 0 end

    self.drawing_area:queue_draw()

    return true
end

function OpenGl:show_about()
    self.dialog:run()
    self.dialog:hide()
end

function OpenGl:run()
    --Timeout (FPS)
    glib.timeout_add(glib.PRIORITY_DEFAULT, 1000/60, self.rotate, self )

    --Run main loop
    gtk.main()

end

-- test

inst = OpenGl.new()
inst:run()

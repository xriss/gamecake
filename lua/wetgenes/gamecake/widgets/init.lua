--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local dprint=function(a) print(wstr.dump(a)) end

--
-- handle widgets
--

--[[#lua.wetgenes.gamecake.widgets

	local widgets=oven.rebake("wetgenes.gamecake.widgets")

A collection of widgets, rendered using gles2 code and controlled 
using the mouse, keyboard or a joystick. EG click fire and move 
left/right to adjust a slider value.

Widgets must be created and bound to an oven, using the 
oven.rebake function.

This has undergone a number of rewrites as we try to simplify the 
widget creation and layout process. Eventually we ended up with a 
fixed size system of widget placement so every widget must have a 
known size in advance, however we allow scaling to occur so for 
instance building a 256x256 widget does not mean that it has to be 
displayed at 256x256 it just means it will be square.

The basic layout just lets you place these widgets in other widgets 
as left to right lines. So as long as you get your sizes right you 
can easily place things just using a list and without keeping track 
of the current position.

Other layout options are available, such as absolute positioning for 
full control and we have simple custom skin versions of the buttons 
as well rather than the built in skins.

All value data is handled by data structures that contain ranges and 
resolutions for data allowing the same data to be bound to multiple 
display widgets. For instance the same data can be linked to the 
position of a slider as well as the content of a text display. I 
think the kids call this sort of thing an MVC pattern but that's a 
terribly dull name.

Swanky paint is probably the most advanced used of the widgets so far
but I suspect we will be making a simple text editor in the near 
future. Designed for advanced in game tweaking of text files.


]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,widgets)
widgets=widgets or {}

local wmeta=oven.rebake("wetgenes.gamecake.widgets.meta")
local wskin=oven.rebake("wetgenes.gamecake.widgets.skin")

--[[#lua.wetgenes.gamecake.widgets.setup

	master=oven.rebake("wetgenes.gamecake.widgets").setup()

	master=oven.rebake("wetgenes.gamecake.widgets").setup(
		{font="Vera",text_size=16,grid_size=32,skin=0} )

Create a master widget, this widget which is considered the root of 
your GUI. It will be filled with functions/data and should contain all 
the functions you need to add data and widgets.

You can pass in these configuration values in a table, the example 
shown above has soom good defaults.

	font="Vera"

The default font to use, this must have already been loaded via 
wetgenes.gamecake.fonts functions.

	text_size=16
	
The default pixel height to render text at.

	grid_size=32
	
The size in pixels that we try and create buttons at.


]]
function widgets.setup(def)

--print("SETUP")
--dprint(def)
--print(debug.traceback())

	local meta={}
	meta.__index=meta
	local master={} -- the master widget, all numerical keys of a widget are the widgets children
	setmetatable(master,meta)
	master.parent=master -- we are our own parent, probably safer than setting as null
	master.master=master -- and our own master
	
	master.font=def.font
	
	def.master=master
	def.meta=meta

	wmeta.setup(def)
	wskin.setup(def)
	
-- default GUI size if no other is specified
	def.hx=def.hx or oven.opts.width
	def.hy=def.hy or oven.opts.height
	def.px=def.px or (oven.opts.width-def.hx)/2
	def.py=def.py or (oven.opts.height-def.hy)/2

	def.class=def.class or "master"
	
	master:setup(def)
	
	master:clean_all()
	
	return master -- our new widget is ready

end

return widgets
end

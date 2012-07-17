-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets
--


local widget_meta=require("wetgenes.gamecake.widget.meta")
local widget_skin=require("wetgenes.gamecake.widget.skin")



module("wetgenes.gamecake.widget")


--
-- create a master widget
--
function setup(win,def)

	local meta={}
	meta.__index=meta
	local master={} -- the master widget, all numerical keys of a widget are the widgets children
	setmetatable(master,meta)
	master.parent=master -- we are our own parent, probably safer than setting as null
	master.master=master -- and our own master
	
	master.font=def.font
	
	def.master=master
	def.meta=meta
	def.win=win

	widget_meta.setup(def)
	widget_skin.setup(def)
	
-- default GUI size if no other is specified
	def.hx=def.hx or 640
	def.hy=def.hy or 480
	def.px=def.px or 0
	def.py=def.py or 0
	def.pxd=def.pxd or 0
	def.pyd=def.pyd or 0

	def.class=def.class or "master"
	
	master:setup(def)
	
	return master -- our new widget is ready

end


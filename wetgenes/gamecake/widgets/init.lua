--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets
--


-- this all needs to be baked into thecake, just hax for now




--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,widgets)
widgets=widgets or {}

local wmeta=oven.rebake("wetgenes.gamecake.widgets.meta")
local wskin=oven.rebake("wetgenes.gamecake.widgets.skin")

--
-- create a master widget
--
function widgets.setup(def)

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
	def.pxd=def.pxd or def.px
	def.pyd=def.pyd or def.py

	def.class=def.class or "master"
	
	master:setup(def)
	
	master:clean_all()
	
	return master -- our new widget is ready

end

return widgets
end


--
-- handle widgets
--


local widget_meta=require("fenestra.widget.meta")
local widget_skin=require("fenestra.widget.skin")


local setmetatable=setmetatable

local require=require

module("fenestra.widget")


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
	
	def.master=master
	def.meta=meta
	def.win=win

	widget_meta.setup(def)
	widget_skin.setup(def)
	
-- default GUI size if no other is specified
	def.hx=def.hx or 640
	def.hy=def.hy or 480
	def.px=def.px or 0
	def.py=def.py or def.hy

	def.class=def.class or "master"
	
	master:setup(def)
	
	return master -- our new widget is ready

end


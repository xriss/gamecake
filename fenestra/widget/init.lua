
--
-- handle widgets
--


local widget_meta=require("fenestra.widget.meta")
local widget_skin=require("fenestra.widget.skin")
local widget_master=require("fenestra.widget.master")


local setmetatable=setmetatable


module("fenestra.widget")

--
-- create a master widget
--
function setup(win,def)

	local meta={}
	meta.__index=meta
	local master={} -- the master widget, all numerical keys of a widget are the widgets children
	setmetatable(master,meta)
	master.parent=master
	
	def.master=master
	def.meta=meta
	def.win=win

	widget_meta.setup(def)
	widget_skin.setup(def)
	widget_master.setup(def)

	master:setup({hx=640,hy=480,px=0,py=480,class="master"}) -- default GUI size
	
	return master

end


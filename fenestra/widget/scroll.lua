-- widget class string
-- a one line string buffer that can be edited



local require=require
local print=print

module("fenestra.widget.scroll")

local string=require("string")
local table=require("table")

function mouse(widget,act,x,y,key)
--	widget.master.focus=widget
	return widget.meta.mouse(widget,act,x,y,key)
end


function key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
end


function update(widget)
	return widget.meta.update(widget)
end


function setup(widget,def)
	local it={}
	widget.string=it
	widget.class="scroll"
	
	widget.key=key
	widget.mouse=mouse
	widget.update=update

	return widget
end

--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local bit=require("bit")
local bnot , band , bor , bxor = bit.bnot , bit.band , bit.bor , bit.bxor

--[[#lua.wetgenes.gamecake.widgets.checkbox

	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="checkbox",...}

A button that can be used to display and toggle a single bit of data.

]]

-- module bake
local M={ modname = (...) } package.loaded[M.modname] = M function M.bake(oven,B) B = B or {}

--[[#lua.wetgenes.gamecake.widgets.checkbox.update

	this function will also call lua.wetgenes.gamecake.widgets.meta.update

If we have a data assigned to this checkbox then make sure that the 
displayed text is up to date. We should really hook into the data so 
any change there is copied here instead?

We use data_mask to check a single bit and then set the text to either 
text_false or text_true depending on the result.


]]
function B.update(widget)

	if widget.data and widget.text_false and widget.text_true then
		local num=widget.data.num or 0
		widget.text= ( band(num,widget.data_mask) == 0 ) and widget.text_false or widget.text_true
	end

	return widget.meta.update(widget)
end

--[[#lua.wetgenes.gamecake.widgets.checkbox.class_hooks

We catch and react to the click hook as we can toggle the data_mask bit 
in the data.

]]
function B.class_hooks(hook,widget,dat)
	if hook=="click" then
		if widget.data then
			local num=widget.data.num or 0
			widget.data:value( ( band(num,widget.data_mask) == 0 ) and bor(num,widget.data_mask) or band(num,bnot(widget.data_mask)) )
		end
	end
end

--[[#lua.wetgenes.gamecake.widgets.checkbox.setup

	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

As a button we always need to be solid so this is forced to true.

cursor will be set to "hand" if it is not already set so you can 
tell it is a button when you hover over it.

data_mask defaults to 1 and represents the bit (or bits) that should be 
tested and toggled in the data. The default of 1 and assuming your data 
starts at 0 means that the data will toggle between 0 and 1 using this 
checkbox.

text_false defaults to " " and is the text that will be displayed when 
a data_mask test is false.

text_true defaults to "X" and is the text that will be displayed when 
a data_mask test is true.

]]
function B.setup(widget,def)

	widget.class="checkbox"
	
	widget.key=key
	widget.update=B.update

	widget.solid=true
	widget.cursor=widget.cursor or "hand"

	widget.color=widget.color or 0

	widget.data_mask=widget.data_mask or 1
	widget.text_false = (not widget.text_false) and " " or widget.text_false
	widget.text_true  = (not widget.text_true ) and "X" or widget.text_true

	widget.class_hooks={B.class_hooks}

	return widget
end

return B
end

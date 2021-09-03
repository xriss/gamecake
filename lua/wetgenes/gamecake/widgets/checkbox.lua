--
-- (C) 2020 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local bit=require("bit")
local bnot , band , bor , bxor = bit.bnot , bit.band , bit.bor , bit.bxor

function M.bake(oven,wcheckbox)
wcheckbox=wcheckbox or {}

function wcheckbox.update(widget)

	if widget.data and widget.text_false and widget.text_true then
		local num=widget.data.num or 0
		widget.text= ( band(num,widget.data_mask) == 0 ) and widget.text_false or widget.text_true
	end

	return widget.meta.update(widget)
end

function wcheckbox.draw(widget)
	return widget.meta.draw(widget)
end

function wcheckbox.class_hooks(hook,widget,dat)
	if hook=="click" then
		if widget.data then
			local num=widget.data.num or 0
			widget.data:value( ( band(num,widget.data_mask) == 0 ) and bor(num,widget.data_mask) or band(num,bnot(widget.data_mask)) )
		end
	end
end

function wcheckbox.setup(widget,def)
--	local it={}
--	widget.button=it
	widget.class="button"
	
	widget.key=key
	widget.mouse=wcheckbox.mouse
	widget.update=wcheckbox.update
	widget.draw=wcheckbox.draw

	widget.solid=true
	widget.cursor=widget.cursor or "hand"

	widget.data_mask=def.data_mask or 1
	widget.text_false=(type(def.text_false)=="nil") and " " or def.text_false
	widget.text_true=(type(def.text_true)=="nil") and "X" or def.text_true

	widget.class_hooks=wcheckbox.class_hooks

	return widget
end

return wcheckbox
end

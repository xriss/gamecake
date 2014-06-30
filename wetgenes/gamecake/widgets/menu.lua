--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenu)
wmenu=wmenu or {}

local function isover(widget)
	local o=widget.master.over
	if o then
		while o~=o.parent do -- need to check all parents
			if o==widget then return true end
			if widget.also_over then -- these widgets also count as over
				for i,v in ipairs(widget.also_over) do
					if o==v then return true end
				end
			end
			o=o.parent
		end
	end
	return false
end

function wmenu.update(widget)

	if     widget.hide_when_not_over then -- must stay over widget
		if isover(widget) then
			widget.over_locked=true
		else
			if widget.over_locked then
				widget.over_locked=false
				widget.hidden=true
				widget.hide_when_not_over=false
				widget.master:layout()
			end
		end
	end
	
	return widget.meta.update(widget)
end

function wmenu.draw(widget)
	return widget.meta.draw(widget)
end


function wmenu.setup(widget,def)

	widget.class="menu"
	
	widget.update=wmenu.update
	widget.draw=wmenu.draw

	widget.solid=true

	return widget
end

return wmenu
end

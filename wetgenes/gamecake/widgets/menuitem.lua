--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenuitem)
wmenuitem=wmenuitem or {}

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

function wmenuitem.update(widget)

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

function wmenuitem.draw(widget)
	return widget.meta.draw(widget)
end

function wmenuitem.hooks(hook,widget,dat)

	if widget.hide_when_clicked then
		if hook=="click" then
			if widget.parent.class=="menu" then -- only the menu?
				widget.parent.over_locked=false
				widget.parent.hidden=true
				widget.parent.hide_when_not_over=false
				widget.master:layout()
			end
		end
	end
	
end

function wmenuitem.setup(widget,def)

	widget.class="menuitem"
	
	widget.update=wmenuitem.update
	widget.draw=wmenuitem.draw

	widget.solid=true

	widget.class_hooks=wmenuitem.hooks
	
	widget.hide_when_clicked=def.hide_when_clicked

	return widget
end

return wmenuitem
end

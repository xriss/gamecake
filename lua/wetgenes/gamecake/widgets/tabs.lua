--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wtabs)
wtabs=wtabs or {}

function wtabs.update(widget)
	return widget.meta.update(widget)
end

function wtabs.draw(widget)
	return widget.meta.draw(widget)
end

function wtabs.layout(widget)

	local class_hook=function(hook,widget)
		if hook=="click" then
			if widget.parent.data then
				widget.parent.data:value(widget.num)
				widget.master.request_layout=true
			end
		end
	end

	local ss=widget.master.theme.grid_size

	local list=widget.data and widget.data.list or {} -- list of {str="name"}
	local hx=1
	if #list>0 and widget.hx then
		hx=widget.hx/#list
	end
	
	for i=#widget,#list+1,-1 do -- remove old if there are any
		widget[i]:remove()
	end
	
	local px=0
	for i=1,#list do -- add or update
		local str=list[i].str
		if not widget[i] then
			local pxf=math.floor(px)
			local hxf=math.floor(px+hx)-math.floor(px)
			local but=widget:add({class="button",text=str,px=pxf,hx=hxf,py=0,hy=ss,color=widget.color,num=i})
			but:add_class_hook(class_hook)
		else
			widget[i].text=str
			widget[i].px=px
			widget[i].hx=hx
		end
		if widget.data and widget.data.num==i then
			widget[i].style="flat"
			widget[i].state="selected"
		else
			widget[i].style="flat"
			widget[i].state=nil
		end
		px=px+hx
	end

	widget.meta.layout(widget)
end

function wtabs.setup(widget,def)

	widget.class="tabs"
	
	local ss=widget.master.theme.grid_size

	widget.update = wtabs.update
	widget.draw   = wtabs.draw
	widget.layout = wtabs.layout


	return widget
end

return wtabs
end

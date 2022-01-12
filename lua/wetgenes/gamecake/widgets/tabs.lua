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

--[[
	widget.key=wtabs.key

-- auto add the draging button as a child
	local ss=16
	if widget.hx<ss*2 then ss=widget.hx/2 end
	if widget.hy<ss*2 then ss=widget.hy/2 end
	local s2=math.ceil(ss/2)

	widget.datx=widget_data.new_data{max=1,master=widget.master}
	widget.daty=widget_data.new_data{max=1,master=widget.master}
	widget.solid=true
	
	widget.color=widget.color or 0
	
	widget.pan=		widget:add({class=def.scroll_pan or "pan",	hx=widget.hx-s2,	hy=widget.hy-s2	,color=widget.color})
	widget.slidey=	widget:add({class="slide",	hx=s2,				hy=widget.hy-s2,	px=widget.hx-s2,	py=0,
		daty=widget.daty,color=widget.color})
	widget.slidex=	widget:add({class="slide",	hx=widget.hx,	hy=s2,           	px=0,           	py=widget.hy-s2,
		datx=widget.datx,color=widget.color})
]]

--	widget.bar      = widget:add({class="fill",		hx=widget.hx,	hy=ss,				py=0,	color=widget.color})
--	widget.children = widget:add({class="pages",	hx=widget.hx,	hy=widget.hy-ss,	py=ss,	color=widget.color})


	return widget
end

return wtabs
end

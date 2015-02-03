--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenu)
wmenu=wmenu or {}

local cake=oven.cake
local canvas=cake.canvas
local font=canvas.font

local function isover(widget)
	local o=widget.master.over
	if o then
		while o~=o.parent do -- need to check all parents
			if o==widget then return true end
			if widget.also_over then -- these widgets also count as over
				for i,v in pairs(widget.also_over) do
					if o==v then return true end
				end
			end
			o=o.parent
		end
	end
	return false
end

function wmenu.update(widget)

	if not widget.hidden then
		if widget.hide_when_not_over then -- must stay over widget
			if not isover(widget) then
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

-- auto resize to text contents vertically
function wmenu.layout(widget)
	
	local py=0
	local hx=0
	for i,v in ipairs(widget) do
		if not v.hidden then
		
			v.hx=0
			v.hy=0
			
			if v[1] then -- we have sub widgets, assume layout will generate a size
				v:layout()
			else -- use text size
				if v.text then
					local f=v:bubble("font") or 1
					v.hy=v:bubble("text_size") or 16
					font.set(cake.fonts.get(f))
					font.set_size(v.hy,0)
					v.hx=font.width(v.text)
					
					v.hx=v.hx+v.hy
					v.hy=v.hy+(v.hy/2)
				end
			end
			
			v.px=0
			v.py=py
			
			py=py+v.hy
			widget.hy=py
			
			if v.hx>hx then hx=v.hx end -- widest


		end
	end
	
	widget.hx=hx

	for i,v in ipairs(widget) do -- set all to widest
		v.hx=hx
	end

	for i,v in ipairs(widget) do -- descend
		if not v.hidden then v:layout() end
	end
end

function wmenu.setup(widget,def)

	widget.class="menu"
	
	widget.update=wmenu.update
	widget.draw=wmenu.draw
	widget.layout=wmenu.layout

	widget.solid=true

	return widget
end

return wmenu
end

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenubar)
wmenubar=wmenubar or {}

local cake=oven.cake
local canvas=cake.canvas
local font=canvas.font


function wmenubar.update(widget)
	return widget.meta.update(widget)
end

function wmenubar.draw(widget)
	if widget.always_draw then -- allways draw this menubar
		return widget.meta.draw(widget)
	else -- only draw when hovering over the menubar 
		local o=widget.master.over
		local d=false
		if widget.parent==o then d=true end
		if widget==o then d=true end
		for i,v in ipairs(widget) do if v==o then d=true end end
		if d then return widget.meta.draw(widget) else return end
	end
end

-- auto resize to text contents horizontally
function wmenubar.layout(widget)
	
	local px=0
	local hy=0
	for i,v in ipairs(widget) do
		if not v.hidden then
		
			v.hx=0
			v.hy=0
			
			if v[1] then -- we have sub widgets, assume layout will generate a size
				v:layout()
			else -- use text size
				if v.text then
					v.hx=(widget:sizeof_text())
					v.hy=widget.hy -- use set height from parent
				end
			end
			
			v.px=px
			v.py=0
			
			px=px+v.hx
--				widget.hx=px
			
			if v.hy>hy then hy=v.hy end -- tallest

		end
	end
	
	widget.hx=px

	for i,v in ipairs(widget) do -- set all to tallest
		v.hy=hy
	end

	widget.meta.layout(widget)
end

function wmenubar.setup(widget,def)

	widget.class="menubar"
	
	widget.update=wmenubar.update
	widget.draw=wmenubar.draw
	widget.layout=wmenubar.layout

	widget.always_draw=widget.always_draw

	widget.solid=true
--	widget.can_focus=true

	return widget
end

return wmenubar
end

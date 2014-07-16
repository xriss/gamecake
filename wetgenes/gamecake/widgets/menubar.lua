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

function wmenubar.update(widget)

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

function wmenubar.draw(widget)
	return widget.meta.draw(widget)
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
					local f=v:bubble("font") or 1
					v.hy=v:bubble("text_size") or 16
					font.set(cake.fonts.get(f))
					font.set_size(v.hy,0)
					v.hx=font.width(v.text)
					
					v.hx=v.hx+v.hy
					v.hy=widget.hy -- use set height from parent
				end
			end
			
			v.px=px
			v.py=0
			
			px=px+v.hx
--				widget.hx=px
			
			if v.hy>hy then hy=v.hy end -- tallest

			v.pxd=widget.pxd+v.px -- absolute position
			v.pyd=widget.pyd+v.py
		end
	end
	
--		widget.hy=hy

	for i,v in ipairs(widget) do -- set all to tallest
		v.hy=hy
	end

	for i,v in ipairs(widget) do -- descend
		if not v.hidden then v:layout() end
	end
end

function wmenubar.setup(widget,def)

	widget.class="menubar"
	
	widget.update=wmenubar.update
	widget.draw=wmenubar.draw
	widget.layout=wmenubar.layout

	widget.solid=true

	return widget
end

return wmenubar
end

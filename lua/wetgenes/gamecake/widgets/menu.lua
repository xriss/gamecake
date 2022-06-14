--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenu)
wmenu=wmenu or {}

local cake=oven.cake
local canvas=cake.canvas
local font=canvas.font

local wwindow=oven.rebake("wetgenes.gamecake.widgets.window")

function wmenu.update(widget)


	if not widget.hidden then
		if widget.hide_when_not and not widget.master.press then -- must stay over widget unless holding button
			if widget:isover(widget.hide_when_not) then
				widget.over_time=wwin.time()
			elseif (not widget.over_time) or (wwin.time() >= widget.over_time+0.25) then -- delay hide
				widget.hidden=true
				widget.hide_when_not=nil
				widget.master.request_layout=true
--				widget.master:layout()
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
				if v.draw_text then
					local f=v:bubble("font") or 1
					local fs=v:bubble("text_size") or 16
					font.set(cake.fonts.get(f))
					font.set_size(fs,0)
					v.hx,v.hy=v:draw_text({size=true})
				elseif v.text then
					local f=v:bubble("font") or 1
					local fs=v:bubble("text_size") or 16
					v.hy=widget.grid_size or fs*1.5
					font.set(cake.fonts.get(f))
					font.set_size(fs,0)
					v.hx=font.width(v.text)+(v.hx_pad or v.hy)
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
	
	local window,screen=widget:window_screen()

	if widget.px+widget.hx > screen.hx then widget.px=screen.hx-widget.hx end -- push away from edges
	if widget.py+widget.hy > screen.hy then widget.py=screen.hy-widget.hy end
	if widget.px<0 then widget.px=0 end
	if widget.py<0 then widget.py=0 end
	
	if widget.hx>screen.hx then widget.sx=screen.hx/widget.hx else widget.sx=1 end -- scale to fit
	if widget.hy>screen.hy then widget.sy=screen.hy/widget.hy else widget.sy=1 end
	
	if widget.sx<widget.sy then widget.sy=widget.sx else widget.sx=widget.sy end -- pick smallest

	for i,v in ipairs(widget) do -- set all to widest
		v.hx=hx
	end
	widget.meta.layout(widget)
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

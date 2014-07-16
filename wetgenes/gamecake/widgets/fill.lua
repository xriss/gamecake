--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- an fbo cached area, that is an area which should always be drawn to a special buffer
-- and then the special buffer should be displayed



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wfill)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wfill=wfill or {}

function wfill.update(widget)
	return widget.meta.update(widget)
end

function wfill.draw(widget)
	return widget.meta.draw(widget)
end

-- this is a fixed layout that works kind of like text
-- we do not adjust the hx,hy size of sub widgets
-- we just place them left to right top to bottom
-- finally we resize this widget to fit its content
function wfill.layout(widget)
	
	local hx,hy=0,0
	local my=0
	widget.hx_max=0
	widget.hy_max=0
	local function addone(w)
		w.px=hx
		w.py=hy
		hx=hx+w.hx
		if hx > widget.hx_max then widget.hx_max=hx end -- max x total size
		if w.hy > my then my=w.hy end -- max y size for this line
--print(w.id or "?",w.px,w.py,w.hx,w.hy)
	end
	
	local function endoflines()
	end
	
	local function endofline()
		hx=0
		hy=hy+my
		my=0
		widget.hy_max=hy
	end
	
	if #widget>0 then
	
		for i,w in ipairs(widget) do
			if not w.hidden then
				if hx+w.hx>widget.hx then
					if hx==0 then -- need one item per line so add it anyway
						addone(w)
						endofline()
					else -- skip this one, push it onto nextline
						endofline()
						addone(w)
					end
				else -- it fits so just add
					addone(w)
				end
			end
		end

		if hx>0 then endofline() end -- final end of line
		
		endoflines()
		
	end
	
	for i,v in ipairs(widget) do
		v.pxd=widget.pxd+v.px
		v.pyd=widget.pyd+v.py
	end

-- layout sub sub widgets	
	for i,v in ipairs(widget) do
		if not v.hidden then v:layout() end
	end
end

function wfill.setup(widget,def)

	widget.class="fill"

	widget.update=wfill.update
	widget.draw=wfill.draw
	widget.layout=wfill.layout
	
	return widget
end

return wfill
end

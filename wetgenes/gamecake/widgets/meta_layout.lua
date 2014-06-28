--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generic default widget layout functions

local tardis=require("wetgenes.tardis")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmeta)
wmeta=wmeta or {}

local cake=oven.cake
local canvas=cake.canvas
local font=canvas.font

--
-- add meta functions
--
function wmeta.setup(def)

--	local master=def.master
	local meta=def.meta
	local win=def.win

--
-- initial layout of widgets, to put them into reasonable positions
--
	function meta.layout(widget)
--print(widget.class)
--		if widget.pan_px and widget.pan_py then -- fidle everything
--print("layout",widget.pan_px,widget.pan_py)
--		end
		
		if widget.class=="fill" or widget.class=="pan" or widget.class=="drag" then
			meta.layout_fill(widget)
		elseif widget.class=="menu" then
			meta.layout_menu(widget)
		else
			meta.layout_base(widget)
		end
	end

	function meta.build_m4(widget)
	
		widget.m4=widget.m4 or tardis.m4.new()
		
		if widget.parent==widget then
			widget.m4:identity()
		else
--print("parent",widget.parent.m4)
			widget.m4:set(widget.parent.m4)
		end
		
		local m4=tardis.m4.new()	
		m4:identity()	

--		local m4=widget.m4

--print("SS",widget.sx,widget.sy)		
		if widget.sx==1 and widget.sy==1 then
				m4:translate( tardis.v3.new(-widget.px,-widget.py,0,0) )
--				x= x-widget.px
--				y= y-widget.py
--print("noscale",sx,sy)
		else
			if widget.smode=="center" then
--print("CCCscale",widget.sx,widget.sy)
				m4:translate( tardis.v3.new(widget.hx*0.5,widget.hy*0.5,0) )
				m4:scale_v3(  tardis.v3.new(1/widget.parent.sx,1/widget.parent.sy,1) )
				m4:translate( tardis.v3.new(-widget.px-widget.hx*0.5,-widget.py-widget.hy*0.5,0,0) )
--				x= ((x-widget.px-widget.hx*0.5)/widget.parent.sx)+widget.hx*0.5
--				y= ((y-widget.py-widget.hy*0.5)/widget.parent.sy)+widget.hy*0.5
			else
--print("scale",widget.sx,widget.sy)
				m4:scale_v3(  tardis.v3.new(1/widget.sx,1/widget.sy,1) )
				m4:translate( tardis.v3.new(-widget.px,-widget.py,0) )
--				x=((x-widget.px)/widget.sx)
--				y=((y-widget.py)/widget.sy)
			end
		end
		
		if widget.pan_px and widget.pan_py then -- fidle everything
--print("build",widget.pan_px,widget.pan_py)
			m4:translate(widget.pan_px,widget.pan_py,0)
		end

		widget.m4:product(m4,widget.m4)


		for i,v in ipairs(widget) do
			meta.build_m4(v)
		end

	end
	

	function meta.layout_base(widget)
		for i,v in ipairs(widget) do
		
			if v.hxf then v.hx=widget.hx*v.hxf end -- generate size as a fraction of parent
			if v.hyf then v.hy=widget.hy*v.hyf end

			v.pxd=widget.pxd+v.px -- absolute position
			v.pyd=widget.pyd+v.py
			
		end
		for i,v in ipairs(widget) do
			v:layout()
		end
	end

-- this is a fixed layout that works kind of like text
-- we do not adjust the hx,hy size of sub widgets
-- we just place them left to right top to bottom
-- finally we resize this widget to fit its content
	function meta.layout_fill(widget)
		
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

			if hx>0 then endofline() end -- final end of line
			
			endoflines()
			
		end
		
		for i,v in ipairs(widget) do
			v.pxd=widget.pxd+v.px
			v.pyd=widget.pyd+v.py
		end

-- layout sub sub widgets	
		for i,v in ipairs(widget) do
			v:layout()
		end
	end

-- this is a fixed layout that works kind of like text
-- we do not adjust the hx,hy size of sub widgets
-- we just place them left to right top to bottom
-- finally we resize this widget to fit its content
	function meta.layout_menu(widget)
		
		local py=0
		local hx=0
		for i,v in ipairs(widget) do
		
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
					
					v.hy=v.hy+8+8
					v.hx=v.hx+8
				end
			end
			
			v.px=0
			v.py=py
			
			py=py+v.hy
			widget.hy=py
			
			if v.hx>hx then hx=v.hx end -- widest


			v.pxd=widget.pxd+v.px -- absolute position
			v.pyd=widget.pyd+v.py	
		end
		
		widget.hx=hx

		for i,v in ipairs(widget) do -- set all to widest
			v.hx=hx
		end

		for i,v in ipairs(widget) do -- descend
			v:layout()
		end
	end

end


return wmeta
end

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
				m4:scale_v3(  tardis.v3.new(1/widget.sx,1/widget.sy,1) )
				m4:translate( tardis.v3.new(-widget.px-widget.hx*0.5,-widget.py-widget.hy*0.5,0) )
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
			m4:translate({widget.pan_px,widget.pan_py,0})
		end

		m4:product(widget.m4,widget.m4)


		for i,v in ipairs(widget) do
			meta.build_m4(v)
		end

	end
	
-- resize is performed recursively before layout so that layout can position its children
	function meta.resize(widget,mini)
		mini=mini or 0
		for i,v in ipairs(widget) do if i>mini and v.size then
		
			for token in string.gmatch(v.size, "[^%s]+") do -- can contain multiple tokens
			
				if token=="full" then -- force full size

					v.px=0
					v.py=0
					v.hx=widget.hx/widget.sx
					v.hy=widget.hy/widget.sy

	--print("full",v.px,v.py,v.hx,v.hy)
	--print("parent class",widget.parent.class)

				elseif token=="fullx" then -- force full size X only

					v.px=0
					v.hx=widget.hx/widget.sx

				elseif token=="fully" then -- force full size Y only

					v.py=0
					v.hy=widget.hy/widget.sy

				elseif token=="border" then -- force a fixed border size

					v.hx=(widget.hx/widget.sx)-(v.px*2)
					v.hy=(widget.hy/widget.sy)-(v.py*2)

				elseif token=="minmax" then -- force a minimum width maximum height with scale on parent
				
					if     v.hx_min and v.hy_max then
--						v.px=0
--						v.py=0
						v.hx=widget.hx
						v.hy=v.hy_max
						if v.hx < v.hx_min then
							v.sx=widget.hx/v.hx_min
							v.sy=v.sx
							v.hx=v.hx_min
							v.hy=v.hy_max--*v.sy
						else
							v.sx=1
							v.sy=1
						end
					elseif v.hy_min and v.hx_max then
						v.px=0
						v.py=0
						v.hx=v.hx_max
						v.hy=widget.hy
						if v.hy < v.hy_min then
							v.sx=widget.hy/v.hy_min
							v.sy=v.sx
							v.hy=v.hy_min
							v.hx=v.hx_max--*v.sx
						else
							v.sx=1
							v.sy=1
						end
					end

				end
			end
			
		end end
		for i,v in ipairs(widget) do
			if not v.hidden then v:resize() end
		end
		for i,v in ipairs(widget) do if i>mini and v.size then
			for token in string.gmatch(v.size, "[^%s]+") do -- can contain multiple tokens
		
				if token=="fitx" then  -- set hx to maximum of children

					v.hx=0
					for i,w in ipairs(v) do
						if not w.hidden then
							local x=(w.hx+w.px)*w.sx
							if x>v.hx then v.hx=x end
						end
					end

				elseif token=="fity" then -- set hy to maximum of children

					v.hy=0
					for i,w in ipairs(v) do
						if not w.hidden then
--print(w.sy)
							local y=(w.hy+w.py)*w.sy
							if y>v.hy then v.hy=y end
						end
					end
				end

			end
		end end
	end

	function meta.layout(widget,mini)
		mini=mini or 0
--		for i,v in ipairs(widget) do if i>mini then
--		end end
		for i,v in ipairs(widget) do
			if not v.hidden then v:layout() end
		end
	end

	function meta.resize_and_layout(widget)
--print("master layout")
		widget:resize()
		widget:layout()
		widget:build_m4()
--exit()
	end

end


return wmeta
end

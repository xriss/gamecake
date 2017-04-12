--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- scale or scroll the *SINGLE* child to fit within this panels size

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wwindow)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wwindow=wwindow or {}

function wwindow.mouse(widget,act,_x,_y,keyname)

-- check children first
	local ret=widget.meta.mouse(widget,act,_x,_y,keyname)
--[[
	if widget.panel_mode=="window" then -- window move?

		local x,y=widget.parent:mousexy(_x,_y)

		if widget.drag_mouse then
			if act==-1 and keyname=="left" then
				widget.drag_mouse=nil
			else
				widget.px=widget.drag_mouse[1]+x
				widget.py=widget.drag_mouse[2]+y
				widget:set_dirty()
				widget.meta.build_m4(widget)
			end
		else
			if act==1 and keyname=="left" then
				if widget.master.over==widget then --clicking on us?
					widget.drag_mouse={widget.px-x,widget.py-y}
					widget.parent:insert(widget) -- move to top
				end
			end
		end

print(ret,x,y,act,keyname)

	end
]]
	return ret
end

function wwindow.update(widget)
	return widget.meta.update(widget)
end

function wwindow.draw(widget)
	return widget.meta.draw(widget)
end

-- this is a magic layout that sizes panels

function wwindow.layout(widget)

	local v=widget[1]
	if v then
		if widget.panel_mode=="scale" then -- maintain aspect

			v.sx=widget.hx/v.hx
			v.sy=widget.hy/v.hy

			if v.sx<v.sy then v.sy=v.sx else v.sx=v.sy end
			
			v.px=(widget.hx-v.hx*v.sx)/2
			v.py=(widget.hy-v.hy*v.sy)/2

		elseif widget.panel_mode=="stretch" then -- stretch to fit any area

			v.sx=widget.hx/v.hx
			v.sy=widget.hy/v.hy

		end
	end

-- also layout any other children
	widget.meta.layout(widget,1)

end

function wwindow.setup(widget,def)

	widget.class="window"

	widget.panel_mode=def.panel_mode or "scale" 	-- scale the child to fit

--	widget.key=wwindow.key
	widget.mouse=wwindow.mouse
	widget.update=wwindow.update
	widget.draw=wwindow.draw
	widget.layout=wwindow.layout
	
	return widget
end

return wwindow
end

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- scale or scroll the *SINGLE* child to fit within this panels size

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wpanel)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wpanel=wpanel or {}

function wpanel.update(widget)
	return widget.meta.update(widget)
end

function wpanel.draw(widget)
	return widget.meta.draw(widget)
end

-- this is a magic layout that sizes panels

function wpanel.layout(widget)

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

function wpanel.setup(widget,def)

	widget.class="panel"

	widget.panel_mode=def.panel_mode or "scale" 	-- scale the child to fit

	widget.update=wpanel.update
	widget.draw=wpanel.draw
	widget.layout=wpanel.layout
	
	return widget
end

return wpanel
end

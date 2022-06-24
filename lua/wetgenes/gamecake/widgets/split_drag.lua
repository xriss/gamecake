--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wsplit_drag)

wsplit_drag=wsplit_drag or {}

function wsplit_drag.setup(widget,def)

	widget.class="split_drag"

	local sub={}
	for a,b in pairs(def) do if type(a)=="string" then sub[a]=b end end -- shallow copy every string value
	sub.class="split"
	sub.size="full"

	widget.split=widget:add(sub)


-- build an invisible dragbar so we can drag to resize left and right

	local snapit=function(ws)
		widget.drag.px=math.floor(widget.drag.px)
		widget.drag.py=math.floor(widget.drag.py)
	
		ws.px=math.floor(ws.px)
		ws.py=math.floor(ws.py)
		ws.hx=math.ceil(ws.hx)
		ws.hy=math.ceil(ws.hy)

--			print(ws.px,ws.py,ws.hx,ws.hy,widget.drag.px,widget.drag.py)
	end

	if widget.split.split_axis=="x" then

		widget.drag=widget:add({style="button",class="drag",px=0,hx=8,size="fully",solid=true,cursor="sizewe",color=widget.slide_color}) -- probably invisible

		local clampit=function()
			local ws=widget.split[ widget.split.split_order ]
			if not ws then return end
			
			if widget.hx < ws.hx then
				ws.hx=widget.hx
				if widget.split.split_order == 1 then
					widget.drag.px=ws.hx-4
				else
					widget.drag.px=widget.hx-(ws.hx+4)
				end
				widget.master.request_layout=true -- need layout
			end
			snapit(ws)
		end

		widget.drag.class_hooks={function(h,w)
			local ws=widget.split[ widget.split.split_order ]
			if not ws then return end

			if h=="slide" then
				if widget.split.split_order == 1 then
					ws.hx=widget.drag.px+4
				else
					ws.hx=widget.hx-(widget.drag.px-4)
				end
				clampit()
			end
		end}

		widget.resize=function(widget)
			local ws=widget.split[ widget.split.split_order ]
			if not ws then return end

			if widget.split.split_order == 1 then
				widget.drag.px=ws.hx-4
			else
				widget.drag.px=widget.hx-(ws.hx+4)
			end
			clampit()
			return widget.meta.resize(widget)
		end

	elseif widget.split.split_axis=="y" then

		widget.drag=widget:add({style="button",class="drag",py=0,hy=8,size="fullx",solid=true,cursor="sizens",color=nil})

		local clampit=function()
			local ws=widget.split[ widget.split.split_order ]
			if not ws then return end
			
			if widget.hy < ws.hy then
				ws.hy=widget.hy
				if widget.split.split_order == 1 then
					widget.drag.py=ws.hy-4
				else
					widget.drag.py=widget.hy-(ws.hy+4)
				end
				widget.master.request_layout=true -- need layout
			end
			snapit(ws)
		end

		widget.drag.class_hooks={function(h,w)
			local ws=widget.split[ widget.split.split_order ]
			if not ws then return end

			if h=="slide" then
				if widget.split.split_order == 1 then
					ws.hy=widget.drag.py+4
				else
					ws.hy=widget.hy-(widget.drag.py-4)
				end
				clampit()
			end
		end}

		widget.resize=function(widget)
			local ws=widget.split[ widget.split.split_order ]
			if not ws then return end

			if widget.split.split_order == 1 then
				widget.drag.py=ws.hy-4
			else
				widget.drag.py=widget.hy-(ws.hy+4)
			end
			clampit()
			return widget.meta.resize(widget)
		end

	end
	
	widget.children=widget.split -- a generic name for subwidget trees to continue down

	return widget
end

return wsplit_drag
end

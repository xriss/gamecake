--
-- (C) 2020 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local function print(...) _G.print(...) end
local function dprint(a) print(require("wetgenes.string").dump(a)) end

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wpath=require("wetgenes.path")

local wgw_mounts=require("wetgenes.gamecake.widgets.mounts")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(oven,wtree)

	wtree=wtree or {} 
	wtree.modname=M.modname
	
-- refresh any item
wtree.item_to_line=function(widget,item)
	
	if item.to_line then -- this must provide a .line output
		item:to_line(widget)
	else
		local ss=widget.master.theme.grid_size		
		local opts={
			class="line",
			id=item.id, -- or widget.id,
			class_hooks=widget.class_hooks,
			hooks=item.hooks, -- or widget.hooks,
			hx=ss,
			hy=ss,
			text_align="left",
			user=item,
			color=0,
--			solid=true,
		}

		item.line=widget:create(opts)
		item.line_text=item.line:add({class="text",text=item.text})

		for _,it in ipairs(item) do
			widget:item_to_line(it)
		end
	end

end


wtree.refresh=function(widget)
	widget:resize_and_layout() -- layout will auto update
	widget:set_dirty()
end

wtree.layout=function(widget)
	local ss=widget.master.theme.grid_size
	local pan=widget.scroll_widget.pan

	pan:remove_all()

-- run refresh_item on each item and add item.line to the display
	if widget.items then
		widget:item_to_line(widget.items) -- auto refresh
		local recurse
		recurse=function(parent)
			for _,item in ipairs(parent.dir or parent) do
				if item.line then -- add this line widget
					pan:insert(item.line)
				end
				recurse(item)
			end
		end
		recurse({widget.items}) -- top item
	end
	
	widget.meta.layout(widget)
end


-- auto resize to text contents vertically
function wtree.pan_layout(widget)

	local py=0
	local hx=0
	for i,v in ipairs(widget) do
		if not v.hidden then
		
			v.hx=0
			v.hy=0
			
			if v[1] then -- we have sub widgets so ask for size
				v:resize()
			else -- use text size
				v.hx,v.hy=v:sizeof_text()
			end
			
			v.px=0
			v.py=py
			py=py+v.hy
			
			if v.hx>hx then hx=v.hx end -- widest
		end
	end
	
	if widget.hx > hx then hx=widget.hx end -- include parent size in maximum width
	widget.hx_max=hx
	widget.hy_max=py
	for i,v in ipairs(widget) do -- set all to widest
		v.hx=hx
	end

	widget.meta.layout(widget)
end


function wtree.setup(widget,def)

	widget.class="tree"

	widget.items={} -- items to display
	widget.refresh=wtree.refresh -- rebuild display from items
	widget.layout=wtree.layout

	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,size="full",class="scroll"})

	widget.scroll_widget.pan.layout=wtree.pan_layout -- custom pan layout
	
-- update item info
	widget.item_to_line=wtree.item_to_line

--print(def.mounts)
	widget.items=def.items or wgw_mounts.mount_meta(unpack(def.mounts or {}))
	
	widget.fbo=oven.cake.framebuffers.create(0,0,0)

--	widget:refresh()

	return widget
end


	return wtree
end

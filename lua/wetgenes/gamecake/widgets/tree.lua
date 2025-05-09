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


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(oven,wtree)

	wtree=wtree or {} 
	wtree.modname=M.modname
	
-- refresh any item
wtree.item_to_line=function(widget,item)
	
	if item.refresh then -- this must provide a .line output
		item:refresh()
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
			solid=true,
		}

		item.line=widget:create(opts)
		item.line_text=item.line:add({class="text",text=item.text})
	end

	for _,it in ipairs(item) do
		it.parent=item
		widget:item_to_line(it)
	end
end

-- set / refresh top level list of items ( which is not itself an item )
wtree.items_to_lines=function(widget,items)
	if items then widget.items=items end
	items=widget.items
	
	for _,item in ipairs(items) do
		item.parent=items
		widget:item_to_line(item)
	end

end

wtree.refresh=function(widget)
	widget:items_to_lines()
	widget:layout()
	widget:set_dirty()
end

wtree.layout=function(widget)
	local ss=widget.master.theme.grid_size
	local pan=widget.scroll_widget.pan

	pan:remove_all()

-- run refresh_item on each item and add item.line to the display
	if widget.items then
		local recurse
		recurse=function(parent)
			for _,item in ipairs(parent) do
				if item.line then -- add this line widget
					pan:insert(item.line)
				end
				if item[1] then -- children of this item in numerical keys
					recurse(item)
				end
			end
		end
		recurse(widget.items) -- this is just a list not an item itself
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
	widget.items_to_lines=wtree.items_to_lines
	widget.item_to_line=wtree.item_to_line

	widget:refresh()

	return widget
end


	return wtree
end

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


M.mount_ms=function(...)
	return M.ms.setup({
		name="",
		path="",
		dir={...},
	})
end


M.ms={}
M.ms_metatable={__index=M.ms}

local wpath=require("wetgenes.path")

M.ms.is="ms"

M.ms.find_prefix=function(ms,path)
	local best
	for _,it in pairs(ms.dir) do -- find best dir prefix for this path ( mount points )
		local c=path:sub(1,#it.path) -- path must begin with
		if c==it.path then
			if not best then best=it end
			if #it.path > #best.path then best=it end -- longest path wins
		end
	end
	return best,path -- may be nil and path may be adjusted
end

M.ms.setup=function(ms)
	if not ms then ms={} end
	setmetatable(ms,M.ms_metatable)		
	return ms
end

M.ms.sort_dir=function(ms)
	if ms.dir then -- safe check
		table.sort(ms.dir,function(a,b)
			if ( not a.dir ) == ( not b.dir ) then -- both are dir or files
				return a.name:lower() < b.name:lower()
			else
				if a.dir then return true end
			end
		end)
	end
end

M.ms.merge_dir=function(ms,dir)

	if not dir then
		if ms.fetch_dir then
			dir=ms:fetch_dir(ms.path)
		else
			return -- nothing to do
		end
	end

	if not ms.dir then -- just set
		ms.dir=dir
	else

		local map={}
		for i,v in ipairs(dir) do
			map[v.name]=v
		end

		for i=#ms.dir,1,-1 do -- backwards so we can remove
			local v=ms.dir[i]
			local o=map[v.name]
			if o then -- merge
				map[v.name]=nil -- forget after merge 
				for k,s in pairs(o) do
					if not v[k] or type(s)~="table" then -- do not merge tables unless dest is empty
						v[k]=s
					end
				end
			else -- delete
				table.remove(ms.dir,i)
			end
		end
		-- any items left in the map are new
		for _,v in pairs(map) do -- add
			ms.dir[#ms.dir+1]=v
		end

	end

	ms:sort_dir()
end

M.ms.toggle_dir=function(ms)

	if not ms.dir then return end
	
	ms.expanded = not ms.expanded

	if ms.expanded then -- expanded so refresh contents
		ms:merge_dir()
	else -- collapsed so remove children
		for i=#ms.dir,1,-1 do
			local v=ms.dir[i]
			if not v.keep then
				table.remove(ms.dir,i)
			end
		end		
	end

end

M.ms.find_it=function(ms,search)
	local found=true
	for k,v in pairs(search) do -- must match every key,value in search
		if ms[k]~=v then found=false break end
	end
	if found then return ms end -- found it
	
	if ms.dir then -- is a dir with data
		for ids,it in pairs(ms.dir) do
			local found=it:find_it(search)
			if found then return found end -- found it
		end
	end
	
	return false
end


M.ms.to_line=function(ms,widget,only)

	local ss=widget.master.theme.grid_size		
	local opts={
		class="line",
		class_hooks=widget.class_hooks,
		hooks=ms.hooks, -- or widget.hooks,
		hx=ss,
		hy=ss,
		text_align="left",
		user=ms,
		color=0,
		solid=true,
	}

	ms.line=widget:create(opts)

	local pp=wpath.split( wpath.unslash(ms.path) )

	ms.text=string.rep(" ",#pp-1)
	if ms.dir then
		if ms.expanded then
			ms.text=ms.text.."< "
		else
			ms.text=ms.text.."= "
		end
	else
		ms.text=ms.text.."- "
	end
	ms.text=ms.text..ms.name

	ms.line_text=ms.line:add({class="text",text=ms.text})

	if not only then
		if ms.dir then
			for _,it in ipairs(ms.dir) do
				it:to_line(widget)
			end
		end
	end
end


M.ms.fetch_attr=function(ms,path)
	local it,path=assert(ms:find_prefix(path))
	return it:fetch_attr(path)
end

M.ms.fetch_dir=function(ms,path)
	local it,path=assert(ms:find_prefix(path))
	return it:fetch_dir(path)
end

M.ms.manifest_path=function(ms,path)
	local it,path=assert(ms:find_prefix(path))
print(it,path)
	return it:manifest_path(path)
end



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
			solid=true,
		}

		item.line=widget:create(opts)
		item.line_text=item.line:add({class="text",text=item.text})

		for _,it in ipairs(item) do
			widget:item_to_line(it)
		end
	end

end


wtree.refresh=function(widget)
--	widget:item_to_line(widget.items)
	widget:resize_and_layout() -- layout will auto update
--	widget:set_dirty()
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
					recurse(item)
				end
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

print(def.mounts)
	widget.items=M.mount_ms(unpack(def.mounts or {}))

--	widget:refresh()

	return widget
end


	return wtree
end

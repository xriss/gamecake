--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.grdhistory

	local wgrdhistory=require("wetgenes.grdhistory")

We use wgrdhistory as the local name of this library.

Add extra functionality to wetgenes.grd primarily these are functions that 
are used by swanky paint to manage its internal data.

]]


local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local cmsgpack=require("cmsgpack")

local zlib=require("zlib")
local inflate=function(d) return ((zlib.inflate())(d)) end
local deflate=function(d) return ((zlib.deflate())(d,"finish")) end

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local grdhistory=M


local palette_nil=("\0\0\0\0"):rep(256) -- an empty all 0 palette

-- deep recursive merge copy from it into base
local deepjson_copy ; deepjson_copy=function(base,it)
	base=base or {}
	for n,v in pairs(it or {}) do
		if type(v)=="table" then -- recurse
			base[n]=deepjson_copy(base[n],v) -- create new table maybe
		else
			base[n]=v
		end
	end
	return base
end

-- deep recursive diff returns things in it that exist and are different to base
-- does not work with fields that are set to nil in it
-- returns nil if all data in it is the same as in base
local deepjson_diff ; deepjson_diff=function(base,it)
	local diff

	for n,v in pairs(it or {}) do
		if type(v)=="table" then -- recurse
			local d=deepjson_diff(base[n] or {},v)
			if d then
				diff=diff or {} -- only create on change
				diff[n]=d -- set new values
			end
		else
			if base[n] ~= v then
				diff=diff or {} -- only create on change
				diff[n]=v -- set new value
			end
		end
	end

	return diff
end

-- return the values that would be replace by applying diff
local deepjson_undo ; deepjson_undo=function(base,diff)
	local undo={}
	for n,v in pairs(diff) do
		if type(v)=="table" then -- recurse
			undo[n]=deepjson_undo(base[n] or {},v)
		else
			undo[n]=base[n]
		end
		undo[n]=undo[n] or false -- convert nil to false
	end
	return undo
end

-- anchor helper
local anchor_helper=function(a,b,anchor)
	if a<b then
		if     anchor<0 then	return a,0,0
		elseif anchor>0 then	return a,0,b-a
		else					return a,0,math.floor((b-a)/2)
		end
	else
		if     anchor<0 then	return b,0,0
		elseif anchor>0 then	return b,a-b,0
		else					return b,math.floor((a-b)/2),0
		end
	end
end


--[[#lua.wetgenes.grdhistory.history

	local history=grdhistory.history(grd)

Create and bind a history object to the given grd object. The history 
lives inside the grd and can be accesd as grd.history just as the grd can 
be accessed through the history as history.grd

]]

-- create a history state within the given grd
grdhistory.history=function(grd)

	local history={grd=grd}	
	grd.history=history
	
	if not grd.layers then require("wetgenes.grdpaint").layers(grd) end -- need layers

	history.reset=function(history)
	
		history.version="Swanky History v1"
		history.start=1
		history.length=1
		history.index=1
		history.memory=0
		history.list={}
		history.json={} -- local cache deep copy that starts empty

-- starting maximum size
		history.width=grd.width		-- the maximum grd size over time
		history.height=grd.height

-- add a first empty state so we can always undo to this state of the image
		history:set(1,{id=1})
		
		return history
	end
	
	history.get=function(history,index)
		if not index then return end
		local it=history.list[index]
		it=it and cmsgpack.unpack(inflate(it))
		return it
	end
	history.set=function(history,index,it)
		it=it and deflate(cmsgpack.pack(it))
		history.list[index]=it
		if index>history.length then -- added a new one
			history.length=index
			history.memory=history.memory+#it -- keep running total
		end
	end
	
-- take a snapshot of this frame for latter diffing (started drawing on this frame only)
	history.draw_begin=function(history,x,y,z,w,h,d)
		history.area={
			x or 0 ,
			y or 0 ,
			z or 0 ,
			w or history.grd.width -(x or 0) ,
			h or history.grd.height-(y or 0) ,
			d or 1 }
		if w==0 and h==0 and d==0 then -- flag palette only with auto merge
			history.area.pal=true
			if not history.pal then -- use original
				history.grd_diff=history.grd:clip(unpack(history.area)):duplicate()
			end
		else
			history.pal=nil -- stop accumulating palette changes
			history.grd_diff=history.grd:clip(unpack(history.area)):duplicate()
		end
	end

-- return a temporray grd of only the frame we can draw into
	history.draw_get=function(history)
		assert(history.grd_diff) -- sanity
		return history.grd:clip(unpack(history.area))
	end

-- revert back to begin state
	history.draw_revert=function(history)
		assert(history.grd_diff) -- sanity
		local c=history.area
		history.grd:pixels(c[1],c[2],c[3],c[4],c[5],c[6],history.grd_diff) -- restore image
		history.grd:palette(0,256, history.grd_diff ) -- restore palette
	end
	
-- stop any accumulated changes
	history.draw_end=function(history)
		history.grd_diff=nil
		history.pal=nil
	end
	
-- push any changes we find into the history, optionally pass in a new json object to diff and apply
	history.draw_save=function(history,json)
		assert(history.grd_diff) -- sanity
		
		if history.grd.width  > history.width  then history.width =history.grd.width  end
		if history.grd.height > history.height then history.height=history.grd.height end
		
		if history.area.pal then -- palette only with auto merge
		
			if history.pal then -- auto merge into last change
			
				local ga=history.grd_diff:duplicate()
				local it=history.pal
				local gb=history.grd:clip(unpack(history.area))
				ga:xor(gb)
				it.palette=ga:palette(0,256,"")
				history:set(it.id,it)

			else -- new
				local ga=history.grd_diff:duplicate()
				local it={}
				local gb=history.grd:clip(unpack(history.area))
				ga:xor(gb)
				it.palette=ga:palette(0,256,"")
				if it.palette==palette_nil then it.palette=nil end -- no colour has changed so do not store diff

				if it.palette or it.redo then -- only remember if any colour or json has changed

					it.prev=history.index -- link to prev
					it.id=history.length+1

					history.index=it.id
					history:set(it.id,it)
					if it.prev then -- link from prev
						local pit=history:get(it.prev)
						if pit then
							pit.next=it.id -- link to *most*recent* next
							history:set(pit.id,pit)
						end
					end

					history.pal=it
				end
			end

		else
		
			local ga=history.grd_diff
			local it={x=0,y=0,z=0,w=ga.width,h=ga.height,d=ga.depth}
			local gb=history.grd:clip(unpack(history.area))
			ga:xor(gb)
			it.palette=ga:palette(0,256,"")
			if it.palette==palette_nil then it.palette=nil end -- no colour has changed so do not store diff
			ga:shrink(it)
			if it.w>0 and it.h>0 and it.d>0 then -- some pixels have changed
				it.data=ga:pixels(it.x,it.y,it.z,it.w,it.h,it.d,"") -- get minimal xored data area as a string
				it.x=it.x+history.area[1]
				it.y=it.y+history.area[2]
				it.z=it.z+history.area[3]
			else
				it.x=nil
				it.y=nil
				it.z=nil
				it.w=nil
				it.h=nil
				it.d=nil
			end

			local redo=deepjson_diff(history.json,json) -- returns nil if json is nil or contains only the same data
			if redo then
				it.json={redo,deepjson_undo(history.json,redo)} -- {redo,undo} values
				deepjson_copy(history.json,it.json[1])
			end

			if it.data or it.palette or it.json then -- only remember if something has actually changed


				it.prev=history.index>0 and  history.index -- link to prev
				it.id=history.length+1

				history.index=it.id
				history:set(it.id,it)
				if it.prev then -- link from prev which must exist
					local pit=history:get(it.prev)
					pit.next=it.id -- link to *most*recent* next
					history:set(pit.id,pit)
				end
			
			end
			
			history:draw_end()
		end
	end

-- push this prebuilt history item onto the history stack linking onto index
	history.push=function(history,it)

		history:draw_end() -- sanity
		
		it.prev=history.index>0 and  history.index -- link to prev
		it.id=history.length+1

		history.index=it.id
		history:set(it.id,it)

		if it.prev then -- link from prev which must exist
			local pit=history:get(it.prev)
			if pit then
				pit.next=it.id -- link to *most*recent* next
				history:set(pit.id,pit)
			end
		end

		return it
	end

-- apply and push a layers rearrange onto the stack
	history.push_rearrange=function(history,x,y,n)

		local layers=history.grd.layers

		if not x and not y and not n then -- auto natural order
			n=layers.count
			y=math.floor(math.sqrt(n))
			x=math.ceil(n/y)
		end
		
		if n==layers.count and x==layers.x and y==layers.y then
			return -- no change
		end
		
		local it={rearrange={}}

		it.rearrange[1]={x,y,n}
		it.rearrange[2]={layers.x,layers.y,layers.count}
		
		layers:rearrange(x,y,n) -- apply

		return history:push(it)
	end

-- apply and push a layers adjust onto the stack
	history.push_layers=function(history,idx,num)
		history:push_rearrange() -- make sure we have a natural order
		
		if num<0 then -- blank the layers we are about to delete first
			for z=0,history.grd.depth-1 do -- across all frames
				for i=idx,(idx-num)-1 do
					history:draw_begin(history.grd.layers:area(i,z))
					history:draw_get():clear()
					history:draw_save()
				end
			end
		end

		local it={layers={}}

		it.layers[1]={idx,num}
		it.layers[2]={idx,-num}

		history.grd.layers:adjust_layer_count(idx,num)

		return history:push(it)
	end

-- apply and push a frames adjust onto the stack
	history.push_frames=function(history,idx,num)

		local g=history.grd
		if num<0 then -- blank the frames we are about to delete first
			for z=idx,(idx-num)-1 do
				history:draw_begin(0,0,z,g.width,g.height,1)
				history:draw_get():clear()
				history:draw_save()
			end
		end

		local it={frames={}}

		it.frames[1]={idx,num}
		it.frames[2]={idx,-num}

		history.grd.layers:adjust_depth(idx,num)

		return history:push(it)
	end
	
-- apply and push a swap onto the stack
	history.push_swap_layers_with_frames=function(history)
		history:push_rearrange() -- make sure we have a natural order

		local it={swap_layers_with_frames=true}

		history.grd.layers:swap_with_frames()

		return history:push(it)
	end

-- apply and push an image size change
	history.push_size=function(history,width,height,ax,ay)

		local layers=history.grd.layers
		local wa,ha=history.grd.width,history.grd.height
		local w,xa,xb=anchor_helper(wa,width, ax)
		local h,ya,yb=anchor_helper(ha,height,ay)

		if width<wa or height<ha then -- if shrinking we need to clear the areas we will lose first
			for z=0,history.grd.depth-1 do -- across all frames
				history.draw_begin(0,0,z,history.grd.width,history.grd.height,1)
				local ga=history:draw_get()
				if width<wa then
					if xa>0 then
						ga:clip(0,0,0,xa,ga.height,1):clear()
					end
					if xa+width<ga.width then
						ga:clip(xa+width,0,0,ga.width-(xa+width),ga.height,1):clear()
					end
				end
				if height<ha then
					if ya>0 then
						ga:clip(0,0,0,ga.width,ya,1):clear()
					end
					if ya+height<ga.height then
						ga:clip(0,ya+height,0,ga.width,ga.height-(ya+height),1):clear()
					end
				end
				history:draw_save()
			end
		end

		local it={size={}}

		it.size[1]={width,height,ax,ay}
		it.size[2]={grd.width,grd.height,ax,ay} -- restore old size

		history.grd.layers:adjust_size(width,height,ax,ay)

		return history:push(it)
	end

-- apply and push an image layer size change
	history.push_layer_size=function(history,width,height,ax,ay)

		local layers=history.grd.layers
		local wa,ha=layers:size() -- original size
		local w,xa,xb=anchor_helper(wa,width, ax)
		local h,ya,yb=anchor_helper(ha,height,ay)

		if width<wa or height<ha then -- if shrinking we need to clear the areas we will lose first
			for z=0,history.grd.depth-1 do -- across all frames
				history:draw_begin(0,0,z,history.grd.width,history.grd.height,1)
				local g=history:draw_get()
				for l=1,layers.count do
					local ga=g:clip(layers:area(l,0))
					if width<wa then
						if xa>0 then
							ga:clip(0,0,0,xa,ga.height,1):clear()
						end
						if xa+width<ga.width then
							ga:clip(xa+width,0,0,ga.width-(xa+width),ga.height,1):clear()
						end
					end
					if height<ha then
						if ya>0 then
							ga:clip(0,0,0,ga.width,ya,1):clear()
						end
						if ya+height<ga.height then
							ga:clip(0,ya+height,0,ga.width,ga.height-(ya+height),1):clear()
						end
					end
				end
				history:draw_save()
			end
		end

		local it={layer_size={}}

		it.layer_size[1]={width,height,ax,ay}
		it.layer_size[2]={wa,ha,ax,ay} -- restore old size

		history.grd.layers:adjust_layer_size(width,height,ax,ay)

		return history:push(it)
	end

-- apply and push an image layer size change
	history.push_rechop=function(history,x,y,count)
	
		count=count or x*y

		local layers=history.grd.layers

		local it={rechop={}}

		it.rechop[1]={x,y,count}
		it.rechop[2]={layers.x,layers.y,layers.count} -- restore old size

		history.grd.layers:config(x,y,count)

		return history:push(it)
	end

-- ru 1==redo , 2==undo	
	history.apply=function(history,index,ru) -- apply diff at this index
		history:draw_end()
		local it=history:get(index or history.index) -- default to current index
		if not it then return end -- nothing to do
		
		local a=it[1] and it or {it} -- an array of things to do
		local fa,fb,fc=1,#a,1 -- process the array forwards
		if ru==2 then fa,fb,fc=#a,1,-1 end -- or backwards if we are undoing
		for i=fa,fb,fc do -- loop over things to do
			local v=a[i] -- and do them
			if v.data then -- xor image
				local ga=wgrd.create(history.grd.format,v.w,v.h,v.d)
				local gb=history.grd:clip(v.x,v.y,v.z,v.w,v.h,v.d)
				ga:pixels(0,0,0,v.w,v.h,v.d,v.data)
				if v.palette then -- xor pal
					ga:palette(0,256,v.palette)
				end
				gb:xor(ga)
				history.grd.layers.frame=v.z -- as we apply changes we should switch active frame to the one we changed
			elseif v.palette then -- xor pal only
				local ga=wgrd.create(history.grd.format,0,0,0)
				ga:palette(0,256,v.palette)
				history.grd:clip(0,0,0,0,0,0):xor(ga)
			end

			if v.json then -- have some json changes to make
				deepjson_copy(history.json,v.json[ru])
			end
			
			if v.rearrange then -- rearrange layout of layers
				history.grd.layers:rearrange(unpack(v.rearrange[ru]))
			end

			if v.layers then -- add or remove some layers
				history.grd.layers:adjust_layer_count(unpack(v.layers[ru]))
			end

			if v.frames then -- add or remove some frames
				history.grd.layers:adjust_depth(unpack(v.frames[ru]))
			end
			
			if v.swap_layers_with_frames then -- this is its own reverse
				history.grd.layers:swap_with_frames()
			end
			
			if v.size then -- change size of image
				history.grd.layers:adjust_size(unpack(v.size[ru]))
			end

			if v.layer_size then -- 
				history.grd.layers:adjust_layer_size(unpack(v.layer_size[ru]))
			end

			if v.rechop then -- adjust layer layout
				history.grd.layers:config(unpack(v.rechop[ru]))
			end
		end

		if ru==1 then -- redo
			history.index=it.id
		elseif ru==2 then -- this was an undo so we are now at prev
			history.index=it.prev
		end
		
		return it
	end

	history.redo=function(history,id) -- go forward a step
		if not id then
			local it=history:get(history.index)
			id=it and it.next
		end
		if id then -- somewhere to go
			history:apply(id,1)
			return true
		end
	end

	history.undo=function(history) -- go back a step
		local it=history:get(history.index)
		if it and it.prev and history.list[it.prev] then -- somewhere to go
			history:apply(history.index,2)
			return true
		end
	end

	
	history.goto=function(history,index) -- goto this undo index
	
		-- this will work if we are on the right branch
		-- and destination is in the future
		while index>history.index do 
			if not history:redo() then break end
		end
		
		-- this will work if we are on the right branch
		-- and destination is in the past
		while index<history.index do
			if not history:undo() then break end
		end
		
		-- did not work so we need to find a common point in history
		if index~=history.index then -- need to find shared ancestor		

			local path={} -- the path to take to move forwards
			local check=index
			local searching=true
			while searching do

				while check and check>history.index do -- work our way backwards from the destination remembering the path
					table.insert(path,check)
					check=history:get_prev(check) -- step back in path
				end				

				while check and check<history.index do
					if not history:undo() then searching=false break end -- break if can go no further
				end
				
				if check and check==history.index then -- we can use the path from here
					searching=false -- we have found the path
					for i=#path,1,-1 do
						history:redo(path[i]) -- walk along the path
					end
				end
				
				if not check then searching=false end -- no path can be found
			end
			
		end

	end

	local saveme={
		"version",
		"start",
		"length",
		"list",
		"index",
		"width",
		"height",
		"json",
	}

	history.save=function(history)

		history:draw_end()

		local it={}
		
		for _,n in ipairs(saveme) do
			it[n]=history[n]
		end

		return deflate(cmsgpack.pack(it))
	end
	
	history.load=function(history,it)

		history:draw_end()
		
		local it=cmsgpack.unpack(inflate(it))

		for _,n in ipairs(saveme) do
			history[n]=it[n] or history[n]
		end
	end

-- reduce amount of memory used by undo
	history.reduce_memory=function(history,n)
		n=n or 1024*1024*64 -- 64meg default
		local total=0
		for i=history.length,history.start,-1 do
			local v=history.list[i]
			if v then
				total=total+#v
			end
			if total>=n then 
				history.list[i]=nil
				if i>=history.start then history.start=i+1 end
			end
		end
		history:get_memory()
	end

-- return amount of memory used by undo
	history.get_memory=function(history)
		local total=0
		local count=0
		local mini=0
		for i=history.length,history.start,-1 do
			local v=history.list[i]
			if v then
				total=total+#v
				count=count+1
				mini=i
			end
		end
		history.memory=total -- keep a running total
		return total,count,mini
	end

-- get the index of the parent of the given index
	history.get_prev=function(history,index)
		index=index or history.index -- use current if missing
		local it=history:get(index) -- unpack
		if it then
			return it.prev -- return index
		end		
	end

-- copy current json into a new or the given json table and return it
	history.get_json=function(history,it)
		return deepjson_copy(it,history.json)
	end


	return 	history:reset()
end


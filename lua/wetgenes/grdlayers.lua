--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.grdlayers

	local wgrdlayers=require("wetgenes.grdlayers")

We use wgrdlayers as the local name of this library.

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
local grdlayers=M



-- create a layers state within the given grd
-- layers are just a way of breaking one grd into discreet 2d areas
-- which can then be, optionally, recombined into a final image/anim
grdlayers.layers=function(grd)

	local layers={grd=grd}	
	grd.layers=layers

	layers.frame=0 -- keep track of frame
	layers.index=0 -- layer index starts at 1, 0 is a special index to mean all layers IE the full grd

	-- set layer config, default to entire frame.
	layers.config=function(layers,x,y,n)
		if n then
			if not x and not y then
				y=math.floor(math.sqrt(n))
			end
			if x and not y then
				y=math.ceil(n/x)
			end
			if y and not x then
				x=math.ceil(n/y)
			end
		end
		x=x or 1
		y=y or 1
		layers.x=x -- number of layers wide
		layers.y=y -- number of layers high
		layers.count=n or x*y -- total layers is optional
		
		return layers
	end

	-- get the size of each layer
	layers.size=function(layers)
		return math.floor(layers.grd.width/layers.x),math.floor(layers.grd.height/layers.y)
	end

-- a grd clip to area
	layers.clip=function(layers,idx,frame,grd)
		grd=grd or layers.grd -- optional other grd
		return grd:clip( layers:area(idx,frame) )
	end
	
	-- return a 3d clip area to get a single layer from the grd
	layers.area=function(layers,idx,frame)
		idx=idx or layers.index
		frame=frame or layers.frame
		if idx==0 then -- layer 0 is special case, full size
			return 0,0,frame, layers.grd.width,layers.grd.height,1
		end
		local lw,lh=layers:size()
		local lx=lw*math.floor((idx-1)%layers.x)
		local ly=lh*math.floor((idx-1)/layers.x)
		return  lx,ly,frame, lw,lh,1
	end

	-- get a new grd of all layers merged for every frame
	-- or just a single frame if requested
	layers.flatten_grd=function(layers)
		local grd=layers.grd
		local gd=grd.depth
		local lw,lh=layers:size()
		local g=wgrd.create(grd.format,lw,lh,gd) -- new size same depth
		if grd.cmap then -- copy palette
			g:palette(0,256,grd)
		end
		for z=0,gd-1 do
			for i=layers.count,1,-1 do
				g:clip(0,0,z,lw,lh,1):paint( grd:clip(layers:area(i,z)) ,0,0,0,0,lw,lh,wgrd.PAINT_MODE_ALPHA,-1,-1)
			end
		end
		return g
	end

	layers.flatten_frame=function(layers,frame,grd)
		frame=frame or layers.frame
		grd=grd or layers.grd -- can choose a temp grd or use default
		local lw,lh=layers:size()
		local g=wgrd.create(grd.format,lw,lh,1) -- new size one frame
		if grd.cmap then -- copy palette
			g:palette(0,256,grd)
		end
		for i=layers.count,1,-1 do
			g:clip(0,0,0,lw,lh,1):paint( grd:clip(layers:area(i,frame)) ,0,0,0,0,lw,lh,wgrd.PAINT_MODE_ALPHA,-1,-1)
		end
		return g
	end

-- rearrange layers, do not pass any values for auto layer layout
	layers.rearrange=function(layers,x,y,n)

		if not n then
			n=layers.count
		end
		if not x and not y then
			y=math.floor(math.sqrt(n))
		end
		if x and not y then
			y=math.ceil(n/x)
		end
		if y and not x then
			x=math.ceil(n/y)
		end
		if x*y<n then n=x*y end -- not big enough, set n to maximum

		local ga=layers.grd -- from
		local w,h=layers:size() -- get original layer size

		local gb=wgrd.create(ga.format,w*x,h*y,ga.depth) -- new image size
		if ga.cmap then -- copy palette
			gb:palette(0,256,ga)
		end
		
		grdlayers.layers(gb) -- we need to temp layers
		gb.layers:config(x,y,n) -- so we can now configure it

		for z=0,ga.depth-1 do -- for each frame
			for l=1,n do
				if l<=ga.layers.count and l<=gb.layers.count then -- we have a layer to copy
					gb.layers:clip(l,z):pixels(0,0,0,w,h,1,ga.layers:clip(l,z))
				end
			end
		end

		local i=layers.index -- save idx
		ga[0]=gb[0] -- transplant the core grd so we can keep the same table
		ga:info()
		layers:config(x,y,n) -- apply new configuration (resets idx)
		layers.index=i -- load idx

	end

-- add or remove a number of layers at the given index
	layers.adjust_layer_count=function(layers,layeridx,layernum)


		local ga=layers.grd -- from
		local w,h=layers:size() -- get original layer size

		local n=layers.count+layernum -- new number of layers
		local y=math.floor(math.sqrt(n)) -- with a simple layout
		local x=math.ceil(n/y)

		local gb=wgrd.create(ga.format,w*x,h*y,ga.depth) -- new image size
		if ga.cmap then -- copy palette
			gb:palette(0,256,ga)
		end
		
		grdlayers.layers(gb) -- we need to temp layers
		gb.layers:config(x,y,n) -- so we can now configure it

		for z=0,ga.depth-1 do -- for each frame
			for la=1,layers.count do
				local lb=la
				if la>=layeridx then
					lb=la+layernum
					if lb < layeridx then
						lb=nil -- do not copy this frame
					end
				end
				if lb then -- copy over layers
					gb.layers:clip(lb,z):pixels(0,0,0,w,h,1,ga.layers:clip(la,z))
				end
			end
		end

		local i=layers.index -- save idx
		ga[0]=gb[0] -- transplant the core grd so we can keep the same table
		ga:info()
		layers:config(x,y,n) -- apply new configuration (resets idx)
		layers.index=i -- load idx

		if layers.index>layeridx then layers.index=layers.index+layernum end -- adjust current layer index

	end

-- add or remove a number of frames at the given index
	layers.adjust_depth=function(layers,frameidx,framenum)
	
		local ga=layers.grd
		local gb=wgrd.create( ga.format , ga.width , ga.height , ga.depth+framenum ) -- the new grd size with adjusted depth

		for za=0,ga.depth-1 do
			local zb=za
			if za>=frameidx then
				zb=za+framenum
				if zb < frameidx then
					zb=nil -- do not copy this frame
				end
			end
			if zb then -- copy over frames
				gb:pixels( 0,0,zb , gb.width,gb.height,1 , ga:clip(0,0,za,ga.width,ga.height,1) )
			end
		end
		
		if ga.cmap then -- copy palette
			gb:palette(0,256,ga)
		end

		ga[0]=gb[0] -- transplant the core grd so we can keep the same table
		ga:info()
		
		if layers.frame>frameidx then layers.frame=layers.frame+framenum end -- adjust current frame
	
	end

-- swap layers and frames
	layers.swap_with_frames=function(layers)

		local ga=layers.grd
		local w,h=layers:size() -- get original layer size
		local n=ga.depth -- turn frames into layers
		local y=math.floor(math.sqrt(n)) -- with a simple layout
		local x=math.ceil(n/y)
		local gb=wgrd.create(ga.format,w*x,h*y,ga.layers.count) -- to
		if ga.cmap then -- copy palette
			gb:palette(0,256,ga)
		end

		grdlayers.layers(gb) -- we need to temp layers
		gb.layers:config(x,y,n) -- so we can now configure it

		for l=0,ga.layers.count-1 do
			for f=0,n-1 do
				gb.layers:clip(f+1,l):pixels(0,0,0,w,h,1,ga.layers:clip(l+1,f))
			end
		end

		ga[0]=gb[0] -- transplant the grd from gb into ga
		ga:info()
		layers:config(x,y,n) -- apply new configuration

	end

	
-- change the size of each layer
	layers.adjust_layer_size=function(layers,width,height,anchor_x,anchor_y)
	
		local ga=layers.grd -- from
		local gb=wgrd.create(ga.format,width*layers.x,height*layers.y,ga.depth) -- to
		if ga.cmap then -- copy palette
			gb:palette(0,256,ga)
		end
				
		grdlayers.layers(gb) -- we need to temp layers
		gb.layers:config(layers.x,layers.y,layers.n) -- so we can now configure it

		local wa,ha=layers:size() -- original size
		local w,xa,xb=anchor_helper(wa,width, anchor_x)
		local h,ya,yb=anchor_helper(ha,height,anchor_y)
		
		for z=0,ga.depth-1 do
			for l=1,ga.layers.count do
				local ca=ga.layers:clip(l,z)
				local cb=gb.layers:clip(l,z)
				cb:pixels(xb,yb,0,w,h,1,ca:clip(xa,ya,0,w,h,1))
			end
		end

		ga[0]=gb[0] -- transplant the grd from gb into ga
		ga:info()

	end

-- change the size of the entire image, layer 0
	layers.adjust_size=function(layers,width,height,anchor_x,anchor_y)

		local ga=layers.grd -- from
		local gb=wgrd.create(ga.format,width,height,ga.depth) -- to
		if ga.cmap then -- copy palette
			gb:palette(0,256,ga)
		end
				
		local w,xa,xb=anchor_helper(ga.width, gb.width, anchor_x)
		local h,ya,yb=anchor_helper(ga.height,gb.height,anchor_y)
		
		for z=0,ga.depth-1 do
			gb:pixels(xb,yb,z,w,h,1,ga:clip(xa,ya,z,w,h,1))		
		end

		ga[0]=gb[0] -- transplant the grd from gb into ga
		ga:info()
		
	end
	
	return layers:config()
end



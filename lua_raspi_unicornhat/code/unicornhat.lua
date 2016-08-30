--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")

local core=require("raspi.unicornhat.core")

local unicornhat={}

local map={
        {7 ,6 ,5 ,4 ,3 ,2 ,1 ,0 },
        {8 ,9 ,10,11,12,13,14,15},
        {23,22,21,20,19,18,17,16},
        {24,25,26,27,28,29,30,31},
        {39,38,37,36,35,34,33,32},
        {40,41,42,43,44,45,46,47},
        {55,54,53,52,51,50,49,48},
        {56,57,58,59,60,61,62,63}
}

unicornhat.map=map

-- we can only be setup once, so keep track here
local active=nil

-- this will be called automatically when you call the other functions
unicornhat.setup=function()

	if not active then
		if core.create() then
			active=pack.alloc(1,{__gc=function() unicornhat.clean() end})
		end
	end
	
end

-- this will be called automatically on exit/garbage collect
unicornhat.clean=function()

	if active then
		core.destroy()
		active=nil
	end

end

-- set the brightness, 0-255 but be aware that large values can hurt your eyes
-- its also probably best to set this to 255 then do your own brightness/gamma
-- in the provided rgb values
unicornhat.brightness=function(b)
	unicornhat.setup()
	return core.brightness(b)
end

-- set a pixel rgb value from 0-255 this function will clamp out of range values
unicornhat.pixel=function(x,y,r,g,b)
	unicornhat.setup()
	if r<0 then r=0 elseif r>255 then r=255 end
	if g<0 then g=0 elseif g>255 then g=255 end
	if b<0 then b=0 elseif b>255 then b=255 end
	return core.pixel(unicornhat.map[x][y],r,g,b)
end

-- like pixel but will set all of the pixels to the same rgb value
-- so to turn lights off use clear(0,0,0)
unicornhat.clear=function(r,g,b)
	unicornhat.setup()
	if r<0 then r=0 elseif r>255 then r=255 end
	if g<0 then g=0 elseif g>255 then g=255 end
	if b<0 then b=0 elseif b>255 then b=255 end
	return core.clear(r,g,b)
end

-- show the current frame buffer, no visible change will be made until you call this
unicornhat.show=function()
	unicornhat.setup()
	return core.show()
end

-- set all colors from a grd (image)
-- *must* be an rgb(a) 8x8 grd
unicornhat.grd=function(g)
	for x=0,7 do
		for y=0,7 do
			local c=g:pixels(x,y,1,1)
			core.pixel(unicornhat.map[x][y],c[1],c[2],c[3])
		end
	end
end

unicornhat.test=function()

	for i=255,0,-1 do

		unicornhat.brightness(i)

		for x=0,7 do
			for y=0,7 do
				local r,g,b
				r=255-(x*32)
				g=0
				b=0
				unicornhat.pixel(x+1,y+1,r,g,b)
			end
		end

		unicornhat.show()
	
	end

end



return unicornhat

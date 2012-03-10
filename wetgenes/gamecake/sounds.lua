-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.sounds")

base=require(...)
meta={}
meta.__index=base

local grd=require("wetgenes.grd")


--[[
local al=require("al")
local alc=require("alc")

	
	local dc=alc.setup()
	
--	alc.test()-- test junk

	local data="00000000zzzzzzzz" -- fake test sample data should be squarewave ishhh
	local sd=sod.create():load("dat/sod/t2.wav")

--print(sd)

	al.Listener(al.POSITION, 0, 0, 0)
	al.Listener(al.VELOCITY, 0, 0, 0)
	al.Listener(al.ORIENTATION, 0, 0, -1, 0,1,0 )

	local source=al.GenSource()

	al.Source(source, al.PITCH, 1)
	al.Source(source, al.GAIN, 1)
	al.Source(source, al.POSITION, 0, 0, 0)
	al.Source(source, al.VELOCITY, 0, 0, 0)
	al.Source(source, al.LOOPING, al.FALSE)

	local buffer=al.GenBuffer()

--	al.BufferData(buffer,al.FORMAT_MONO16,data,#data,261.626*8) -- C4 hopefully?
	al.BufferData(buffer,sd) -- all loaded

	al.Source(source, al.BUFFER, buffer)
	al.Source(source, al.LOOPING,al.TRUE)

	al.SourcePlay(source)
	require("socket").sleep(2)
	
	al.CheckError()

	al.DeleteSource(source)
	al.DeleteBuffer(buffer)
	
	dc:clean() -- should really clean up when finished


]]


function bake(opts)

	local sounds={}
	setmetatable(sounds,meta)
	
	sounds.cake=opts.cake
	sounds.al=opts.al
	sounds.alc=opts.alc
	
	sounds.data={}
	
	sounds.zip=opts.zip
	sounds.prefix=opts.prefix or "data"
	sounds.postfix=opts.postfix or ".wav"
	
	return sounds
end

setup=function(sounds)
	sounds.al=sounds.al or require("al")
	sounds.alc=sounds.alc or require("alc")
	local al=sounds.al
	local alc=sounds.alc
	
	sounds.context=alc.setup()
	
	al.Listener(al.POSITION, 0, 0, 0)
	al.Listener(al.VELOCITY, 0, 0, 0)
	al.Listener(al.ORIENTATION, 0, 0, -1, 0,1,0 )
	
	sounds.sources={}
	sounds.sources[1]=al.GenSource()

	al.Source(sounds.sources[1], al.PITCH, 1)
	al.Source(sounds.sources[1], al.GAIN, 1)
	al.Source(sounds.sources[1], al.POSITION, 0, 0, 0)
	al.Source(sounds.sources[1], al.VELOCITY, 0, 0, 0)
	al.Source(sounds.sources[1], al.LOOPING, al.FALSE)
	
	sounds.buffers={}
	sounds.buffers[1]=al.GenBuffer()

	local d="00000000zzzzzzzz" -- fake test sample data should be squarewave ishhh
	al.BufferData(sounds.buffers[1],al.FORMAT_MONO16,d,#d,261.626*8) -- C4 hopefully?

	al.Source(sounds.sources[1], al.BUFFER, sounds.buffers[1])
	al.Source(sounds.sources[1], al.LOOPING,al.TRUE)

end

clean=function(sounds)
	sounds.al=sounds.al
	sounds.alc=sounds.alc

	al.DeleteSource(sounds.sources[1])
	al.DeleteBuffer(sounds.buffers[1])

	context:clean()
end
	
	
get=function(sounds,id,name)
	name=name or "base"
	return sounds.data[name] and sounds.data[name][id]
end

set=function(sounds,d,id,name)
	name=name or "base"
	local tab
	
	if sounds.data[name] then
		tab=sounds.data[name]		
	else
		tab={}
		sounds.data[name]=tab
	end
	
	tab[id]=d	
end

beep=function(sounds,id,name)
	local al=sounds.al

	al.SourcePlay(sounds.sources[1])

end

--
-- unload a previously loaded image
--
unload=function(sounds,id,name)
	local al=sounds.al
	local alc=sounds.alc
	name=name or "base"
	
	local t=sounds:get(id,name)

	if t then
		if al then --gl mode
--				gl.DeleteTexture( t.id )			
		end
		sounds:set(nil,id,name)
	end
end

--
-- load a single image, and make it easy to lookup by the given id
--
load=function(sounds,filename,id,name)
	local al=sounds.al
	name=name or "base"

	local t=sounds:get(id,name)
	
	if t then return t end --first check it is not already loaded


	local fname=sounds.prefix..filename..sounds.postfix
	
--	local g=assert(grd.create())
	
	if sounds.zip then -- load from a zip file
		
--		local f=assert(sounds.zip:open(fname))
--		local d=assert(f:read("*a"))
--		f:close()

--		assert(g:load_data(d,"png"))
	else
--		assert(g:load_file(fname,"png"))
	end
	
	if al then --al mode
	
		sounds:set(t,id,name)

-- do the loading
		
		t.filename=filename
		return t
	else
	
		return nil
	end
	
end

--
-- load many sounds from id=filename table
--
loads=function(sounds,tab)

	for i,v in pairs(tab) do
	
		if type(v)=="table" then -- use a subtable and its name
		
			for ii,vv in pairs(v) do
			
				if type(ii)=="number" then -- just use filename twice
					sounds:load(i.."_"..vv,vv,i)
				else
					sounds:load(i.."_"..vv,ii,i)
				end
				
			end
			
		elseif type(i)=="number" then -- just use filename twice
			sounds:load(v,v)
		else
			sounds:load(v,i)
		end
		
	end

end


start = function(sounds)

	for v,n in pairs(sounds.remember or {}) do
		sounds:load(v.filename,n[1],n[2])
	end
	sounds.remember=nil
end

stop = function(sounds)

	sounds.remember={}
	
	for n,tab in pairs(sounds.data) do

		for i,t in pairs(tab) do
		
			sounds.remember[t]={i,n}
		
			sounds:unload(i,n)
			
		end

	end

end



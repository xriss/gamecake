-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.sounds")

local zips=require("wetgenes.zips")

local grd=require("wetgenes.grd")
local sod=require("wetgenes.sod")


function bake(opts)

	local sounds={}
	
	sounds.cake=opts.cake
	
	sounds.al=opts.al
	sounds.alc=opts.alc
	
	local cake=sounds.cake
	local al=sounds.al
	local alc=sounds.alc
	
	sounds.data={}
	
	sounds.zip=opts.zip
	sounds.prefix=opts.sodprefix or "data/"
	sounds.postfix=opts.sodpostfix or ".wav"
	

sounds.setup=function()

	sounds.al=sounds.al or require("al")
	sounds.alc=sounds.alc or require("alc")

-- copy into locals
	al=sounds.al
	alc=sounds.alc

	sounds.start()

end

sounds.clean=function()
	sounds.stop()
end

sounds.get=function(id)
	return sounds.data[id]
end

sounds.set=function(d,id)
	sounds.data[id]=d
end

sounds.beep_idx=1
sounds.beep=function(d)
	
	al.SourceStop(sounds.sources[sounds.beep_idx])

	al.Source(sounds.sources[sounds.beep_idx], al.BUFFER, d.buff)
	al.Source(sounds.sources[sounds.beep_idx], al.LOOPING, d.loop)

	al.SourcePlay(sounds.sources[sounds.beep_idx])

	sounds.beep_idx=sounds.beep_idx+1
	if sounds.beep_idx > #sounds.sources then sounds.beep_idx=1 end
end

--
-- unload a previously loaded image
--
sounds.unload=function(id)
	
	local t=sounds.get(id)

	if t then
		sounds.set(nil,id)
	end
end


--
-- pre bake some speech, and make it easy to lookup by the given ids
--
sounds.load_speak=function(tab,id)
	if type(tab)=="string" then tab={text=tab} end -- default options

	name=name or "base"
	local t=sounds.get(id)
	if t then return t end --first check it is not already loaded
	
	t={}
	t.filename=tab
	
	t.loop=al.FALSE
	
	t.buff=al.GenBuffer()

	local dat,len=require("wetgenes.speak.core").test(tab.text)
	al.BufferData(t.buff,al.FORMAT_MONO16,dat,len,261.626*8*8) -- C4 hopefully?
	
	sounds.set(t,id) -- remember

end

--
-- load a single sound, and make it easy to lookup by the given id
--
sounds.load=function(filename,id)

	local t=sounds.get(id)
	
	if t then return t end --first check it is not already loaded

	local fname=sounds.prefix..filename..sounds.postfix
	
	local x=assert(sod.create())
	
	local d=assert(zips.readfile(fname))
	assert(x:load_data(d,"wav"))
	t={}
	t.filename=filename
	
	if al then --al mode
	
		t.loop=al.FALSE
		
		t.buff=al.GenBuffer()
		al.BufferData(t.buff,x) -- all loaded
		
		sounds.set(t,id) -- remember

print("loaded",filename)		
		return t
		
	else
	
		return nil
	end
	
end

--
-- load many sounds from id=filename table
--
sounds.loads=function(tab)

	for i,v in pairs(tab) do
	
		if type(i)=="number" then -- just use filename twice
			sounds.load(v,v)
		else
			sounds.load(v,i)
		end
		
	end

end


sounds.start = function()

	if not sounds.context then
	
		sounds.context=alc.setup()
		
		al.Listener(al.POSITION, 0, 0, 0)
		al.Listener(al.VELOCITY, 0, 0, 0)
		al.Listener(al.ORIENTATION, 0, 0, -1, 0,1,0 )
		
		sounds.sources={}
		for i=1,4 do
			sounds.sources[i]=al.GenSource()
			local s=sounds.sources[i]
			al.Source(s, al.PITCH, 1)
			al.Source(s, al.GAIN, 1)
			al.Source(s, al.POSITION, 0, 0, 0)
			al.Source(s, al.VELOCITY, 0, 0, 0)
			al.Source(s, al.LOOPING, al.FALSE)
		end


		for v,n in pairs(sounds.remember or {}) do
			if type(v)=="table" then
				sounds.load_speak(v,n)
			else
				sounds.load(v,n)
			end
		end
		sounds.remember=nil
	end
end

sounds.stop = function()

	sounds.remember={}
	
	for n,t in pairs(sounds.data) do
		
		sounds.remember[t.filename]=n		
		sounds.unload(n)

	end

	for i,v in pairs(sounds.sources) do
		al.DeleteSource(v)
	end
	sounds.sources={}

	sounds.context:clean()

	sounds.context=nil
end

	if sounds.cake.opts.disable_sounds then -- disable all function in this file
		for n,v in pairs(sounds) do
			if type(v)=="function" then
				sounds[n]=function() end
			end
		end
	end

	return sounds
end



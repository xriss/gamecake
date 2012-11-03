-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.sounds")

local zips=require("wetgenes.zips")

local grd=require("wetgenes.grd")
local sod=require("wetgenes.sod")


function bake(opts)

	local sounds={}

	local str_func={}
	local str_meta={__index=str_func}
	
	local sfx_func={}
	local sfx_meta={__index=sfx_func}
	
	local sfxmax=opts.sfxmax or 4
	local strmax=opts.strmax or 2
	
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

	local sfx=sounds.sfxs[sounds.beep_idx]
	
	al.SourceStop(sfx.source)

	al.Source(sfx.source, al.BUFFER, d.buff)
	al.Source(sfx.source, al.LOOPING, d.loop)

	al.SourcePlay(sfx.source)

	sounds.beep_idx=sounds.beep_idx+1
	if sounds.beep_idx > sfxmax then sounds.beep_idx=1 end
end

sounds.queue_talk=function(d)

	local str=sounds.strs[1]

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

-- one off sound effect type things		
		sounds.sfxs={}
		for i=1,sfxmax+strmax do
			local sfx={}
			sounds.sfxs[i]=sfx
			local s=al.GenSource()
			sfx.source=s
			al.Source(s, al.PITCH, 1)
			al.Source(s, al.GAIN, 1)
			al.Source(s, al.POSITION, 0, 0, 0)
			al.Source(s, al.VELOCITY, 0, 0, 0)
			al.Source(s, al.LOOPING, al.FALSE)
			setmetatable(sfx,sfx_meta)
		end
		
-- streaming music type things
		sounds.strs={}
		for i=1,strmax do
			local str={}
			sounds.strs[i]=str
			local s=al.GenSource()
			str.source=s
			al.Source(s, al.PITCH, 1)
			al.Source(s, al.GAIN, 1)
			al.Source(s, al.POSITION, 0, 0, 0)
			al.Source(s, al.VELOCITY, 0, 0, 0)
			al.Source(s, al.LOOPING, al.FALSE)

			str.buffers={al.GenBuffer(),al.GenBuffer()} -- double buffer sound renderer
			str.full={} -- these buffers are full and queued
			str.empty={} -- these buffers are empty and waiting to be queued
			for i,v in ipairs(str.buffers) do str.empty[#str.empty+1]=v end
			
			str.state="none"
			
			setmetatable(str,str_meta)
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

	for i,v in pairs(sounds.sfxs) do
		al.DeleteSource(v.src)
	end
	sounds.sfxs={}
	for i,v in pairs(sounds.strs) do
		al.DeleteSource(v.src)
	end
	sounds.strs={}

	sounds.context:clean()

	sounds.context=nil
end


sounds.update = function()

	for i,v in ipairs(sounds.strs) do
		v:update()
	end

end


function str_func.fill(str,b)
	if str.talks and str.talks[1] then
		local t=table.remove(str.talks,1)
		local dat,len=require("wetgenes.speak.core").test(t)
		al.BufferData(b,al.FORMAT_MONO16,dat,len,261.626*8*8) -- C4 hopefully?
		return true
	end
end

-- default stream update func
function str_func.update(str)

-- remove finished buffers
	local processed=al.GetSource(str.source,al.BUFFERS_PROCESSED)
	for i=1,processed do
		local b=al.SourceUnqueueBuffer(str.source)
		local idx
		for i,v in ipairs(str.full) do -- find and remove, it should be the first one.
			if v==b then idx=i break end
		end
		assert(idx)
		table.remove(str.full,idx)
		table.insert(str.empty,b)
--print("unqueue ",b,idx)
	end

	while str.empty[1] do -- fill the empty queue
		local b=str.empty[1]
		if str:fill(b) then
			al.SourceQueueBuffer(str.source,b)			
			table.remove(str.empty,1)
			table.insert(str.full,b)
--			print("queue ",b)
		else
			break
		end
	end
	
	if str.state=="play_queue" and str.full[1] then -- start to play wenever we have a buffer filled
		str.state="play"
		al.SourcePlay(str.source)
	end

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



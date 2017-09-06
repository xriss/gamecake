--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local print=function(...) return _G.print(...) end

local wpack=require("wetgenes.pack")
local zips=require("wetgenes.zips")
local grd=require("wetgenes.grd")
local sod=require("wetgenes.sod")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,sounds)

	local str_func={}
	local str_meta={__index=str_func}
	
	local sfx_func={}
	local sfx_meta={__index=sfx_func}
	
	local opts=oven.opts
	local cake=oven.cake

--opts.disable_sounds=true

	local sfxmax=opts.sfxmax or 4
	local strmax=opts.strmax or 2
		
-- probably nil but we may need to override?
	sounds.al=opts.al
	sounds.alc=opts.alc	
	local al=sounds.al
	local alc=sounds.alc
	
	sounds.vol_stream=1
	sounds.vol_beep=1
	
	sounds.data={}
	
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
sounds.beep_max=sfxmax
sounds.beep=function(d)

	local inc=true
	local sfx=sounds.sfxs[sounds.beep_idx]
	if d.idx then sfx=sounds.sfxs[d.idx] inc=false end
	
	al.SourceStop(sfx.source)

	al.Source(sfx.source, al.BUFFER, d.buff)
	al.Source(sfx.source, al.LOOPING, d.loop or al.FALSE)

	al.Source(sfx.source, al.PITCH, (d.pitch or 1) )
	al.Source(sfx.source, al.GAIN, sounds.vol_beep * (d.gain or 1) )

	al.SourcePlay(sfx.source)

	if inc then
--print("SFX",sounds.beep_idx,d.name)
		sounds.beep_idx=sounds.beep_idx+1
		if sounds.beep_idx > sounds.beep_max then sounds.beep_idx=1 end
--	else
--print("SFI",d.idx,d.name)
	end
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
-- load a generated sound, replace any other sound already loaded with this name
--
sounds.load_wavtab=function(tab,id,freq)

	local t=sounds.get(id)

-- this is generated sounds so allow changes 
--	if t then return t end --first check it is not already loaded
	
	t=t or {}
	t.filename=tab
	
	t.loop=al.FALSE
	
	if t.buff then al.DeleteBuffer(t.buff) end -- free old buffer
	t.buff=al.GenBuffer()

	local dat,len=wpack.save_array(tab,"s16")
	al.BufferData(t.buff,al.FORMAT_MONO16,dat,len,freq or 48000) -- C4 hopefully?
	
	sounds.set(t,id) -- remember

	return t
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

oven.preloader("speak",id)

	return t
end

--
-- Load an ogg for use in a soundeffect so read it all in and then push it over to opengl
--
sounds.load_ogg=function(filename,id)

	local t=sounds.get(id)
	
	if t then return t end --first check it is not already loaded

	local fname=sounds.prefix..filename..".ogg"
	
	local d=assert(zips.readfile(fname))

	t={}
	t.filename=filename
	

--do return false end	
		
	local ogg=require("wetgenes.ogg")
	local og=ogg.create()
	og:open()
	
	local rr={}
	repeat
		local done=false
		local r=og:pull()
		if not r then
			if og.err=="push" then
				og:push(d)
			elseif og.err=="end" then done=true
			elseif og.err then error( og.err ) end
		else
			rr[#rr+1]=r
			if og.err=="end" then done=true
			elseif og.err then error( og.err ) end
		end
		
	until done


	local fmt=al.FORMAT_MONO16
	if og.channels==2 then fmt=al.FORMAT_STEREO16 end
	local rate=og.rate

	t.loop=al.FALSE		
	t.buff=al.GenBuffer()

	local r=table.concat(rr)
	al.BufferData(t.buff,fmt,r,#r,rate)

	sounds.set(t,id) -- remember

--print("load",filename)
--print(#rr,"chunks",#table.concat(rr))

--[[
if filename=="oggs/munch" then
	print(filename)
	print("in",#d)
	print("buffs",#rr)
	print("out",#r)

exit(0)
end
]]

	og:close()

--print("loaded ogg",filename)
oven.preloader("ogg",filename)

	return t
end

--
-- load a single sound, and make it easy to lookup by the given id
--
sounds.load=function(filename,id)

	local t=sounds.get(id)
	
	if t then return t end --first check it is not already loaded

	if zips.exists(sounds.prefix..filename..".ogg") then return sounds.load_ogg(filename,id) end

	local fname=sounds.prefix..filename..sounds.postfix
	
	local x=assert(sod.create())
	
	local d=assert(zips.readfile(fname),fname)
	assert(x:load_data(d,"wav"))
	t={}
	t.filename=filename
	

	t.loop=al.FALSE
	
	t.buff=al.GenBuffer()
	al.BufferData(t.buff,x) -- all loaded
	
	sounds.set(t,id) -- remember

--print("loaded",filename)
oven.preloader("wav",filename)

	return t
	
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
--		al.Listener(al.VELOCITY, 0, 0, 0)
		al.Listener(al.ORIENTATION, 0, 0, -1, 0,1,0 )

-- one off sound effect type things		
		sounds.sfxs={}
		for i=1,sfxmax do
			local sfx={idx=i}
			sounds.sfxs[i]=sfx
			local s=al.GenSource()
			sfx.source=s
			al.Source(s, al.PITCH, 1)
			al.Source(s, al.GAIN, 1)
			al.Source(s, al.POSITION, 0, 0, 0)
--			al.Source(s, al.VELOCITY, 0, 0, 0)
			al.Source(s, al.LOOPING, al.FALSE)
			setmetatable(sfx,sfx_meta)
		end
		
-- streaming music type things
		sounds.strs={}
		for i=1,strmax do
			local str={idx=i}
			sounds.strs[i]=str
			local s=al.GenSource()
			str.source=s
			al.Source(s, al.PITCH, 1)
			al.Source(s, al.GAIN, 1)
			al.Source(s, al.POSITION, 0, 0, 0)
--			al.Source(s, al.VELOCITY, 0, 0, 0)
			al.Source(s, al.LOOPING, al.FALSE)

			str.buffers={al.GenBuffer(),al.GenBuffer(),al.GenBuffer()} -- triple buffer sound renderer
			str.full={} -- these buffers are full and queued
			str.empty={} -- these buffers are empty and waiting to be queued
			for i,v in ipairs(str.buffers) do str.empty[#str.empty+1]=v end
			
			setmetatable(str,str_meta)
		end

-- queues contain data that is kept valid between stop/starts
-- this way we can kind of keep playing at more or less the same point in a stream
		if not sounds.queues then
			sounds.queues={}
			for i=1,strmax do
				local r={idx=i}
				sounds.queues[i]=r

				r.push=function(qq,t)
					if not t then -- pass in an empty table if you just want it filled
						t={}
						qq.stack[#qq.stack+1]=t
					end
						
					for i,v in pairs(qq) do -- copy state into stack
						t[i]=v
					end

					return t
				end
				r.pop=function(qq,t)
					local t=t or qq.stack[#qq.stack] -- from the top of the stack or a given table
					if not t then return end -- ignore bad pops
					qq.stack[#qq]=nil
					
					for i,v in pairs(qq) do -- clear current state
						qq[i]=nil
					end
					for i,v in pairs(t) do -- copy state from stack
						qq[i]=v
					end

				end
				r.clear=function(qq)
					qq.stack={}
				end
				r:clear()

				r.stream_ogg=function(qq,d)
					local str=sounds.strs[qq.idx]
					
					if d.mode=="stop" then
					
						qq.state=nil
						qq.ogg_loop=false
						qq.og=false
						qq.oggs=nil
						
					else
						
						qq.ogg_loop=true
						qq.state="play_queue"
						if not qq.oggs then
							qq.oggs={d.name}
						end
						if d.mode=="restart" then -- force a restart of the ogg
							qq.og=false
							qq.oggs={d.name}
							al.SourceStop(str.source)
						else -- otherwise continue playing what we already are if we can
							if #qq.oggs==1 and qq.oggs[1]==d.name then -- nothing to change
							else
								qq.og=false
								qq.oggs={d.name}
								al.SourceStop(str.source)
							end
						end
					end
				end
			end
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
		al.DeleteSource(v.source)
	end
	sounds.sfxs={}
	for i,v in pairs(sounds.strs) do
		al.DeleteSource(v.source)
	end
	sounds.strs={}

	sounds.context:clean()

	sounds.context=nil
end


sounds.update = function()

	if not oven.focus then return end -- only play streams when focused

	for i,v in ipairs(sounds.strs) do
		v:update()
	end

end

local dbgfp

function str_func.fill(str,b)

local qq=sounds.queues[str.idx]

	if qq.talks and qq.talks[1] then
	
--	al.Source(str.source, al.GAIN, 1)
	
		local wspeak=require("wetgenes.speak")
		local voice=qq.voice or {} -- make sure we have a voide

		local t=table.remove(qq.talks,1)
		wspeak.voice(voice)
		local dat,len=wspeak.text(t)
		al.BufferData(b,al.FORMAT_MONO16,dat,len,0x4000 )--this depends on the voice used
		qq.pitch=voice.pitch -- use the pitch from the voice
		return true
		
	elseif qq.oggs then
--do return false end	
		
		if not qq.og and qq.oggs[1] then
			local ogg=require("wetgenes.ogg")
			local fnam=table.remove(qq.oggs,1)
			if qq.fname~=fnam then -- need reload only if the off changes?
-- streaming from within a zip seems to fuckup, possibly having multiple files of a zip open is the problem?
-- reading it all in at once fixed this			
				qq.fname=fnam
				qq.fpdat=zips.readfile("data/"..qq.fname..".ogg")
			end
			qq.fpidx=1
			qq.fpsiz=#qq.fpdat
			qq.og=ogg.create()
			qq.og:open()
		end
		
		if qq.og then
--			local rr
			local function save(f)
				if qq.rr then
--print("buff",#qq.rr)
					local fmt=al.FORMAT_MONO16
					if qq.og.channels==2 then fmt=al.FORMAT_STEREO16 end
					local rate=qq.og.rate
					if qq.BufferData then -- special munge callback function
						qq.BufferData(b,fmt,qq.rr,#qq.rr,rate) -- C4 hopefully?
					else
						al.BufferData(b,fmt,qq.rr,#qq.rr,rate) -- C4 hopefully?
					end
					qq.rr=nil
				end
			end
			local function done(f)
--				save() -- save what we have
				if qq.ogg_loop then
					qq.oggs[#qq.oggs+1]=qq.fname -- insert ogg back into the end of the list
				end
				qq.og:close()
				qq.og=nil -- flag end of file
				return f
			end
			for i=1,128 do -- may take a few loops before we can return any data
				local r=qq.og:pull()
				if not r then
					if qq.og.err=="push" or qq.og.err=="end" then
--print(qq.fpidx , #qq.fpdat , qq.og.err)
						if qq.fpidx<#qq.fpdat then -- not the end, squirt some more data in
							local dat=string.sub(qq.fpdat,qq.fpidx,qq.fpidx+4096-1)
							qq.fpidx=qq.fpidx+4096
							qq.og:push(dat)
						elseif qq.og.err=="end" then -- really really the end
							return done(false)
						end
					elseif qq.og.err then error( qq.og.err ) end
				else
--print(#r,qq.og.err)
					if not qq.rr then qq.rr=r else qq.rr=qq.rr..r end

					if #qq.rr>=4096*8 then -- prefer a reasonable chunk of data
						save()
						if qq.og.err then error( qq.og.err ) end
						return true
					end
					
				end
			end
		end

	end
end

-- default stream update func
function str_func.update(str)
local qq=sounds.queues[str.idx]

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
--print("queue ",b)
--al.CheckError()
			table.remove(str.empty,1)
			table.insert(str.full,b)
		else
			break
		end
	end
	
	if qq.state=="play_queue" and str.full[1] then -- start to play whenever we have a buffer filled
		local astate=al.GetSource(str.source, al.SOURCE_STATE)
		if astate ~= al.PLAYING then
--print("astate",astate)
			al.SourceStop(str.source)
			al.SourcePlay(str.source)
		end
	end

--update every frame
	al.Source(str.source, al.GAIN, (qq.gain or 1) * sounds.vol_stream )
	al.Source(str.source, al.PITCH, qq.pitch or 1)

end



	if opts.disable_sounds then -- disable all function in this file
		for n,v in pairs(sounds) do
			if type(v)=="function" then
				sounds[n]=function() end
			end
		end
		sounds.queues={{},{}}
		sounds.disabled=true
	end

	return sounds
end



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
	sounds.prefix=opts.sodprefix or "data/sfx_"
	sounds.postfix=opts.sodpostfix or ".wav"
	

sounds.setup=function()
	sounds.al=sounds.al or require("al")
	sounds.alc=sounds.alc or require("alc")

	sounds.start()

end

sounds.clean=function()
	sounds.stop()
end

sounds.get=function(id,name)
	name=name or "base"
	return sounds.data[name] and sounds.data[name][id]
end

sounds.set=function(d,id,name)
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

sounds.beep=function(d)
	
	al.Source(sounds.sources[1], al.BUFFER, d.buff)
	al.Source(sounds.sources[1], al.LOOPING, d.loop)

	al.SourcePlay(sounds.sources[1])

end

--
-- unload a previously loaded image
--
sounds.unload=function(id,name)

	name=name or "base"
	
	local t=sounds.get(id,name)

	if t then
		if al then --gl mode
--				gl.DeleteTexture( t.id )			
		end
		sounds.set(nil,id,name)
	end
end

--
-- load a single image, and make it easy to lookup by the given id
--
sounds.load=function(filename,id,name)
	if sounds.cake.opts.disable_sounds then
		return
	end

	name=name or "base"

	local t=sounds.get(id,name)
	
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
		
		sounds.set(t,id,name) -- remember

print("loaded",id,name,filename)		
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
	
		if type(v)=="table" then -- use a subtable and its name
		
			for ii,vv in pairs(v) do
			
				if type(ii)=="number" then -- just use filename twice
					sounds.load(i.."_"..vv,vv,i)
				else
					sounds.load(i.."_"..vv,ii,i)
				end
				
			end
			
		elseif type(i)=="number" then -- just use filename twice
			sounds.load(v,v)
		else
			sounds.load(v,i)
		end
		
	end

end


sounds.start = function()

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
		sounds.load(v,n[1],n[2])
	end
	sounds.remember=nil
end

sounds.stop = function()

	sounds.remember={}
	
	for n,tab in pairs(sounds.data) do

		for i,t in pairs(tab) do
		
			sounds.remember[t.filename]={i,n}
		
			sounds.unload(i,n)
			
		end

	end

	for i,v in pairs(sounds.sources) do
		al.DeleteSource(v)
	end
	sounds.sources={}

	sounds.context:clean()

end


	return sounds
end



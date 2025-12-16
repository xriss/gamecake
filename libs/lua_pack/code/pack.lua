--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local pack={}

local core=require("wetgenes.pack.core")
pack.core=core

local wstr=require("wetgenes.string")

local _,bit=pcall(function() return require("bit") end) ; bit=(_ and bit) or require("bit32")

--
-- Read a single member and return it
--
pack.read=function(dats,fmt,off)
	local datt,len=core.load(dats,{fmt},off)
	return datt and datt[1]
end

--
-- Read an array of the same type
--
pack.load_array=function(dats,fmt,off,size)
	return core.load(dats,fmt,off,size and size+(off or 0))
end

--
-- write an array of the same type
--
pack.save_array=function(dats,fmt,off,count,buff)
	return core.save(dats,fmt,off,buff)
end


-- raw access to the core load/save functions
pack.load_raw=function(...) return core.load(...) end
pack.save_raw=function(...) return core.save(...) end

--
-- wrap the core functions with easier to use utility code
--
-- format is "u32","name",...
-- where "u32" is a string id of a data type or the length of a string to read
-- name is the name of the table field to return the data in
--
-- we return a table and the length of the data read in bytes
-- 
--
pack.load=function(dats,fmts,off)

	local fmtt={} -- format field type
	local fmtn={} -- format field name
	
	local len=0
	for i=1,#fmts,2 do -- deinterlace
		local vt=fmts[i]
		local vn=fmts[i+1]
		
		len=len+1
		
		fmtt[len]=vt -- break input into two tables type and name
		fmtn[len]=vn
		
	end

-- parse the data	
	local datt,len=core.load(dats,fmtt,off)

-- now we assign the fields to their given names

	local datr={}
	for i,v in ipairs(fmtn) do
		datr[v]=datt[i]
	end

	return datr,len -- we return the parsed data and the length of the data we just read in bytes
end

--
-- the reverse of load
-- we return a string and the length of the data written in bytes, ie the length of the string
--
pack.save=function(data,fmts,off)

	if data and (not fmts or type(fmts)=="number") then -- interleaved save  eg {"u8",24,"s32",16}
		off=fmts fmts=nil -- offset is in wrong var, fix it
		local d,f={},{}
		for i=1,#data,2 do
			f[#f+1]=data[i]
			d[#d+1]=data[i+1]
		end
		local dat,len=core.save(d,f,off)
		return dat,len -- we return the parsed data and the length of the data we just read in bytes
	end

	local fmtt={} -- format field type
	local fmtn={} -- format field name
	
	local len=0
	for i=1,#fmts,2 do
		local vt=fmts[i]
		local vn=fmts[i+1]
		
		len=len+1
		
		fmtt[len]=vt -- break input into two tables type and name
		fmtn[len]=vn
		
	end
	
	local datd={} -- place in correct order
	for i,v in pairs(fmtn) do
		datd[i]=data[v]
	end

-- parse the data into a string
	local dat,len=core.save(datd,fmtt,off)

	return dat,len -- we return the parsed data and the length of the data we just read in bytes
end

pack.copy=function(src,dst)
	return core.copy(src,dst)
end

pack.alloc=function(size,meta)
	return core.alloc(size,meta)
end
pack.sizeof=function(ud)
	return core.sizeof(ud)
end
pack.tostring=function(ud,size)
	return core.tostring(ud,size)
end
pack.tolightuserdata=function(ud,off)
	return core.tolightuserdata(ud,off)
end


-- convert a 16bit color to 4 floats, with premultiplied alpha
-- ie so it can be used in a gl.Color() call
pack.argb4_pmf4=function(c)
	local r,g,b,a
	
	a=bit.band(bit.rshift(c,12),0xf)
	r=bit.band(bit.rshift(c, 8),0xf)
	g=bit.band(bit.rshift(c, 4),0xf)
	b=bit.band(c,0xf)

	a=a/0xf
	return a*r/0xf,a*g/0xf,a*b/0xf,a
end

-- same again but 32bit
pack.argb8_pmf4=function(c)
	local r,g,b,a
	
	a=bit.band(bit.rshift(c,24),0xff)
	r=bit.band(bit.rshift(c,16),0xff)
	g=bit.band(bit.rshift(c, 8),0xff)
	b=bit.band(c,0xff)

	a=a/0xff
	return a*r/0xff,a*g/0xff,a*b/0xff,a
end
pack.argb_pmf4=pack.argb8_pmf4



-- and without the multiply
pack.argb8_f4=function(c)
	local r,g,b,a
	
	a=bit.band(bit.rshift(c,24),0xff)
	r=bit.band(bit.rshift(c,16),0xff)
	g=bit.band(bit.rshift(c, 8),0xff)
	b=bit.band(c,0xff)

	return r/0xff,g/0xff,b/0xff,a/0xff
end
pack.argb_f4=pack.argb8_f4

-- pack 4 gl floats into 1 32bit color
pack.f4_argb=function(r,g,b,a)
	
	a=bit.band(a*0xff,0xff)
	r=bit.band(r*0xff,0xff)
	g=bit.band(g*0xff,0xff)
	b=bit.band(b*0xff,0xff)
	
	return  (a*0x01000000 + r*0x00010000 + g*0x00000100 + b*0x00000001)
end



-- pull 4 premultiplied byte values out of a single 32bit color
pack.argb8_pmb4=function(c)
	local r,g,b,a,af
	
	a=bit.band(bit.rshift(c,24),0xff)
	r=bit.band(bit.rshift(c,16),0xff)
	g=bit.band(bit.rshift(c, 8),0xff)
	b=bit.band(c,0xff)

	af=a/0xff
	return math.floor(af*r),math.floor(af*g),math.floor(af*b),a
end
pack.argb_pmb4=pack.argb8_pmb4




-- pull 4 byte values out of a single 32bit color
pack.argb8_b4=function(c)
	local r,g,b,a
	
	a=bit.band(bit.rshift(c,24),0xff)
	r=bit.band(bit.rshift(c,16),0xff)
	g=bit.band(bit.rshift(c, 8),0xff)
	b=bit.band(c,0xff)

	return r,g,b,a
end
pack.argb_b4=pack.argb8_b4

-- pack 4 bytes into 1 32bit color
pack.b4_argb8=function(r,g,b,a)
	
	a=bit.band(a,0xff)
	r=bit.band(r,0xff)
	g=bit.band(g,0xff)
	b=bit.band(b,0xff)
	
	return  (a*0x01000000 + r*0x00010000 + g*0x00000100 + b*0x00000001)
end
pack.b4_argb=pack.b4_argb8




pack.stream={}

pack.stream.new=function(initial_size)

	local stream={}
	
	stream.pos=0
	stream.size=initial_size or 1024 -- default size of 1k
	
	stream.data=assert(pack.alloc(stream.size))
	
	stream.ptr=function(off)
		if not off then
			return stream.data
		elseif off>=0 then
			return pack.tolightuserdata(stream.data,off)
		else
			return pack.tolightuserdata(stream.data,stream.pos+off)
		end
	end

	stream.grow=function(d)
		stream.size=stream.size*d
		stream.data=assert(core.grow(stream.data , stream.size ))
	end
	
	stream.check=function(len)
		if strem.pos + len > stream.size then
			local s=math.ceil( (strem.pos + len)/stream.size )
			stream.grow(s)
		end
	end

	stream.write=function(fmt,buf)
		stream.check(#buff*4) -- assume max data size of 4bytes
		core.save(stream.data,fmt,stream.pos,buff)
	end

	return stream
end



return pack

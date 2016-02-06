

module(...,package.seeall)

local wstr=require("wetgenes.string")
local sod=require("wetgenes.sod")
local pack=require("wetgenes.pack")


local kissfft=require("kissfft.core")

function test_sod()

--print(wstr.dump(sod))

local sd=assert(sod.create())

--print(wstr.dump(sd))

	assert(sd:load("dat/sod/t1.wav"))

--print(wstr.dump(sd))

local fsiz=512


	local t=kissfft.start(fsiz)

--[[
	local data="00000000zzzzzzzz" -- fake test sample data should be squarewave ishhh
	local data=pack.save_array({-32767,-32767,-32767,-32767,0,0,0,0,32767,32767,32767,32767,0,0,0,0},"s16")
	local dd=string.rep(data,2*fsiz/#data)
--print(#dd)
	kissfft.push(t,dd,fsiz*2)
]]

	kissfft.push(t,sd.data,sd.data_sizeof)--sd.data_sizeof)

	local o,cc=kissfft.pull(t)
	local ot=pack.load_array(o,"f32",0,4*(fsiz/2))

end

function do_file_read(f)
	local fp=assert(io.open(f,"rb"))
	local d=assert(fp:read("*a"))
	fp:close()
	return d
end


function test_al()

local al=require("al")
local alc=require("alc")

	
	local dc=alc.setup()
	
--	alc.test()-- test junk

	local data="00000000zzzzzzzz" -- fake test sample data should be squarewave ishhh
	local sd=sod.create():load_data(do_file_read("dat/sod/t2.wav"))

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

--	print("AL",wstr.dump(al))
--	print("ALC",wstr.dump(alc))
	
--	print(al.NO_ERROR,"==",al[al.NO_ERROR])

--	al.test()

end


module(...,package.seeall)

local wstr=require("wetgenes.string")


local function test_al()

local al=require("al")
local alc=require("alc")

	
	local dc=alc.setup()
	
--	alc.test()-- test junk

	local data="00000000zzzzzzzz" -- fake test sample data should be squarewave ishhh

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

	al.BufferData(buffer,al.FORMAT_MONO16,data,#data,261.626*8) -- C4 hopefully?

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



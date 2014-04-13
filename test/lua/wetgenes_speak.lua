

module(...,package.seeall)

local wstr=require("wetgenes.string")
local sod=require("wetgenes.sod")
local al=require("al")
local alc=require("alc")



function no_speak()

	local t=require("wetgenes.speak.core")

	local dat,len=t.test("Welcome! To the world of toomorrow...")
	
--print("SPEAK",dat,len)


	local dc=alc.setup()
	
--	alc.test()-- test junk

--	local data="00000000zzzzzzzz" -- fake test sample data should be squarewave ishhh
--	local sd=sod.create():load("dat/sod/t2.wav")

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

	al.BufferData(buffer,al.FORMAT_MONO16,dat,len,261.626*8*8) -- C4 hopefully?
--	al.BufferData(buffer,sd) -- all loaded

	al.Source(source, al.BUFFER, buffer)
	al.Source(source, al.LOOPING,al.TRUE)

	al.SourcePlay(source)
	require("socket").sleep(8)
	
	al.CheckError()

	al.DeleteSource(source)
	al.DeleteBuffer(buffer)
	
	dc:clean() -- should really clean up when finished


end



local wstr=require("wetgenes.string")

local al=require("al")
local alc=require("alc")


function alc_CheckError(device)
	local err=alc.GetError(device)
	local str=("0x%04x"):format(err)
	assert(err==0,str)
end

function alc_setup(opts)
	opts=opts or {}
	local dc={}
	dc.alc=alc
	dc.opts=opts
print("AL","OpenDevice")
	dc.device=alc.OpenDevice()
	alc_CheckError(dc.device)
print("AL","CreateContext")
	dc.context=alc.CreateContext(dc.device)
	alc_CheckError(dc.device)
	dc.clean=alc.clean
print("AL","MakeContextCurrent")
	alc.MakeContextCurrent(dc.context)
	alc_CheckError(dc.device)
	
	if opts.capture then
print("AL","CaptureOpenDevice")
--		dc.capture_device=alc.CaptureOpenDevice(nil,44100,alc.CAPTURE_SAMPLES,44100)
		dc.capture_device=alc.CaptureOpenDevice(nil,48000,al.FORMAT_MONO16,32768)
		alc_CheckError()
	end
	
--print("AL","setup done","freq",	alc.Get(dc.device,alc.FREQUENCY),
--			dc.capture_device and alc.Get(dc.capture_device,alc.FREQUENCY) )
	
	return dc
end
	local dc=alc_setup()
	alc_CheckError(dc.device)

	
--	alc.test()-- test junk

	local data="00000000zzzzzzzz" -- fake test sample data should be squarewave ishhh

--	al.Listener(al.POSITION, 0, 0, 0)
--	al.CheckError()
--	al.Listener(al.VELOCITY, 0, 0, 0)
--	al.CheckError()
--	al.Listener(al.ORIENTATION, 0, 0, -1, 0,1,0 )
--	al.CheckError()

	local source=al.GenSource()
print(source)
	al.CheckError()

	al.Source(source, al.PITCH, 1)
	al.CheckError()
	al.Source(source, al.GAIN, 1)
	al.CheckError()
	al.Source(source, al.POSITION, 0, 0, 0)
	al.CheckError()
	al.Source(source, al.VELOCITY, 0, 0, 0)
	al.CheckError()
	al.Source(source, al.LOOPING, al.FALSE)
	al.CheckError()

	local buffer=al.GenBuffer()
	al.CheckError()

	al.BufferData(buffer,al.FORMAT_MONO16,data,#data,261.626*8) -- C4 hopefully?
	al.CheckError()

	al.Source(source, al.BUFFER, buffer)
	al.CheckError()

	al.Source(source, al.LOOPING,al.TRUE)
	al.CheckError()

	al.SourcePlay(source)
	require("socket").sleep(2)
	
	al.CheckError()

	al.SourceStop(source)
	al.CheckError()
	al.Source(source, al.BUFFER, 0)
	al.CheckError()

	al.DeleteBuffer(buffer)
	al.CheckError()
	al.DeleteSource(source)
	al.CheckError()
	
	dc:clean() -- should really clean up when finished

--	print("AL",wstr.dump(al))
--	print("ALC",wstr.dump(alc))
	
--	print(al.NO_ERROR,"==",al[al.NO_ERROR])

--	al.test()





local pack=require("wetgenes.pack")
local al=require("al")
local alc=require("alc")
local kissfft=require("kissfft.core")

local dev=alc.CaptureOpenDevice(nil,44100,al.FORMAT_MONO16,44100)
alc.CaptureStart(dev)

local fftsiz=1024

local sn="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

local fft=kissfft.start(fftsiz)
local d=pack.alloc(fftsiz*2)
local working=true
while working do
	local c=alc.Get(dev,alc.CAPTURE_SAMPLES)
	if c>fftsiz then
		alc.CaptureSamples(dev,d)
		kissfft.reset(fft)
		kissfft.push(fft,d,fftsiz)
		local t=pack.load_array(kissfft.pull(fft),"f32",0,4*(fftsiz/2))
		local ss={}
		for i=1,128 do
			ss[i]=math.floor( (t[i]) )
			if ss[i]>35 then ss[i]=35 end
			ss[i] = string.sub(sn, ss[i]+1,ss[i]+1 )
		end
		print( table.concat(ss) )
	end
end

alc.CaptureStop(dev)
alc.CaptureCloseDevice(dev)

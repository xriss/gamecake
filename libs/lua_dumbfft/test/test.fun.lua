
-- expect this many s16 audio samples per second
local sample_rate=48000
local buffer_size=sample_rate/16 -- bigger the buffer steadier/lower the signal ( 8hz is midi 0 ish )

-- here we choose the buckets we will fill up, specifically the midi notes 0-127
local note_freq_octs={
{ [4]=261.63 },
{ [4]=277.18 },
{ [4]=293.66 },
{ [4]=311.13 },
{ [4]=329.63 },
{ [4]=349.23 },
{ [4]=369.99 },
{ [4]=392.00 },
{ [4]=415.30 },
{ [4]=440.00 },
{ [4]=466.16 },
{ [4]=493.88 },
}
for i,v in ipairs(note_freq_octs) do -- build other octaves
	v[-1]=v[4]/32
	v[0]=v[4]/16
	v[1]=v[4]/8
	v[2]=v[4]/4
	v[3]=v[4]/2
	--
	v[5]=v[4]*2
	v[6]=v[4]*4
	v[7]=v[4]*8
	v[8]=v[4]*16
	v[9]=v[4]*32
end

local midi_wavelen={} -- 0-127
for o=-1,9 do
	for n=1,12 do -- build other octaves
		local f=note_freq_octs[n][o]
		local m=((o+1)*12)+n-1
		midi_wavelen[m]=1/f
	end
end

-- the midi notes we plan to bucket
-- could also be worth doing the half notes so we get 256 outputs
local min_bucket=0
local max_bucket=127

--
-- Here begins the DumbFT code
--

-- output buckets, each one has its own circular buffer to maintain a running total
-- this buffer size does not slow us down but does increase memory usage
local new_bucket=function(midi,base_size)
	local bucket={}
	
	bucket.midi=midi
	bucket.wavelen=midi_wavelen[midi]
	bucket.size=base_size -- requested time sample resolution
	bucket.probe_size=math.floor(0.5+(sample_rate*bucket.wavelen))
	bucket.probe_cos=math.ceil(bucket.probe_size/4)
	bucket.probe_idx=0
	bucket.probe_data={}
	bucket.idx=0
	bucket.stotal=0
	bucket.ctotal=0
	bucket.sdata={}
	bucket.cdata={}

-- keep bucket size to multiple of probe size.
	bucket.size=math.floor(bucket.size/bucket.probe_size)*bucket.probe_size

-- probe must fit in the bucket ( eg low hz will increase bucket size )
	if bucket.size<bucket.probe_size then bucket.size=bucket.probe_size end

	bucket.wave=function(t)
		return math.sin(math.pi*2*t)
	end
-- square wave picks up more noise and sub harmonic peaks but maybe acceptable if faster?
	bucket.sqwave=function(t)
		if t<0.5 then return 1 end
		return -1
	end
	
	bucket.reset=function()
		for i=0,bucket.size-1 do bucket.sdata[i]=0 end -- fill buffer with 0
		for i=0,bucket.size-1 do bucket.cdata[i]=0 end -- fill buffer with 0
		for i=0,bucket.probe_size-1 do bucket.probe_data[i]=bucket.wave( i/bucket.probe_size ) end -- probe wave
		bucket.probe_idx=0
		bucket.idx=0
		bucket.stotal=0
		bucket.ctotal=0
	end
	bucket.reset()
	-- push a new s16 sample into the bucket
	bucket.push=function(num)
		local sf=math.floor(num*bucket.probe_data[bucket.probe_idx])
		local cf=math.floor(num*bucket.probe_data[ (bucket.probe_idx+bucket.probe_cos)%bucket.probe_size ])
		bucket.probe_idx=(bucket.probe_idx+1)%bucket.probe_size -- advance idx along rotational buffer

		local sold=bucket.sdata[bucket.idx]
		bucket.sdata[bucket.idx]=sf
		bucket.stotal=bucket.stotal+sf-sold -- keep running total

		local cold=bucket.cdata[bucket.idx]
		bucket.cdata[bucket.idx]=cf
		bucket.ctotal=bucket.ctotal+cf-cold -- keep running total

		bucket.idx=(bucket.idx+1)%bucket.size -- advance idx along rotational buffer
	end

	-- get the current totals, can be called anytime
	bucket.get=function()
		local t=math.sqrt( (bucket.stotal*bucket.stotal) + (bucket.ctotal*bucket.ctotal) )
		local n=t/(bucket.size*0x3fff) -- aim for 0-1 ish might go a bit over
		return n
	end
	
	return bucket
end

local buckets={}
for m=min_bucket,max_bucket do
	buckets[m]=new_bucket(m,buffer_size)
end

local push_sample=function(num)
	for m=min_bucket,max_bucket do
		buckets[m].push(num)
	end
end

local pull_buckets=function()
	local bs={}
	for m=min_bucket,max_bucket do
		bs[m]=buckets[m].get()
	end
	return bs
end
--
-- Here ends the DumbFT code
--

--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--
local fats=require("wetgenes.fats")

oven.opts.fun="" -- back to menu on reset

sysopts={
	mode="swordstone", -- select a characters+sprites on a 256x128 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
	lox=256,loy=192, -- minimum size
	hix=256,hiy=256, -- maximum size
	autosize="lohi", -- flag that we want to auto resize
}

hardware,main=system.configurator(sysopts)

local px,py=0,0
local vx,vy=2,1
local speed=0

setup=function()
	oven.cake.sounds.start_capture()
end

update=function()

    if setup then setup() ; setup=nil end

	local buff,len=oven.cake.sounds.get_capture()
	local s16s
	if buff then
		s16s=fats.int16s_to_table(buff)
	else
		s16s={}
	end
	for i,v in ipairs(s16s) do
		push_sample(v)
	end
	local bs=pull_buckets()


    local cscreen=system.components.screen
    local ctext=system.components.text
    local bg=9
    local fg=31 -- system.ticks%32 -- cycle the foreground color

	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color
	
	local tx=math.ceil(cscreen.hx/4)
	local ty=math.ceil(cscreen.hy/8)

	for m=min_bucket+(12*0),max_bucket do
		local n=bs[m]
		n=math.ceil((n^1)*4*2^(m/24))
		local s=string.rep("*",n)
		local x=0
		local y=m
		fg=24
		while y>=12 do
			y=y-12
			x=x+6
			fg=fg-1
		end
		ctext.text_print(s,x-(6*0),y,fg,bg)
	end
end

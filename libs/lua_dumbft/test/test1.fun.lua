
-- expect this many s16 audio samples per second
local sample_rate=48000
local buffer_size=math.floor(sample_rate/16) -- (sample_rate/buffer_rate)  ( 8hz is midi 0 ish )

local midinote_to_freq=function(m)
	return (2^((m-69)/12))*440
end
local midinote_to_wavelen=function(m)
	return math.ceil(sample_rate/midinote_to_freq(m))
end

local math_log_2=math.log(2)

local freq_to_midinote=function(f)
	return 69+((math.log(f/440)/math_log_2)*12)
end

local wavelen_to_midinote=function(w)
	return freq_to_midinote(sample_rate/w)
end

-- idx of max bucket
local max_bucket=255

-- we are zooming in here on the midinote range in a non linear array
-- to get this quality of data from a standard FFT would need at least 16k? maybe 32k? of buckets
-- most of which we would ignore and that huge bucket size means a "laggy" sample.
-- So an FFT is only "faster" at generating data we do not want or need.

--
-- Here begins the DumbFourierTransform code, plenty of room for optimization
--

-- output buckets, each one has its own circular buffer to maintain a running total
-- this buffer size does not slow us down but does increase memory usage
local new_bucket=function(idx,base_size,probe_size)
	local bucket={}
	
	bucket.midinote=wavelen_to_midinote(probe_size)
	bucket.probe_size=probe_size -- wavelength in samples
	bucket.size=base_size -- requested time sample resolution
	bucket.probe_cos=math.ceil(bucket.probe_size/4)
	bucket.probe_idx=0
	bucket.idx=0
	bucket.stotal=0
	bucket.ctotal=0
	bucket.sdata={}
	bucket.cdata={}
	bucket.probe_sdata={} -- should replace this with global wave
	bucket.probe_cdata={}

	-- prob must be a int number of samples, or we get garbage noise
	-- the magical 0 output from probes breaks due to aliasing 
	bucket.probe_size=math.ceil(bucket.probe_size)

-- round bucket size to a multiple of probe size?
	bucket.size=math.floor(bucket.size/bucket.probe_size)*bucket.probe_size
-- probe must fit in the bucket ( eg low hz will increase bucket size )
	if bucket.size<bucket.probe_size then bucket.size=bucket.probe_size end

	bucket.size=math.ceil(bucket.size) -- force int

	bucket.wave=function(t)
		return math.sin(math.pi*2*t) -- fucking radians
	end
-- square wave picks up more noise and sub harmonic peaks but maybe acceptable if faster?
-- also works badly for non even buckets and higher freqs, but possibly?
	bucket.sqwave=function(t)
		if t>1 then t=t-1 end
		if t<0.5 then return 1 end
		return -1
	end
	
	bucket.reset=function()
		for i=0,bucket.size-1 do bucket.sdata[i]=0 end -- fill buffer with 0
		for i=0,bucket.size-1 do bucket.cdata[i]=0 end -- fill buffer with 0
		for i=0,bucket.probe_size-1 do bucket.probe_sdata[i]=bucket.wave( i/bucket.probe_size ) end -- probe wave
		for i=0,bucket.probe_size-1 do bucket.probe_cdata[i]=bucket.wave( 0.25+(i/bucket.probe_size) ) end -- probe wave
		bucket.probe_idx=0
		bucket.idx=0
		bucket.stotal=0
		bucket.ctotal=0
	end
	bucket.reset()
	-- push a new s16 sample into the bucket
	bucket.push=function(num)
		local sf=math.floor(num*bucket.probe_sdata[bucket.probe_idx])
		local cf=math.floor(num*bucket.probe_cdata[bucket.probe_idx])
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

-- max bucket has a wave length of 4 samples and we
-- work backwards adding 1 sample each time for max resolution
-- we also work midinotes upwards starting at midi 0
-- switching when these two sequences meet
-- the full midirange should be covered this way
-- using integer length probe waves
local buckets={}
for idx=0,max_bucket do
	local min_wavelen=math.floor(midinote_to_wavelen(idx))
	local wavelen=4+(max_bucket-idx)
	if min_wavelen>wavelen then wavelen=min_wavelen end
	buckets[idx]=new_bucket(idx, buffer_size , wavelen )
end

local push_sample=function(num)
	for m=0,max_bucket do
		buckets[m].push(num)
	end
end

local pull_buckets=function()
	local bs={}
	for m=0,max_bucket do
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
	lox=256,loy=288, -- minimum size
	hix=256,hiy=288, -- maximum size
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

	for idx=0,max_bucket do
		local bucket=buckets[idx]
		local m=bucket.midinote
		local n=bs[idx]
		local o=math.floor(m/12)
		n=math.ceil((n*(2^(m/12)))/2) -- *double loudness every octave* and tweak to view size
		local s=string.rep("*",n)
		local x=0
		local y=idx
		fg=23-o -- change color on octave
		while y>=36 do
			y=y-36
			x=x+8
		end
		ctext.text_print(s,x,y,fg,fg)
	end
end


-- expect this many s16 audio samples per second
local sample_rate=48000
local buffer_size=sample_rate/8 -- bigger the buffer better the signal

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

local midi_wavelen={} -- 0-127 but 
for o=-1,9 do
	for n=1,12 do -- build other octaves
		local f=note_freq_octs[n][o]
		local m=((o+1)*12)+n-1
		midi_wavelen[m]=1/f
	end
end

for i=0,127 do
	print( 1/midi_wavelen[i] , math.ceil(sample_rate*midi_wavelen[i]) )
end

-- the midi notes we plan to bucket
local min_bucket=0
local max_bucket=127

-- try multiple probe waves into the data
-- output buckets


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
	bucket.size=math.ceil(bucket.size/bucket.probe_size)*bucket.probe_size

-- probe must fit in the bucket ( eg low hz must increase bucket size )
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

local print_buckets=function()
	print("...")
	local t={}
	local tots=0
	local best_num=0
	local best_m=0
	for m=min_bucket,max_bucket do
		local num=buckets[m].get()*128
		local fnum=math.floor(num)
		local mnum=fnum
		if mnum>128 then mnum=128 end
		tots=tots+num
		print(m,math.floor(1/midi_wavelen[m]),fnum,string.rep("#",mnum))
--		buckets[m].reset()
		if num>best_num then
			best_num=num
			best_m=m
		end
	end
	print("...",math.floor(1/midi_wavelen[best_m]),tots)
end

for m=6,127,16 do
local f=sample_rate/buckets[m].probe_size
for i=1,buffer_size do 
	local w=math.sin( ( f*i/sample_rate )*math.pi*2 )
	push_sample( math.floor(w*0x7fff) )
end
print_buckets()

end


-- expect this many s16 audio samples per second
local sample_rate=48000

local note_wavelen_octs={
{ [4]=1/261.63 },
{ [4]=1/277.18 },
{ [4]=1/293.66 },
{ [4]=1/311.13 },
{ [4]=1/329.63 },
{ [4]=1/349.23 },
{ [4]=1/369.99 },
{ [4]=1/392.00 },
{ [4]=1/415.30 },
{ [4]=1/440.00 },
{ [4]=1/466.16 },
{ [4]=1/493.88 },
}
for i,v in ipairs(note_wavelen_octs) do -- build other octaves
	v[-1]=v[4]*32
	v[0]=v[4]*16
	v[1]=v[4]*8
	v[2]=v[4]*4
	v[3]=v[4]*2
	--
	v[5]=v[4]/2
	v[6]=v[4]/4
	v[7]=v[4]/8
	v[8]=v[4]/16
	v[9]=v[4]/32
end

local midi_wavelen={} -- 0-127 but 
for o=-1,9 do
	for n=1,12 do -- build other octaves
		local f=note_wavelen_octs[n][o]
		local m=((o+1)*12)+n-1
		midi_wavelen[m]=f
	end
end

for i=0,127 do
	print( midi_wavelen[i] , math.ceil(sample_rate*midi_wavelen[i]) )
end

-- the midi notes we plan to bucket
local min_bucket=24
local max_bucket=107

-- try multiple probe waves into the data
-- output buckets


local new_bucket=function(midi,size)
	local bucket={}
	
	bucket.midi=midi
	bucket.wavelen=midi_wavelen[midi]
	bucket.size=size -- fixed size giving us a time sample resolution
	bucket.probe_size=math.floor(0.5+(sample_rate*bucket.wavelen))
	bucket.probe_cos=math.ceil(bucket.probe_size/4)
	bucket.probe_idx=0
	bucket.probe_data={}
	bucket.idx=0
	bucket.stotal=0
	bucket.ctotal=0
	bucket.last=0
	bucket.sdata={}
	bucket.cdata={}
	
--	bucket.size=math.ceil(bucket.size/bucket.probe_size)*bucket.probe_size

	bucket.wave=function(t)
		return math.sin(math.pi*2*t)
	end
-- square wave picks up more noise but maybe acceptable if faster?
	bucket.sqwave=function(t)
		if t<0.5 then return 1 end
		return -1
	end
	
	for i=0,bucket.size-1 do bucket.sdata[i]=0 end -- fill buffer with 0
	for i=0,bucket.size-1 do bucket.cdata[i]=0 end -- fill buffer with 0
	for i=0,bucket.probe_size-1 do bucket.probe_data[i]=bucket.wave( i/bucket.probe_size ) end -- probe wave
	
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
		return ( math.abs(bucket.stotal/bucket.size) + math.abs(bucket.ctotal/bucket.size) ) * bucket.wavelen
	end
	
	return bucket
end

local buckets={}
for m=min_bucket,max_bucket do
	buckets[m]=new_bucket(m,2048)
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
	for m=min_bucket,max_bucket do
		local num=buckets[m].get()/8
		if num>100 then num=100 end
		num=math.floor(num)
		tots=tots+num
		print(m,math.floor(1/midi_wavelen[m]),num,string.rep("#",num))
	end
	print("...",tots)
end

for f=100,500,100 do

for i=1,4096 do 
	local w=math.sin( ( f*i/sample_rate )*math.pi*2 )
	push_sample( math.floor(w*32767) )
end
print_buckets()
--for i=1,2048 do push_sample(0) end

end

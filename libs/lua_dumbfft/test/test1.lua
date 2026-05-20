
-- expect this many s16 audio samples per second
local sample_rate=48000

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

local midi_freqs={} -- 0-127 but 
for o=-1,9 do
	for n=1,12 do -- build other octaves
		local f=note_freq_octs[n][o]
		local m=((o+1)*12)+n-1
		midi_freqs[m]=f
	end
end

for i=0,127 do
	print( midi_freqs[i] , math.ceil(sample_rate/midi_freqs[i]) )
end

-- the midi notes we plan to bucket
local min_bucket=24
local max_bucket=107

-- multiple low pass filters maybe?
-- dont think so, thought this might be too easy

-- sliding averages
local new_slide=function(midi)
	local slide={}
	
	slide.midi=midi
	slide.hz=(midi_freqs[midi]+midi_freqs[midi-1])/2 -- bottom of window
	slide.size=math.floor(0.5+(sample_rate/slide.hz))
	slide.idx=0
	slide.total=0
	slide.data={}
	for i=0,slide.size-1 do slide.data[i]=0 end -- fill buffer with 0
	
	slide.push=function(num)
		local old=slide.data[slide.idx]
		slide.data[slide.idx]=num
		slide.idx=(slide.idx+1)%slide.size -- advance idx along rotational buffer
		slide.total=slide.total+num-old -- keep running total
		return slide.total/slide.size
	end
	
	return slide
end

-- output buckets
local new_bucket=function(midi,size)
	local bucket={}
	
	bucket.midi=midi
	bucket.hz=midi_freqs[midi]
	bucket.size=size -- fixed size giving us a time sample resolution
	bucket.idx=0
	bucket.total=0
	bucket.last=0
	bucket.data={}
	for i=0,bucket.size-1 do bucket.data[i]=0 end -- fill buffer with 0
	
	bucket.push=function(num)
		local diff=math.abs(num) -- sample diff is "loudness"
		bucket.last = num
		local old=bucket.data[bucket.idx]
		bucket.data[bucket.idx]=diff
		bucket.idx=(bucket.idx+1)%bucket.size -- advance idx along rotational buffer
		bucket.total=bucket.total+diff-old -- keep running total
	end
	bucket.get=function()
		return (bucket.total/bucket.size) -- *(sample_rate/bucket.hz)
	end
	
	return bucket
end

local slides={}
for m=min_bucket,max_bucket+1 do
	slides[m]=new_slide(m)
end

local buckets={}
for m=min_bucket,max_bucket do
	buckets[m]=new_bucket(m,2048)
end

local push_sample=function(num)
 -- start with extra slide so we can diff
	local high_samp=slides[max_bucket+1].push(num)
	for m=max_bucket,min_bucket,-1 do
		local low_samp=slides[m].push(num)
		buckets[m].push(high_samp)
		high_samp=low_samp
	end
end

local print_buckets=function()
	print("...")
	local t={}
	for m=min_bucket,max_bucket do
		local num=buckets[m].get()/800
		if num>100 then num=100 end
		num=math.floor(num)
		print(m,math.floor(midi_freqs[m]),num,string.rep("#",num))
	end
	print("...")
end

for i=1,4096 do 
	local w=math.sin( ( 100*i/sample_rate )*math.pi*2 )
	push_sample( math.floor(w*32767) )
end
print_buckets()
for i=1,2048 do push_sample(0) end

for i=1,4096 do 
	local w=math.sin( ( 500*i/sample_rate )*math.pi*2 )
	push_sample( math.floor(w*32767) )
end
print_buckets()
for i=1,2048 do push_sample(0) end

for i=1,4096 do 
	local w=math.sin( ( 2000*i/sample_rate )*math.pi*2 )
	push_sample( math.floor(w*32767) )
end
print_buckets()
for i=1,2048 do push_sample(0) end

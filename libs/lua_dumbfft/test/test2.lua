
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

-- try multiple probe waves into the data

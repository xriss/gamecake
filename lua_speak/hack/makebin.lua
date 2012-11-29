
local wpack=require("wetgenes.pack")

local fname="flite/voices/cmu_us_slt/cmu_us_slt_cg_single_params.c"

local num=0
local dat={}

local nextline=nil

for line in io.lines(fname) do

num=num+1

	local d={}
	
	if nextline then
	

		for n in string.gfind(line,"(%d+)") do
			d[#d+1]=tonumber(n)
		end
	
		nextline=nil
		dat[#dat+1]=d

	end

	for s in string.gfind(line,"static const unsigned short cmu_us_slt_single_param_frame_(%d+)%[%] = { ") do
		nextline=tonumber(s)
	end


end

print(num,"lines")
print(#dat,"datas")


local idx=0
local idxs={}

local fname="hack/slt_data.bin"
local fp=io.open(fname,"wb")
for i,v in ipairs(dat) do

	idxs[#idxs+1]=idx -- build index
	idx=idx+(#v*2)

	fp:write(wpack.save_array(v,"u16",0,#v))

end
fp:close()


local fname="hack/slt_data.idx"
local fp=io.open(fname,"wb")
fp:write(wpack.save_array(idxs,"u32",0,#idxs))
fp:close()


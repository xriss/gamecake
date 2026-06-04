
local TAU=math.pi*2

local ps={}

for junk=1,200,11 do

for lp=4,512 do

	local sp=0
	local cp=0
	for i=1,65536 do
		local sa=math.sin(TAU*((i-0.5+junk)/lp))
		local ca=math.sin(TAU*((i-0.5+junk)/lp))

		local d=math.sin(TAU*((i-0.5)/512))

		sp=sp+sa*d
		cp=cp+ca*d
	end
	local p=math.sqrt(sp*sp+cp*cp)
	ps[lp]=(ps[lp] or "")..math.floor(p).." "

end

end

for lp=1,512 do
	ps[lp]=(ps[lp] or "").."\n"
end

print(table.concat(ps,""))

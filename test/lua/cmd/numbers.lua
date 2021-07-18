

DISABLE_WETGENES_TARDIS_CORE=false

require("apps").default_paths()


local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")


print(os.time())

for i=1,100000 do

	local m=M4()

	m:translate(i*3,i*7,i*13)
	m:translate(i*3,i*7,i*13)
	m:rotate(i,1,10,0)
	m:rotate(i,0,1,0)
	m:rotate(i,0,0,1)
	m:scale(i/100)

	m:inverse()

end

print(os.time())

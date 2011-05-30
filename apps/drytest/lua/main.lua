

print("start\n")

print( arg[1] or "" ) -- command
print( arg[2] or "" ) -- hwnd


local grd=require("grd")

print(debug.getinfo(1))

print(grd)
--for i,v in pairs(grd) do
--	print(i," = ",v)
--end

local fname=wetlua.dir.."atest.32.png"
print(fname)
local ga=grd.create("GRD_FMT_U8_BGRA",fname)

for i,v in pairs(ga) do
	print(i," = ",v)
end

--local gb=grd.create(ga)

--ga:scale(100,100,1)
--print(os.time())
ga:quant(256)
--print(os.time())

--ga:pixels(20,0,10,1,"azazazazaz")

--print(#s)

ga:save(fname..".out.png")

print("end\n")

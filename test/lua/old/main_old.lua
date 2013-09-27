

print("start\n")

print( arg[1] or "" ) -- command
print( arg[2] or "" ) -- hwnd


local grd=require("grd")

local freetype=require("freetype")



print(debug.getinfo(1))



function dump(tab)
	for i,v in pairs(tab) do

		
		if (type(v)=="table") then
			print( tostring(i).."="..tostring(v) )
			for i,v in pairs(v) do
				print( "\t"..tostring(i).."="..tostring(v) )
			end
		elseif (type(v)=="function") then
		elseif (type(v)=="userdata") then
		else
			print( tostring(i).."="..tostring(v) )
		end
	end
end




local f=freetype.create(wetlua.dir.."dat/DejaVuSans.ttf")

f:size(16,16)
--f:glyph(string.byte("B"))
f:render(string.byte("B"))

local t=f:bitmap()

print("freetype result "..tostring(f))

dump(f)

local str="Hello World"
for idx=1,#str do

	f:render(string.byte(str,idx))

	local t=f:bitmap()
	local s=" "
	print("");
	print(str:sub(idx,idx).." = "..f.bitmap_width.."x"..f.bitmap_height)
	print("");
	for i,v in ipairs(t) do

		if v>256*3/4 then s=s.."X"
		elseif v>256*2/4 then s=s.."x"
		elseif v>256*1/4 then s=s.."."
		else s=s.." " end
		
		if ((i)%f.bitmap_width) == 0 then
			print(s.." ")
			s=" "
		end
	end
	print("")

end

--dump(t)


if false then

	print(grd)
	--for i,v in pairs(grd) do
	--	print(i," = ",v)
	--end

	local fname=wetlua.dir.."atest.32.png"
	print(fname)
	local ga=grd.create("GRD_FMT_U8_RGBA",fname)

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


end

print("end\n")



module(...,package.seeall)

local wstr=require("wetgenes.string")

local grd=require("wetgenes.grd")

local grdmap=require("wetgenes.grdmap")


function test_grdmap()

--	print(wstr.dump(grdmap))
	
	local gm=grdmap.create()
	local gk=grdmap.create()

	gm:setup( assert(grd.create("GRD_FMT_U8_ARGB","dat/grdmap/t1.map.png","png")) )	
	gk:setup( assert(grd.create("GRD_FMT_U8_ARGB","dat/grdmap/t1.key.png","png")) )	
	
	gm:cutup(8,8)
	gk:cutup(8,8)
	
	gm:keymap(gk)
	
--	print("GM\n",wstr.dump(gm))
--	print("GK\n",wstr.dump(gk))
	
	local s=""
	for y=0,gm.th-1 do
		
		for x=0,gm.tw-1 do

			local t=gm:tile(x,y)

			if t.master then
				s=s..string.format("%02d ",t.master)
			else
				s=s.."00 "
			end

		end
		s=s.."\n"
	end
--	print(s)


	local s=""
	local xp=1
	for y=gm.th-2,0,-2 do
			
		if xp==1 then xp=0 else xp=1 end -- flip offset each line
	
		for x=xp,gm.tw-2,2 do

			local t=gm:tile(x,y)

			if t.master then
				s=s..string.format("%01X ",math.floor(t.master/2))
			else
				s=s.."0 "
			end

		end
		
		s=s.."\n"
	end

expected=[[
1 1 1 3 1 1 
1 3 3 3 3 1 
3 1 3 3 1 1 
3 1 3 3 3 1 
1 1 3 3 1 1 
6 6 7 6 6 6 
7 7 7 7 7 7 
7 7 7 7 7 7 
0 0 0 0 0 0 
0 0 0 0 0 0 
0 0 0 0 0 0 
0 0 0 0 0 0 
0 0 0 0 0 0 
0 0 0 0 0 0 
]]

--	print("\n"..s)
	assert( s==expected ,"\n"..expected.."\n\n==\n\n"..s.."\n")

--[[
	local g=assert(grd.create("GRD_FMT_U8_ARGB","dat/grd/"..name..".bse.png","png"))
	assert( g:convert("GRD_FMT_U8_INDEXED") )
	assert( g:convert("GRD_FMT_U8_ARGB") )
	assert( g:save("dat/grd/"..name..".8x.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8x.out.png","dat/grd/"..name..".8x.chk.png") )
]]
end



module(...,package.seeall)

local wstr=require("wetgenes.string")

local grd=require("wetgenes.grd")

local grdpaint=require("wetgenes.grdpaint")


function do_file_write(f,d)
	local fp=assert(io.open(f,"wb"))
	local d=assert(fp:write(d))
	fp:close()
end

function do_file_read(f)
	local fp=assert(io.open(f,"rb"))
	local d=assert(fp:read("*a"))
	fp:close()
	return d
end

function do_file_compare(f1,f2)
	local d1=do_file_read(f1)
	local d2=do_file_read(f2)
	return d1==d2
end

local function do_file_assert(g,name,test)
	assert( g:save("dat/grd/"..name.."."..test..".out.png","png") )	
	local fnamea,fnameb="dat/grd/"..name.."."..test..".out.png","dat/grd/"..name.."."..test..".chk.png"
	local diff=do_file_compare(fnamea,fnameb)
	if not diff then print("\n\nFILES DIFFER : "..fnamea.."   "..fnameb.."\n") end
	assert_true( diff )
end

function do_skew(name,dir,val)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	
	local o=grdpaint.skew(g,0,dir,val)
	
	do_file_assert(o,name,"skew"..dir..val)
end


function test_skew_wide8_1_1()		do_skew("wide8",1, 1)	end
function test_skew_wide8_2_1()		do_skew("wide8",2, 1)	end
function test_skew_wide8_3_1()		do_skew("wide8",3, 1)	end
function test_skew_wide8_4_1()		do_skew("wide8",4, 1)	end
function test_skew_wide8_5_1()		do_skew("wide8",5, 1)	end
function test_skew_wide8_6_1()		do_skew("wide8",6, 1)	end
function test_skew_wide8_7_1()		do_skew("wide8",7, 1)	end
function test_skew_wide8_8_1()		do_skew("wide8",8, 1)	end

function test_skew_wide8_1_2()		do_skew("wide8",1, -1)	end
function test_skew_wide8_2_2()		do_skew("wide8",2, -1)	end
function test_skew_wide8_3_2()		do_skew("wide8",3, -1)	end
function test_skew_wide8_4_2()		do_skew("wide8",4, -1)	end
function test_skew_wide8_5_2()		do_skew("wide8",5, -1)	end
function test_skew_wide8_6_2()		do_skew("wide8",6, -1)	end
function test_skew_wide8_7_2()		do_skew("wide8",7, -1)	end
function test_skew_wide8_8_2()		do_skew("wide8",8, -1)	end

function test_skew_tall8_1_1()		do_skew("tall8",1, 1)	end
function test_skew_tall8_2_1()		do_skew("tall8",2, 1)	end
function test_skew_tall8_3_1()		do_skew("tall8",3, 1)	end
function test_skew_tall8_4_1()		do_skew("tall8",4, 1)	end
function test_skew_tall8_5_1()		do_skew("tall8",5, 1)	end
function test_skew_tall8_6_1()		do_skew("tall8",6, 1)	end
function test_skew_tall8_7_1()		do_skew("tall8",7, 1)	end
function test_skew_tall8_8_1()		do_skew("tall8",8, 1)	end

function test_skew_tall8_1_2()		do_skew("tall8",1, -1)	end
function test_skew_tall8_2_2()		do_skew("tall8",2, -1)	end
function test_skew_tall8_3_2()		do_skew("tall8",3, -1)	end
function test_skew_tall8_4_2()		do_skew("tall8",4, -1)	end
function test_skew_tall8_5_2()		do_skew("tall8",5, -1)	end
function test_skew_tall8_6_2()		do_skew("tall8",6, -1)	end
function test_skew_tall8_7_2()		do_skew("tall8",7, -1)	end
function test_skew_tall8_8_2()		do_skew("tall8",8, -1)	end



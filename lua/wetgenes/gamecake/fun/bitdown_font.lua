--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local funfont64=require("wetgenes.gamecake.fun.funfont64")

M.fontdata4x8=funfont64.data4x8
M.fontdata8x8=funfont64.data8x8
M.fontdata8x16=funfont64.data8x16

M.grds={} -- cache

-- build a bitmap which is a 128x2 array of characters using above fontdata
-- might grow to 128x3 or more in the future as we add more unicode characters
-- chances are you just need the first 128x1 ascii chunk
M.build_grd=function(hx,hy,style)
	style=style or ""

	local name=hx.."x"..hy..style
	
	if not M.grds[name] then -- render
	
		local data=assert( funfont64["data"..hx.."x"..hy..style] or funfont64["data"..hx.."x"..hy] ) -- get data and check size is valid

		local g=wgrd.create("U8_RGBA_PREMULT",128*hx,2*hy,1)
		
		for i=0,255 do
			local s=data[i]
			if s then
				bitdown.pix_grd(s,bitdown.cmap_swanky32,g,(i%128)*hx,math.floor(i/128)*hy,hx,hy)
			end
		end
		
		M.grds[name]=g

	end

	return M.grds[name]
end


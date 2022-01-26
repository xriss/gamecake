--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wgrd=require("wetgenes.grd")

local windows={}

local core=require("wetgenes.win.windows.core")

windows.icon=function(w,g)

	assert(g:convert(wgrd.U8_RGBA)) -- make sure it is this format
	local argb=g:pixels(0,0,g.width,g.height)
	
-- data hacks...
	for i=0,g.width*g.height*4-1,4 do argb[i+1],argb[i+3]=argb[i+3],argb[i+1] end -- swap reg and blue
	local argbf={} -- flip top/bottom
	for y=g.height-1,0,-1 do
		for x=0,g.width-1 do
			argbf[#argbf+1]=argb[1+4*(x+(y*g.width))]
			argbf[#argbf+1]=argb[2+4*(x+(y*g.width))]
			argbf[#argbf+1]=argb[3+4*(x+(y*g.width))]
			argbf[#argbf+1]=argb[4+4*(x+(y*g.width))]
		end
	end
	argb=argbf

-- windows BMAP header
	local head=pack.save_raw({
		"u32",40, 					-- sizeofheader
		"s32",g.width, 				-- width
		"s32",g.height*2,			-- height
		"u16",1,					-- planes
		"u16",32,					-- bitcount
		"u32",0,					-- compression (none=0)
		"u32",g.width*g.height*4,	-- sizeofdata
		"u32",0,					-- x pixels per meter
		"u32",0,					-- y pixels per meter
		"u32",0,					-- unused
		"u32",0,					-- important
	})

	core.bmpicon(w,head .. pack.save_array(argb,"u8",0,#argb) )

end

--
-- export all core functions not wrapped above
--
for n,v in pairs(core) do -- check the core
	if type(v)=="function" then -- only functions
		if not windows[n] then -- only if not prewrapped
			windows[n]=v
		end
	end
end



return windows

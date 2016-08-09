--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-- swanky32

M.char_to_color={

	["."]=0x00000000,
	["g"]=0xff336622,
	["G"]=0xff448822,
	["d"]=0xff66aa33,
	["D"]=0xff66bb77,
	["C"]=0xff66cccc,
	["c"]=0xff5599cc,
	["B"]=0xff5577cc,

	["b"]=0xff445599,
	["I"]=0xff333366,
	["i"]=0xff332244,
	["j"]=0xff442233,
	["f"]=0xff663333,
	["F"]=0xff884433,
	["s"]=0xffbb7766,
	["S"]=0xffeeaa99,

	["M"]=0xffee88bb,
	["m"]=0xffdd6666,
	["R"]=0xffcc3333,
	["r"]=0xffdd5533,
	["O"]=0xffdd7733,
	["o"]=0xffddaa33,
	["Y"]=0xffdddd44,
	["y"]=0xff888833,

	["0"]=0xff000000,
	["1"]=0xff222222,
	["2"]=0xff444444,
	["3"]=0xff666666,
	["4"]=0xff888888,
	["5"]=0xffaaaaaa,
	["6"]=0xffcccccc,
	["7"]=0xffffffff,
	
}





M.font_grd=function(s,g,cx,cy,w)

	-- calling this function repeatedly is inefficient but we do not care
	local solid_check=function(idx,x,y)
		local m=s[string.char(idx)]
		if m then
			local lines=wstr.split(m,"\n")
			local line=lines[y+1]
			if line then
				local c=line:sub(1+x*2,1+x*2)
				if c=="#" then return true end
			end
		end
		return false
	end

	local dx=0
	local dy=0

	for i=0,127 do -- import each ascii char from textmaps above
		local t={}
		for y=0,cy-1 do
			for x=0,cx-1 do
				if solid_check(i,x,y) then
					t[#t+1]=255
					t[#t+1]=255
					t[#t+1]=255
					t[#t+1]=255
				else
					t[#t+1]=0
					t[#t+1]=0
					t[#t+1]=0
					t[#t+1]=0
				end
			end
		end
		g:pixels(dx*cx,dy*cy,cx,cy,t)
		dx=dx+1
		if dx>=w then
			dx=0
			dy=dy+1
		end
	end

end

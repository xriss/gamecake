--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wpack=require("wetgenes.pack")
local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-- swanky32 default color map

M.map={

	[0]={bgra=0x00000000}, -- default

	["."]={bgra=0x00000000},
	["g"]={bgra=0xff336622},
	["G"]={bgra=0xff448822},
	["d"]={bgra=0xff66aa33},
	["D"]={bgra=0xff66bb77},
	["C"]={bgra=0xff66cccc},
	["c"]={bgra=0xff5599cc},
	["B"]={bgra=0xff5577cc},

	["b"]={bgra=0xff445599},
	["I"]={bgra=0xff333366},
	["i"]={bgra=0xff332244},
	["j"]={bgra=0xff442233},
	["f"]={bgra=0xff663333},
	["F"]={bgra=0xff884433},
	["s"]={bgra=0xffbb7766},
	["S"]={bgra=0xffeeaa99},

	["M"]={bgra=0xffee88bb},
	["m"]={bgra=0xffdd6666},
	["R"]={bgra=0xffcc3333},
	["r"]={bgra=0xffdd5533},
	["O"]={bgra=0xffdd7733},
	["o"]={bgra=0xffddaa33},
	["Y"]={bgra=0xffdddd44},
	["y"]={bgra=0xff888833},

	["0"]={bgra=0xff000000},
	["1"]={bgra=0xff222222},
	["2"]={bgra=0xff444444},
	["3"]={bgra=0xff666666},
	["4"]={bgra=0xff888888},
	["5"]={bgra=0xffaaaaaa},
	["6"]={bgra=0xffcccccc},
	["7"]={bgra=0xffffffff},
	
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

-- turn 32bit .bgra numbers into 4 bytes rgba with a premultiplyed alpha placed in map[1] to map[4]
M.map_bgra_premultiply=function(map)
	for n,v in pairs(map) do
		v[1],v[2],v[3],v[4]=wpack.argb_pmb4(v.bgra)
	end
	return map
end

-- write some ascii art into an x,y grd location
M.pix_grd=function(str,map,gout,xp,yp,xh,yh)

	local ls=wstr.split(str,"\n")

	map=map or M.map

	xp=xp or 0
	yp=yp or 0
	
	xh=xh or (#(ls[1]))/2
	yh=yh or #ls-1

	local getxy=function(x,y)
		local l=ls[1+y]
		if l then
			return l:sub(1+x*2,1+x*2+1)
		end
	end

	local getc=function(s)
		return map[s] or map[s:sub(1,1)] or map[0]
	end
	
	local t={}
	for y=0,yh-1 do
		for x=0,xh-1 do
			local s=getxy(x,y) or ". "
			local c=getc(s) or {0,0,0,0}
			local l=#t
			t[l+1]=c[1] t[l+2]=c[2] t[l+3]=c[3] t[l+4]=c[4]
		end
	end
	gout:pixels(xp,yp,xh,yh,t)

end

M.map_bgra_premultiply(M.map)


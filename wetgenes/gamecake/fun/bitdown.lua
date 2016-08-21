--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wpack=require("wetgenes.pack")
local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local bitdown=M

M.map={

-- swanky32 by index

	[ 0]={bgra=0x00000000},
	[ 1]={bgra=0xff336622},
	[ 2]={bgra=0xff448822},
	[ 3]={bgra=0xff66aa33},
	[ 4]={bgra=0xff66bb77},
	[ 5]={bgra=0xff66cccc},
	[ 6]={bgra=0xff5599cc},
	[ 7]={bgra=0xff5577cc},

	[ 8]={bgra=0xff445599},
	[ 9]={bgra=0xff333366},
	[10]={bgra=0xff332244},
	[11]={bgra=0xff442233},
	[12]={bgra=0xff663333},
	[13]={bgra=0xff884433},
	[14]={bgra=0xffbb7766},
	[15]={bgra=0xffeeaa99},

	[16]={bgra=0xffee88bb},
	[17]={bgra=0xffdd6666},
	[18]={bgra=0xffcc3333},
	[19]={bgra=0xffdd5533},
	[20]={bgra=0xffdd7733},
	[21]={bgra=0xffddaa33},
	[22]={bgra=0xffdddd44},
	[23]={bgra=0xff888833},

	[24]={bgra=0xff000000},
	[25]={bgra=0xff222222},
	[26]={bgra=0xff444444},
	[27]={bgra=0xff666666},
	[28]={bgra=0xff888888},
	[29]={bgra=0xffaaaaaa},
	[30]={bgra=0xffcccccc},
	[31]={bgra=0xffffffff},

-- hex maps

	["00"]=0,
	["01"]=1,
	["02"]=2,
	["03"]=3,
	["04"]=4,
	["05"]=5,
	["06"]=6,
	["07"]=7,

	["08"]=8,
	["09"]=9,
	["0A"]=10,
	["0B"]=11,
	["0C"]=12,
	["0D"]=13,
	["0E"]=14,
	["0F"]=15,

	["10"]=16,
	["11"]=17,
	["12"]=18,
	["13"]=19,
	["14"]=20,
	["15"]=21,
	["16"]=22,
	["17"]=23,

	["18"]=24,
	["19"]=25,
	["1A"]=26,
	["1B"]=27,
	["1C"]=28,
	["1D"]=29,
	["1E"]=30,
	["1F"]=31,

-- custom maps for nicer looking handmade ascii

	["."]=0,
	["g"]=1,
	["G"]=2,
	["d"]=3,
	["D"]=4,
	["C"]=5,
	["c"]=6,
	["B"]=7,

	["b"]=8,
	["I"]=9,
	["i"]=10,
	["j"]=11,
	["f"]=12,
	["F"]=13,
	["s"]=14,
	["S"]=15,

	["M"]=16,
	["m"]=17,
	["R"]=18,
	["r"]=19,
	["O"]=20,
	["o"]=21,
	["Y"]=22,
	["y"]=23,

	["0"]=24,
	["1"]=25,
	["2"]=26,
	["3"]=27,
	["4"]=28,
	["5"]=29,
	["6"]=30,
	["7"]=31,
	
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
		while v and type(v)~="table" do v=map[v] end -- fill in any references
		map[n]=v -- remember the result
		if v then -- turn hex value into premult-bytes
			v[1],v[2],v[3],v[4]=wpack.argb_pmb4(v.bgra)
		end
	end
	return map
end

-- find the size of some bitdown in pixels, possibly rounded up
M.pix_size=function(str,rx,ry)

	rx=rx or 1
	ry=ry or 1

	local ls=wstr.split(str,"\n")

	if string.len(ls[1])==0 then table.remove(ls,1) end
	if string.len(ls[#ls])==0 then table.remove(ls,#ls) end
	
	local l=0 for i=1,#ls do l=l+string.len(ls[i]) end
	
	local x=math.floor(l/(#ls*2))
	local y=#ls

	x=math.ceil(x/rx)*rx
	y=math.ceil(y/ry)*ry

	return x,y
end


-- write some ascii art into an x,y grd location
M.pix_grd=function(str,map,gout,px,py,hx,hy)

	local ls=wstr.split(str,"\n")

	map=map or M.map

	px=px or 0
	py=py or 0
	
	if not hx and not hy then hx,hy=M.pix_size(str) end

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
	for y=0,hx-1 do
		for x=0,hx-1 do
			local s=getxy(x,y) or ". "
			local c=getc(s) or {0,0,0,0}
			local l=#t
			t[l+1]=c[1] t[l+2]=c[2] t[l+3]=c[3] t[l+4]=c[4]
		end
	end
	gout:pixels(px,py,hx,hy,t)

end

-- write a bunch of ascii into this bitmap, using its index as its location 0xYYXX,
M.pixtab_grd=function(tab,map,gout)

	for n,v in pairs(tab) do
		local hx,hy=bitdown.pix_size(v,8,8)
		local px=math.floor(n%256)*hx
		local py=math.floor((n)/256)*hy
		bitdown.pix_grd(v,map,gout,px,py,hx,hy)
	end

end
	
M.map_bgra_premultiply(M.map)


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

-- custom map codes for nicer looking handmade ascii

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
for i=32,255 do M.map[i]={bgra=0x00000000} end -- fill in the rest of the colors, so we can user all 255 indexs

for n,v in pairs(M.map) do
	if type(v)=="number" and type(n)=="string" then
		local t=M.map[v]
		if t then
			if #n==1 then -- force chars , set the prefered color code when asciiing
				t.code=n.." "
			else
				t.code=n
			end
		end
	end
end

for i=0,255 do M.map[ string.format("%02X",i)]=i M.map[ string.format("%02x",i)]=i end -- allow hex maps, upper or lower case


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
		if type(n)=="number" and type(v)=="table" then v.idx=n end -- remember color index
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

-- write some ascii art into an x,y grd location
M.pix_grd_idx=function(str,map,gout,px,py,hx,hy)

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
			t[l+1]=c.idx
		end
	end
	gout:pixels(px,py,hx,hy,t)

end

-- write a grd into some ascii art from an x,y grd location
M.grd_pix_idx=function(g,map,px,py,hx,hy)

	map=map or M.map

	px=px or 0
	py=py or 0
	
	if not hx and not hy then hx,hy=g.width,g.height end

	local ss=""
	
	local swanky32=true -- use the swanky32 palette nicer looking ascii codes

	local t=g:palette(0,256)
	for i=0,255 do
		local r=t[1+i*4]
		local g=t[2+i*4]
		local b=t[3+i*4]
		local a=t[4+i*4]
		local bgra=a*0x01000000 + r*0x00010000 + g*0x00000100 + b -- 32bit little endian
		if bgra ~= map[i].bgra then swanky32=false break end
	end

	for y=py,py+hy-1 do -- one line at a time

		local t=g:pixels(px,y,hx,1)

		local s={}
		for i,v in ipairs(t) do
			s[#s+1]=swanky32 and bitdown.map[v].code or string.format("%02X",v)
		end

		ss=ss..table.concat(s,"").."\n"
	end
	
	return ss
end

-- convert some pixel art into 2D tile array
M.pix_tiles=function(str,map,tout,px,py,hx,hy,tiles)

	local ls=wstr.split(str,"\n")

	map=map or M.map

	px=px or 0
	py=py or 0

	if not hx and not hy then hx,hy=M.pix_size(str) end

	tiles=tiles or {}

	local getxy=function(x,y)
		local l=ls[1+y]
		if l then
			return l:sub(1+x*2,1+x*2+1)
		end
	end

	local getc=function(s)
		return map[s] or map[s:sub(1,1)] or map[0]
	end
	
	local scopy=function(t) -- shallow copy
		local v={}
		for a,b in pairs(t) do v[a]=b end
		return v
	end
	
	for y=0,hy-1 do
		tiles[y+py]=tiles[y+py] or {}
		for x=0,hx-1 do
			local s=getxy(x,y) or ". "
			local c=scopy(getc(s) or {0,0,0,0}) -- this will be a new table per cell, shallow copy
			tiles[y+py][x+px]=c
		end
	end


	return tiles
end

-- take a tile map and merge adjacent tiles to create strips of parent/child tiles
-- you can then add each of these strips as a single shape instead of one per tile
M.map_build_collision_strips=function(map,callback)

	local tile_get=function(x,y,member)
		local l=map[y] if not l then return false end -- outer space returns nil 
		local c=l[x] if not c then return false end
		if member then return c[member] else return c end -- return the tile or a member
	end

	local tile_cmp=function(x,y,member,value)
		local l=map[y] if not l then return true end -- outer space compares true with anything
		local c=l[x] if not c then return true end
		return (c[member]==value)
	end

	local tile_is_solid=function(x,y)
		local l=map[y] if not l then return true end -- outer space is solid
		local c=l[x] if not c then return true end
		return c.solid and true or false
	end
	
-- go through and mark the solid/dense tiles we will build a level from

	for y,line in pairs(map) do for x,tile in pairs(line) do
		if tile.dense then
			tile.link=0
			tile.flow=0
			tile.coll="dense"
		elseif tile.solid then
			tile.link=0
			tile.flow=0
			if tile_is_solid(x,y-1) or tile_is_solid(x,y+1) then -- a solid tile with another solid tile above/below becomes dense
				tile.coll="dense"
				tile.dense=tile.solid
			else
				tile.coll="solid"
			end
		end
		if callback then callback(tile) end -- user can customise the coll member, IE split the map up into more discrete types.
		-- just append a type to tile.coll if tile.coll is not nil
	end end

-- we use flow>0 for x strips and flow<0 for y strips ( flow==nil for an empty space )

	for y,line in pairs(map) do for x,tile in pairs(line) do
		if tile.flow then
			if not tile_cmp(x-1,y,"coll",tile.coll) then
				tile.flow=tile.flow-1
			end
			if not tile_cmp(x+1,y,"coll",tile.coll) then
				tile.flow=tile.flow-1
			end
			if not tile_cmp(x,y-1,"coll",tile.coll) then
				tile.flow=tile.flow+1
			end
			if not tile_cmp(x,y+1,"coll",tile.coll) then
				tile.flow=tile.flow+1
			end
		end
	end end

-- try and make y strips

	for y,line in pairs(map) do for x,tile in pairs(line) do
		if tile.flow then
			if tile.flow<0 and tile.link==0 then -- this tile really wants to link up/down so grab all tiles
				tile.link=-1
				for ny=y-1,0,-1 do
					local ts=tile_get(x,ny)
					if ts and ts.coll==tile.coll and ts.link==0 and tile.flow<=0 then -- one of us
						ts.link=-1
					else break end
				end
				for ny=y+1,#map,1 do
					local ts=tile_get(x,ny)
					if ts and ts.coll==tile.coll and ts.link==0 and tile.flow<=0 then -- one of us
						ts.link=-1
					else break end
				end
			end
		end
	end end

-- try and make x strips

	for y,line in pairs(map) do for x,tile in pairs(line) do
		if tile.flow then
			if tile.flow>0 and tile.link==0 then -- this tile really wants to link left/right so grab all tiles
				tile.link=1
				for nx=x-1,0,-1 do
					local ts=tile_get(nx,y)
					if ts and ts.coll==tile.coll and ts.link==0 and tile.flow>=0 then -- one of us
						ts.link=1
					else break end
				end
				for nx=x+1,#line,1 do
					local ts=tile_get(nx,y)
					if ts and ts.coll==tile.coll and ts.link==0 and tile.flow>=0 then -- one of us
						ts.link=1
					else break end
				end
			end
		end
	end end

-- glob the rest together any old how we can

	for y,line in pairs(map) do for x,tile in pairs(line) do
		if tile.flow then
			for _,d in ipairs{ {-1,0,1} , {1,0,1} , {0,-1,-1} , {0,1,-1} } do
				if tile.link==0 then
					local ts=tile_get(x+d[1],y+d[2])
					if ts and ts.coll==tile.coll and ts.link==0 or ts.link==d[3] then
						tile.link=d[3]
						ts.link=d[3]
					end
				end
			end
		end
	end end

-- set parents / child of each strip. The parent is the tile that will generate a collision strip

	for y,line in pairs(map) do for x,tile in pairs(line) do
		if tile.link==1 then
			local ts=tile_get(x-1,y)
			if ts and ts.coll==tile.coll and ts.link==1 then
				tile.parent=ts
				ts.child=tile
			end
		end
		if tile.link==-1 then
			local ts=tile_get(x,y-1)
			if ts and ts.coll==tile.coll and ts.link==-1 then
				tile.parent=ts
				ts.child=tile
			end
		end
	end end


--debug dump of collision links
--[[
	for y,line in pairs(map) do
		local s=""
		for x,tile in pairs(line) do
			if not tile.parent then
				if     tile.link== 1 then s=s.."X "
				elseif tile.link==-1 then s=s.."Y "
				elseif tile.link== 0 then s=s.."0 "
				else                      s=s..". "
				end
			else
				if     tile.link== 1 then s=s.."x "
				elseif tile.link==-1 then s=s.."y "
				elseif tile.link== 0 then s=s.."o "
				else                      s=s..". "
				end
			end
		end
		print(s)
	end
]]

	return map
end

-- write a bunch of ascii into this bitmap, using its index as its location 0xYYXX,
M.pixtab_tiles=function(tab,map,tout)

	for n,v in pairs(tab) do
		local hx,hy=bitdown.pix_size(v,8,8)
		local px=math.floor(n%256)*tout.tile_hx
		local py=math.floor((n)/256)*tout.tile_hy
		bitdown.pix_grd(v,map,tout.bitmap_grd,px,py,hx,hy)
	end

end
	
M.map_bgra_premultiply(M.map)


-- a simple way of writing some text directly into a bitmap
M.setup_blit_font=function(g,w,h)
	local it={}
	
	if not g then -- build a 4x8 font
		w=4 h=8
		g=require("wetgenes.gamecake.fun.bitdown_font_4x8").grd_mask
	end
	
	it.grd=g
	it.w=w
	it.h=h

	it.draw=function(dest,x,y,s) -- blit some text at this location

		for c in s:gmatch("([%z\1-\127\194-\244][\128-\191]*)") do
			dest:blit(it.grd,x,y,(c:byte()%128)*it.w,0,it.w,it.h)
			x=x+it.w
		end

	end
	
	return it
end


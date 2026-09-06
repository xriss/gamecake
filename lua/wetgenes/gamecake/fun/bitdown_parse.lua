--
-- (C) 2016 Kriss@XIXs.com
--
--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local codes="%s" -- any white space
for i=0,31 do
	local v=bitdown.cmap_swanky32[i]
	local c=v.code:sub(1,1)
	if c == c:match("%p+") then -- is char punctuation
		c="%"..c -- escape punctuation
	end
	codes=codes..c
end
codes="["..codes.."]+" -- all valid bitdown chars patterns


M.scan_parts_from_text=function(funtext)

--	print("input size:",#funtext)

	local parts={}
	local part=""
	local chunk
	local append_part_string=function(s)
		if s=="" then return end
		if type(parts[#parts])=="string" then
			parts[#parts]=parts[#parts]..s
		else
			parts[#parts+1]=s
		end
	end

	local line_idx=1
	local char_idx=0
	for line in string.gmatch(funtext, "[^\n]*\n?") do -- all lines, last line will be ""

		if chunk then -- continue thinking
		
			part=part..line

			if line:sub(1,2)=="]]" then -- last line
				chunk.foot=line
				-- check bitmap
				local g=bitdown.pix_grd_idx(chunk.body)  -- convert from bitdown
				local s=bitdown.grd_pix_idx(g)           -- convert into bitdown
				if s==chunk.body then -- must match so we can recreate exactly
					parts[#parts+1]=chunk
					part=""
					chunk=nil
				else -- bad image
					chunk=nil
				end
			else -- continue body
				if line == line:match(codes) then -- possible image data
					chunk.body=chunk.body..line
				else
					chunk=nil -- failed to find
				end
			end
		
		else
			if line:sub(-3,-1)=="[[\n" then -- check for head line
				chunk={} -- start thinking
				chunk.head=line
				chunk.body=""
				append_part_string(part)
				part=""
			end
			part=part..line
		end

		line_idx=line_idx+1
		char_idx=char_idx+(#line)
	end
	append_part_string(part) -- final part

--[[
	for idx,part in ipairs(parts) do
		if type(part)=="string" then
			print("STRING",idx,part)
		else
			print("BITMAP",idx)
			print("HEAD" , part.head )
			print("BODY" , part.body )
			print("FOOT" , part.foot )
		end
	end
]]

	-- only return parts if we found some bitdown to convert
	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			return parts
		end
	end

	return nil
end


M.render_grd_from_parts=function(parts)

	local totx=0
	local toty=0
	local maxx=0
	local maxy=0
	local hx,hy=0,0
	local area=0
	
	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			part.grd=bitdown.pix_grd_idx(part.body)
			part.hx=math.ceil(part.grd.width/8)*8
			part.hy=math.ceil(part.grd.height/8)*8
			totx=totx+part.hx+16
			toty=toty+part.hy+16
			if part.hx+16 > maxx then maxx=part.hx+16 end -- max
			if part.hy+16 > maxy then maxy=part.hy+16 end -- max
			area=area+(part.hx+16)*(part.hy+16)
		end
	end
	hx=math.ceil(math.sqrt(area)/8)*8
	if hx<maxx then hx=maxx end
--	print( "total x" , totx )
--	print( "img x" , hx )
	
	-- layout and workout height
	local px,py=0,0
	local line=0
	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			if px+part.hx+16<=hx then -- fit
				part.px=px+8
				part.py=py+8
				px=px+part.hx+16
				if line<part.hy then line=part.hy end
			else -- next line
				py=py+line+16
				px=0
				line=0
				part.px=px+8
				part.py=py+8
				px=px+part.hx+16
				if line<part.hy then line=part.hy end
			end
		end
	end
	hy=py+line+16

	local g=wgrd.create("U8_INDEXED",hx,hy,1)
	g:palette(0,256,bitdown.cmap_swanky32.data) -- with palette

	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			g:pixels( part.px, part.py, part.grd.width, part.grd.height, part.grd )
			part.grd=nil -- forget bitmap, just remember px,py,hx,hy
		end
	end
	
--	g:save({fmt="png",filename="test.png"})

	g.json={}
	g.json.fun64={}
	g.json.fun64.parts=parts -- remember code and part layout

	return g
end


M.update_parts_from_grd=function(parts,grd)

	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			part.body_new=bitdown.grd_pix_idx( grd, nil, part.px, part.py, part.hx, part.hy )
		end
	end

end

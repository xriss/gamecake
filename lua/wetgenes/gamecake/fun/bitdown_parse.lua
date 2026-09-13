--
-- (C) 2026 Kriss@XIXs.com
--


local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local codes_swanky="%s" -- any white space
for i=0,31 do
	local v=bitdown.cmap_swanky32[i]
	local c=v.code:sub(1,1)
	if c == c:match("%p+") then -- is char punctuation
		c="%"..c -- escape punctuation
	end
	codes_swanky=codes_swanky..c
end
codes_swanky="["..codes_swanky.."]+" -- all valid bitdown chars patterns

local codes_hex="[0-9a-fA-F]+" -- any hex

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
				if chunk.is=="bmap" then
					if chunk.body_type=="swanky" then
						local g=bitdown.pix_grd_idx(chunk.body)  -- convert from bitdown
						local s=bitdown.grd_pix_idx(g)           -- convert into bitdown
						if s==chunk.body then -- must match so we can recreate exactly
							parts[#parts+1]=chunk
							part=""
							chunk=nil
						else -- bad image
							chunk=nil
						end
					else -- hex
						local g=bitdown.pix_grd_idx(chunk.body,bitdown.cmap_grey256)  -- convert from bitdown hex only
						local s=bitdown.grd_pix_idx(g,bitdown.cmap_grey256)           -- convert into bitdown hex only
						if s==chunk.body then -- must match so we can recreate exactly
							parts[#parts+1]=chunk
							part=""
							chunk=nil
						else -- bad image
							chunk=nil
						end
					end
				else
					chunk=nil
				end
			else -- continue body
				local nosline=line:match("^%s*(.-)%s*$") -- trim whitespace

				-- is hex or swanky
				if not chunk.body_type and nosline~="" then
					if nosline == nosline:match(codes_hex) then
						chunk.body_type="hex"
					elseif nosline == nosline:match(codes_swanky) then
						chunk.body_type="swanky"
					end
				end

				if chunk.body_type=="hex" -- possible hex image data
				and nosline == nosline:match(codes_hex) -- must match
				then
					chunk.body=chunk.body..line

				elseif chunk.body_type=="swanky" -- possible swanky image data
				and nosline == nosline:match(codes_swanky) -- must match
				then
					chunk.body=chunk.body..line

				else
					chunk=nil -- failed to find

				end
			end
		
		else
			if line:sub(-3,-1)=="[[\n" then -- check for head line
				chunk={} -- start thinking
				chunk.is="bmap"
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
	
	-- scan the parts looking for chunks we can merge into a tilemap
	local tilemap
	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			if part.is == "bmap" then
			end
		end
	end
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

M.render_text_from_parts=function(parts)

	local tt={}
	local push=function(s) tt[#tt+1]=s end

	for idx,part in ipairs(parts) do
		if type(part)=="table" then

			if part.is == "bmap" then
				push(part.head_new or part.head)
				push(part.body_new or part.body)
				push(part.tail_new or part.tail_new)
			else
				error( "unknown part "..part.is )
			end
		else
			push(part)
		end
	end

	return table.concat(tt)

end


M.debug_parts_string=function(parts)

	local tt={}
	local push=function(s) tt[#tt+1]=s end
	local prints=function(...)
		local tt={...}
		if tt[1] then
			for i=1,#tt do tt[i]=tostring(tt[i]) end
			push( table.concat(tt,"\t") )
		end
		push("\n")
	end

	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			if part.is == "bmap" then
				local g=bitdown.pix_grd_idx(part.body)  -- convert from bitdown
				prints("bmap",g.width,g.height, (part.head_new or part.head) )
			else
				error( "unknown part "..part.is )
			end
		else
			prints("string",#part)
		end
	end
	
	local g=M.render_grd_from_parts(parts)
			prints("renders-into",g.width,g.height)

	return table.concat(tt)

end

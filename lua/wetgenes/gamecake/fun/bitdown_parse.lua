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
				if chunk.is=="bitmap" then
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
				chunk.is="bitmap"
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
	local tilemaps={}
	local tilemap
	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			if part.is == "bitmap" then
				local g=bitdown.pix_grd_idx(part.body)  -- convert from bitdown
				local check_start=function()
					if part.body_type=="swanky" and g.width==8 and g.height==8 then -- start tiles
						tilemap={}
						tilemap.tile_head=idx
						tilemap.tile_foot=idx
					end
				end
				if not tilemap then
					check_start()
				else -- continue
					if tilemap.map_head then -- continue maps
						if part.body_type=="hex" then -- continue maps
							tilemap.map_foot=idx
						else
							tilemaps[#tilemaps+1]=tilemap -- finish tilemap
							tilemap=nil
							check_start()
						end
					else -- continue tiles or start maps
						if part.body_type=="swanky" and g.width==8 and g.height==8 then
							tilemap.tile_foot=idx
						elseif part.body_type=="hex" then -- start maps
							tilemap.map_head=idx
							tilemap.map_foot=idx
						else
							tilemap=nil
						end
					end
				end
			end
		end
	end
	if tilemap and tilemap.map_foot then -- finish
		tilemaps[#tilemaps+1]=tilemap -- finish tilemap
		tilemap=nil
	end
	if tilemaps[1] then -- rejiggle for found tilemaps
		for ti,tilemap in ipairs(tilemaps) do
			tilemap.is="tilemap"
			tilemap.tiles={}
			tilemap.maps={}
			local tile
			local newtile=function(part)
				local tile={}
				tilemap.tiles[#tilemap.tiles+1]=tile
				tile.is="tile"
				tile.head=part.head
				tile.body=part.body
				tile.foot=part.foot
				tile.tail=""
				return tile
			end
			for i=tilemap.tile_head,tilemap.map_head-1 do
				local part=parts[i]
				if type(part)=="string" then -- append between strings
					tile.tail=tile.tail..part
				else
					tile=newtile(part)
				end
			end
			local last_tile=tilemap.tiles[#tilemap.tiles]
			tilemap.tiles_tale=last_tile.tail
			last_tile.tail=""
			local map
			local newmap=function(part)
				local map={}
				tilemap.maps[#tilemap.maps+1]=map
				map.is="map"
				map.head=part.head
				map.body=part.body
				map.foot=part.foot
				map.tail=""
				return map
			end
			for i=tilemap.map_head,tilemap.map_foot do
				local part=parts[i]
				if type(part)=="string" then -- append between strings
					map.tail=map.tail..part
				else
					map=newmap(part)
				end
			end
			local bb={}
			for _,tile in ipairs(tilemap.tiles) do
				bb[#bb+1]=tile.head
				bb[#bb+1]=tile.body
				bb[#bb+1]=tile.foot
				bb[#bb+1]=tile.tail
			end
			bb[#bb+1]=tilemap.tiles_tale
			for _,map in ipairs(tilemap.maps) do
				bb[#bb+1]=map.head
				bb[#bb+1]=map.body
				bb[#bb+1]=map.foot
				bb[#bb+1]=map.tail
			end
			tilemap.head=""
			tilemap.body=table.concat(bb) -- shove everything in body
			tilemap.foot=""
		end
		-- put tilemaps into parts
		local oldparts=parts
		local idx=0
		parts={}
		for ti,tilemap in ipairs(tilemaps) do
			while idx<tilemap.tile_head-1 do
				idx=idx+1
				parts[#parts+1]=oldparts[idx]
			end
			parts[#parts+1]=tilemap
			idx=tilemap.map_foot
			-- remove old part indexes
			tilemap.tile_head=nil
			tilemap.tile_foot=nil
			tilemap.map_head=nil
			tilemap.map_foot=nil
		end
		while idx<#oldparts do
			idx=idx+1
			parts[#parts+1]=oldparts[idx]
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

	local maxx=0
	local maxy=0
	local hx,hy=0,0
	local area=0
	
	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			if part.is=="bitmap" then
				part.grd=bitdown.pix_grd_idx(part.body)
				part.hx=math.ceil(part.grd.width/8)*8
				part.hy=math.ceil(part.grd.height/8)*8
				if part.hx+16 > maxx then maxx=part.hx+16 end -- max
				if part.hy+16 > maxy then maxy=part.hy+16 end -- max
				area=area+(part.hx+16)*(part.hy+16)
			elseif part.is=="tilemap" then
				local tc=#part.tiles
				if 16*8+16 > maxx then maxx=16*8+16 end -- max
				if    8+16 > maxy then maxy=   8+16 end -- max
				area=area+(tc*8+16)*(8+16)
				for _,tile in ipairs(part.tiles) do
					tile.grd=bitdown.pix_grd_idx(tile.body)
					tile.hx=8
					tile.hy=8
				end
				for _,map in ipairs(part.maps) do
					map.grd=bitdown.pix_grd_idx(map.body)
					map.hx=map.grd.width*8
					map.hy=map.grd.height*8
					if map.hx+16 > maxx then maxx=map.hx+16 end -- max
					if map.hy+16 > maxy then maxy=map.hy+16 end -- max
					area=area+(map.hx+16)*(map.hy+16)
				end
			end
		end
	end
	hx=math.ceil(math.sqrt(area)/8)*8
	if hx<maxx then hx=maxx end
--	print( "total x" , totx )
--	print( "img x" , hx )
	
	-- layout and workout height
	local px,py=0,0
	local line=0
	hy=0
	local next_line=function()
		if line>0 then
			py=py+line+16
			if py>hy then hy=py end
		end
		line=0
		px=0
	end
	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			if part.is=="bitmap" then
				if px+part.hx+16<=hx then -- fit
					part.px=px+8
					part.py=py+8
					px=px+part.hx+16
					if line<part.hy then line=part.hy end
				else -- next line
					next_line()
					part.px=px+8
					part.py=py+8
					px=px+part.hx+16
					if line<part.hy then line=part.hy end
				end
			elseif part.is=="tilemap" then
				-- always take full width
				next_line()
				local tc=#part.tiles
				line=8
				part.px=px+8
				part.py=py+8
				part.hx=16*8 -- force 16 tiles across
				line=8 -- start height
				local tx,ty=0,0
				for _,tile in ipairs(part.tiles) do
					if 8+tx+8+8>hx then -- wrap
						tx=0
						ty=ty+8
						line=line+8
					end
					tile.px=px+8+tx
					tile.py=py+8+ty
					tx=tx+8
				end
				part.hy=line
				next_line()
				for _,map in ipairs(part.maps) do -- each map on its own line
					map.px=px+8
					map.py=py+8
					line=map.hy
					next_line()
				end
			end
		end
	end
	next_line() -- in case we need a final one

	local g=wgrd.create("U8_INDEXED",hx,hy,1)
	g:palette(0,256,bitdown.cmap_swanky32.data) -- with palette

	for idx,part in ipairs(parts) do
		if type(part)=="table" then
			if part.is=="bitmap" then
				g:pixels( part.px, part.py, part.hx, part.hy, part.grd )
				part.grd=nil -- forget bitmap, just remember px,py,hx,hy
			elseif part.is=="tilemap" then
				for _,tile in ipairs(part.tiles) do
					g:pixels( tile.px, tile.py, tile.hx, tile.hy, tile.grd )
				end
				for _,map in ipairs(part.maps) do
					local mx=map.grd.width
					local my=map.grd.height
					local pp=map.grd:pixels(0,0,mx,my) -- get tile indexes ( max 256 tiles )
					for y=0,my-1 do
						for x=0,mx-1 do
							local ti=pp[y*mx+x+1]
							local tile=part.tiles[ti+1]
							if tile then
								g:pixels( map.px + x*8, map.py + y*8, 8, 8, tile.grd )
							end
						end
					end
				end
				for _,tile in ipairs(part.tiles) do
					tile.grd=nil -- forget bitmap, just remember px,py,hx,hy
				end
				for _,map in ipairs(part.maps) do
					map.grd=nil -- forget bitmap, just remember px,py,hx,hy
				end
			end
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
			if part.is=="bitmap" then
				part.body_new=bitdown.grd_pix_idx( grd, nil, part.px, part.py, part.hx, part.hy )
			elseif part.is=="tilemap" then
				local gt=wgrd.create("U8_INDEXED",8,8,1)
				gt:palette(0,256,bitdown.cmap_swanky32.data) -- with palette
				local bitcache={}
				for idx,tile in ipairs(part.tiles) do
					tile.body_new=bitdown.grd_pix_idx( grd, nil, tile.px, tile.py, tile.hx, tile.hy )
					tile.bitstr=grd:pixels(tile.px, tile.py,8,8,"")
					tile.idx=idx
					bitcache[tile.bitstr]=tile
				end
				local last_tile=part.tiles[#part.tiles]
				local manifest_tile=function(bitstr)
					local tile=bitcache[bitstr]
					if not tile then -- add new one
						tile={}
						tile.is="temp"
						tile.bitstr=bitstr
						tile.idx=#part.tiles+1
						part.tiles[tile.idx]=tile
						bitcache[tile.bitstr]=tile
						gt:pixels( 0, 0, 8, 8, bitstr )
						tile.head_new=last_tile.head
						tile.body_new=bitdown.grd_pix_idx( gt, nil, 0, 0, 8, 8 )
						tile.foot_new=last_tile.foot
					end
					return tile
				end
				for _,map in ipairs(part.maps) do
					local mapdata={}
					for y=map.py,map.py+map.hy-1,8 do
						for x=map.px,map.px+map.hx-1,8 do
							local bitstr=grd:pixels(x,y,8,8,"")
							local tile=manifest_tile(bitstr)
							mapdata[#mapdata+1]=tile and tile.idx-1 or 0
						end
					end
					gt:pixels( 0, 0, map.hx/8, map.hy/8, mapdata )
					map.body_new=bitdown.grd_pix_idx( gt, bitdown.cmap_grey256, 0, 0, map.hx/8, map.hy/8 )
				end
				local bb={}
				for _,tile in ipairs(part.tiles) do
					bb[#bb+1]=tile.head_new or tile.head or ""
					bb[#bb+1]=tile.body_new or tile.body or ""
					bb[#bb+1]=tile.foot_new or tile.foot or ""
					bb[#bb+1]=tile.tail_new or tile.tail or ""
				end
				bb[#bb+1]=part.tiles_tale
				for _,map in ipairs(part.maps) do
					bb[#bb+1]=map.head_new or map.head or ""
					bb[#bb+1]=map.body_new or map.body or ""
					bb[#bb+1]=map.foot_new or map.foot or ""
					bb[#bb+1]=map.tail_new or map.tail or ""
				end
				part.body_new=table.concat(bb) -- shove everything in body
				-- remove the extra data and temp tiles we added
				for idx=#part.tiles,1,-1 do
					local tile=part.tiles[idx]
					if tile.is=="temp" then
						table.remove(part.tiles,idx)
					else
						tile.bitstr=nil
						tile.idx=nil
					end
				end
				for _,map in ipairs(part.maps) do
					map.bitstr=nil
				end
			else
				-- unknown
			end
		end
	end

end

M.render_string_from_parts=function(parts)

	local tt={}
	local push=function(s) tt[#tt+1]=s end

	for idx,part in ipairs(parts) do
		if type(part)=="table" then

			if part.is == "bitmap" then
				push(part.head_new or part.head)
				push(part.body_new or part.body)
				push(part.foot_new or part.foot)
			elseif part.is == "tilemap" then
				push(part.head_new or part.head)
				push(part.body_new or part.body)
				push(part.foot_new or part.foot)
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
			if part.is == "bitmap" then
				local g=bitdown.pix_grd_idx(part.body)  -- convert from bitdown
				prints("bitmap",g.width,g.height, (part.head_new or part.head) )
			elseif part.is == "tilemap" then
				prints("tilemap","tiles:"..#part.tiles,"maps:"..#part.maps )
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

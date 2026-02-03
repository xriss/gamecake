--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wgrdpaint=require("wetgenes.grdpaint")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")
local wstr=require("wetgenes.string")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,tiles)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

tiles.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return tiles
end

tiles.setup=function()

	tiles.load()

	return tiles
end

tiles.create=function(it,opts)
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.colors=assert(it.system.components[opts.colors or "colors"])
	it.opts=opts
	it.component="tiles"
	it.name=opts.name or it.component
	
	it.names={}
	
	it.tile_hx=it.opts.tile_size and it.opts.tile_size[1] or 8
	it.tile_hy=it.opts.tile_size and it.opts.tile_size[2] or 8

	it.bitmap_hx=it.opts.bitmap_size and it.opts.bitmap_size[1] or 16
	it.bitmap_hy=it.opts.bitmap_size and it.opts.bitmap_size[2] or 16
	
	it.hx=it.tile_hx*it.bitmap_hx
	it.hy=it.tile_hy*it.bitmap_hy
	
	it.idxnames={}
	
--	it.hx=2^math.ceil( math.log(it.hx)/math.log(2) ) -- force power of 2?
--	it.hy=2^math.ceil( math.log(it.hy)/math.log(2) )


--	it.drawtype=opts.drawtype or "first"
	it.layer=opts.layer or 0

	it.setup=function(opts)
		
		it.bitmap_grd  =wgrd.create("U8_RGBA", it.hx , it.hy , 1)
		it.canvas=wgrdpaint.canvas(it.bitmap_grd) -- drawing functions

		it.bitmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)
		
		it.dirty(true)

	end

	it.clean=function()
		if it.bitmap_tex then
			gl.DeleteTexture( it.bitmap_tex )
			it.bitmap_tex=nil
		end
	end

	it.update=function()
	end
	
	it.draw=function()

-- update the bitmap, but do not draw anything
		if it.dirty() then
			it.dirty(false)
			gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
			gl.TexImage2D(
				gl.TEXTURE_2D,
				0,
				gl.RGBA,
				it.bitmap_grd.width,
				it.bitmap_grd.height,
				0,
				gl.RGBA,
				gl.UNSIGNED_BYTE,
				it.bitmap_grd.data )
		end

	end

	it.reset_tiles=function()
		for n,v in pairs( it.idxnames ) do
			it.idxnames[n]=nil
		end
		for n,v in pairs( it.names ) do
			it.idxnames[n]=nil
		end
		it.bitmap_grd:clear(0)
		it.dirty(true)
	end
	
	-- first line
	it.upload_default_font_4x8=function()
		local bitdown_font=require("wetgenes.gamecake.fun.bitdown_font")
		-- copy font data tiles into top line
		it.bitmap_grd:pixels(0,0 ,128*4,8, bitdown_font.build_grd(4,8):pixels(0,0,128*4,8,"") )
		it.upload_tiles( { {0x0000,"_font_4x8",0x0140} } )
		it.dirty(true)
	end
	-- second and third line
	it.upload_default_font_8x8=function()
		local bitdown_font=require("wetgenes.gamecake.fun.bitdown_font")
		-- and 8x8 font 
		local g=bitdown_font.build_grd(8,8) -- 1 long line
		it.bitmap_grd:pixels(0,8 ,64*8,8, g:pixels(0,   0,64*8,8,"") ) -- 2 lines
		it.bitmap_grd:pixels(0,16,64*8,8, g:pixels(64*8,0,64*8,8,"") )
		it.upload_tiles( { {0x0100,"_font_8x8",0x0240} } )
		it.dirty(true)
	end

-- upload all the given tiles t[1]=idx t[2]=name t[3]=ascii
	it.upload_tiles=function(graphics)
		for n,v in ipairs(graphics) do
			local t={}
			t.idx=v[1]
			t.name=v[2]
			t.cuts=v[4]

			if type(v[3])=="number" then -- just mark as allocated
			
				t.ascii=nil

				t.hxt=math.floor(v[3]%256)
				t.hyt=math.floor((v[3])/256)
				
				t.hx=t.hxt*it.tile_hx
				t.hy=t.hyt*it.tile_hy
			
			else

				t.ascii=v[3]

				t.hx,t.hy=bitdown.pix_size(t.ascii,it.tile_hx,it.tile_hy)
				
				t.hxt=math.floor(t.hx/it.tile_hx)
				t.hyt=math.floor(t.hy/it.tile_hy)
			
			end
			
			if t.idx then

				t.pxt=math.floor(t.idx%256)
				t.pyt=math.floor((t.idx)/256)

				t.px=t.pxt*it.tile_hx
				t.py=t.pyt*it.tile_hy
				
			else -- find a free spot
			
				local tilecheck=function(px,py,hx,hy)
					for y=py,py+hy-1 do
						for x=px,px+hx-1 do
							if x<1 or y<1 or x>it.bitmap_hx or y>it.bitmap_hy then return false end -- out of bounds
							if it.idxnames[ x + ((y-1)*256) ] then return false end -- already used
						end
					end
					return true
				end
			
				local hx,hy=t.hx/it.tile_hx,t.hy/it.tile_hy
--				print(t.name,hx,hy)
				for y=1,it.bitmap_hy do
					for x=1,it.bitmap_hx do
						if tilecheck(x,y,hx,hy) then
							t.pxt=(x-1)
							t.pyt=(y-1)
							t.px=t.pxt*it.tile_hx
							t.py=t.pyt*it.tile_hy
							t.idx=(y-1)*256+(x-1)
							break
						end
					end
					if t.idx then break end
				end
--				print(string.format("%04x",t.idx),hx,hy,t.name)
			
			end
			
			assert(t.idx) -- make sure we found a place to live
			
			if t.ascii then
				bitdown.pix_grd(t.ascii,map,it.bitmap_grd,t.px,t.py,t.hx,t.hy)
			end
			
			-- keep lookups
			it.names[t.name]=t -- by name
			it.names[t.idx]=t -- by number

			-- remember allocated zones

			for y=t.pyt,t.pyt+t.hyt-1 do
				for x=t.pxt,t.pxt+t.hxt-1 do
					it.idxnames[ 1 + x + y*256 ]=t.name
				end
			end
			
			-- cut horizontal slices into sub sprites
			if t.cuts then
				local cuts={}
				for i=1,t.cuts do
					local c={}
					cuts[i]=c
					for n,v in pairs(t) do c[n]=v end -- dupe
					c.cuts=nil
					c.ascii=nil
					c.hx=math.floor(t.hx/t.cuts)
					c.hxt=math.floor(t.hxt/t.cuts)
					c.pxt=c.pxt+((i-1)*c.hxt)
					c.px=c.pxt*it.tile_hx
					c.idx=c.idx+((i-1)*c.hxt)
				end
				t.cuts=cuts
			end
		end
		
--[[
			for y=0,it.bitmap_hy-1 do
				for x=0,it.bitmap_hx-1 do
					if it.idxnames[ 1 + x + y*256 ] then
						print(string.format("%04x %s",x+y*256,it.idxnames[ 1 + x + y*256 ]))
					end
				end
			end
]]

	end
		
	it.dirty_flag=true
	it.dirty=function(flag)
		if type(flag)=="boolean" then it.dirty_flag=flag end
		return it.dirty_flag
	end

	return it
end

	return tiles
end



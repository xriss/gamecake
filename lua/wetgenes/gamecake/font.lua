--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,font)

	local canvas=oven.rebake("wetgenes.gamecake.canvas")
	local flat=oven.rebake("wetgenes.gamecake.flat")

	local gl=oven.gl
	local cake=oven.cake
	local win=oven.win
	local fonts=cake.fonts
	local images=cake.images
	local buffers=cake.buffers


font.set = function(dat)
	do local t=type(dat) if t=="string" or t=="number" then dat=fonts.get(dat) end end
	if dat and dat~=font.dat then -- newfont, autokill the cache?
		font.dat=dat
	end
	font.dat=dat or font.dat
	font.size=16
	font.add=0
	font.x=0
	font.y=0
--	if gl.patch_functions_method~="disable" then
		core.canvas_font_sync(font)
--	end
end

font.set_size = function(size,add)
	font.size=size
	font.add=add or 0 -- clear the x space tweak
--	if gl.patch_functions_method~="disable" then
		core.canvas_font_sync(font)
--	end
end
font.set_xy = function(x,y)
	font.x=x or font.x
	font.y=y or font.y
--	if gl.patch_functions_method~="disable" then
		core.canvas_font_sync(font)
--	end
end

font.xindex=function(text,px,dat,size,add)
	if px<0 then return 1 end
	
	local font_dat=dat or font.dat
	local s=(size or font.size)/font_dat.size
	local x=0
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]
		
		x=x+(c.add*s)+(add or font.add)
		
		if x>=px then return i-1 end
	end

	return #text
end

font.width=function(text)

	local font_dat=font.dat
	local s=font.size/font_dat.size
	local x=0
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]
		
		x=x+(c.add*s)+font.add
	end

	return x
end


function font.wrap(text,opts)
	local w=opts.w
	
	local ls=wstr.split_whitespace(text)
	local t={}
	
	local wide=0
	local line={}
	
	local function newline()
		t[#t+1]=table.concat(line," ") or ""
		wide=0
		line={}
	end
	
	for i,v in ipairs(ls) do
	
		if v:find("%s") then -- just white space

			for i,v in string.gmatch(v,"\n") do -- keep newlines
				newline()
			end
		
		else -- a normal word
		
			local fw=font.width(v)
			if #line>0 then wide=wide+font.width(" ") end

			if wide + fw > w then -- split
				newline()
			end
			
			line[#line+1]=v
			wide=wide+fw
			
		end
	end
	if wide~=0 then newline() end -- final newline
	
	return t
end

font.cache_begin = function()
--print("font begine",tostring(font.dat))
	local t={}
	local old=font.cache
	font.cache=t
	return function()
		local r,g,b,a=gl.color_get_rgba()
		gl.Color(1,1,1,1)	
		for d,v in pairs(t) do -- multifonts
--print("font draw",#v,tostring(d))
			if v[1] then
				images.bind(d.images[1])
				flat.tristrip("rawuvrgba",v)
			end
		end
		gl.Color(r,g,b,a)
		font.cache=old
	end
end

font.cache_predraw = function(text)

	local font_dat=font.dat
	local font_cache=font.cache[ font_dat ] or {}
	font.cache[ font_dat ]=font_cache
	
	local s=font.size/font_dat.size
	local x=font.x
	local y=font.y

	local insert=function(a,b,c,d,e,f,g,h,i)
		local idx=#font_cache
		local fc=font_cache
		fc[idx+1]=a	fc[idx+2]=b	fc[idx+3]=c
		fc[idx+4]=d	fc[idx+5]=e
		fc[idx+6]=f	fc[idx+7]=g	fc[idx+8]=h	fc[idx+9]=i			
	end

	local r,g,b,a=gl.color_get_rgba()

	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]
				
		local vx=x+(c.x*s);
		local vxp=c.w*s;
		local vy=y+(c.y*s);
		local vyp=c.h*s;

		local v1=gl.apply_modelview( {vx,vy,0,1} )
		local v2=gl.apply_modelview( {vx+vxp,vy,0,1} )
		local v3=gl.apply_modelview( {vx,vy+vyp,0,1} )
		local v4=gl.apply_modelview( {vx+vxp,vy+vyp,0,1} )
		
		insert(	v1[1],v1[2],v1[3],	c.u1,c.v1,	r,g,b,a	)
		insert(	v1[1],v1[2],v1[3],	c.u1,c.v1,	r,g,b,a	)
		insert(	v2[1],v2[2],v2[3],	c.u2,c.v1,	r,g,b,a	)
		insert(	v3[1],v3[2],v3[3],	c.u1,c.v2,	r,g,b,a	)
		insert(	v4[1],v4[2],v4[3],	c.u2,c.v2,	r,g,b,a	)
		insert(	v4[1],v4[2],v4[3],	c.u2,c.v2,	r,g,b,a	)
		
		x=x+(c.add*s)+font.add
	end


end


font.draw = function(text)

	if font.cache then
		return font.cache_predraw(text)
	end


	local dataraw,datalen=core.canvas_font_draw(font,text)
	if datalen/5>=1 then -- need something to draw
		gl.ActiveTexture( gl.TEXTURE0 )
		images.bind(font.dat.images[1])
		local it=flat.array_predraw({fmt="posuv",dataraw=dataraw,datalen=datalen,array=gl.TRIANGLES})
		it.draw(cb)
	end

end

	cake.fonts.load(1,1) -- make sure we have loaded the 8x8 font
	font.set( cake.fonts.get(1) ) -- now use it

	return font
end

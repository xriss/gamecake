--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- A copper like background, uses a glsl fragment program to fill the screen,

local wzips=require("wetgenes.zips")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,copper)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat
	local layouts=cake.layouts

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")


copper.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return copper
end

copper.setup=function()

	copper.load()

	return copper
end

copper.create=function(it,opts)
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.opts=opts
	it.component="copper"
	it.name=opts.name or it.component

	it.lox=it.opts.lox
	it.loy=it.opts.loy
	it.hix=it.opts.hix
	it.hiy=it.opts.hiy

	it.layer=opts.layer or 1
	local layer=assert(it.screen.layers[it.layer])
	
	it.hx=it.opts.size and it.opts.size[1] or layer.hx or 320
	it.hy=it.opts.size and it.opts.size[2] or layer.hy or 240

-- set shader program name and callback to fill in uniform values

	it.shader_name="" -- "fun_copper_back_y3"
	it.shader_uniforms={
		ticks={0,0,0,0},
		sizpos={it.hx,it.hy,0,0},
		cy0={0,0,0,1},
		cy1={0,0,0,1},
		cy2={0,0,0,1},
		cy3={0,0,0,1},
		cy4={0,0,0,1},
	}
	it.shader_function=function()end
	
	it.screen_resize=function(hx,hy)
		if hx~=it.hx or hy~=it.hy then -- new size
		
			it.hx=hx
			it.hy=hy

			it.shader_uniforms.sizpos[1]=hx
			it.shader_uniforms.sizpos[2]=hy

		end
	end

	it.update=function()
	end

	it.draw=function()
	
		if not it.shader_name or it.shader_name=="" then return end

		local layer=it.screen.layers[it.layer]

		gl.Color(1,1,1,1)

		local v3=gl.apply_modelview( {layer.hx*-0.0,	layer.hy* 1.0,	-32,1} )
		local v1=gl.apply_modelview( {layer.hx*-0.0,	layer.hy*-0.0,	-32,1} )
		local v4=gl.apply_modelview( {layer.hx* 1.0,	layer.hy* 1.0,	-32,1} )
		local v2=gl.apply_modelview( {layer.hx* 1.0,	layer.hy*-0.0,	-32,1} )

		local t={
			v3[1],	v3[2],	v3[3],	0,		it.hy,
			v1[1],	v1[2],	v1[3],	0,		0,
			v4[1],	v4[2],	v4[3],	it.hx,	it.hy, 			
			v2[1],	v2[2],	v2[3],	it.hx,	0,
		}

		gl.DepthMask(gl.FALSE)

		flat.tristrip("rawuv",t,it.shader_name,function(p)

--			gl.Uniform2f( p:uniform("projection_zxy"), 0,0)

			for n,v in pairs(it.shader_uniforms) do

				local id=p:uniform(n)
				
				if id then
					if #v==4 then
						gl.Uniform4f( id, unpack(v) )
					end
				end

			end

-- shadertoy compatability
			local id=p:uniform("iResolution")
			if id then
				gl.Uniform3f( id, it.hx,it.hy,0 )
			end

			local id=p:uniform("iGlobalTime")
			if id then
				gl.Uniform1f( id, it.system.ticks/60 )
			end




		end)
		gl.DepthMask(gl.TRUE)
		
	end

	return it
end


	return copper
end

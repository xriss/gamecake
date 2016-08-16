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
	it.screen=it.system.components.screen -- system will have been passed in
	it.opts=opts
	it.component="copper"
	it.name=opts.name
	
	it.xh=it.opts.size and it.opts.size[1] or it.screen.xh or 360
	it.yh=it.opts.size and it.opts.size[2] or it.screen.yh or 240

-- set shader program name and callback to fill in uniform values

	it.shader_name="fun_copper_back_y5"
	it.shader_uniforms={
		ticks={0,0,0,0},
		sizpos={it.xh,it.yh,0,0},
		cy0={ 0.5 , 0   , 0.0 , 1 },
		cy1={ 0   , 0   , 1.0 , 1 },
		cy2={ 0   , 0.5 , 1.0 , 1 },
		cy3={ 0   , 0   , 1.0 , 1 },
		cy4={ 0   , 0.5 , 0.0 , 1 },
	}
	it.shader_function=function()end
	
	it.update=function()
		local u=it.shader_uniforms
		u.ticks[1]=u.ticks[1]+1
		u.ticks[2]=u.ticks[1]%(256*256*256)
		u.ticks[3]=u.ticks[1]%(256*256)
		u.ticks[4]=u.ticks[1]%(256)
	end

	it.draw=function()

--			gl.PushMatrix()

		local v3=gl.apply_modelview( {it.screen.xh*-0.0,	it.screen.yh* 1.0,	0,1} )
		local v1=gl.apply_modelview( {it.screen.xh*-0.0,	it.screen.yh*-0.0,	0,1} )
		local v4=gl.apply_modelview( {it.screen.xh* 1.0,	it.screen.yh* 1.0,	0,1} )
		local v2=gl.apply_modelview( {it.screen.xh* 1.0,	it.screen.yh*-0.0,	0,1} )

		local t={
			v3[1],	v3[2],	v3[3],	0,		it.yh, 			
			v1[1],	v1[2],	v1[3],	0,		0,
			v4[1],	v4[2],	v4[3],	it.xh,	it.yh, 			
			v2[1],	v2[2],	v2[3],	it.xh,	0,
		}

		flat.tristrip("rawuv",t,it.shader_name,function(p)
			for n,v in pairs(it.shader_uniforms) do

				local id=p:uniform(n)
				
				if id then
					if #v==4 then
						gl.Uniform4f( id, unpack(v) )
					end
				end

			end
		end)

--			gl.PopMatrix()
		
	end

	return it
end


	return copper
end

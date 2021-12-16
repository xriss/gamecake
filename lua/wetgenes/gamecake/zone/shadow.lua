--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wzips=require("wetgenes.zips")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,shadow)
	shadow=shadow or {}
	shadow.modname=M.modname
	
	local gl=oven.gl

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local screen=oven.rebake("wetgenes.gamecake.zone.screen")

	shadow.mtx=M4()

	shadow.mapsize=2048*2

	shadow.loads=function()

		local filename="lua/"..string.gsub(M.modname,"%.","/")..".glsl"
		gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	end
	
	shadow.setup=function()

		shadow.loads()

		shadow.fbo=framebuffers.create(shadow.mapsize,shadow.mapsize,-1,{ no_uptwopow=true , depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT, gl.FLOAT} } )
		shadow.view=oven.cake.views.create({
			mode="fbo",
			fbo=shadow.fbo,
			fov=0,
			cx=0.5,cy=0.5,cz=0.5,
		})

		gl.uniforms.shadow_map=function(u)
			gl.ActiveTexture(gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			shadow.fbo:bind_depth()
			gl.Uniform1i( u , gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
		end

		gl.uniforms.shadow_mtx=function(u)
			gl.UniformMatrix4f( u,  shadow.mtx )
		end


	end
	
--	shadow.default="0.0,0.0,0.0,0.0"
	shadow.draw_head=function(scene)

		gl.PushMatrix()
		shadow.fbo:bind_frame()
		oven.cake.views.push_and_apply(shadow.view)
		gl.state.push(gl.state_defaults)

		gl.state.set({
			[gl.BLEND]						=	gl.FALSE,
			[gl.DEPTH_TEST]					=	gl.TRUE,
			[gl.CULL_FACE]					=	gl.TRUE,
			[gl.FRONT_FACE]					=	gl.CW,	-- shadows are drawn upside down?
		})

		gl.Clear(gl.DEPTH_BUFFER_BIT)

		local camera=scene.get("camera")
		local sky=scene.systems.sky or {time=0}
		if camera then

			local s=256 -- 40*shadow.mapsize/1024
			local sd=1024
			local x=(math.floor(camera.pos[1]))*-1/s	-- swap z/y as rotation
			local y=(math.floor(camera.pos[3]))*-1/s
			local z=(math.floor(camera.pos[2]))*-1/s
			
			screen.shader_qs.zone_screen_build_occlusion.SHADOW="0.6,"..0.000000*s/sd..","..0.000008*s/sd..",0.0"

			shadow.mtx=M4{
				1/s,		0,			0,			0,
				0,			0,			1/s,		0,
				0,			1/s,		0,			0,
				0,			0,			0,			1,
			}
			local r=360*(((sky.time)/60)%1)
			if r>180 and r<360 then
				r = 90 + r
			else
				r = 90 + 360-r
			end

			if r < 90+180+6 then r=90+180+6 end
			if r > 90+360-6 then r=90+360-6 end

			shadow.mtx:pretranslate(x,y,z)
			shadow.mtx:rotate( 120 , 0,1,0 ) -- eastwest ish
			shadow.mtx:prerotate( r , -1,0,0 ) -- time of day
			shadow.mtx:prescale(1,1,1/sd)

			gl.MatrixMode(gl.PROJECTION)
			gl.LoadMatrix( shadow.mtx )

			gl.MatrixMode(gl.MODELVIEW)
			gl.LoadIdentity()

		end



	end

	shadow.draw_tail=function()

		gl.state.pop()
		oven.cake.views.pop_and_apply()
		gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
		gl.PopMatrix()

	end

	shadow.draw_test=function()

		gl.PushMatrix()
		gl.Translate(20,-256,-256)

		local w=256
		local h=256

		local v1=gl.apply_modelview( {w*-0,	h* 1,	0,1} )
		local v2=gl.apply_modelview( {w*-0,	h*-0,	0,1} )
		local v3=gl.apply_modelview( {w* 1,	h* 1,	0,1} )
		local v4=gl.apply_modelview( {w* 1,	h*-0,	0,1} )
		local t={
			0,	1,	0,	0,	0,
			0,	0,	0,	0,	1,
			1,	1,	0,	1,	0,
			1,	0,	0,	1,	1,
		}

		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_shadow_test",function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
--				shadow.fbo:bind_texture()
				shadow.fbo:bind_depth()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		gl.PopMatrix()

	end
	
	return shadow
end


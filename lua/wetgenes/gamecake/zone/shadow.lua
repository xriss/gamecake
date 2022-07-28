--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wzips=require("wetgenes.zips")

local log,dump=require("wetgenes.logs"):export("log","dump")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,shadow)
	shadow=shadow or {}
	shadow.modname=M.modname
	
	local gl=oven.gl

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local screen=oven.rebake("wetgenes.gamecake.zone.screen")

	shadow.mtx=M4() -- the transform matrix to render from the lights point of view

	shadow.light=V3() -- the normal of the light casting the shadow (sun or moon)
	shadow.power=1.0 -- the power of the light so we can fade out when swapping between sun and moon

	shadow.mapsize=4096
	shadow.maparea=256

-- number of shadows to draw
	shadow.count=1

	shadow.loads=function()

		local filename="lua/"..string.gsub(M.modname,"%.","/")..".glsl"
		gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	end
	
	shadow.setup=function()
	log("setup",M.modname)

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

		gl.uniforms.shadow_light=function(u)
			gl.Uniform4f( u,  shadow.light[1] , shadow.light[2] , shadow.light[3] , shadow.power )
		end

	end

	shadow.update=function()
		if shadow.fbo.w ~= shadow.mapsize or shadow.fbo.h ~= shadow.mapsize then -- auto resize
			shadow.fbo:resize( shadow.mapsize , shadow.mapsize , -1 )
		end
	end
	
--	shadow.default="0.0,0.0,0.0,0.0"
	shadow.draw_head=function(scene,shadow_idx)
	
-- special shadow transform to make the area around 0 more detailed
--		gl.program_defs["DRAW_SHADOW_SQUISH"]=screen.shader_qs.zone_screen_build_occlusion.SHADOW_SQUISH
		gl.program_defs["DRAW_SHADOW_SQUISH"]=screen.shader_qs.zone_screen_build_occlusion.SHADOW_SQUISH

		gl.PushMatrix()
		shadow.fbo:bind_frame()
		oven.cake.views.push_and_apply(shadow.view)
		gl.state.push(gl.state_defaults)

		gl.state.set({
			[gl.BLEND]						=	gl.FALSE,
			[gl.DEPTH_TEST]					=	gl.TRUE,
			[gl.CULL_FACE]					=	gl.TRUE,
--			[gl.FRONT_FACE]					=	gl.CW,	-- shadows are drawn upside down?
		})

		gl.Clear(gl.DEPTH_BUFFER_BIT)

		local camera=scene.get("camera")
		local sky=scene.systems.sky or {time=0}
		if camera then

			local s=shadow.maparea -- 40*shadow.mapsize/1024
			local sd=1024*8
			local x=(math.floor(camera.pos[1]))--*-1/s	-- swap z/y as rotation
			local y=(math.floor(camera.pos[2]))--*-1/s
			local z=(math.floor(camera.pos[3]))--*-1/s
			
			screen.shader_qs.zone_screen_build_occlusion.SHADOW="0.6,"..(0.04/sd)..","..(0.08/sd)..",0.0"

			local r=(sky.time+360-90-5)%360

			local  calculate_matrix=function()
				shadow.mtx=M4{
					1,			0,			0,			0,
					0,			1,			0,			0,
					0,			0,			1,			0,
					0,			0,			0,			1,
				}
				shadow.mtx:scale(1/s,1/s,1/sd)
				shadow.mtx:rotate( 90 , 1,0,0 ) -- top down
				shadow.mtx:rotate( 35 , 1,0,0 ) -- hemasphere
				shadow.mtx:rotate( -90 + r ,  0,0,1 ) -- time of day
				shadow.mtx:translate(-x,-y,-z)

			end
			
			-- calculate light matrix
			calculate_matrix()

			-- remember light normal
			sky.sun[1]=-shadow.mtx[3]
			sky.sun[2]=-shadow.mtx[7]
			sky.sun[3]=-shadow.mtx[11]
			sky.sun:normalize()

			shadow.mtx:rotate( 180 ,  0,0,1 ) -- time of day
			-- remember light normal
			sky.moon[1]=-shadow.mtx[3]
			sky.moon[2]=-shadow.mtx[7]
			sky.moon[3]=-shadow.mtx[11]
			sky.moon:normalize()


			local ddd=10

			if r<180 then
				if r<=0+ddd then		screen.day_night[1]=0.5-0.5*(r)/ddd
				elseif r>=180-ddd then	screen.day_night[1]=0.5-0.5*(180-r)/ddd
				else					screen.day_night[1]=0.0
				end
			else
				if r<=180+ddd then		screen.day_night[1]=0.5+0.5*(r-180)/ddd
				elseif r>=360-ddd then	screen.day_night[1]=0.5+0.5*(360-r)/ddd
				else					screen.day_night[1]=1.0
				end
			end

			shadow.light=V3(sky.sun)
			if r > 180 then -- moon or sun
				r = r-180
				shadow.light=V3(sky.moon)
			end
			
			shadow.power=1
			
			if r < ddd then
				shadow.power=r/ddd
			end
			if r > 180-ddd then
				shadow.power=(180-r)/ddd
			end
			

			-- calculate shadow matrix
			calculate_matrix()

			gl.MatrixMode(gl.PROJECTION)
			gl.LoadMatrix( shadow.mtx )

			gl.MatrixMode(gl.MODELVIEW)
			gl.LoadIdentity()

		end



	end

	shadow.draw_tail=function(scene,shadow_idx)

-- turn off special shadow transform
		gl.program_defs["DRAW_SHADOW_SQUISH"]=nil

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
			0,	1,	0,	0,	1,
			0,	0,	0,	0,	0,
			1,	1,	0,	1,	1,
			1,	0,	0,	1,	0,
		}

		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_shadow_test",function(p)

--[[
				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
--				shadow.fbo:bind_texture()
				shadow.fbo:bind_depth()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
]]
		end)

		gl.PopMatrix()

	end
	
	return shadow
end


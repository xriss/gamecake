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
			cx=0.5,cy=0.5,
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
	
	shadow.default="0.0,0.0,0.0,0.0"
	shadow.draw_head=function(scene)

		gl.PushMatrix()
		shadow.fbo:bind_frame()
		oven.cake.views.push_and_apply(shadow.view)
		gl.state.push(gl.state_defaults)

		gl.state.set({
			[gl.BLEND]						=	gl.FALSE,
			[gl.DEPTH_TEST]					=	gl.TRUE,
		})
		gl.Clear(gl.DEPTH_BUFFER_BIT)

		local camera=scene.get("camera")
		local sky=scene.systems.sky
		if camera then

	--		print( M4(gl.matrix(gl.MODELVIEW)) )

			gl.Translate( camera.inv[13] , camera.inv[14] , camera.inv[15] )

			gl.Rotate( -90 , 1,0,0 )
			gl.Translate( 0,-512,0 )

			gl.Scale( 2 , 2 , 2 )

--			local t=M4( gl.SaveMatrix() )			
--			print(t)



			local s=256 -- 40*shadow.mapsize/1024
			local sd=1024
			local x=(camera.mtx[ 9]*-s + camera.mtx[13])*-1/s	-- swap z/y as rotation
			local y=(camera.mtx[11]*-s + camera.mtx[15])*-1/s
			local z=(camera.mtx[10]*-s + camera.mtx[14])*-1/s

			local snap=16
			x=math.floor(0.5+x*snap)/snap
			y=math.floor(0.5+y*snap)/snap
			z=math.floor(0.5+z*snap)/snap

			shadow.default="0.5,"..0.000000*s/sd..","..0.000008*s/sd..",0.0"

--			x=0
--			y=0
--			z=0

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

--			r=math.floor((r+0.5)/1)*1

--			shadow.mtx:rotate( r , 1,0,0 )
			
--			shadow.mtx:rotate( -10 , 1,0,0 )


			shadow.mtx:pretranslate(x,y,z)
			shadow.mtx:prerotate( r , -1,0,0 )
			shadow.mtx:prescale(1,1,1/sd)

-- snap to pixels?
--[[
			local vv=shadow.mtx:v3(4)
			shadow.mtx[13]=math.floor( vv[1] * (shadow.mapsize) ) / (shadow.mapsize)
			shadow.mtx[14]=math.floor( vv[2] * (shadow.mapsize) ) / (shadow.mapsize)
			shadow.mtx[15]=math.floor( vv[3] * (shadow.mapsize) ) / (shadow.mapsize)
]]
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


--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


-- This curve used here is the curvy part of splines.
-- input 0 to 1 and output 0 to 1 smoothing into and from the 0 and 1  (like a sinwave going from -1 to +1)
local function spine(a)
	local aa=a*a
	return ((aa+(aa*2))-((aa*a)*2))
end
-- input 0 to 1 and output 0 to 1 to 0 so it loops smoothly
local function spine2(a)
	a=a*2
	if a>1 then a=2-a end
	local aa=a*a
	return ((aa+(aa*2))-((aa*a)*2))
end



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,about)
	about=about or {}

	local gl=oven.gl
	local cake=oven.cake
	
	local sounds=cake.sounds
	local canvas=cake.canvas
	local fbs=cake.framebuffers
	
	local flat=canvas.flat
	local font=canvas.font
	
	local wetiso=oven.rebake("wetgenes.gamecake.spew.geom_wetiso")
	local geom=oven.rebake("wetgenes.gamecake.spew.geom")
--	local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps")


	local opts={
		width=640,
		height=480,
	}
	
	about.title="WetGenes presents GameCake"
	about.text=[[
*SKIP*

	Welcome to the world of tomorrow!!!
	
	Nice to have you visit.

*SKIP*
Cracked by unknownKLOWN!!!
*SKIP*
Supplied by dIvhEdbUy0!!!
*SKIP*
Greetz to No1 and No6!
*SKIP*

]]
	
--	about.layout=cake.layouts.create{}
	about.view=cake.views.create({
		parent=cake.views.get(),
		mode="clip",
		vx=640,
		vy=480,
		fov=0,
	})
	
	about.shaders=function()
		gl.program_source("about_sinescroll",[[
		
#version 100
#version 120

#ifdef VERTEX_SHADER

#ifdef VERSION_ES
precision mediump float;
#endif
		
		uniform mat4 modelview;
		uniform mat4 projection;
		uniform vec4 color;

		attribute vec3 a_vertex;
		attribute vec2 a_texcoord;

		varying vec4  v_color;
		varying vec3  v_pos;
		varying vec2  v_texcoord;
		 
		void main()
		{
			gl_Position = projection * modelview * vec4(a_vertex, 1.0);
			v_color=color;
			v_texcoord=a_texcoord;
		}

#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER

#ifdef VERSION_ES
precision mediump float;
#endif

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

		uniform vec4 vars;

		uniform sampler2D tex;

		varying vec4  v_color;
		varying vec3  v_pos;
		varying vec2  v_texcoord;

		void main(void)
		{
			float n=v_texcoord.x + vars[0]*3.0;
			vec2  t=v_texcoord * vec2(1.0,480.0/128.0) ;
			t.y+=(-140.0/128.0) + sin(n*16.0)*128.0/480.0;
			
			vec4 c=vec4( sin(t.y*2.0),sin(t.y*3.0),sin(t.y*4.0),1.0);
			gl_FragColor=texture2D(tex, t) * c;
		}

#endif //FRAGMENT_SHADER

		]])
	end
	
	about.setup=function()

		if oven.last and oven.last~=about then about.exit=oven.last end -- remeber where we came from


		wetiso.setup()
	
		about.words=wstr.split_words(about.text)
		about.words_idx=1 -- the next word to get
		about.scroll={dx=0}

		about.shaders()
		about.co=coroutine.create(about.thunk)
		
		if about.playtune then
			about.playtune()
		end
		
	end
	
	about.clean=function()
		if about.fbo then
			about.fbo:clean()
			about.fbo=nil
		end
	end
	
	about.update=function()
		
--		srecaps.step()

		assert(coroutine.resume(about.co))
	end
	
	about.predraw=function()

	end

	about.draw=function()

		cake.views.push_and_apply(about.view)
--		about.layout.apply(opts.width,opts.height,1/4,opts.width*4,"clip")

		canvas.gl_default() -- reset gl state

		gl.ClearColor(gl.C4(0x0000))
		gl.Clear(gl.COLOR_BUFFER_BIT)--+gl.DEPTH_BUFFER_BIT)

		gl.PushMatrix()

		if about.thunkdraw then about.thunkdraw() end
		
		gl.PopMatrix()
		cake.views.pop_and_apply()

	end

-- use an fbo for the scroll text so we can jiggle it in a shader	
	about.draw_fbo=function(f)

		if not about.fbo then about.fbo=fbs.create() end

		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()
		
		local fsx=1024
		local fsy=128

		about.fbo:resize(fsx,fsy,1)
		fbs.bind_frame(about.fbo)
--		local old_layout=cake.layouts.create{parent={w=fsx,h=fsy,x=0,y=0}}.apply(fsx,fsy,1/4,fsx*8)
		cake.views.push_and_apply_fbo(about.fbo)
		
		gl.ClearColor(gl.C4(0x0000))
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		
		-- draw stuff
		if f then f() end
		
		fbs.bind_frame(nil) -- remove fbs binding, should pop a stack?
		
		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()
		
--		old_layout.restore()
		cake.views.pop_and_apply()

		
	end


	about.thunk=function()
		local tim=0
	
		local next_word=function()
			local s=about.words[ about.words_idx]
			about.words_idx=about.words_idx+1
			if about.words_idx > #about.words then about.words_idx=1 end				
			return s
		end
		local fill_words=function()
			local scr=about.scroll

			font.set(cake.fonts.get("Vera")) -- need to calculate widths
			font.set_size(64,0)
			
			local dx=scr.dx
			for i,v in ipairs(scr) do
				dx=dx+v.w
			end
			while dx<640+128 do
				local s=next_word()
				local w=font.width(s)
				w=w+24
				if s=="*SKIP*" then
					s=""
					w=640
				end
				scr[#scr+1]={s=s,w=w}
				dx=dx+w
			end
		end
		local remove_words=function()
			local scr=about.scroll
			while scr[1] and scr.dx + scr[1].w < -128 do
				scr.dx=scr.dx+scr[1].w
				table.remove(scr,1)
			end
		end
		local update_scroll=function()
			local scr=about.scroll

			scr.dx=scr.dx-4			

			fill_words()
			remove_words()


		end
		local draw_scroll=function()
			local scr=about.scroll

			about.draw_fbo(function()
			
				font.set(cake.fonts.get("Vera")) -- default font
				font.set_size(64,0)

				gl.Color(spine2((tim%128)/128),spine2((tim%96)/96),spine2((tim%64)/64)+0.5,1)

				local dx=scr.dx
				for i,v in ipairs(scr) do
					font.set_xy(dx,0)
					font.draw(v.s)
					dx=dx+v.w
				end
				
			end)

			gl.BindTexture(gl.TEXTURE_2D, about.fbo.texture)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.LINEAR)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
			gl.Color(gl.C4(0xffff))
			
			local p=gl.program("about_sinescroll")
			gl.UseProgram( p[0] )
			p:uniform_v4("vars", { ((tim%1024)/1024)*math.pi ,0,0,0 } )

			flat.tristrip("xyzuv",{
				0,		0,			0,		0,				1,
				640,	0,			0,		640/1024,		1,
				0,		480,		0,		0,				0,
				640,	480,		0,		640/1024,		0,
			},"about_sinescroll")


		end

		local draw_title=function()
			local s=about.title

			font.set(cake.fonts.get("Vera")) -- default font
			font.set_size(160,0)
			local w=font.width(s)

			gl.Color(spine2((tim%64)/64)+0.5,spine2((tim%128)/128),spine2((tim%96)/96),1)

			font.set_xy((w-640)*-spine2((tim%128)/128),-40)
			font.draw(s)

			font.set_xy((w-640)*-spine2(((tim+64)%128)/128),480-160)
			font.draw(s)

		end

		local setup_bobs=function()
			about.bobs={}
			for i=1,32 do
				local v={}
				about.bobs[#about.bobs+1]=v
				
				v.px=math.random(0,1280)
				v.py=math.random(0,480)
				v.ss=math.random(8,64)
				v.t=math.random(0,16384)
			end
			table.sort(about.bobs,function(a,b)return a.ss<b.ss end)
		end
		
		local update_bobs=function()
		
			for i,v in ipairs(about.bobs) do
				v.px=v.px+v.ss*0.25
				if v.px>640+128 then v.px=v.px-1280 end
				v.t=v.t-(v.ss*4/64)
			end
		end

		local draw_bobs=function()
			
			for i,v in ipairs(about.bobs) do
				gl.Color(0,0.25*v.ss/64,0.75*v.ss/64,1)
				gl.PushMatrix()
				gl.Translate(v.px,v.py,0)
				gl.Scale(v.ss,v.ss,v.ss)
				gl.Rotate(v.t,0,-1,0)
				gl.Rotate(v.t/8,1,0,0)
--				gl.state.push({
--					[gl.CULL_FACE]					=	gl.TRUE,
--				})
				wetiso.draw()
--				gl.state.pop()
				gl.PopMatrix()
			end
				
		end

		local update=function()
			if about.exitnow then
				about.exitnow=false
				if about.exitname then
					oven.next=oven.rebake(about.exitname)
				else
					if about.exit then oven.next=about.exit end
				end
			end
			tim=tim+1
			update_bobs()
			update_scroll()
			coroutine.yield()
		end
		
		local draw=function()
			draw_bobs()
			draw_scroll()
			draw_title()

			local c=about.clear_color or {0,0,0,0}
			if c[4]>0 then
				gl.Color(c[1],c[2],c[3],c[4])
				flat.tristrip("xyz",{
					0,		0,			0,
					640,	0,			0,
					0,		480,		0,
					640,	480,		0,
				})
			end
		end
		
		setup_bobs()
		about.thunkdraw=draw -- set what to draw

		about.clear_color={1,1,1,1}
		coroutine.yield()

		for i=254,0,-2 do
			local a=i/255 a=a*a
			about.clear_color={a,a,a,a}
			update()
		end
		about.clear_color={0,0,0,0}

		while true do
			update()
		end
		
		while true do coroutine.yield() end
	end

	function about.msg(m)
	
		if m.class=="key" or m.class=="mouse" or m.class=="joykey" or m.class=="padkey" then
			if m.action==-1 then
				about.exitnow=true
			end
		end

	end

	return about
end


local fenestra_avatar=require('fenestra.avatar')

local bit=require('bit')
local gl=require('gl')
local grd=require('wetgenes.grd')


local _G=_G

local win=win

local print=print
local pairs=pairs
local ipairs=ipairs
local tonumber=tonumber
local tostring=tostring
local string=string
local type=type
local unpack=unpack

local io=io
local math=math


local apps=apps
local wldir=apps.dir or ""


local fbo_w=640/1
local fbo_h=480/1

module("state.avatar")

local anim_names={

{	name="angry",				xsx="pose_angry"			},
{	name="awake",				xsx="pose_awake"			},
{	name="bird",				xsx="pose_bird"				},
{	name="confused",			xsx="pose_confused"			},
{	name="determind",			xsx="pose_determind"		},
{	name="devious",				xsx="pose_devious"			},
{	name="embarrassed",			xsx="pose_embarrassed"		},
{	name="energetic",			xsx="pose_energetic"		},
{	name="enthralled",			xsx="pose_enthralled"		},
{	name="excited",				xsx="pose_excited"			},
{	name="gunner",				xsx="pose_gunner"			},
{	name="hips",				xsx="pose_hands_on_hips"	},
{	name="happy",				xsx="pose_happy"			},
{	name="idle",				xsx="pose_idle"				},
{	name="indescribable",		xsx="pose_indescribable"	},
{	name="nerdy",				xsx="pose_nerdy"			},
{	name="okay",				xsx="pose_okay"				},
{	name="pointup",				xsx="pose_point_down_up"	},
{	name="pointdown",			xsx="pose_point_up_down"	},
{	name="sad",					xsx="pose_sad"				},
{	name="scared",				xsx="pose_scared"			},
{	name="sleepy",				xsx="pose_sleepy"			},
{	name="teapot",				xsx="pose_teapot"			},
{	name="thoughtful",			xsx="pose_thoughtful"		},
{	name="working",				xsx="pose_working"			},

{	name="bulbaceous",			xsx="anim_bulbaceous"		},
{	name="push",				xsx="anim_push"				},
{	name="splat",				xsx="anim_splat_to_idle"	},
{	name="stab",				xsx="anim_stab"				},
{	name="swing",				xsx="anim_swing"			},

{	name="breath",				xsx="cycle_breath"			},
{	name="push",				xsx="cycle_push"			},
{	name="walk",				xsx="cycle_walk"			},
{	name="zeewalk",				xsx="cycle_zeewalk"			},
{	name="qrun",				xsx="cycle_quad_run"		},
{	name="qwalk",				xsx="cycle_quad_walk"		},

{	name="qbase",				xsx="pose_quad_base"		},

}
-- allow name lookup
for i=1,#anim_names do local v=anim_names[i]
	anim_names[v.name]=v
end

-----------------------------------------------------------------------------
--
-- setup
--
-----------------------------------------------------------------------------
function setup()

filter="test"


print("setup")
fbo1=win.fbo_setup(fbo_w,fbo_h,24)
fbo2=win.fbo_setup(fbo_w,fbo_h,24)


soul_filename=nil

w={}

pick_colors={

0xff000000,0xff444444,0xff666666,0xff888888,0xffaaaaaa,0xffcccccc,0xffeeeeee,0xffffffff,
0xff0000ff,0xff0088ff,0xff8800ff,0xff8888ff,0xff00ff00,0xff00ff88,0xff88ff00,0xff88ff88,
0xff0000cc,0xff0066cc,0xff6600cc,0xff6666cc,0xff00cc00,0xff00cc66,0xff66cc00,0xff66cc66,
0xff999900,0xff666600,0xff000066,0xff000099,0xff990000,0xff660000,0xff006600,0xff009900,
0xffcccc00,0xffcccc66,0xffcc6600,0xffcc0000,0xffcc0066,0xffcc6666,0xffcc66cc,0xff66cccc,
0xffffff00,0xffffff88,0xffff8800,0xffff0000,0xffff0088,0xffff8888,0xffff88ff,0xff88ffff,
0xffaa6644,0xffdd9977,0xffddcc99,0xffeebb99,0xffffbb99,0xffffaaaa,0xffddbbbb,0xffbbbbdd,

}

pick_surfs={

"skin",
"lips",
"nipples",
"hair",
"hairhi",
"hairlo",
"eyebrows",
"beard",
"specs",
"eye",
"iris",
"body",
"bodyhi",
"bodylo",
"foot",
"toecaps",
"sole",
"socks",

}

	rotate=180

	frame=0

	part_group="head"
	part_side="both"
	part_page=1
	part_layer=1
	part_cache={}


	xsx_dat=win.data.load("data/avatar/xsx/cycle_walk.xsx")
	xsx=win.xsx(xsx_dat)

	parts={}
	thunk_parts()


	load_soul(wldir.."local/avatar/soul/kolumbo_bob.xml")
	
	_G.page=page

	page("colors")
end
-----------------------------------------------------------------------------
--
-- clean
--
-----------------------------------------------------------------------------
function clean()
print("clean")
	win.widget:clean_all()
	
	win.fbo_clean(fbo2)
	fbo2=nil
	win.fbo_clean(fbo1)
	fbo1=nil
end


-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
function update()

	frame=(frame+0.020)
	if frame>xsx.length then frame=frame-xsx.length end 

end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
function draw_parts()
local fov=4

	win.clip2d(0,0,0,0)
	win.project23d(640/480,fov,1024)
	gl.MatrixMode("MODELVIEW")
	gl.LoadIdentity()
	gl.PushMatrix()
	
	if w.avatar then
	
		local x,y,s
		
		x=(2*(w.avatar.pxd+(w.avatar.hx/2)-320)/(60*fov))
		y=(2*(w.avatar.pyd-(w.avatar.hy-w.avatar_rot.hy)-240))/(60*fov)
		s=(w.avatar.hy*15/16)/(60*fov)
	
		gl.Translate(x,y, -8)
		gl.Scale(s,s,s)
		gl.Rotate(rotate,0,1,0)
		
		xsx.draw(frame)
	end
	
	gl.PopMatrix()
	gl.PushMatrix()

	if w.parts and parts then
		for i=1,16 do local v=w.parts[i]
			if v and parts[v.user] and parts[v.user].xox then
				local x,y,s
				
				x=(2*(v.pxd+(v.hx/2)-320))/(60*fov)
				y=(2*(v.pyd-(v.hy*3/4)-240))/(60*fov)
				s=(2*(v.hy*12/16))/(60*fov)
			
				gl.Translate(x,y, -8)
				gl.Scale(s,s,s)
				gl.Rotate(rotate+180,0,1,0)
				
				parts[v.user].xox.draw()
				
				gl.PopMatrix()
				gl.PushMatrix()
			end
		end
	end

	gl.PopMatrix()

end

function draw()


local fov=4


	if not filter then

		win.begin()
		gl.ClearColor(0,0,0.25,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
		draw_parts()

	elseif filter=="test" then
	
		win.fbo_bind(fbo1)
		win.begin(fbo_w,fbo_h)
		gl.ClearColor(0,0,0,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
		
		draw_parts()
		

	-- drawing to the screen
		win.fbo_bind()

		win.begin()
		gl.ClearColor(0,0,0.25,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
		win.clip2d(0,0,0,0)
		win.project23d(640/480,fov,1024)

		gl.MatrixMode("MODELVIEW")
		gl.LoadIdentity()
		gl.Translate(0,0,-4)
		
		
		gl.Disable('LIGHTING')
		gl.Disable('DEPTH_TEST')

		gl.Enable('COLOR_MATERIAL')
		
		gl.Enable('BLEND')

		win.fbo_texture(fbo1)
		gl.Disable('CULL_FACE')
		gl.Enable('TEXTURE_2D')
		gl.TexParameter('TEXTURE_2D','TEXTURE_MAG_FILTER','LINEAR')
		gl.BlendFunc('SRC_ALPHA', 'ONE_MINUS_SRC_ALPHA')

--		win.set("blend_sub")
		for i=0,0 do
			local s=0 --(i-1)*2
			local x=0
			local y=0
			
			gl.Begin('QUADS')
				gl.Color({1,1,1,1})
				gl.TexCoord(0, 0) gl.Vertex((x-(320+s))/(60*fov), (y-(240+s))/(60*fov))
				gl.TexCoord(1, 0) gl.Vertex((x+(320+s))/(60*fov), (y-(240+s))/(60*fov))
				gl.TexCoord(1, 1) gl.Vertex((x+(320+s))/(60*fov), (y+(240+s))/(60*fov))
				gl.TexCoord(0, 1) gl.Vertex((x-(320+s))/(60*fov), (y+(240+s))/(60*fov))
			gl.End()
		end
--		win.set("blend_add")

	elseif filter=="blur" then
	
		win.fbo_bind(fbo1)
		win.begin(fbo_w,fbo_h)
		gl.ClearColor(0,0,0,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);

--		win.set("force_diffuse",0xff000000)
--		win.set("force_gloss",64)
		draw_parts()
--		win.set("force_diffuse",0)
--		win.set("force_gloss",-1)
		
	
		win.fbo_bind(fbo2)
		win.begin(fbo_w,fbo_h)
		gl.ClearColor(0,0,0,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);

	--	win.set("clear_diffuse",true)
		draw_parts()
	--	win.set("clear_diffuse",false)
		
	--	gl.Finish()
		
	-- drawing to the screen
		win.fbo_bind()

		win.begin()
		gl.ClearColor(0,0,0.25,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
		win.clip2d(0,0,0,0)
		win.project23d(640/480,fov,1024)

		gl.MatrixMode("MODELVIEW")
		gl.LoadIdentity()
		gl.Translate(0,0,-4)
		
		
		gl.Disable('LIGHTING')
		gl.Disable('DEPTH_TEST')

		gl.Enable('COLOR_MATERIAL')
		
		gl.Enable('BLEND')

		win.fbo_texture(fbo2)
		gl.Disable('CULL_FACE')
		gl.Enable('TEXTURE_2D')
		gl.TexParameter('TEXTURE_2D','TEXTURE_MAG_FILTER','LINEAR')
		gl.BlendFunc('SRC_ALPHA', 'ONE_MINUS_SRC_ALPHA')

--		win.set("blend_sub")
		for i=0,0 do
			local s=0 --(i-1)*2
			local x=0
			local y=0
			
			gl.Begin('QUADS')
				gl.Color({1,1,1,1})
				gl.TexCoord(0, 0) gl.Vertex((x-(320+s))/(60*fov), (y-(240+s))/(60*fov))
				gl.TexCoord(1, 0) gl.Vertex((x+(320+s))/(60*fov), (y-(240+s))/(60*fov))
				gl.TexCoord(1, 1) gl.Vertex((x+(320+s))/(60*fov), (y+(240+s))/(60*fov))
				gl.TexCoord(0, 1) gl.Vertex((x-(320+s))/(60*fov), (y+(240+s))/(60*fov))
			gl.End()
		end
--		win.set("blend_add")

		gl.BlendFunc('SRC_ALPHA', 'ONE')
		win.fbo_texture(fbo1)
		gl.Disable('CULL_FACE')
		gl.Enable('TEXTURE_2D')
		gl.TexParameter('TEXTURE_2D','TEXTURE_MAG_FILTER','LINEAR')
		
		for i=0,8 do
			local s=0 --(i-1)*2
			local x=(-1+(           (i%3) ))*8
			local y=(-1+( math.floor(i/3) ))*8
			
			gl.Begin('QUADS')
				gl.Color({1,1,1,1/9})
				gl.TexCoord(0, 0) gl.Vertex((x-(320+s))/(60*fov), (y-(240+s))/(60*fov))
				gl.TexCoord(1, 0) gl.Vertex((x+(320+s))/(60*fov), (y-(240+s))/(60*fov))
				gl.TexCoord(1, 1) gl.Vertex((x+(320+s))/(60*fov), (y+(240+s))/(60*fov))
				gl.TexCoord(0, 1) gl.Vertex((x-(320+s))/(60*fov), (y+(240+s))/(60*fov))
			gl.End()
		end
		

		gl.Disable('TEXTURE_2D')
	  
		gl.Enable('DEPTH_TEST')
		gl.BlendFunc('SRC_ALPHA', 'ONE_MINUS_SRC_ALPHA')
		
		
	end
  
	gl.MatrixMode("MODELVIEW")
	gl.LoadIdentity()

	if dumpscreen then
		dumpscreen=false
		print("dumptest start")
		sheet()
		print("dumptest end")
	end
	
end



-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function explode_color(c)

	local r,g,b,a
	
	a=bit.band(bit.rshift(c,24),0xff)
	r=bit.band(bit.rshift(c,16),0xff)
	g=bit.band(bit.rshift(c, 8),0xff)
	b=bit.band(c,0xff)

	return r/0xff,g/0xff,b/0xff,a/0xff
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function implode_color(r,g,b,a)

	if type(r)=="table" then a=r[4] b=r[3] g=r[2] r=r[1] end -- convert from table?

	local c
	
	c=             bit.band(b*0xff,0xff)
	c=c+bit.lshift(bit.band(g*0xff,0xff),8)
	c=c+bit.lshift(bit.band(r*0xff,0xff),16)
	c=c+bit.lshift(bit.band(a*0xff,0xff),24)

	return c
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function get_color()
	return w.color.color
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_color(c)

	local t={explode_color(c)}
	
--	w.color_alpha:set("slide",t[4])
	w.color_red:set("slide",t[1])
	w.color_green:set("slide",t[2])
	w.color_blue:set("slide",t[3])
	w.color.color=c
	w.color.text=string.format("%06x",bit.band(c,0xffffff))
	
	w.color.state="selected"
	w.spec.state="none"
	
	if w.surfs then
		for i,v in pairs(w.surfs) do
			if v.state=="selected" then
				v.color=c
				local s=soul.vanilla.surfaces[v.user]
				if s then
					s.argb=c
				end
			end
		end
		
		win.avatar.map_xsx(xsx,soul)
		
	end
end


-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_gloss(gg)
	local g=gg
	
--	g=1-g
	g=g*g*g*g -- range of 1 to 0, made more sensible
--	g=1-g
	g=0+(g*128) -- back to opengl range

	w.color_gloss:set("slide",gg)
		
	if w.surfs then
	local it
		for i,v in pairs(w.surfs) do
			if v.state=="selected" then
				it=v
			end
		end
		for i,v in pairs(w.specs) do
			if v.state=="selected" then
				it=v
			end
		end

		if it then
		
			local s=soul.vanilla.surfaces[it.user]
			if s then
				s.gloss=g
			end
				
			win.avatar.map_xsx(xsx,soul)
		end
	end
end


-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function all_parts(part,side,f)

local part_lookup={
	["head"]={"head"},
	["body"]={"body"},
	["hands"]={"left_hand","right_hand"},
	["feet"]={"left_foot","right_foot"},
	["brows"]={"left_eye","right_eye"},
	["eyes"]={"left_eyeball","right_eyeball"},
	["mouth"]={"mouth"},
	["nose"]={"nose"},
	["ears"]={"left_ear","right_ear"},
	["tail"]={"tail"},
	}

	local avapart_base=part_lookup[part]

		
	if avapart_base then 
	
		local ia=1
		local ib=2
		
		if side=="left"  then ia=1 ib=1 end
		if side=="right" then ia=2 ib=2 end
		if #avapart_base==1 then ia=1 ib=1 end -- only choose left/right when we have a choice
		
		for i=ia,ib do
		
			local avapart=avapart_base[i]
			
			if avapart then
			
				local p=soul.vanilla.parts[avapart]
				
				f(p,avapart) -- the callback
				
			end
			
		end
		
	end
	
end
	
-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_part(part,layer,xinf,side)

	all_parts(part,side,function(p)
		if p and p[layer] then
			p[layer].group=xinf and xinf.group -- xinf may be nil
			p[layer].name=xinf and xinf.name
			p[layer].xox_info=xinf
			win.avatar.map_xsx(xsx,soul)
		end
	end)
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_size(part,ss,side)

	all_parts(part,side,function(p)
		if not p.size then p.size=((p.xsize or 1)+(p.ysize or 1)+(p.zsize or 1))/3 end
		local sx=(p.xsize or 1)/p.size
		local sy=(p.ysize or 1)/p.size
		local sz=(p.zsize or 1)/p.size
		p.size=ss
		p.xsize=p.size*sx
		p.ysize=p.size*sy
		p.zsize=p.size*sz
		win.avatar.map_xsx(xsx,soul)
	end)

end



-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_pos(part,x,y,z,side)

	all_parts(part,side,function(p,name)
		p.xpos=x
		if name:sub(1,4)=="left" then p.xpos=-x end -- mirror
		p.ypos=y
		p.zpos=z
		win.avatar.map_xsx(xsx,soul)
	end)

end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_aspect(part,ss,side)

	all_parts(part,side,function(p)
		if not p.size then p.size=((p.xsize or 1)+(p.ysize or 1)+(p.zsize or 1))/3 end
		local sx=1
		local sy=1
		local sz=1
		if ss<0.5 then
			sy=0.5+ss
		else
			sx=1.5-ss
		end
		p.xsize=p.size*sx
		p.ysize=p.size*sy
		p.zsize=p.size*sz
		win.avatar.map_xsx(xsx,soul)
	end)

end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_morph(part,ss,side)

	all_parts(part,side,function(p)
		if ss<0.5 then
			p.morph0=1-(ss*2)
			p.morph1=0
		else
			p.morph0=0
			p.morph1=((ss-0.5)*2)
		end
		win.avatar.map_xsx(xsx,soul)
	end)

end


-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_anim(ainfo)

	if ainfo then
		xsx_dat=win.data.load("data/avatar/xsx/"..ainfo.xsx..".xsx")
		xsx=win.xsx(xsx_dat)
		win.avatar.map_xsx(xsx,soul)
	end

end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function get_spec()
	return w.spec.color
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_spec(c)

	local t={explode_color(c)}
	
--	w.color_alpha:set("slide",t[4])
	w.color_red:set("slide",t[1])
	w.color_green:set("slide",t[2])
	w.color_blue:set("slide",t[3])
	w.spec.color=c
	w.spec.text=string.format("%06x",bit.band(c,0xffffff))
	
	w.spec.state="selected"
	w.color.state="none"
	
	if w.surfs then
		for i,v in pairs(w.specs) do
			if v.state=="selected" then
				v.color=c
				local s=soul.vanilla.surfaces[v.user]
				if s then
					s.spec=c
				end
			end
		end
		
		win.avatar.map_xsx(xsx,soul)
		
	end
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function get_argb()
	if w.color then
		if w.color.state=="selected" then return get_color()
		else return get_spec() end
	end
end
-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_argb(c)
	if w.color then
		if w.color.state=="selected" then return set_color(c)
		else return set_spec(c) end
	end
end
-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function set_edit(to)
print(to)
	local c
	if w.color.state=="selected" then
		if to=="spec" then --swap
			for i,v in pairs(w.surfs) do
				w.specs[v.user].state=v.state
				v.state="none"
			end
			set_spec(w.spec.color)
		end
	else
		if to=="color" then --swap
			for i,v in pairs(w.specs) do
				w.surfs[v.user].state=v.state
				v.state="none"
			end
			set_color(w.color.color)
		end
	end
end

-----------------------------------------------------------------------------
--
-- load surfaces from the soul into the buttons
--
-----------------------------------------------------------------------------
local function load_surf(soul)

	for i,v in pairs(soul.vanilla.surfaces) do
		local s=w.surfs[i]
		if s then
			s.color=v.argb		
		end
		local s=w.specs[i]
		if s then
			s.color=v.spec
		end
	end
end


-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function select_surf(n)

local it
local g

	for i,v in pairs(w.surfs) do
		if v.user==n then
			v.state="selected"
			it=v
		else
			v.state="none"
		end
	end
	for i,v in pairs(w.specs) do
		v.state="none"
	end

	if it then
		local s=soul.vanilla.surfaces[it.user]
		if s then
			set_gloss(math.pow(s.gloss/128,0.25))
			set_color(s.argb)
		end
	end
	
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local function select_spec(n)

local it
	for i,v in pairs(w.specs) do
		if v.user==n then
			v.state="selected"
			it=v
		else
			v.state="none"
		end
	end
	for i,v in pairs(w.surfs) do
		v.state="none"
	end

	if it then
		local s=soul.vanilla.surfaces[it.user]
		if s then
			set_gloss(math.pow(s.gloss/128,0.25))
			set_spec(s.spec)
		end
	end
	
end

--	local block=master:add({hx=640,hy=480,color=0x00000000,static=true})

-----------------------------------------------------------------------------
--
-- main hooks for widget callbacks, not sure but it does feel simpler to just
-- use one function with a big old switch, just keeping the code in one place really
--
-----------------------------------------------------------------------------
local hooks={}
	function hooks.click(widget)
print(widget.id)
	
		if widget.id=="rotate" then
		
		elseif widget.id=="color" then
		
			set_argb(widget.color)
			
		elseif widget.id=="surf" then
		
			select_spec(widget.user)
			select_surf(widget.user)
			
		elseif widget.id=="spec" then
		
			select_surf(widget.user)
			select_spec(widget.user)
			
		elseif widget.id=="set_color" then
		
			set_edit("color")
		
		elseif widget.id=="set_spec" then
		
			set_edit("spec")
			
		elseif widget.id=="goto_colors" then
		
			page("colors")
			
		elseif widget.id=="goto_soul" then
		
			page("soul")
			
		elseif widget.id=="goto_parts" then
		
			page("parts")
			
		elseif widget.id=="goto_anims" then
		
			page("anims")
			
		elseif widget.id=="part_group" then
		
			part_group=widget.user
			thunk_parts(t)
			page_thunk("parts")
			
		elseif widget.id=="part_layer" then
			
			part_layer=widget.user
			thunk_parts(t)
			page_thunk("parts")
			
		elseif widget.id=="part_page" then
			
			part_page=widget.user
			thunk_parts(t)
			page_thunk("parts")
			
		elseif widget.id=="part_side" then
			
			part_side=widget.user
			thunk_parts(t)
			page_thunk("parts")
			
		elseif widget.id=="part" then
		
			set_part( part_group , part_layer , part_cache[widget.user] , part_side )
						
		elseif widget.id=="anim" then
		
			set_anim( anim_names[widget.user] )
			
		elseif widget.id=="load_soul" then
		
			local new_name=win.choose_file(soul_filename,"load")
			
			if new_name then
				load_soul(new_name)
			end
		
		elseif widget.id=="save_soul" then
		
			local new_name=win.choose_file(soul_filename,"save")
			
			if new_name then
				save_soul(new_name)
			end
			
		elseif widget.id=="dumptest" then
		
			dumpscreen=true
		
		elseif widget.id=="pos_reset" then
		
			set_pos( part_group , 0 , 0 , 0 , part_side )		
			
		elseif widget.id=="size_reset" then		
		
			set_size( part_group , 1 , part_side )
			
		elseif widget.id=="aspect_reset" then
		
			set_aspect( part_group , 0.5 , part_side )
			
		elseif widget.id=="morph_reset" then
		
			set_morph( part_group , 0.5 , part_side )
			
		end
	end
	
	function hooks.slide(widget)
		if widget.id=="rotate" then
		
			local x,y=widget:get("slide")
			rotate= (x*360)
			
		elseif widget.id=="alpha" then
		
			local x,y=widget:get("slide")
			local t={explode_color(get_argb())}
			t[4]=x
			set_argb(implode_color(t))
			
		elseif widget.id=="red" then
		
			local x,y=widget:get("slide")
			local t={explode_color(get_argb())}
			t[1]=x
			set_argb(implode_color(t))
			
		elseif widget.id=="green" then
		
			local x,y=widget:get("slide")
			local t={explode_color(get_argb())}
			t[2]=x
			set_argb(implode_color(t))
			
		elseif widget.id=="blue" then
		
			local x,y=widget:get("slide")
			local t={explode_color(get_argb())}
			t[3]=x
			set_argb(implode_color(t))
			
		elseif widget.id=="gloss" then
		
			local x,y=widget:get("slide")
			set_gloss(x)
			
		elseif widget.id=="pos" then
		
			local x,y=widget:get("slide")
			set_pos( part_group , (x-0.5)*0.25 , ((1+y)-0.5)*0.25 , 0 , part_side )
			
		elseif widget.id=="size" then
		
			local x,y=widget:get("slide")
			set_size( part_group , 0.5+(x*1.0) , part_side )
			
		elseif widget.id=="aspect" then
		
			local x,y=widget:get("slide")
			set_aspect( part_group , x , part_side )
			
		elseif widget.id=="morph" then
		
			local x,y=widget:get("slide")
			set_morph( part_group , x , part_side )
			
		end
	end
	

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
function load_soul(filename)
	soul_filename=filename
	soul=win.avatar.load_soul(soul_filename)
	win.avatar.map_xsx(xsx,soul)
end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
function save_soul(filename)
	soul_filename=filename
	win.avatar.save_soul(soul,soul_filename)
end


-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
function thunk_parts()

local xox_pre="data/avatar/xox/"
local xox_pst=".xox"

local group
local groups

	if part_group=="head" then
		if part_layer==1 then
			group="head"
		elseif part_layer==2 then -- hair1
			groups={"hair_base","hair","hair_xtra","hat"}
		elseif part_layer==3 then -- hair2
			groups={"hair_base","hair_xtra","hair","hat"}
		elseif part_layer==4 then -- hair3
			groups={"hat","hair_base","hair","hair_xtra"}
		end
	elseif part_group=="body" then
		if part_layer==1 then
			group="body"
		elseif part_layer==2 then
			group="body"
		end
	elseif part_group=="hands" then
		if part_layer==1 then
			group="hand"
		elseif part_layer==2 then
			group="inhand"
		end
	elseif part_group=="feet" then
		if part_layer==1 then
			group="foot"
		end
	elseif part_group=="tail" then
		if part_layer==1 then
			group="tail"
		end
	elseif part_group=="brows" then
		if part_layer==1 then
			group="eye"
		end
	elseif part_group=="eyes" then
		if part_layer==1 then
			group="eyeball"
		end
	elseif part_group=="nose" then
		if part_layer==1 then
			group="nose"
		elseif part_layer==2 then -- glasses
			group="specs"
		end
	elseif part_group=="mouth" then
		if part_layer==1 then
			group="mouth"
		elseif part_layer==2 then -- beard
			group="beard"
		elseif part_layer==3 then -- fags
			group="inmouth"
		end
	elseif part_group=="ears" then
		if part_layer==1 then
			group="ear"
		end
	end
	
local fnames={}
part_cache={}

	for i,group in ipairs(groups or {group}) do

local avatar_parts=fenestra_avatar.xox_groups[group]

		if avatar_parts then
			for n,v in pairs(avatar_parts) do
			
				if v.active==1 then
					part_cache[#part_cache+1]=v
					fnames[#fnames+1]=xox_pre..v.xox..xox_pst
				end
			
			end
		end
		
	end

	setup_parts(fnames)

end

-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
function setup_parts(t)

	for i=1,64 do
	
		if parts[i] then -- free old parts
			local v=parts[i]
			if v.xox then
				v.xox.clean()
			end
			parts[i]=nil
		end
		
		if t[i] then -- load new parts
			local v={}
			parts[i]=v
			v.dat=win.data.load(t[i])
			v.xox=win.xox(v.dat)
		end
	end

end


-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
function page(hash)

	win.widget:clean_all()
	w={}
	page_thunk(hash)
	
end

function page_thunk(hash)

-- make an avatar with scrolly bar on the main page
	local function w_avatar(x,y,hx,hy)
	
		if w.avatar then
			w.avatar:clean_all()
		else
			w.avatar=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end

		w.avatar_goto=w.avatar:add{class="flow",hxf=1,hyf=1/12,pxf=0,pyf=0,mx=3}
		w.avatar_colors=w.avatar_goto:add{text="colors",id="goto_colors",hooks=hooks,color=0x44ffffff}
		w.avatar_parts=w.avatar_goto:add{text="parts",id="goto_parts",hooks=hooks,color=0x44ffffff}
		w.avatar_soul=w.avatar_goto:add{text="soul",id="goto_soul",hooks=hooks,color=0x44ffffff}
		w.avatar_anim=w.avatar_goto:add{text="anims",id="goto_anims",hooks=hooks,color=0x44ffffff}
		w.avatar_x5=w.avatar_goto:add{}
		w.avatar_x6=w.avatar_goto:add{}
		w.avatar_rot=w.avatar:add{class="oldslide",color=0x22ffffff,hxf=1,hyf=1/24,pxf=0,pyf=23/24}
		w.avatar_rot:add{id="rotate",color=0x88ffffff,hooks=hooks,pxf=1/2,pyf=0,hxf=3/10,hyf=1}
		
	end

-- make a color selector
	local function w_colors(x,y,hx,hy)
	
		if w.colors then
			w.colors:clean_all()
		else
			w.colors=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end
		
		
		local colors=w.colors:add({hx=hx,hy=hy/2,mx=8,class="flow",pxf=0,pyf=0})
		for i,v in ipairs(pick_colors) do
			colors:add{color=v,id="color",hooks=hooks,highlight="shrink"}
		end
		
		local slides=w.colors:add({hx=hx,hy=hy/2,mx=2,class="flow",pxf=0,pyf=1/2})

--[[		
		w.color_alpha=slides:add{class="oldslide",sx=2}
		w.color_alpha:add{text="Alpha",color=0x88ffffff,text_color=0xffffffff,id="alpha",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}
]]		
		w.color_red=slides:add{class="oldslide",sx=2}
		w.color_red:add{text="Red",color=0x88ffffff,text_color=0xffffffff,id="red",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}
		
		w.color_green=slides:add{class="oldslide",sx=2}
		w.color_green:add{text="Green",color=0x88ffffff,text_color=0xffffffff,id="green",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}

		w.color_blue=slides:add{class="oldslide",sx=2}
		w.color_blue:add{text="Blue",color=0x88ffffff,text_color=0xffffffff,id="blue",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}

		w.color=slides:add{text="ff00ff00",highlight="text",color=0xff00ff00,id="set_color",hooks=hooks}
		w.spec=slides:add{text="ff00ff00",highlight="text",color=0xff00ff00,id="set_spec",hooks=hooks}
		
		w.color_gloss=slides:add{class="oldslide",sx=2}
		w.color_gloss:add{text="Gloss",color=0x88ffffff,text_color=0xffffffff,id="gloss",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}
		
		set_color(0xff888888)
		
	end

-- make a surface selector
	local function w_surf(x,y,hx,hy)
	
		if w.surf then
			w.surf:clean_all()
		else
			w.surf=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end
		
		w.surfs={}
		w.specs={}

		local surf=w.surf:add({hx=hx,hy=hy*3/4,mx=10,class="flow",pxf=0,pyf=1/4})
		for i,v in ipairs(pick_surfs) do
			w.surfs[v]=surf:add{color=0xff888888,sx=4,text=v,id="surf",user=v,hooks=hooks,highlight="text"}
			w.specs[v]=surf:add{color=0xff888888,sx=1,id="spec",user=v,hooks=hooks,highlight="text"}
		end
		
		load_surf(soul)
		
		
		select_spec("skin")
		select_surf("skin")
		
	end
		
-- make a loadsave selector
	local function w_loadsave(x,y,hx,hy)
	
		if w.loadsave then
			w.loadsave:clean_all()
		else
			w.loadsave=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end

		local ls=w.loadsave:add({hx=hx,hy=hy*3/4,mx=1,class="flow",pxf=0,pyf=1/4})

		w.load_soul=ls:add{color=0xff888888,sx=4,text="Load soul",id="load_soul",hooks=hooks}
		w.save_soul=ls:add{color=0xff888888,sx=4,text="Save soul",id="save_soul",hooks=hooks}
		
		w.dumptest=ls:add{color=0xffff0000,sx=4,text="dumptest",id="dumptest",hooks=hooks}
				
	end

-- make a parts selector
	local function w_parts(x,y,hx,hy)
	
		if w.parts_base then
			w.parts_base:clean_all()
		else
			w.parts_base=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end
		
		local ls=w.parts_base:add({hx=hx,hy=hy*1/12,mx=5,class="flow",pxf=0,pyf=0})
		for i,v in ipairs{"Layer 1","2","3","4"} do
			local sx=1
			if i==1 then sx=2 end
			local color=0x44ffffff
			if part_layer==i then
				color=0x88ffffff
			end
			w.groups[i]=ls:add{color=color,sx=sx,sy=1,id="part_layer",text=v,user=i,hooks=hooks}
		end	
		
		local ls=w.parts_base:add({hx=hx,hy=hy*11/12,mx=4,class="flow",pxf=0,pyf=1/12})
		w.parts={}
		for i=1,16 do
			w.parts[i]=ls:add{color=0x44ffffff,sx=1,sy=1,id="part",user=i+((part_page-1)*16),hooks=hooks}
		end	
		

	end

-- make a parts group selector
	local function w_groups(x,y,hx,hy)
	
		if w.groups_base then
			w.groups_base:clean_all()
		else
			w.groups_base=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end

		local ls=w.groups_base:add({hx=hx,hy=hy,mx=2,class="flow",pxf=0,pyf=0})

		w.groups={}
		for i,v in ipairs{
		"Head","Body","Hands","Feet","Brows","Eyes","Mouth","Nose","Ears","Tail"} do
		
			local color=0x44ffffff
			if part_group==v:lower() then
				color=0x88ffffff
			end
			w.groups[i]=ls:add{color=color,sx=1,sy=1,id="part_group",text=v,user=v:lower(),hooks=hooks}
		end	
	end

-- make a parts modifier
	local function w_pmod(x,y,hx,hy)
	
		if w.pmod_base then
			w.pmod_base:clean_all()
		else
			w.pmod_base=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end
		
		local ls=w.pmod_base:add({hx=hx,hy=hy*1/7,mx=5,class="flow",pxf=0,pyf=0})

		w.pages={}
		for i,v in ipairs{" Page 1","2","3","4"} do
			local sx=1
			if i==1 then sx=2 end
			local color=0x44ffffff
			if part_page==i then
				color=0x88ffffff
			end
			w.pages[i]=ls:add{color=color,sx=sx,sy=1,id="part_page",text=v,user=i,hooks=hooks}
		end	
		
		local ls=w.pmod_base:add({hx=hx,hy=hy*1/7,mx=3,class="flow",pxf=0,pyf=1/7})
		
		w.sides={}
		for i,v in ipairs{"Left","Right","Both"} do
			local color=0x44ffffff
			if part_side==v:lower() then
				color=0x88ffffff
			end
			w.sides[i]=ls:add{color=color,sx=1,sy=1,id="part_side",text=v,user=v:lower(),hooks=hooks}
		end
		
		w.part_pos=w.pmod_base:add{class="oldslide",hx=hx*1/4,hy=hy*3/7,pxf=0,pyf=2/7,id="pos_reset",color=0x22ffffff,hooks=hooks}
		w.part_pos:add{text="X",color=0x88ffffff,text_color=0xffffffff,id="pos",hooks=hooks,pxf=0,pyf=0,hxf=1/3,hyf=1/3}
		
		w.part_size=w.pmod_base:add{class="oldslide",hx=hx*3/4,hy=hy*1/7,pxf=1/4,pyf=2/7,id="size_reset",color=0x22ffffff,hooks=hooks}
		w.part_size:add{text="Size",color=0x88ffffff,text_color=0xffffffff,id="size",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}
		
		w.part_aspect=w.pmod_base:add{class="oldslide",hx=hx*3/4,hy=hy*1/7,pxf=1/4,pyf=3/7,id="aspect_reset",color=0x22ffffff,hooks=hooks}
		w.part_aspect:add{text="Aspect",color=0x88ffffff,text_color=0xffffffff,id="aspect",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}

		w.part_morph=w.pmod_base:add{class="oldslide",hx=hx*3/4,hy=hy*1/7,pxf=1/4,pyf=4/7,id="morph_reset",color=0x22ffffff,hooks=hooks}
		w.part_morph:add{text="Morph",color=0x88ffffff,text_color=0xffffffff,id="morph",hooks=hooks,pxf=0,pyf=0,hxf=4/10,hyf=1}

	end
	
-- make an anims selector
	local function w_anims(x,y,hx,hy)
	
		if w.anims_base then
			w.anims_base:clean_all()
		else
			w.anims_base=win.widget:add{class="rel",hx=hx,hy=hy,px=x,py=y}
		end

		local ls=w.anims_base:add({hx=hx,hy=hy,mx=20,class="flow",px=0,py=0})

		w.anims={}
		for i,v in ipairs(anim_names) do
		
			local color=0x44ffffff
			if anim_name==v.name:lower() then
				color=0x88ffffff
			end
			w.anims[i]=ls:add{color=color,sx=#v.name+2,sy=1,id="anim",text=v.name,user=v.name:lower(),hooks=hooks}
		end	
	end
	

-- test page layouts

	if tonumber(hash)==0 then
	
		win.widget:layout()
		
		win.widget.state="ready"
	
	elseif tonumber(hash)==1 then
	
		local top=win.widget:add({hx=640,hy=480,mx=5,class="flow",pxf=0,pyf=0})
		local colors=win.widget:add({hx=320,hy=240,mx=8,class="flow",pxf=0,pyf=0.5})
		for i,v in ipairs(pick_colors) do
			colors:add{color=v,id="color",hooks=hooks,highlight="shrink"}
		end
		
		top:add({text="random",color=0x88ff0000,id="random",hooks=hooks})
		top:add({text="colors",color=0x88ff0000,id="colors",hooks=hooks})
		top:add({text="pose",color=0x88ff0000,id="pose",hooks=hooks})
		top:add({text="parts",color=0x88ff0000,id="parts",hooks=hooks})
		top:add({text="save",color=0x88ff0000,id="save",hooks=hooks})
		top:add({sy=22,sx=5})
		local drag=top:add({sy=1,sx=5,class="oldslide",color=0x22ffffff})
		drag:add{text="rotate",color=0x88ffffff,id="rotate",hooks=hooks,pxf=0,pyf=0,hxf=2/10,hyf=1}

		win.widget:layout()
		
		win.widget.state="ready"
	
	elseif tostring(hash)=="colors" then
	
		w_avatar(320,0,320,480)
		w_colors(0,0,320,240)
		w_surf(0,240,320,240)
		
		win.widget:layout()
		
		win.widget.state="ready"
	
	elseif tostring(hash)=="soul" then
	
		w_avatar(320,0,320,480)
		w_loadsave(0,0,320,120)
		
		win.widget:layout()
		
		win.widget.state="ready"
	
	elseif tostring(hash)=="parts" then
	
		w_avatar(320,0,320,480)
		w_groups(0,0,320,100)
		w_parts(0,100,320,240)
		w_pmod(0,340,320,140)
		
		win.widget:layout()
		
		win.widget.state="ready"
	
	elseif tostring(hash)=="anims" then
	
		w_avatar(320,0,320,480)
		w_anims(0,0,320,480)
		
		win.widget:layout()
		
		win.widget.state="ready"
	
	end

end
	


-----------------------------------------------------------------------------
--
-- build and save a sheet of avatar images for use in flash
--
-----------------------------------------------------------------------------
function sheet(soulname)

	if soulname then
		load_soul(wldir.."local/avatar/soul/"..soulname..".xml")
	else
		soulname="me"
	end
	

	set_anim( anim_names["walk"] )
	local walk_length=xsx.length

	set_anim( anim_names["breath"] )
	local breath_length=xsx.length
	
	local que={
	
		{ "walk"		,  60, walk_length*0/8},
		{ "walk"		,  60, walk_length*1/8},
		{ "walk"		,  60, walk_length*2/8},
		{ "walk"		,  60, walk_length*3/8},
		{ "walk"		,  60, walk_length*4/8},
		{ "walk"		,  60, walk_length*5/8},
		{ "walk"		,  60, walk_length*6/8},
		{ "walk"		,  60, walk_length*7/8},
		
		{ "walk"		, -60, walk_length*0/8},
		{ "walk"		, -60, walk_length*1/8},
		{ "walk"		, -60, walk_length*2/8},
		{ "walk"		, -60, walk_length*3/8},
		{ "walk"		, -60, walk_length*4/8},
		{ "walk"		, -60, walk_length*5/8},
		{ "walk"		, -60, walk_length*6/8},
		{ "walk"		, -60, walk_length*7/8},
		
		{ "walk"		,   0, walk_length*0/4},
		{ "walk"		,   0, walk_length*1/4},
		{ "walk"		,   0, walk_length*2/4},
		{ "walk"		,   0, walk_length*3/4},
		
		{ "walk"		, 180, walk_length*0/4},
		{ "walk"		, 180, walk_length*1/4},
		{ "walk"		, 180, walk_length*2/4},
		{ "walk"		, 180, walk_length*3/4},
		
		{ "breath"		,   0, breath_length*0/4},
		{ "breath"		,   0, breath_length*1/4},
		{ "breath"		,   0, breath_length*2/4},
		{ "breath"		,   0, breath_length*3/4},
		
		{ "splat"		,   0, 0},
		{ "breath"		, -60, breath_length*0/4},
		{ "breath"		, 180, breath_length*0/4},
		{ "breath"		,  60, breath_length*0/4},
		
		{ "teapot"			,   0, 0},
		{ "angry"			,   0, 0},
		{ "confused"		,   0, 0},
		{ "determind"		,   0, 0},
		{ "devious"			,   0, 0},
		{ "embarrassed"		,   0, 0},
		{ "energetic"		,   0, 0},
		{ "excited"			,   0, 0},
		
		{ "bird"			,   0, 0},
		{ "indescribable"	,   0, 0},
		{ "nerdy"			,   0, 0},
		{ "sad"				,   0, 0},
		{ "scared"			,   0, 0},
		{ "sleepy"			,   0, 0},
		{ "thoughtful"		,   0, 0},
		{ "working"			,   0, 0},
	}
	
	local x=0
	local y=0
	local max_x=800
	local max_y=600
	
	local gd=grd.create("GRD_FMT_U8_RGBA",max_x,max_y,1)
		
	for i=1,#que do local v=que[i]
	
print( v[1].." "..v[2].." "..v[3] )
	
		set_anim( anim_names[ v[1] ] )
	
		local fov=16
		local r=win.target(512,512)
		
		win.begin(512,512)
		
		win.clip2d(0,0,0,0)
		win.project23d(512/512,fov,1024)
		gl.MatrixMode("MODELVIEW")
	
		gl.Translate(0,-46/fov, -50)
		gl.Scale(46/fov,46/fov,46/fov)
		gl.Rotate( v[2] +180 ,0,1,0)		
		xsx.draw(v[3])
		
		-- grab display
		local w,h,s=win.readpixels()
		win.target(0,0) -- unbind	
		local ga=grd.create("GRD_FMT_U8_RGBA",w,h,1)
		ga:pixels(0,0,w,h,s)
		ga:flipy() -- silly upside down ogl
		
		-- scale to 100x100
		ga:scale(100,100,1)
		
		-- paste into sheet
		local s=ga:pixels(0,0,100,100,"")
		gd:pixels(x,y,100,100,s)
	
		x=x+100
		if x>= max_x then x=0 y=y+100 end
	end

	gd:save(wldir.."local/avatar/tards/"..soulname..".png")

-- save an xml file too, all it does is point to this png

local	fp=io.open( wldir.."local/avatar/tards/"..soulname..'.xml' , "w" )
		fp:write(			
[[<?xml version="1.0" encoding="UTF-8"?>

<ville><vtard>

<img type="base" src="http://data.wetgenes.com/game/s/ville/test/vtard/]]..soulname..[[.png" />

</vtard></ville>
]])
		fp:close()
		
end

_G.sheet=sheet




--[[ some old junk
		
--		win.swap()

	
		local fov=16
		local r=win.target(512,512)
		print(r)
		
		win.begin(512,512)
		
		win.clip2d(0,0,0,0)
		win.project23d(512/512,fov,1024)
		gl.MatrixMode("MODELVIEW")

		do
		local x,y,s
		
			gl.Translate(0,-46/fov, -50)
			gl.Scale(46/fov,46/fov,46/fov)
			gl.Rotate(-45,0,1,0)
			
			xsx.draw(0)
			
		end
		
		win.swap()
		
		local w,h,s=win.readpixels()
		
		
		win.target(0,0)
		

		print(w,h)
		
		local ga=grd.create("GRD_FMT_U8_RGBA",w,h,1)
		ga:pixels(0,0,w,h,s)
		ga:flipy() -- silly upside down ogl
		
		ga:save(wldir.."test.png")
		
		ga:scale(100,100,1)
		ga:save(wldir.."test2.png")
		
--		ga:quant(256)
--		ga:save(wldir.."test3.png")
		
-- [ [
		local head={0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x80,0x07,0x38,0x04,0x20,0x00}
		head[13]=w%256
		head[14]=math.floor(w/256)
		head[15]=h%256
		head[16]=math.floor(h/256)
		local fp=io.open(wldir.."test.tga","wb")
		fp:write(string.char(unpack(head)))
		fp:write(s)
		fp:close()
] ]

]]

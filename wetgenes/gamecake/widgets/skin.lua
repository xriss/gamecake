-- copy all globals into locals, some locals are renamed to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,luaload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local bit=require('bit')
local gl=require('gles').gles1
local grd=require('wetgenes.grd')
local pack=require('wetgenes.pack')

local wzips=require("wetgenes.zips")


local apps=apps

local function explode_color(c)

	local r,g,b,a
	
	a=bit.band(bit.rshift(c,24),0xff)
	r=bit.band(bit.rshift(c,16),0xff)
	g=bit.band(bit.rshift(c, 8),0xff)
	b=bit.band(c,0xff)

	return r/0xff,g/0xff,b/0xff,a/0xff
end

local function implode_color(r,g,b,a)

	if type(r)=="table" then a=r[4] b=r[3] g=r[2] r=r[1] end -- convert from table?
	
	local c
	
	c=             bit.band(b*0xff,0xff)
	c=c+bit.lshift(bit.band(g*0xff,0xff),8)
	c=c+bit.lshift(bit.band(r*0xff,0xff),16)
	c=c+bit.lshift(bit.band(a*0xff,0xff),24)

	return c
end



module("wetgenes.gamecake.widgets.skin")

function bake(state,wskin)
wskin=wskin or {}


local cake=state.cake
local images=cake.images
local sheets=cake.sheets
local canvas=state.canvas
local font=canvas.font
local flat=canvas.flat


local mode=nil
local texs={}

local margin=0 -- whitespace
local border=0 -- solidspace
--
-- unload a skin, go back to the "builtin" default
--
function wskin.unload()

	mode=nil
	texs={}
	
end


--
-- load a skin
--
function wskin.load(name)
	
	wskin.unload()
	
	local images=state.cake.images

	if name then -- load a named skin
	
		if name=="soapbar" then
			mode=name
			
			images.TEXTURE_MIN_FILTER=gl.LINEAR -- disable mipmapping? it seems to feck draw33 up somehow?
			images.loads{
				"wskins/"..mode.."/border",
				"wskins/"..mode.."/buttof",
				"wskins/"..mode.."/button",
				"wskins/"..mode.."/buttin",
			}
			images.TEXTURE_MIN_FILTER=nil
			margin=15
			border=0
			
		end
			
	end

end


--
-- add meta functions
--
function wskin.setup(def)

--	load(def.win,"test")


	local master=def.master
	local meta=def.meta
	local win=def.win
--	local font=--[[def.font]]def.state.cake.fonts.get(1)

	
local function draw33(tw,th, mw,mh, vxs,vys, vw,vh)
		
--		local vw,vh=512,52
--		local mw,mh=24,24

		if mw*2 > vw then mw=vw/2 end
		if mh*2 > vh then mh=vh/2 end

		
		local tww={mw/tw,(tw-2*mw)/tw,mw/tw}
		local thh={mh/th,(th-2*mh)/th,mh/th}
		local vww={mw,vw-2*mw,mw}
		local vhh={mh,vh-2*mh,mh}
		
--[[
		gl.Disable(gl.DEPTH_TEST)
		gl.Disable(gl.LIGHTING)
		gl.Disable(gl.CULL_FACE)
		gl.Enable(gl.TEXTURE_2D)

		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
		gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
]]


--		gl.Begin(gl.QUADS)
--			gl.Color(1,1,1,1)
			local t={}
			local function drawbox() -- draw all 9 parts in one go
				flat.tristrip("xyzuv",t)
			end
			local function tdrawbox( tx,ty, vx,vy , txp,typ, vxp,vyp )
			
				local ht=#t
				for i,v in ipairs{
					vx,		vy,		0,	tx,		ty, -- doubletap hack so we can start at any location
					vx,		vy,		0,	tx,		ty,
					vx+vxp,	vy,		0,	tx+txp,	ty,
					vx,		vy+vyp,	0,	tx,		ty+typ,
					vx+vxp,	vy+vyp,	0,	tx+txp,	ty+typ,
					vx+vxp,	vy+vyp,	0,	tx+txp,	ty+typ, -- doubletap hack so we can start at any location
				} do
					t[ht+i]=v
				end
--[[
					tx,		ty,		0,	--vx,		vy,
					tx+txp,	ty,		0,	--vx+vxp,	vy,
					tx,		ty+typ,	0,	--vx,		vy+vyp,
					tx+txp,	ty+typ,	0,	--vx+vxp,	vy+vyp,


				gl.TexCoord(tx    ,ty)     gl.Vertex(vx,    vy)
				gl.TexCoord(tx+txp,ty)     gl.Vertex(vx+vxp,vy)
				gl.TexCoord(tx+txp,ty+typ) gl.Vertex(vx+vxp,vy+vyp)
				gl.TexCoord(tx    ,ty+typ) gl.Vertex(vx,    vy+vyp)
]]
			end
			
		local tx,ty=0,0
		local vx,vy=vxs,vys-- -vw/2,vh/2

			for iy=1,3 do
				tx=0
				vx=vxs-- -vw/2
				for ix=1,3 do

					tdrawbox( tx,ty, vx,vy , tww[ix],thh[iy], vww[ix],vhh[iy] )

					tx=tx+tww[ix]
					vx=vx+vww[ix]
				end
				ty=ty+thh[iy]
				vy=vy+vhh[iy]
			end

		drawbox()
			
--		gl.End()


end
	
--
-- display this widget and its sub widgets
--
	function meta.draw(widget)
		
		if debug_hook then debug_hook("draw",widget) end
		
		

		-- draw rectangle


		gl.PopMatrix() -- expect the base to be pushed
		gl.PushMatrix()
		
		gl.Translate(widget.pxd,widget.pyd,0)
		
		if widget.fbo then
			if widget.fbo.width~=widget.sx or widget.fbo.height~=widget.sy then -- resize so we need a new fbo
				widget.fbo:clean()
				widget.fbo=nil
			end
			if not widget.fbo then -- allocate a new fbo
--print("new fbo",widget.sx,widget.sy)
				widget.fbo=_G.win.fbo(widget.sx,widget.sy,0)
				widget.dirty=true -- flag redraw
			end
				
		end

if ( not widget.fbo ) or widget.dirty then -- if no fbo and then we are always dirty... Dirty, dirty, dirty.

		if widget.fbo then
--print("into fbo")
			
			gl.MatrixMode(gl.PROJECTION)
			gl.PushMatrix()

			widget.fbo:bind()
			
--			gl.ClearColor(14/15,14/15,14/15,1)
			gl.ClearColor(0,0,0,0)
			gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

			win.project23d(widget.sx/widget.sy,1,1024)
						
			gl.MatrixMode(gl.MODELVIEW)
			gl.LoadIdentity()
			gl.Translate(-widget.sx/2,widget.sy/2,-widget.sy/2)
			gl.Translate(-widget.pxd,-widget.pyd,0)

			if widget.pan_px and widget.pan_py then -- fidle everything
				gl.Translate(widget.pan_px,widget.pan_py,0)
			end
						
			gl.PushMatrix() -- put new base matrix onto stack
		end
		
		widget.dirty=nil
				
--		gl.Disable(gl.LIGHTING)
--		gl.Disable(gl.DEPTH_TEST)
--		gl.Disable(gl.CULL_FACE)
--		gl.Disable(gl.TEXTURE_2D)
		
		local txp,typ=0,0
		
		if widget.color then
		
			buttdown=false
			if ( master.press and master.over==widget ) or widget.state=="selected" then
				buttdown=true
			end


			if widget.highlight=="shrink" then
			
				gl.Color( explode_color(widget.color))
					
				if master.over==widget then
					gl.Translate(widget.hx/16,widget.hy/16,0)
					gl.Scale(7/8,7/8,1)
				end
			
			elseif widget.highlight=="none" then
			
				gl.Color( explode_color(widget.color))
				
			elseif widget.highlight=="text" then
			
				gl.Color( explode_color(widget.color))
				
			else -- widget.highlight=="dark" -- default is to darken everything slightly when it is not the active widget
			
				if master.over==widget then
					if buttdown then
						local c={explode_color(widget.color)}
						c[3]=c[3]*14/16
						c[2]=c[2]*14/16
						c[1]=c[1]*14/16
						gl.Color( c[1],c[2],c[3],c[4] )
					else
						gl.Color( explode_color(widget.color))
					end
				else
					local c={explode_color(widget.color)}
					c[3]=c[3]*12/16
					c[2]=c[2]*12/16
					c[1]=c[1]*12/16
					gl.Color( c[1],c[2],c[3],c[4] )
				end
				
			end
			
			local hx=widget.hx
			local hy=widget.hy
			local bb=2
			local tl={1,1,1,0.25}
			local br={0,0,0,0.25}
			tl[1]=tl[1]*tl[4]
			tl[2]=tl[2]*tl[4]
			tl[3]=tl[3]*tl[4]
			br[1]=br[1]*br[4]
			br[2]=br[2]*br[4]
			br[3]=br[3]*br[4]
			
			if buttdown then -- flip highlight
				tl,br=br,tl
				txp=1
				typ=1
			end
			
			if widget.sheet then -- custom graphics

				sheets.get(widget.sheet):draw(1,0,0,0,hx,hy)

			
			elseif mode then

		
				if ( master.press and master.over==widget ) or widget.state=="selected" then
					images.bind(images.get("wskins/"..mode.."/buttin"))
					txp=0
					typ=-1
				else
					images.bind(images.get("wskins/"..mode.."/button"))
					txp=0
					typ=-2
				end
				
				if widget.class=="string" then -- hack
					images.bind(images.get("wskins/"..mode.."/border"))
				end
				
				draw33(128,128, 24,24, 0-margin,0-margin, hx+(margin*2),hy+(margin*2))
			
			else -- builtin
			
			
			flat.quad(	0,		0,
						hx,		0,
						hx,		hy,
						0,		hy)
			gl.Color( tl[1],tl[2],tl[3],tl[4] )
			flat.quad(	0,		0,
						hx,		0,
						hx-bb,	bb,
						0+bb, 	bb)
			flat.quad(	0,		0,
						0+bb,	bb,
						0+bb, 	hy-bb,
						0,    	hy)
			gl.Color( br[1],br[2],br[3],br[4] )
			flat.quad( hx,  	hy,
						0,  	hy,
						0+bb,	hy-bb,
						hx-bb,	hy-bb)
			flat.quad(  hx,    0,
						hx,    hy,
						hx-bb, hy-bb,
						hx-bb, bb)
			end
			
		end
		
		if widget.text then
		
--print("using font "..(widget.font or widget.master.font or 1))

			local ty=widget:bubble("text_size") or 16

			local f=widget.font or widget.master.font or 1
			if f then
				if type(f)=="number" then
				else
					typ=typ-ty/8 -- reposition font slightly as fonts other than the builtin probably have decenders
				end
			end
			
			font.set(cake.fonts.get(f))
			font.set_size(ty,0)
			local tx=font.width(widget.text)

--			local tx,ty=font.size(widget.text,widget.text_size)
			
			local c=widget.text_color
			
			if widget.text_color_over then
				if master.over==widget then
					c=widget.text_color_over
				end
			end
			
			if widget.text_align=="left" then
				tx=0
				ty=0			
			elseif widget.text_align=="right" then
				tx=(widget.hx-tx)
				ty=(widget.hy-ty)			
			else
				tx=(widget.hx-tx)/2
				ty=(widget.hy-ty)/2
			end
			
			tx=tx+txp
			ty=ty+typ
			
			widget.text_x=tx
			widget.text_y=ty

			if widget.text_color_shadow then
				gl.Color( pack.argb8_pmf4(widget.text_color_shadow) )
				font.set_xy(tx+1,ty+1)
				font.draw(widget.text)
			end
			
			gl.Color( pack.argb8_pmf4(c) )
			font.set_xy(tx,ty)
			font.draw(widget.text)


--			font.set(tx,-ty,c,widget.text_size)
--			font.draw(widget.text)
		

				if widget.class=="textedit" then -- hack
					if widget.master.focus==widget then --only draw curser in active widget
						if widget.master.throb>=128 then
							local sw=font.width(widget.text:sub(1,widget.data.str_idx))

			font.set_xy(tx+sw,ty)
			font.draw("_")
						end
					end
				end

		end
		
		for i,v in ipairs(widget) do
			if not v.fbo or not v.dirty then -- terminate recursion at dirty fbo
				v:draw()
			end
		end

		if widget.fbo then -- we have drawn into the fbo
			
			gl.MatrixMode(gl.PROJECTION)
			gl.PopMatrix()
			
			gl.MatrixMode(gl.MODELVIEW)
			gl.PopMatrix()
			
			win.fbo_bind()
		end
		
else -- we can only draw once

		if widget.fbo then -- we need to draw our cached fbo
		
			gl.Disable(gl.LIGHTING)
			gl.Disable(gl.DEPTH_TEST)
			gl.Disable(gl.CULL_FACE)
			gl.Disable(gl.TEXTURE_2D)
		
			gl.Translate(widget.sx/2,-widget.sy/2,0)
			gl.Color(1,1,1,1)
			widget.fbo:draw()
--print("draw fbo")
		end
		
end
	
		
		return widget
	end



end

return wskin
end

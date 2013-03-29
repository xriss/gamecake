-- copy all globals into locals, some locals are renamed to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,luaload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local bit=require('bit')
local grd=require('wetgenes.grd')
local pack=require('wetgenes.pack')
local tardis=require('wetgenes.tardis')

local wzips=require("wetgenes.zips")
local wstr=require("wetgenes.string")


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



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wskin)
wskin=wskin or {}

local gl=oven.gl
local cake=oven.cake
local images=cake.images
local sheets=cake.sheets
local canvas=cake.canvas
local font=canvas.font
local flat=canvas.flat

local layouts=cake.layouts


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

	
local function draw33(tw,th, mw,mh, vxs,vys, vw,vh,invert)
		
--		local vw,vh=512,52
--		local mw,mh=24,24

		local force_tww,force_thh
		
		if mw*4>tw then twwidx=1 end -- mipmap hack for small buttons
		if mh*4>th then thhidx=1 end

		if mw*2 > vw then mw=vw/2 end
		if mh*2 > vh then mh=vh/2 end

		
		local tww={mw/tw,(tw-2*mw)/tw,mw/tw}
		local thh={mh/th,(th-2*mh)/th,mh/th}
		local vww={mw,vw-2*mw,mw}
		local vhh={mh,vh-2*mh,mh}
		

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
			end
			
			if invert then -- draw inverted texture, sometimes useful as it flips highlights
				tdrawbox=function ( tx,ty, vx,vy , txp,typ, vxp,vyp )
					local ht=#t
					for i,v in ipairs{
						vx,		vy,		0,	1-(tx),		1-(ty), -- doubletap hack so we can start at any location
						vx,		vy,		0,	1-(tx),		1-(ty),
						vx+vxp,	vy,		0,	1-(tx+txp),	1-(ty),
						vx,		vy+vyp,	0,	1-(tx),		1-(ty+typ),
						vx+vxp,	vy+vyp,	0,	1-(tx+txp),	1-(ty+typ),
						vx+vxp,	vy+vyp,	0,	1-(tx+txp),	1-(ty+typ), -- doubletap hack so we can start at any location
						vx+vxp,	vy+vyp,	0,	1-(tx+txp),	1-(ty+typ), -- doubletap hack so we can start at any location
					} do
						t[ht+i]=v
					end
				end
			end
			
		local tx,ty=0,0
		local vx,vy=vxs,vys-- -vw/2,vh/2

			for iy=1,3 do
				tx=0
				vx=vxs-- -vw/2
				for ix=1,3 do

					tdrawbox( tx,ty, vx,vy , tww[twwidx or ix],thh[thhidx or iy], vww[ix],vhh[iy] ) -- fake texture sample to con mipmap mode

					tx=tx+tww[ix]
					vx=vx+vww[ix]
				end
				ty=ty+thh[iy]
				vy=vy+vhh[iy]
			end

		drawbox()


end
	
--
-- display this widget and its sub widgets
--
	function meta.draw(widget)
		
		if debug_hook then debug_hook("draw",widget) end

--		gl.PopMatrix() -- expect the base to be pushed
		gl.PushMatrix()
		gl.Translate(widget.px,widget.py,0)
		
		if widget.anim then
			widget.anim:draw()		
		end
		
		if widget.clip then
		
			widget.layout=layouts.create{parent2={x=0,y=0,w=widget.master.hx,h=widget.master.hy},
				x=widget.pxd,
				y=widget.pyd,
				w=widget.hx,
				h=widget.hy}
			
			gl.MatrixMode(gl.PROJECTION)
			gl.PushMatrix()
			
			gl.MatrixMode(gl.MODELVIEW)
			gl.PushMatrix()


			widget.old_layout=widget.layout.apply(true) -- forced size

--			gl.Translate(-widget.pxd,-widget.pyd,0)
		end
		
		if widget.pan_px and widget.pan_py and not widget.fbo  then -- fidle everything
--print("draw",widget.pan_px,widget.pan_py)
			gl.Translate(-widget.pan_px,-widget.pan_py,0)
		end
		
		if widget.fbo then
			if widget.fbo.w~=widget.hx or widget.fbo.h~=widget.hy then -- resize so we need a new fbo
--print("new fbo",widget.sx,widget.sy)
				widget.fbo:resize(widget.hx,widget.hy,0)
				widget.dirty=true -- flag redraw
			end				
		end

--widget.old_layout
--widget.layout

if ( not widget.fbo ) or widget.dirty then -- if no fbo and then we are always dirty... Dirty, dirty, dirty.

		if widget.fbo then
--print("drawing into fbo")

			widget.fbo:bind_frame()
			widget.layout=layouts.create{parent={x=0,y=0,w=widget.fbo.w,h=widget.fbo.h}}
			
			gl.MatrixMode(gl.PROJECTION)
			gl.PushMatrix()
			
			gl.MatrixMode(gl.MODELVIEW)
			gl.PushMatrix()

			widget.old_layout=widget.layout.apply()

			gl.ClearColor(0,0,0,0)
			gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		
			gl.ClearColor(0,0,0,0)
			gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

			gl.Translate(-widget.px,-widget.py,0)
			if widget.pan_px and widget.pan_py then -- fidle everything
				gl.Translate(-widget.pan_px,-widget.pan_py,0)
			end
			
			gl.PushMatrix() -- put new base matrix onto stack so we can pop to restore?
--			gl.PushMatrix() -- put new base matrix onto stack so we can pop to restore?

		end
		
		widget.dirty=nil
						
		local txp,typ=0,0
		
		if widget.color then -- need a color to draw, so no color, no draw
		
			local style=widget.style
			
			if not style then -- make assumptions
			
				style="button" -- default

				if  widget.class=="textedit" then
				
					style="indent" -- draw upside down button?
					
				elseif (not widget.hooks) then -- probably not a button
				
					style="flat"
				
				end
			end
				
			local buttdown=false
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
			
				if style=="indent" then
					local c={explode_color(widget.color)}
					c[3]=c[3]*12/16
					c[2]=c[2]*12/16
					c[1]=c[1]*12/16
					gl.Color( c[1],c[2],c[3],c[4] )
				elseif master.over==widget then
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

			
			elseif mode then -- got some images to play with
			
				if style=="flat" then
				
					images.bind(images.get("wskins/"..mode.."/border"))
					txp=0
					typ=-1

					draw33(128,128, 24,24, 0-margin,0-margin, hx+(margin*2),hy+(margin*2))

				elseif style=="indent" then

					images.bind(images.get("wskins/"..mode.."/buttin"))
					txp=0
					typ=0

					draw33(128,128, 24,24, 0-margin,0-margin, hx+(margin*2),hy+(margin*2),true)

				elseif style=="button" then

					if ( master.press and master.over==widget ) or widget.state=="selected" then
						images.bind(images.get("wskins/"..mode.."/buttin"))
						txp=0
						typ=-1
					else
						images.bind(images.get("wskins/"..mode.."/button"))
						txp=0
						typ=-2
					end
					
					draw33(128,128, 24,24, 0-margin,0-margin, hx+(margin*2),hy+(margin*2))
				end
								
			
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
		
			local fy=widget:bubble("text_size") or 16

			local f=widget.font or widget.master.font or 1
			if f then
				if type(f)=="number" then
				else
					typ=typ-fy/8 -- reposition font slightly as fonts other than the builtin probably have descenders
				end
			end
			
			font.set(cake.fonts.get(f))
			font.set_size(fy,0)

			local lines
			
			if widget.text_align=="wrap" then
				lines=font.wrap(widget.text,{w=widget.hx}) -- break into lines
				widget.lines=lines -- remember wraped text
			else
				lines={widget.text}
			end

			local ty=typ
			local c=widget.text_color
			if widget.text_color_over then
				if master.over==widget then
					c=widget.text_color_over
				end
			end
			
			for i,line in ipairs(lines) do
			
				local tx=font.width(line)
				
				if widget.text_align=="left" or widget.text_align=="wrap" then
					tx=0
				elseif widget.text_align=="right" then
					tx=(widget.hx-tx)
				elseif widget.text_align=="centerx" then
					tx=(widget.hx-tx)/2
				else -- center a single line vertically as well
					tx=(widget.hx-tx)/2 
					ty=((widget.hy-fy)/2)+typ
				end
				
				tx=tx+txp
--				ty=ty+typ
				
				if i==1 then -- remember topleft of text position for first line
					widget.text_x=tx
					widget.text_y=ty
				end

				if widget.text_color_shadow then
					gl.Color( pack.argb8_pmf4(widget.text_color_shadow) )
					font.set_xy(tx+1,ty+1)
					font.draw(line)
				end
				
				gl.Color( pack.argb8_pmf4(c) )
				font.set_xy(tx,ty)
--print(wstr.dump(line))
				font.draw(line)
				
				if widget.class=="textedit" then -- hack
					if widget.master.focus==widget then --only draw curser in active widget
						if widget.master.throb>=128 then
							local sw=font.width(widget.text:sub(1,widget.data.str_idx))

							font.set_xy(tx+sw,ty)
							font.draw("_")
						end
					end
				end

				ty=ty+fy
			end
		end

		for i,v in ipairs(widget) do
			if not v.fbo or not v.dirty then -- terminate recursion at dirty fbo
				v:draw()
			end
		end

		if widget.fbo then -- we have drawn into the fbo
			
--			gl.PopMatrix()
			gl.PopMatrix()

--			widget.layout.clean()


			gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
			

			widget.old_layout.restore() --restore old viewport
			gl.MatrixMode(gl.PROJECTION)
			gl.PopMatrix()			
			gl.MatrixMode(gl.MODELVIEW)
			gl.PopMatrix()


			widget.layout=nil
			widget.old_layout=nil
		end
		
else -- we can only draw once

		if widget.fbo then -- we need to draw our cached fbo

		
			gl.Disable(gl.DEPTH_TEST)
			gl.Disable(gl.CULL_FACE)
		
--			gl.Translate(widget.sx,-widget.sy,0)
			gl.Color(1,1,1,1)
			
			widget.fbo:bind_texture()
			flat.tristrip("xyzuv",{
				0,				0,				0,	0,1,
				widget.fbo.w,	0,				0,	1,1,
				0,				widget.fbo.h,	0,	0,0,
				widget.fbo.w,	widget.fbo.h,	0,	1,0,
			})

--print("draw fbo")
		end
		
end

		if widget.clip then
		
			widget.old_layout.restore() --restore old viewport
			
			gl.MatrixMode(gl.PROJECTION)
			gl.PopMatrix()			
			gl.MatrixMode(gl.MODELVIEW)
			gl.PopMatrix()


			widget.layout=nil
			widget.old_layout=nil

		end
		
		gl.PopMatrix() -- expect the base to be pushed
	
		
		return widget
	end



end

return wskin
end

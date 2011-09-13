
local bit=require('bit')
local gl=require('gl')
local grd=require('grd')

local math=math
local table=table

local ipairs=ipairs

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


local print=print

module("fenestra.widget.skin")

local mode=nil
local texs={}

local margin=0 -- whitespace
local border=0 -- solidspace
--
-- unload a skin, go back to the "builtin" default
--
function unload(win)

	mode=nil
	texs={}
	
end


--
-- load a skin
--
function load(win,name)

	unload(win)

	if name then -- load a named skin
	
		if name=="test" then
			mode=name

			texs.buttof=win.tex( grd.create("GRD_FMT_U8_BGRA",apps.dir.."data/skins/test/button_high.png") )
			texs.button=win.tex( grd.create("GRD_FMT_U8_BGRA",apps.dir.."data/skins/test/button_high.png") )
			texs.buttin=win.tex( grd.create("GRD_FMT_U8_BGRA",apps.dir.."data/skins/test/button_high_in.png") )
		
			margin=15
			border=0
		end
			
	end

end


function draw33(tw,th, mw,mh, vxs,vys, vw,vh)
		
--		local vw,vh=512,52
--		local mw,mh=24,24

		if mw*2 > vw then mw=vw/2 end
		if mh*2 > vh then mh=vh/2 end

		
		local tww={mw/tw,(tw-2*mw)/tw,mw/tw}
		local thh={mh/th,(th-2*mh)/th,mh/th}
		local vww={mw,vw-2*mw,mw}
		local vhh={mh,vh-2*mh,mh}
		
		gl.Disable(gl.DEPTH_TEST)
		gl.Disable(gl.LIGHTING)
		gl.Disable(gl.CULL_FACE)
		gl.Enable(gl.TEXTURE_2D)


		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
		gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)


		gl.Begin(gl.QUADS)
--			gl.Color({1,1,1,1})
			
			local function drawbox( tx,ty, vx,vy , txp,typ, vxp,vyp )
				gl.TexCoord(tx    ,ty)     gl.Vertex(vx,    vy)
				gl.TexCoord(tx+txp,ty)     gl.Vertex(vx+vxp,vy)
				gl.TexCoord(tx+txp,ty+typ) gl.Vertex(vx+vxp,vy+vyp)
				gl.TexCoord(tx    ,ty+typ) gl.Vertex(vx,    vy+vyp)
			end
			
		local tx,ty=0,0
		local vx,vy=vxs,vys-- -vw/2,vh/2

			for iy=1,3 do
				tx=0
				vx=vxs-- -vw/2
				for ix=1,3 do

					drawbox( tx,ty, vx,vy , tww[ix],thh[iy], vww[ix],-vhh[iy] )

					tx=tx+tww[ix]
					vx=vx+vww[ix]
				end
				ty=ty+thh[iy]
				vy=vy-vhh[iy]
			end
			
		gl.End()


end

--
-- add meta functions
--
function setup(def)

--	load(def.win,"test")


	local master=def.master
	local meta=def.meta
	local win=def.win
	local font=def.font
	
--
-- display this widget and its sub widgets
--
	function meta.draw(widget)
		
		-- draw rectangle


		gl.PopMatrix() -- expect the base to be pushed
		gl.PushMatrix()
		gl.Translate(widget.px,widget.py,0)
		gl.Rotate(widget.pa,0,0,1)
		
		gl.Disable(gl.LIGHTING)
		gl.Disable(gl.DEPTH_TEST)
		gl.Disable(gl.CULL_FACE)
		gl.Disable(gl.TEXTURE_2D)
		
		local txp,typ=0,0
		
		if widget.color then
		
			if widget.highlight=="shrink" then
			
				gl.Color( explode_color(widget.color))
					
				if master.over==widget then
					gl.Translate(widget.hx/16,-widget.hy/16,0)
					gl.Scale(7/8,7/8,1)
				end
			
			elseif widget.highlight=="none" then
			
				gl.Color( explode_color(widget.color))
				
			elseif widget.highlight=="text" then
			
				gl.Color( explode_color(widget.color))
				
			else -- alpha default
			
				if master.over==widget then
					gl.Color( explode_color(widget.color))
				else
					local c={explode_color(widget.color)}
					c[4]=c[4]*0.85
					gl.Color( c )
				end
				
			end
			
			local hx=widget.hx
			local hy=widget.hy
			local bb=2
			local tl={1,1,1,0.25}
			local br={0,0,0,0.25}
			
			if ( master.active==widget and master.over==widget ) or widget.state=="selected" then
				tl,br=br,tl
				txp=1
				typ=1
			end
			
			if mode then
			
			if ( master.active==widget and master.over==widget ) or widget.state=="selected" then
				texs.buttin:bind()
				txp=0
				typ=-1
			else
				texs.button:bind()
				txp=0
				typ=-2
			end
			
			draw33(128,128, 24,24, 0-margin,0+margin, hx+(margin*2),hy+(margin*2))
			
			else -- builtin
			
				gl.Begin(gl.QUADS)
					gl.Vertex(  0,   0)
					gl.Vertex( hx,   0)
					gl.Vertex( hx, -hy)
					gl.Vertex(  0, -hy)
				gl.End()
				gl.Color( tl )
				gl.Begin(gl.QUADS)
					gl.Vertex(  0,   0  )
					gl.Vertex( hx,   0  )
					gl.Vertex( hx-bb, -bb)
					gl.Vertex(  0+bb, -bb)
					
					gl.Vertex(  0,    0   )
					gl.Vertex(  0+bb, -bb )
					gl.Vertex(  0+bb, -hy+bb)
					gl.Vertex(  0,    -hy  )
				gl.End()
				gl.Color( br )
				gl.Begin(gl.QUADS)
					gl.Vertex( hx,   -hy  )
					gl.Vertex(  0,   -hy  )
					gl.Vertex(  0+bb, -hy+bb)
					gl.Vertex( hx-bb, -hy+bb)
					
					gl.Vertex(  hx,    0   )
					gl.Vertex(  hx,    -hy  )
					gl.Vertex(  hx-bb, -hy+bb)
					gl.Vertex(  hx-bb, -bb )
				gl.End()
			end
			
		end
		
		if widget.text then
		
			local tx,ty=font.size(widget.text,widget.text_size)
			
			local c=widget.text_color
			
			if widget.highlight=="text" then
				local t={explode_color(c)}
				if master.over==widget then
				elseif ( master.active==widget and master.over==widget ) or widget.state=="selected" then
					t[4]=t[4]*3/4
				else
					t[4]=t[4]*3/4
				end
				c=implode_color(t)
			end
			
			tx=(widget.hx-tx)/2
			ty=(widget.hy-ty)/2
			
			tx=tx+txp
			ty=ty+typ
			
			font.set(tx,-ty,c,widget.text_size)
			font.draw(widget.text)
		
		end
		
		for i,v in ipairs(widget) do v:draw() end
		
		
		return widget
	end



end

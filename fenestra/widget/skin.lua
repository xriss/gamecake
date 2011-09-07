
local bit=require('bit')
local gl=require('gl')

local math=math
local table=table

local ipairs=ipairs


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



module("fenestra.widget.skin")

--
-- add meta functions
--
function setup(def)

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
		
		gl.Disable('LIGHTING')
		gl.Disable('DEPTH_TEST')
		gl.Disable('CULL_FACE')
		gl.Disable('TEXTURE_2D')
		
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
					c[4]=c[4]/2
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

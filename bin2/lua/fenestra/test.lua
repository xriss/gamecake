

local bit=require('bit')
local gl=require('gl')

win=require('fenestra.wrap').win()



win.setup(_G) -- create and associate with this global table, eg _G.print gets replaced

win.width=win.get("width")
win.height=win.get("height")


local moddat,t=win.data.load("data/objects/xox/aball.xox")
local xox=win.xox(moddat)


local xsx_dat,t=win.data.load("data/avatar/xsx/cycle_walk.xsx")
local xsx=win.xsx(xsx_dat)
local soul=win.avatar.load_soul("local/avatar/soul/default.cthulhu.xml")


local items={}

world=require("box2d.wrap").world({gravity={0,-10}})

local ground = world.body{}
ground.shape{box={width=64,height=10},density=1,friction=0.3}
ground.set{x=0,y=-24-10,a=0}


function new_item(x,y,a)

	local it={}

	it.text="hello"
	it.ts=1.6
	it.tc=0xff00ff00
	
	it.tx,it.ty=win.font_debug.size(it.text,it.ts)
	
	it.tx=-(it.tx/2)
	it.ty= (it.ty/2)

	it.body=world.body{}
	it.body.shape{box={width=-it.tx,height=it.ty,center={0,0}},density=1,friction=0.3,restitution=0.25}
	it.body.set{mass="shapes",x=x,y=y,a=a} -- calculate from shapes
	
	return it
end

table.insert(items, new_item(0,10,0) )


function win.mouse(act,x,y,key)


	local hx=win.width/2
	local hy=win.height/2

	local tx,ty
	
	if win.height/(win.width or 1) > (3/4) then -- deal with new viewport sizeing
	
		tx=(4/3)*(x-hx)/hx
		ty=(4/3)*(hy-y)/hx
		
	else
	
		tx=(x-hx)/hy
		ty=(hy-y)/hy

	end
	
		
	win.widget.mouse(win.widget,act,320+tx*240,240+ty*240,key)

	if act=="down" then
	

		table.insert(items, new_item(tx*24,ty*24,0) )
	
	end

end
	
--print(xsx)
--	xsx.set(t)

	win.avatar.map_xsx(xsx,soul)


function modl(name)

	moddat,t=win.data.load("data/avatar/xox/"..name..".xox")
	
	if moddat then
	
		xox.clean()
		xox=win.xox(moddat)
		
	end
end

local rx,ry,rz=0,0,0
local frame=0

local last=win.time()
local frame_last=last
local frame_count=0
local fps=0

local times={}

local function times_setup()
	local t={}
	t.time=0
	t.time_live=0
	
	t.hash=0
	t.hash_live=0
	
	t.started=0
	
	function t.start()
		t.started=win.time()
	end
	
	function t.stop()
		local ended=win.time()
		
		t.time_live=t.time_live + ended-t.started
		t.hash_live=t.hash_live + 1
	end
	
	function t.done()
		t.time=t.time_live
		t.hash=t.hash_live
		t.time_live=0
		t.hash_live=0
		
	end
	
	return t
end



times.update=times_setup()
times.draw=times_setup()
times.swap=times_setup()

win.update=function()

	win.width=win.get("width")
	win.height=win.get("height")
	
	
	local t=win.time()
	local d=t-last
	local d_orig=d

-- count frames	
	if t-frame_last >= 1 then
	
		fps=frame_count
		frame_count=0
		frame_last=t
	
		times.update.done()
		times.draw.done()
		times.swap.done()
	end
	
-- update

	local do_draw=false
	while d >= 0.020 do
	
		times.update.start()
		
		world.step(1/50,2)
		
		rx=(rx+1)%360
		ry=(ry+1)%360
		rz=(rz+1)%360
		
		frame=(frame+0.020)
		if frame>xsx.length then frame=frame-xsx.length end 
		
		win.widget:update()
		
		win.console.update()
		
		times.update.stop()
		
		if d>1 then -- reset when very out of sync
			last=t
			d=0
		else
			last=last+0.020
			d=d-0.020
		end
		
		do_draw=true
	end

-- draw


	if do_draw then

		times.draw.start()
		win.begin()
		
		gl.MatrixMode("MODELVIEW")
		gl.LoadIdentity()
		

		

--		for x=-10,10,10 do
--			for y=-10,10,10 do

		local x,y=0,0
			
				gl.PushMatrix()
				
				gl.Translate(x/2,y/2, 0)
				
		gl.Translate(0,0, -32)
		gl.Scale(8,8,8)
		gl.Rotate(rz,0,0,1);
		gl.Rotate(ry,0,1,0);
		gl.Rotate(rx,1,0,0);
		
--				win.draw_cube(0.5)
--				xox.draw()
				xsx.draw(frame);
				
				gl.PopMatrix()
				
--			end
--		end


		for i,v in ipairs(items) do
		
			
			v.body.get()
			gl.PushMatrix()
			
			gl.Translate(v.body.x,v.body.y, -24)
			gl.Rotate(v.body.a,0,0,1);
--[[
			gl.Rotate(90,0,1,0);
			xsx.draw(frame);
			gl.Rotate(-90,0,1,0);
]]			
			win.font_debug.set(v.tx,v.ty,v.tc,v.ts)
			win.font_debug.draw(v.text)

		
			gl.PopMatrix()
		end
		
		
		gl.PushMatrix()
		gl.Translate(-320,-240, -240*1.0)
		win.widget:draw()
		gl.PopMatrix()
		
		win.console.draw()
		
--		win.debug_rect(0,0,640,480,0x44000000)
	
		win.swap()
		times.draw.stop()
		
		frame_count=frame_count+1
		
		local gci=gcinfo()
		win.console.display(string.format("fps=%02.0f t=%03.0f u=%03.0f d=%03.0f gc=%0.0fk",fps,math.floor(0.5+(10000/fps)),math.floor(0.5+times.update.time*10000),math.floor(0.5+times.draw.time*10000/times.draw.hash),math.floor(gci) ))
		
	end

end



while win.msg("wait") do

	win.update()

end



win.clean()


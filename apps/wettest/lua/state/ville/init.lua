

local webcache=require("webcache")

local comm=require("spew.client.comm")

local wetstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")

local sxml=require("simpxml")



local win=win
local gl=require("gl")

local str_split=wetstr.str_split

local string=string
local math=math
local table=table
local pairs=pairs
local ipairs=ipairs
local coroutine=coroutine
local error=error
local tonumber=tonumber
local print=print
local require=require

local _M=module(...)
local vobj=require("state.ville.vobj")
local chat=require("state.ville.chat")
local vloc   =require("state.ville.vobj.loc")


concall={} -- console helper functions

function concall.say(...)
	local s=table.concat({...}," ") or ""
	comm.send({cmd="say",txt=s})
end

function concall.cmd(...)
	local s=table.concat({...}," ") or ""
	comm.send({cmd="cmd",txt=s})
end

function concall.vrot(n)
	n=tonumber(n)
	_M.vrot=n
	return vrot
end

function setup()

	print("setup")
	
	show_chat=true

	my_vid=nil
	my_vobj=nil
	
	mtx_proj=tardis.m4.new()
	mtx_view=tardis.m4.new()
	
	mtx_3d_to_2d=tardis.m4.new() -- 3d space to screen space
	mtx_2d_to_3d=tardis.m4.new() -- screen space to 3d space
	
	mouse_x=0
	mouse_y=0

	concall.vrot(15)

	comm.setup()
	
	comm.hooks.vmsg=vmsg -- we want to handle all ville msgs
	comm.hooks.cmsg=cmsg -- and chat msgs
	
	comm.hooks.sent=sent -- and get to look at all sent msgs
	
	clean_vobjs()
	
-- load a pointer object
	pointer_xox=win.xox( win.data.load("data/objects/xox/aball.xox") )
	
	
-- we need a coroutine so we can handle comm callbacks that can yield
	update_co=coroutine.create(update_co_func)
	
	for n,v in pairs(concall) do
		win.console.call[n]=v
	end
	
	comm.send{cmd="note",note="playing",arg1="WetVille"}
--	comm.send{cmd="login",name="test"..math.random(10000,99999),pass=nil}
	comm.send{cmd="ville",vcmd="setup"}
--	comm.send{cmd="cmd",txt="/join limbo"}
--	comm.send{cmd="cmd",txt="/join public"}

	chat.setup(_M)

end

function keypress(ascii,key,act)


	if ascii=="	" and act=="down" then -- tab toggles chat display
	
		show_chat=not show_chat
		
	else
	
		if show_chat then
			chat.keypress(ascii,key,act)
		end
		
	end
	

	
end

function mouse(act,x,y,key)

	if show_chat then
		chat.mouse(act,x,y,key)
	end
		
	mouse_x=x
	mouse_y=y
	if act=="down" then
		mouse_change=act
	end
	if act=="up" then
		mouse_change=act
	end

end


function clean_vobjs()

	if vobjs then
		for i,v in pairs(vobjs) do
			v:clean()
			vobjs[i]=nil
		end
	end
	vobjs={}  -- items to render
	vtypes={} -- items lookup by type
	vnames={} -- items lookup by name
	vtards={} -- tards lookup by owner (user names)
end
	
function clean()

	chat.clean()

	clean_vobjs()
	
	comm.clean()

end


function update_co_func()

	while true do
	
		comm.update()
		coroutine.yield()
		
	end
	
end

function update()

	if show_chat then
		chat.update()
	end
	
	local ret,err=coroutine.resume(update_co)
	if err then
		error(err)
	end

	for i,v in pairs(vobjs) do
		v:update()
	end
	
-- dumb under mouse test
	mouse_over=nil
	local mx= mouse_x
	local my=win.height-mouse_y
	for i,v in pairs(vobjs) do
		local a=v.area
		if a then
			if a.min[1]<mx and a.max[1]>mx then
				if a.min[2]<my and a.max[2]>my then
					mouse_over=v
				end
			end
		end
	end
	
end


function draw()

	win.begin()
	gl.ClearColor(0,0,0.25,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
	
	
	
	if show_chat then
		win.clip2d(0,0,2/3,win.height)
	else
		win.clip2d(0,0,0,0)
	end
	win.project23d(480/640,8,32768)
	
	gl.MatrixMode("MODELVIEW")
	gl.LoadIdentity()

-- dumb camera position	
	gl.Translate(-400,-200,-2400)
	gl.Rotate(vrot,1,0,0)

-- save current mtx for reverse mouse pointer projections
	mtx_proj:set(gl.Get("PROJECTION_MATRIX"))
	mtx_view:set(gl.Get("MODELVIEW_MATRIX"))
	mtx_view:product(mtx_proj,mtx_3d_to_2d) 	-- combine
	mtx_3d_to_2d:inverse(mtx_2d_to_3d)         	-- inverse


--[[
	if not doonce then
		doonce=true
		print(mtx_proj)
		print(mtx_proj:determinant())
		print(mtx_proj:inverse())
		print()
		print(mtx_view)
		print(mtx_view:determinant())
		print(mtx_view:inverse())
		exit()
	end
]]
	gl.PushMatrix()
		
	for i,v in pairs(vobjs) do
		v:draw()
	end
	
	draw_mouse()
	
	gl.PopMatrix()



	if false then
		win.debug_begin()
		local w=win.width/2
		local h=win.height/2
		for i,v in pairs(vobjs) do
			if v.area then
				win.debug_polygon_begin()
				win.debug_polygon_vertex(v.area.min[1],v.area.min[2],0x8800ff00)
				win.debug_polygon_vertex(v.area.max[1],v.area.min[2],0x8800ff00)
				win.debug_polygon_vertex(v.area.max[1],v.area.max[2],0x88008800)
				win.debug_polygon_vertex(v.area.min[1],v.area.max[2],0x88008800)
				win.debug_polygon_end()
			end
		end
	end
	
	if show_chat then

	gl.Disable('LIGHTING')
	gl.Disable('DEPTH_TEST')
	gl.Disable('CULL_FACE')
	
		win.clip2d(2/3,0,1/3,win.height)
		win.project23d(480/320,1,32768)
		gl.ClearColor(0,0,0,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
		gl.MatrixMode("MODELVIEW")
		gl.LoadIdentity()
		gl.Translate(0,0,-240)
		
		gl.PushMatrix()
		chat.draw()
		gl.PopMatrix()
	end

--[[window test

	win.clip2d(0,7/8,1/8,1/8)
	win.project23d(1/1,8,32768)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
	gl.MatrixMode("MODELVIEW")
	gl.LoadIdentity()

-- dumb camera position	
	gl.Translate(-200,0,-2400)
	gl.Rotate(15,1,0,0)
	
	gl.PushMatrix()		
	for i,v in pairs(vobjs) do
		v:draw()
	end
	gl.PopMatrix()
]]

end

function draw_mouse()

	if mouse_over then --object highlight
	
		if mouse_change=="up" then
			mouse_change=nil
			if mouse_over.use then mouse_over:use() end
		end
		
		local it=mouse_over
		local sp={}
		sp.alpha=255
		sp.lines={}
		sp.lines[1]="hello world"
		if it.type then
			sp.lines[1]=it.type
			if it.type=="tard" then
				sp.lines[1]=it.owner
			elseif it.type=="door" then
--print(it.props)
				sp.lines[1]=it.props.dest
			end
		end
		
		gl.PushMatrix()

		if it.loc then
			local p=it.loc.xyz
			gl.Translate(p[1]-(12*16),p[2]+80+(#sp.lines*16),p[3]+40)
		elseif it.props and it.props.xyz then
			local p=it.props.xyz
			gl.Translate(p[1]-(12*16),p[2]+80+(#sp.lines*16),p[3]+40)
		end
			
		for i,v in ipairs(sp.lines) do
			win.font_debug.set((24-#v)*8,(i-1)*-16,0xffffff+(sp.alpha*0x1000000),16,16)
			win.font_debug.draw(v)
		end
		
		gl.PopMatrix()


		return
	end
	gl.PushMatrix()

-- the floor	
	local plane=tardis.plane.new()
	plane[2][2]=1
	
-- this will be view along the mouse pointer
	local line=tardis.line.new()
	
-- first we need an xy screen position, in 2d screen space
	local p1=tardis.v3.new((mouse_x/(win.width/2))-1,(mouse_y/(win.height/2))-1)
-- then we need two points in 3d space
	p1[3]=0
	mtx_2d_to_3d:product(p1,line[1])
	p1[3]=1
	mtx_2d_to_3d:product(p1,line[2])
-- these two points make a line
	line[2]:sub(line[1]):normalize()

-- now we find where they intersect	with the floor plane
	local p2=tardis.line_intersect_plane(line,plane)

-- and clip to its extents	
	local r=vobjs[ vtypes.room ]
	if r then
		local n=r.xyz.min
		local x=r.xyz.max
		if p2[1] > x[1] then p2[1]=x[1] end
		if p2[3] > x[3] then p2[3]=x[3] end
		if p2[1] < n[1] then p2[1]=n[1] end
		if p2[3] < n[3] then p2[3]=n[3] end
	end
-- finally we make sure they are integers
	p2[1]=math.floor(p2[1])
	p2[2]=math.floor(p2[2])
	p2[3]=math.floor(p2[3])
	
-- mouse button released
	if mouse_change=="up" then
		mouse_change=nil
		if my_vobj then -- walk to where we clicked
			my_vobj:send("xyz",p2[1]..":"..p2[2]..":"..(-p2[3]))
		end
	end
	
	gl.Translate(p2[1],p2[2],p2[3])
	
--	vobj.draw_box(it,{{-10,-10,-10},{10,10,10}})
	gl.Scale(50,50,50)	
	pointer_xox.draw()
--	if my_vobj and my_vobj.xsx then my_vobj.xsx.draw(0) end
	
	gl.PopMatrix()
end


local split_prop_names={
	xyz=true,
	rot=true,
	balloon=true,
}

function string_to_props(sprops)

	aprops=str_split(",",sprops)
	
	props={}
	for i=1,#aprops,2 do
		local n=aprops[i]
		local v=aprops[i+1]
		
		if split_prop_names[n] then
			v=str_split(":",v)
			if n=="xyz" then v[3]=-tonumber(v[3]) end -- flip z
		end
		props[n]=v
	end

	return props
end

function sent(msg)

--print(msg.cmd)

	if msg.cmd=="cmd" then
		if msg.txt:sub(1,6)=="/login" then -- we are trying to login
			comm.send{cmd="ville",vcmd="setup"}
		end
	
	end
	
end


function cmsg(msg)

	chat.cmsg(msg)
	
	if msg.cmd=="say" then
	
		if not vtards then return end
		
		local vid=vtards[ string.lower(msg.frm or "") ]
		
		if not vid then return end
		
		local vobj=vobjs[vid]
		
		if not vobj then return end
		
		vobj:say(msg.txt)
	end

end

function vmsg(msg)

--print(msg.cmd,msg.vcmd)

local props

	if     msg.vcmd=="vid" then 		-- who we are
	
		my_vid=msg.vobj -- just remember

	elseif msg.vcmd=="vupd" then 		-- update
	
--print(msg.cmd,msg.vcmd,msg.vprops)

		vmsg_vupd{
			vobj	=msg.vobj,
			props	=string_to_props(msg.vprops),
		}
	
	elseif msg.vcmd=="vobj" then 		-- create
	
		vmsg_vobj{
			vobj	=msg.vobj,
			vparent	=msg.vparent,
			vowner	=msg.vowner,
			vtype	=msg.vtype,
			vurl	=msg.vurl,
			props	=string_to_props(msg.vprops),
		}
						
	elseif     msg.vcmd=="vdel" then 	-- delete
	
		vmsg_vdel{
			vobj	=msg.vobj,
		}
	
	end

end


--
-- create a new vobj from tab
--
function vmsg_vobj(tab)
	
	if tab.vtype=="room" then
print("removing all vobjs")
		clean_vobjs() -- remove all vobjs
	end
	
	-- create a new vobj and add it to the list
	local v=vobj.setup(tab)
	vobjs[ tab.vobj ]=v
	vtypes[ v.type ]=tab.vobj -- soft link
	
	if v.type=="tard" then
		vtards[ v.owner ]=tab.vobj -- soft link
	end
	
	if my_vid and my_vid==tab.vobj then -- this is us
		my_vobj=v
	end
		
--dbg	
--	if v.xsx then xsx=v.xsx end
--	print(t)
end

--
-- delete an old vobj
--
function vmsg_vdel(tab)

end

--
-- update a new vobj with tab
--
function vmsg_vupd(tab)

	local v=vobjs[tab.vobj]
	if v then v:change( tab ) end

end


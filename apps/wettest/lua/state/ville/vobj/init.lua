


local webcache=require("webcache")
local comm=require("spew.client.comm")
local wetstr=require("wetgenes.string")
local sxml=require("simpxml")
local gl=require("gl")

local win=win

local table=table
local ipairs=ipairs
local pairs=pairs
local coroutine=coroutine
local error=error
local require=require
local print=print

module(...)

local room	=require("state.ville.vobj.room")
local tard	=require("state.ville.vobj.tard")
local door	=require("state.ville.vobj.door")
local varea	=require("state.ville.vobj.area")



function setup(tab)
local it={}

	it.type=tab.vtype
	it.id=tab.vobj
	it.parent=tab.parent
	it.owner=tab.vowner
	it.props={}

--	print(tab.vtype,tab.vurl)
	
	local t=webcache.get_url(tab.vurl)
	if t and t.body then -- got info
		tab.data=sxml.parse(t.body)
	end
	
-- default null functions
	it.update=update
	it.draw=draw
	it.clean=clean
	it.change=change
	it.send=send
	
	for i,v in pairs(tab.props) do
		it.props[i]=v
	end
--print(tab.vtype)
	if tab.vtype=="room" then
		room.setup(it,tab)
	elseif tab.vtype=="user" or  tab.vtype=="bot" then -- a bot or user is a tard
		tard.setup(it,tab)
	elseif tab.vtype=="door" then
		door.setup(it,tab)
	else
print("loading vobj "..tab.vurl)
	end

	varea.setup(it,tab)

	return it
end

function clean(it)
end

function change(it,tab)

	for i,v in pairs(tab.props) do
		it.props[i]=v
		if i=="try" then it.plan="try" end
	end

	return it
end

function update(it)
	varea.update(it,it.props.xyz)
end

function draw(it)

	gl.PushMatrix()
	
	local p=it.props.xyz
	if p then gl.Translate(p[1],p[2],p[3]) end
	
	win.draw_cube(10)
	
	gl.PopMatrix()

end


-- send a mesage to the server to update property nam to dat
function send(it,nam,dat)

	local msg={cmd="ville",vcmd="vupd",vobj=it.id}

	msg.vprops=nam..","..dat

	comm.send(msg)
end





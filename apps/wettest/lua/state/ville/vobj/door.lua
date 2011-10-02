
local webcache=require("webcache")
local comm=require("spew.client.comm")
local wetstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")
local sxml=require("simpxml")

local win=win

local table=table
local ipairs=ipairs
local coroutine=coroutine
local error=error
local require=require
local print=print

local gl=require("gl")

local wldir=apps.dir or ""


module(...)
local vobj   =require("state.ville.vobj")
local varea  =require("state.ville.vobj.area")



function setup(it,tab)
	
	it.type="door"
	it.update=update
	it.draw=draw
	it.clean=clean
	it.change=change
	it.use=use

	
	return it
end


function clean(it)
end

function change(it,tab)
	vobj.change(it,tab)
end

function update(it)

	varea.update(it,it.props.xyz)
	local a=it.area
	local t=tardis.v3.new() -- a temp vector
	a.min:set(a.p):add(a.y:scale(-10,t)):add(a.x:scale(-40,t))
	a.max:set(a.p):add(a.y:scale(100,t)):add(a.x:scale( 40,t))
	if a.min[1] > a.max[1] then a.min[1] , a.max[1] = a.max[1] , a.min[1] end
	if a.min[2] > a.max[2] then a.min[2] , a.max[2] = a.max[2] , a.min[2] end
	
end

function draw(it)
	vobj.draw(it,tab)
end

function use(it,v)

	it:send("use",v or "use")

end


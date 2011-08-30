
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
local vloc   =require("state.ville.vobj.loc")
local vspeach=require("state.ville.vobj.speach")



function setup(it,tab)

	
print("loading tard "..tab.vurl)
--http://data.wetgenes.com/souls/

	it.type="tard"
	it.update=update
	it.draw=draw
	it.clean=clean
	it.change=change

	do
		local t=wetstr.str_split("/",tab.vurl)
		local u=t[#t]
		u="http://data.wetgenes.com/souls/"..u
		local d=webcache.get_url(u)
		if d.headers~=200 then d=nil else d=d.body end
		if not d then u=wldir.."local/avatar/soul/me.xml" end
		it.xsx=win.xsx( win.data.load("data/avatar/xsx/cycle_walk.xsx") )
		it.soul=win.avatar.load_soul(u,d)
		win.avatar.map_xsx(it.xsx,it.soul)
		it.frame=0
	end
	
	vloc.setup(it,tab)
	vspeach.setup(it,tab)
	
	return it
end


function clean(it)
end

function change(it,tab)
	vloc.change(it,tab)
	vobj.change(it,tab)
	vspeach.change(it,tab)
end

function update(it)

	it.frame=it.frame+0.02
	if it.frame>it.xsx.length then it.frame=it.frame-it.xsx.length end 
	
	vloc.update(it)
	vspeach.update(it)
	varea.update(it,it.loc.xyz)
	local a=it.area
	local t=tardis.v3.new() -- a temp vector
	a.min:set(a.p):add(a.y:scale(  0,t)):add(a.x:scale(-30,t))
	a.max:set(a.p):add(a.y:scale( 90,t)):add(a.x:scale( 30,t))
	if a.min[1] > a.max[1] then a.min[1] , a.max[1] = a.max[1] , a.min[1] end
	if a.min[2] > a.max[2] then a.min[2] , a.max[2] = a.max[2] , a.min[2] end
	
end

function draw(it)

	gl.PushMatrix()
	
	
	local yrot=vloc.set_translate(it)
--	do local p=it.loc.xyz gl.Translate(p[1],p[2],p[3]) end

	gl.Scale(50,50,50)
	gl.Rotate(yrot,0,1,0)
	
	if it.xsx then it.xsx.draw(it.frame) end
--	vobj.draw(it)

	
	gl.PopMatrix()
	
	vspeach.draw(it)

	
end



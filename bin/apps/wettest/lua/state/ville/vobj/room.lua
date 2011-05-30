
local webcache=require("webcache")
local comm=require("spew.client.comm")
local wetstr=require("wetgenes.string")
local sxml=require("simpxml")


local table=table
local ipairs=ipairs
local coroutine=coroutine
local error=error
local require=require
local print=print
local tonumber=tonumber

local gl=require("gl")

module(...)
local vobj=require("state.ville.vobj")



function setup(it,tab)

	it.type="room"
	
print("loading room "..tab.vurl)

		local v=sxml.child(tab.data,{"ville","room","back"})
		if v and v.src then
			local t=webcache.get_url(v.src)
			if t and t.body then -- got info
				tab.back=sxml.parse(t.body)
			end
		end
	
		if tab.back then
			tab.xy =sxml.child(tab.back,{"ville","back","xy"})
			tab.xyz=sxml.child(tab.back,{"ville","back","xyz"})
		end
		

	it.xyz={}
	it.xyz.min={ tonumber(tab.xyz.x_min) , tonumber(tab.xyz.y_min) , -tonumber(tab.xyz.z_max) }
	it.xyz.max={ tonumber(tab.xyz.x_max) , tonumber(tab.xyz.y_max) , -tonumber(tab.xyz.z_min) }
	it.xyz.org={ tonumber(tab.xyz.x_org) , tonumber(tab.xyz.y_org) , -tonumber(tab.xyz.z_org) }


	it.update=update
	it.draw=draw
	it.clean=clean
	it.change=change

	return it
end


function clean(it)
end

function change(it,tab)
	vobj.change(it,tab)
end

function update(it)
end

function draw(it)


--	gl.BlendFunc('SRC_ALPHA', 'ONE')
	gl.Disable('CULL_FACE')
	gl.Disable('TEXTURE_2D')
	gl.Enable('COLOR_MATERIAL')

	gl.PushMatrix()
	
	gl.Begin('QUADS')
		gl.Color({1,1,1,1})
		gl.Vertex( it.xyz.min[1] , it.xyz.max[2] , it.xyz.min[3] )
		gl.Vertex( it.xyz.max[1] , it.xyz.max[2] , it.xyz.min[3] )
		gl.Vertex( it.xyz.max[1] , it.xyz.max[2] , it.xyz.max[3] )
		gl.Vertex( it.xyz.min[1] , it.xyz.max[2] , it.xyz.max[3] )
	gl.End()

	gl.PopMatrix()
	
end



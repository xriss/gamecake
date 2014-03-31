#!/usr/local/bin/gamecake

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local opts={
	times=true, -- request simple time keeping samples
	
	width=640,	-- display basics
	height=480,
	title="testing",
	fps=60,
}

local demo={} -- a demo state

local bake=function()
	local state=require("wetgenes.gamecake.state").bake(opts)

	do
		local screen=wwin.screen()
		local inf={width=opts.width,height=opts.height,title=opts.title}
		inf.x=(screen.width-inf.width)/2
		inf.y=(screen.height-inf.height)/2
		state.win=wwin.create(inf)
		state.gl=require("gles").gles1
		state.win:context({})

		state.frame_rate=1/opts.fps -- how fast we want to run
		state.frame_time=0

		require("wetgenes.gamecake").bake({
			state=state,
			width=inf.width,
			height=inf.height,
		})
		
	end
	
	state.require_mod("wetgenes.gamecake.mods.escmenu") -- escmenu gives us a doom style escape menu
	state.require_mod("wetgenes.gamecake.mods.console") -- console gives us a quake style tilda console

	state.next=demo -- we want to run a demo state

	return state
end


demo.loads=function(state)

	state.cake.fonts.loads({1}) -- load 1st builtin font, a basic 8x8 font

end
		
demo.setup=function(state)

	demo.loads(state)
	
	
local dprint=function(...) print(wstr.dump(...)) end
local wxml=require("wetgenes.simpxml")
local wzips=require("wetgenes.zips")

--local s=wzips.readfile("multimtl.dae")
--local s=wzips.readfile("hand.dae")
--local s=wzips.readfile("test2.dae")
--local s=wzips.readfile("dice.dae")
local s=wzips.readfile("cog.dae")

print("loaded ",#s,"bytes")

local x=wxml.parse(s)

--print("loaded ",wxml.unparse(x))

local ids={}
local function do_ids(t)
	for i=1,#t do local v=t[i]
		if type(v)=="table" then
			if v.id then ids[v.id]=v end
			do_ids(v)
		end
	end
end
do_ids(x)

for id,v in pairs(ids) do
print("\""..id.."\"")
end

local function get_dat(id)
	if id:sub(1,1) == "#" then
		id=id:sub(2)
	end
	local d=ids[id]
	if d[0]=="source" or d[0]=="float_array" then
		return d 
	else
		local t=wxml.descendent(d,"input")
		return get_dat(t.source)
	end
end

local function scan_nums(s)
	local a={}
	for w in string.gfind(s, "([^%s]+)") do
		local n=tonumber(w)
		a[#a+1]=n
	end	
	return a
end

local sources={}
local function get_source(id)
	if sources[id] then return sources[id] end
	
	local d=get_dat(id)
	
	local a=wxml.descendent(d,"accessor")
	local it={}
	
	it.stride=tonumber(a.stride)
	it.names={}

	for i,v in ipairs( wxml.descendents(a,"param") ) do
		it.names[#it.names+1]=v.name
	end
	
	it.data=scan_nums( get_dat(a.source)[1] )
	
	sources[id]=it
	return it
end


local geo

local t=wxml.descendent(x,"library_geometries")
for i=1,#t do local v=t[i]
	if v[0]=="geometry" then
		geo={}
		geo.name=v.name
		geo.mesh=wxml.descendent(v,"mesh")
		break
	end
end



print("loading object named \""..geo.name.."\"")

local polys={}
for i,v in ipairs( wxml.descendents(geo.mesh,"polylist")) do -- handle each polylist chunk

	local p={}
	polys[#polys+1]=p
	
	p.inputs={}
	
	p.stride=0
	for i,l in ipairs( wxml.descendents(v,"input") ) do
		local m={}
		p.inputs[#p.inputs+1]=m
		
		m.semantic=l.semantic
		m.offset=tonumber(l.offset)
		m.source=get_source(l.source)
		if m.offset > p.stride then p.stride=m.offset end
	end
	p.stride=p.stride+1 -- this is how we guess this number?
	p.vcount=scan_nums( wxml.descendent(v,"vcount")[1] )
	p.p=scan_nums( wxml.descendent(v,"p")[1] )
	
--	dprint( p )

end

print("found poly list count \""..#polys.."\"")

demo.draw_test=function(state)

local canvas=state.canvas
local cake=state.cake
local gl=cake.gl

	for ips,ps in ipairs(polys) do
	

		local off=1
		for ipc,pc in ipairs(ps.vcount) do
		
			local db={}
			local function push(n)
				db[#db+1]=n
			end

			local pxx
			if pc==3 then
				pxx={1,2,3}
			elseif pc==4 then
				pxx={1,2,4,3}
			end
			
			for _,i in ipairs(pxx) do
				
				for j=1,ps.stride do
					local v=ps.p[ off+(ps.stride*(i-1))+j-1 ]
					
					if ps.inputs[j].semantic == "VERTEX" then
						local s=ps.inputs[j].source
						for n=1,3 do
							push( s.data[ (v*s.stride) +n ] )
						end
					end
					
				end
				
			end
			off=off+ps.stride*pc
		
			gl.Color(0.5,ipc/#ps.vcount,0.5,1) -- draw drop shadow
			canvas.flat.tristrip("xyz",db)
			
		end
	

	end


end

end

demo.clean=function(state)

end

demo.msg=function(state,m)
end

demo.update=function(state)
end

local ii=0
demo.draw=function(state)

	ii=ii+1
	
--print("draw")
	local cake=state.cake
	local canvas=state.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=cake.gl
	
	canvas.viewport() -- did our window change?
	canvas.project23d(opts.width,opts.height,0.25,opts.height*4)
	canvas.gl_default() -- reset gl state
		
	gl.ClearColor(0,0,0,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.MatrixMode(gl.PROJECTION)
	gl.LoadMatrix( canvas.pmtx )

	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()
	gl.Translate(-opts.width/2,-opts.height/2,-opts.height*2) -- top left corner is origin

	gl.PushMatrix()
	

	gl.Color(pack.argb4_pmf4(0x800f))
	flat.quad(0,(opts.height/2)-32,opts.width,(opts.height/2)+32) -- draw a blue box as a background

	font.set(cake.fonts.get(1)) -- default font
	font.set_size(32,0) -- 32 pixels high
	
	local s="Hello World!"
	local sw=font.width(s) -- how wide the string is
	local x,y=(opts.width-(sw))/2,(opts.height-32)/2 -- center the text in  middle of display

	gl.Color(pack.argb4_pmf4(0xf000)) -- draw drop shadow
	font.set_xy(x+4,y+4)
	font.draw(s)
	
	gl.Color(pack.argb4_pmf4(0xffff)) -- draw white text
	font.set_xy(x,y)
	font.draw(s)


	gl.Color(pack.argb4_pmf4(0xffff)) -- draw white text
	canvas.flat.tristrip("xyzrgba",{
		100,100,0, 1,0,0,1,
		200,100,0, 0,1,0,1,
		100,200,0, 0,0,1,1,
		})
	
	gl.Translate(200,200,0)
--	gl.Scale(300,300,300)
	gl.Scale(100,100,100)
	gl.Rotate(ii,1,1,0)
--	gl.Rotate(ii,1,0,0)
	
	gl.Enable(gl.DEPTH_TEST)
--	gl.Enable(gl.CULL_FACE)

	demo.draw_test(state)

	gl.Disable(gl.DEPTH_TEST)
--	gl.Disable(gl.CULL_FACE)
	
	gl.PopMatrix()
	
end



-- this will busy loop or hand back control depending on system we are running on
return bake():serv()




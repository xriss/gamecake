--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- we handle the lua side loading and manipulation of 3d vertex/polygon buffers
-- this can be used for file/data processing but needs you to pass in contexts
-- for basic opengl drawing funcs

local dprint=function(...) print(wstr.dump(...)) end
local wxml=require("wetgenes.simpxml")
local wzips=require("wetgenes.zips")

local wxox={}
wxox.name="wetgenes.xox"
package.loaded[wxox.name]=wxox

--the idx used here is 1 based like all lua tables
local meta_buff={__index={
	get=function(buff,idx,tab)
		if type(idx)=="number" then
			tab=tab or {}
			local base=(idx-1)*buff.stride
			for i=1,buff.stride do
				tab[i]=buff.list[base+i]
			end
			return tab
		else
		end
	end,
	set=function(buff,idx,tab)
		if type(idx)=="number" then
			local base=(idx-1)*buff.stride
			for i=1,buff.stride do
				buff.list[base+i]=tab[i]
			end
			if idx>buff.count then buff.count=idx end -- keep length
			return idx
		else
		end
	end,
	push=function(buff,tab)
		return buff:set(buff.count+1,tab)
	end,
	iter=function(buff)
		local i = 0
		local n = buff.count
		return function ()
			i = i + 1
			if i <= n then return i,buff:get(i) end
		end
	end,
}}

function wxox.create_buff(opts)
	buff=opts and opts.buff or {}
	setmetatable(buff,meta_buff)
	buff.list={}
	buff.stride=opts.stride or 1
	buff.count=opts.count or 0
	return buff
end

function wxox.create_verts(opts)
	verts=opts and opts.verts or {}
	verts.buff=wxox.create_buff({stride=3})
	return verts
end

function wxox.create_tris(opts)
	tris=opts and opts.tris or {}
	tris.buff=wxox.create_buff({stride=4})	
	return tris
end

function wxox.create_edges(opts)
	edges=opts and opts.edges or {}
	edges.buff=wxox.create_buff({stride=3})
	return edges
end

function wxox.create_mats(opts)
	mats=opts and opts.mats or {}

	mats.list={}
	
	return mats
end


function wxox.load_dae(xox,fname)

local s=wzips.readfile(fname)

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

--[[
for id,v in pairs(ids) do
print("\""..id.."\"")
end
]]

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



--print("loading object named \""..geo.name.."\"")

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
				pxx={1,2,4,4,2,3}
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
			
-- the data that came in *could* be all over the place
-- so we duplicate points with a plan to auto merge them back together later

			for i=1,#db,9 do -- 3 verts per tri , 3 numbers per vert
				local v1=xox.verts.buff:push({db[i+0],db[i+1],db[i+2]})
				local v2=xox.verts.buff:push({db[i+3],db[i+4],db[i+5]})
				local v3=xox.verts.buff:push({db[i+6],db[i+7],db[i+8]})
				xox.tris.buff:push({v1,v2,v3,1})
			end
			
--			gl.Color(0.5,ipc/#ps.vcount,0.5,1) -- draw drop shadow
--			canvas.flat.tristrip("xyz",db)
			
		end
		
	end
	
	return xox
end

-- draw using a gamecake canvas
function wxox.draw_canvas(xox,canvas)

	for i,v in xox.tris.buff:iter() do
	
		local p1=xox.verts.buff:get(v[1])
		local p2=xox.verts.buff:get(v[2])
		local p3=xox.verts.buff:get(v[3])

		canvas.flat.tristrip("xyz",{
			p1[1],p1[2],p1[3],
			p2[1],p2[2],p2[3],
			p3[1],p3[2],p3[3],
		})
		
	end

	return xox
end

local meta_xox={__index={
	load_dae=wxox.load_dae,
	draw_canvas=wxox.draw_canvas,
}}


-- create verts / tris / mats in a single object
function wxox.create_xox(opts)
	local xox=opts and opts.xox or {}

	setmetatable(xox,meta_xox)
	
	xox.verts=wxox.create_verts()
	xox.tris =wxox.create_tris()
	xox.mats =wxox.create_mats()

--	xox.edges=wxox.create_edges()

	
	return xox
end

return wxox

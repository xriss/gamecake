--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- we handle the lua side loading and manipulation of 3d vertex/polygon buffers
-- this can be used for file/data processing but needs you to pass in contexts
-- for basic opengl drawing funcs

local wstr=require("wetgenes.string")
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
	verts.buff=wxox.create_buff({stride=6})
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


function wxox.load_dae(xox,opts)

if type(opts)=="string" then
	opts={filename=opts}
end


local s=wzips.readfile(opts.filename)

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
		m.idx=#p.inputs+1		
		m.semantic=l.semantic
		m.offset=tonumber(l.offset)
		m.source=get_source(l.source)
		if m.offset > p.stride then p.stride=m.offset end

		p.inputs[m.idx]=m
		p.inputs[m.semantic]=m -- easy lookup by name
	end
	p.stride=p.stride+1 -- this is how we guess this number, it will be one more than the inputs?
	p.vcount=scan_nums( wxml.descendent(v,"vcount")[1] )
	p.p=scan_nums( wxml.descendent(v,"p")[1] )
	
--	dprint( p )

end

print("found poly list count \""..#polys.."\"")


	for ips,ps in ipairs(polys) do
	

print("found poly count \""..#ps.vcount.."\"")
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
			
			for _,i in ipairs(pxx) do -- each vertex 1tri or 2tri (quad)
				
				local inp=ps.inputs["VERTEX"]
				local v=ps.p[ off+(ps.stride*(i-1))+inp.offset ]
				local s=inp.source
				for n=1,3 do -- push xyz
					push( s.data[ (v*s.stride) +n ] or 0 )
				end

				local inp=ps.inputs["NORMAL"]
				local v=ps.p[ off+(ps.stride*(i-1))+inp.offset ]
				local s=inp.source
				for n=1,3 do -- push xyz
					push( s.data[ (v*s.stride) +n ] or 0 )
				end

			end
			off=off+ps.stride*pc
			
-- the data that came in *could* be all over the place
-- so we duplicate points with a plan to auto merge them back together later

	local get_poly_normal=function(p)
	
		local va={}
		local vb={}
		va[1]=p[1]-p[4]
		va[2]=p[2]-p[5]
		va[3]=p[3]-p[6]
		vb[1]=p[7]-p[4]
		vb[2]=p[8]-p[5]
		vb[3]=p[9]-p[6]
		
		-- face normal
		local vn={}
		vn[1]=va[2]*vb[3] - va[3]*vb[2]
		vn[2]=va[3]*vb[1] - va[1]*vb[3]
		vn[3]=va[1]*vb[2] - va[2]*vb[1]
		
		return vn
	end


			for i=1,#db,9*2 do -- 3 verts per tri , 3 numbers per vert
			
-- normal are bollox, calculate them			
			local n=get_poly_normal({db[i+ 0],db[i+ 1],db[i+ 2],db[i+ 6],db[i+ 7],db[i+ 8],db[i+12],db[i+13],db[i+14]})
			
				local v1=xox.verts.buff:push({db[i+ 0],db[i+ 1],db[i+ 2],n[1],n[2],n[3]})
				local v2=xox.verts.buff:push({db[i+ 6],db[i+ 7],db[i+ 8],n[1],n[2],n[3]})
				local v3=xox.verts.buff:push({db[i+12],db[i+13],db[i+14],n[1],n[2],n[3]})
				xox.tris.buff:push({v1,v2,v3,1})
			end
			
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
		canvas.flat.tristrip("xyznrm",{
			p1[1],p1[2],p1[3],p1[4],p1[5],p1[6],
			p2[1],p2[2],p2[3],p2[4],p2[5],p2[6],
			p3[1],p3[2],p3[3],p3[4],p3[5],p3[6],
		},"xyz_normal")
		
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

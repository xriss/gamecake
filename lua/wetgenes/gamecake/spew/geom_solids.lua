--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.fill=function(oven,geom)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat

	geom.tetrahedron=function(it)
		it=geom.new(it)
		
		it.verts={
			{ 0.5, 0.5, 0.5},
			{-0.5, 0.5,-0.5},
			{ 0.5,-0.5,-0.5},
			{-0.5,-0.5, 0.5},
		}
			
		it.polys={
			{1,2,3},
			{2,4,3},
			{1,3,4},
			{1,4,2},
		}
			
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

	geom.octahedron=function(it)
		it=geom.new(it)
		
		local a=1/(2*math.sqrt(2))
		local b=1/2
		
		it.verts={
			{ a, 0, a},
			{ a, 0,-a},
			{-a, 0, a},
			{-a, 0,-a},
			{ 0, b, 0},
			{ 0,-b, 0},
		}
			
		it.polys={
			{3,4,5},
			{4,2,5},
			{2,1,5},
			{1,3,5},
			{2,4,6},
			{4,3,6},
			{1,2,6},
			{3,1,6},
		}
			
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end
	
	geom.hexahedron=function(it)
		it=geom.new(it)
		
		it.verts={
			{ 0.5, 0.5, 0.5},
			{ 0.5, 0.5,-0.5},
			{ 0.5,-0.5, 0.5},
			{ 0.5,-0.5,-0.5},
			{-0.5, 0.5, 0.5},
			{-0.5, 0.5,-0.5},
			{-0.5,-0.5, 0.5},
			{-0.5,-0.5,-0.5},
		}
			
		it.polys={
			{8,4,3,7},
			{8,7,5,6},
			{7,3,1,5},
			{6,5,1,2},
			{4,2,1,3},
			{8,6,2,4},
		}
			
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

	geom.icosahedron=function(it)
		it=geom.new(it)
		
		local a=1/2
		local b=1/(1+math.sqrt(5))
		
		it.verts={
			{ 0, b, a}, -- 1
			{ 0, b,-a}, -- 2
			{ 0,-b, a}, -- 3
			{ 0,-b,-a}, -- 4
			{ a, 0, b}, -- 5
			{ a, 0,-b}, -- 6
			{-a, 0, b}, -- 7
			{-a, 0,-b}, -- 8
			{ b, a, 0}, -- 9
			{ b,-a, 0}, --10
			{-b, a, 0}, --11
			{-b,-a, 0}, --12
		}
			
		it.polys={
			{2,9,11},
			{1,11,9},
			{1,3,7},
			{1,5,3},
			{2,4,6},
			{2,8,4},
			{3,10,12},
			{4,12,10},
			{11,7,8},
			{12,8,7},
			{9,6,5},
			{10,5,6},
			{1,7,11},
			{1,9,5},
			{2,11,8},
			{2,6,9},
			{4,8,12},
			{4,10,6},
			{3,12,7},
			{3,5,10}, 
		}
		
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

	geom.dodecahedron=function(it)
		it=geom.new(it)
		
		local phi=(1+math.sqrt(5))/2
		local a=1/2
		local b=(1/phi)/2
		local c=(2-phi)/2
		
		it.verts={
			{ 0, a, c}, -- 1
			{ 0, a,-c}, -- 2
			{ 0,-a, c}, -- 3
			{ 0,-a,-c}, -- 4
			{ c, 0, a}, -- 5
			{ c, 0,-a}, -- 6
			{-c, 0, a}, -- 7
			{-c, 0,-a}, -- 8
			{ a, c, 0}, -- 9
			{ a,-c, 0}, --10
			{-a, c, 0}, --11
			{-a,-c, 0}, --12
			{ b, b, b}, --13
			{ b, b,-b}, --14
			{ b,-b, b}, --15
			{ b,-b,-b}, --16
			{-b, b, b}, --17
			{-b, b,-b}, --18
			{-b,-b, b}, --19
			{-b,-b,-b}, --20
		}
		
		it.polys={
			{5,7,17,1,13},
			{7,5,15,3,19},
			{6,8,20,4,16},
			{8,6,14,2,18},
			{14,9,13,1,2},
			{17,11,18,2,1},
			{20,12,19,3,4},
			{15,10,16,4,3},
			{9,10,15,5,13},
			{10,9,14,6,16},
			{11,12,20,8,18},
			{12,11,17,7,19},
		}
			
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

-- create a grid, intended to be drawn as lines, give number of divisions
-- with 0 for flat in that dimension
	geom.grid=function(it,gx,gy,gz)
		it=geom.new(it)

		it.verts={}
		it.polys={}
		
		local sx,sy,sz=((gx>0) and (1/gx) or 1),((gy>0) and (1/gy) or 1),((gz>0) and (1/gz) or 1)
		
		local dx,dy,dz=(gz+1)*(gy+1),gz+1,1
		
		for px=0,gx do
			for py=0,gy do
				for pz=0,gz do
					local idx=#it.verts
					it.verts[#it.verts+1]={px*sx,py*sy,pz*sz}

					if pz<gz and py<gy then
						it.polys[#it.polys+1]={idx+1,idx+dy+1,idx+dy+dz+1,idx+dz+1}
					end
					if pz<gz and px<gx then
						it.polys[#it.polys+1]={idx+1,idx+dx+1,idx+dx+dz+1,idx+dz+1}
					end
					if py<gy and px<gx then
						it.polys[#it.polys+1]={idx+1,idx+dx+1,idx+dx+dy+1,idx+dy+1}
					end

				end
			end
		end
		


		return it
	end

-- line drawing only, +-1 in all 3 directions, no polys only lines
	geom.crosshair=function(it)
		it=geom.new(it)
		
		it.verts={}
		it.polys={}
		it.lines={}
		
		it.verts[1]={ 0, 0, 0}
		it.verts[2]={ 1, 0, 0}
		it.verts[3]={-1, 0, 0}
		it.verts[4]={ 0, 1, 0}
		it.verts[5]={ 0,-1, 0}
		it.verts[6]={ 0, 0, 1}
		it.verts[7]={ 0, 0,-1}
		
		it.lines[1]={1,2}
		it.lines[2]={1,3}
		it.lines[3]={1,4}
		it.lines[4]={1,5}
		it.lines[5]={1,6}
		it.lines[6]={1,7}

		return it
	end
	
	geom.box=function(it,dd)
		it=geom.new(it)
		
		local minx=dd[1][1]
		local miny=dd[1][2]
		local minz=dd[1][3]

		local maxx=dd[2][1]
		local maxy=dd[2][2]
		local maxz=dd[2][3]
		
		if minx>maxx then minx,maxx=maxx,minx end
		if miny>maxy then miny,maxy=maxy,miny end
		if minz>maxz then minz,maxz=maxz,minz end
		
		it.verts={
			{ minx, miny, minz},
			{ minx, miny, maxz},
			{ minx, maxy, minz},
			{ minx, maxy, maxz},
			{ maxx, miny, minz},
			{ maxx, miny, maxz},
			{ maxx, maxy, minz},
			{ maxx, maxy, maxz},
		}
		
		it.polys={
			{8,4,3,7},
			{8,7,5,6},
			{7,3,1,5},
			{6,5,1,2},
			{4,2,1,3},
			{8,6,2,4},
		}
		
		return it
	end

			
	return geom
end


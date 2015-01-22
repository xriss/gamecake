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


	geom.mask_clear=function(it)
		it.mask={} -- remove all maske data
	end


	geom.mask_count_verts=function(it)
		return it.mask and it.mask.verts and #it.mask.verts or 0
	end
	geom.mask_count_polys=function(it)
		return it.mask and it.mask.polys and #it.mask.polys or 0
	end
	geom.mask_count_lines=function(it)
		return it.mask and it.mask.lines and #it.mask.lines or 0
	end


	geom.mask_select_all_verts=function(it)
		if not it.mask then it:mask_clear() end
		
		it.mask.verts=it.mask.verts or {}
		local m=it.mask.verts
		
		for i,v in ipairs(it.verts) do
		
			if not m[v] then m[#m+1]=i end -- ordered list
			m[v]=1 -- lookup list of weights

		end
	end

	geom.mask_select_all_polys=function(it)
		if not it.mask then it:mask_clear() end

		it.mask.polys=it.mask.polys or {}
		local m=it.mask.polys
		
		for i,v in ipairs(it.polys) do
		
			if not m[v] then m[#m+1]=i end -- ordered list
			m[v]=1 -- lookup list of weights

		end

	end

	geom.mask_select_all_lines=function(it)
		if not it.mask then it:mask_clear() end

		it.mask.lines=it.mask.verts or {}
		local m=it.mask.lines
		
		for i,v in ipairs(it.lines) do
		
			if not m[v] then m[#m+1]=i end -- ordered list
			m[v]=1 -- lookup list of weights

		end

	end

	geom.mask_save_verts=function(it)
		if not it.mask then it:mask_clear() end
		it.mask.verts=it.mask.verts or {}
		local m=it.mask.verts
		local r={}
		it.mask.verts_save=r
		for i=#m,1,-1 do local v=it.verts[ m[i] ]
			r[#r+1]=v[1]
			r[#r+1]=v[2]
			r[#r+1]=v[3]
		end
	end

	geom.mask_load_verts=function(it)
		if not it.mask then it:mask_clear() end
		it.mask.verts=it.mask.verts or {}
		local m=it.mask.verts
		local idx=0
		local r=it.mask.verts_save
		for i=#m,1,-1 do local v=it.verts[ m[i] ]
			v[1]=r[idx+1]
			v[2]=r[idx+2]
			v[3]=r[idx+3]
			idx=idx+3
		end
	end

	geom.mask_move_verts=function(it,d)
		it.mask.verts=it.mask.verts or {}
		local m=it.mask.verts
		for i=#m,1,-1 do local v=it.verts[ m[i] ]
			v[1]=v[1]+d[1]
			v[2]=v[2]+d[2]
			v[3]=v[3]+d[3]
		end
	end
	
	geom.mask_select_box_verts=function(it,box,mode)
		if not it.mask then it:mask_clear() end
		mode=mode or "auto"
		
		local minx=box[1][1]
		local miny=box[1][2]
		local minz=box[1][3]

		local maxx=box[2][1]
		local maxy=box[2][2]
		local maxz=box[2][3]
		
		if minx>maxx then minx,maxx=maxx,minx end
		if miny>maxy then miny,maxy=maxy,miny end
		if minz>maxz then minz,maxz=maxz,minz end


		it.mask.verts=it.mask.verts or {}
		local m=it.mask.verts
		
		local test
		
		test=function(v)
			if	v[1] >= minx and v[1]<=maxx and
				v[2] >= miny and v[2]<=maxy and			
				v[3] >= minz and v[3]<=maxz then
				return true
			end
			return false
		end

		if minx==maxx then
			test=function(v)
				if	v[2] >= miny and v[2]<=maxy and			
					v[3] >= minz and v[3]<=maxz then
					return true
				end
				return false
			end
		end

		if miny==maxy then
			test=function(v)
				if	v[1] >= minx and v[1]<=maxx and			
					v[3] >= minz and v[3]<=maxz then
					return true
				end
				return false
			end
		end
	
		if minz==maxz then
			test=function(v)
				if	v[1] >= minx and v[1]<=maxx and			
					v[2] >= miny and v[2]<=maxy then
					return true
				end
				return false
			end
		end
		
		
		if mode=="sub" or ( mode=="auto" and (#m > 0) ) then -- remove
		
			for i=#m,1,-1 do local v=it.verts[ m[i] ]
				if test( v ) then
					table.remove(m,i)
					m[v]=nil
				end
			end

		else -- add

			for i,v in ipairs(it.verts) do
			
				if not m[v] and test(v) then
					m[#m+1]=i -- ordered list
					m[v]=1 -- lookup list of weights
				end

			end
		
		end
		
	end


	return geom
end


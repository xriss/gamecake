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

	local getfmt=function(it,m)

		local fmt={"xyz"}
		if it.verts[1] then
			if it.verts[1][4]  then fmt.nrm=true  fmt[#fmt+1]="nrm" end
			if it.verts[1][7]  then fmt.uv=true   fmt[#fmt+1]="uv" end
		end
		if m               then fmt.m=true    fmt[#fmt+1]="m" end
		if it.verts[1] then
			if it.verts[1][9]  then fmt.tans=true fmt[#fmt+1]="tans" end
			if it.verts[1][13] then fmt.bone=true fmt[#fmt+1]="bone" end
		end
		return fmt
	end

	local pushvtx=function(t,fmt,m,v)

		t[#t+1]=v[1]
		t[#t+1]=v[2]
		t[#t+1]=v[3]

		if fmt.nrm then
			t[#t+1]=v[4] or 0
			t[#t+1]=v[5] or 0
			t[#t+1]=v[6] or 0
		end

		if fmt.uv then
			t[#t+1]=v[7] or 0
			t[#t+1]=v[8] or 0
		end

		if fmt.m then
			t[#t+1]=m
		end

		if fmt.tans then
			t[#t+1]=v[9] or 0
			t[#t+1]=v[10] or 0
			t[#t+1]=v[11] or 0
			t[#t+1]=v[12] or 0
		end

		if fmt.bone then
			t[#t+1]=v[13] or 0
			t[#t+1]=v[14] or 0
			t[#t+1]=v[15] or 0
			t[#t+1]=v[16] or 0
		end
		
	end
	
-- make a predraw buffer to draw triangles
	geom.predraw_polys=function(it)

		local fmt=getfmt(it,it.polys[1] and it.polys[1].mat)
		local t={}
		for i,p in ipairs(it.polys) do
			for ti=0,(#p-3) do
				for i=2,0,-1 do
					local tv=1+ti+i if i==0 then tv=1 end
					local idx=p[tv]
					local v=it.verts[idx]
					pushvtx(t,fmt,(p.mat or 1)-1,v)
				end
			end
		end
		return flat.array_predraw({fmt=table.concat(fmt),data=t,array=gl.TRIANGLES,vb=true})
	end

-- make a predraw buffer to draw triangles
	geom.predraw_flatpolys=function(it)

		local fmt=getfmt(it,it.polys[1].mat)
		local t={}
		for i,p in ipairs(it.polys) do
			local n=geom.get_poly_normal(it,p)
			for ti=0,(#p-3) do
				for i=2,0,-1 do
					local tv=1+ti+i if i==0 then tv=1 end
					local idx=p[tv]
					local v=it.verts[idx]
					pushvtx(t,fmt,(p.mat or 1)-1,v)
				end
			end
		end
		return flat.array_predraw({fmt=table.concat(fmt),data=t,array=gl.TRIANGLES,vb=true})
	end

-- make a predraw buffer to draw triangles but with mask instead of material
	geom.predraw_polys_mask=function(it)
	
		local fmt=getfmt(it,true)
		local t={}
		local mask=it.mask and it.mask.polys or {}
		for i,p in ipairs(it.polys) do
			for ti=0,(#p-3) do
				for i=2,0,-1 do
					local tv=1+ti+i if i==0 then tv=1 end
					local idx=p[tv]
					local v=it.verts[idx]
					local v=it.verts[idx]
					pushvtx(t,fmt,mask[p],v)
				end
			end
		end
		return flat.array_predraw({fmt=table.concat(fmt),data=t,array=gl.TRIANGLES,vb=true})
	end

-- make a predraw buffer to draw lines not triangles
	geom.predraw_lines=function(it)

		local fmt=getfmt(it,it.polys[1].mat)
		local t={}
		for i,p in ipairs(it.lines) do
			for i=1,2 do
				local idx=p[i]
				local v=it.verts[idx]
				pushvtx(t,fmt,(p.mat or 1)-1,v)
			end
		end
		return flat.array_predraw({fmt=table.concat(fmt),data=t,array=gl.LINES,vb=true})
	end

-- make a predraw buffer to draw lines not triangles and use ask rather than materials
	geom.predraw_lines_mask=function(it)

		local fmt=getfmt(it,true)
		local t={}
		local mask=it.mask and it.mask.lines or {}
		for i,p in ipairs(it.lines) do
			for i=1,2 do
				local idx=p[i]
				local v=it.verts[idx]
				pushvtx(t,fmt,mask[v],v)
			end
		end
		return flat.array_predraw({fmt=table.concat(fmt),data=t,array=gl.LINES,vb=true})
	end

-- make a predraw buffer to draw points not triangles and use mask rather than materials
	geom.predraw_verts_mask=function(it)

		local fmt=getfmt(it,true)
		local t={}
		local mask=it.mask and it.mask.verts or {}
		for i,v in ipairs(it.verts) do
			pushvtx(t,fmt,mask[v],v)
		end
		return flat.array_predraw({fmt=table.concat(fmt),data=t,array=gl.POINTS,vb=true})
	end

-- make a predraw line buffer containing normals for each vertex, scale controls its length.
	geom.predraw_normals=function(it,scale)
		scale=scale or it.normal_scale or 1
		local t={}
		local mask=it.mask and it.mask.verts or {}
		for i,v in ipairs(it.verts) do
			t[#t+1]=v[1]
			t[#t+1]=v[2]
			t[#t+1]=v[3]

			t[#t+1]=v[1]+((v[4] or 0)*scale)
			t[#t+1]=v[2]+((v[5] or 0)*scale)
			t[#t+1]=v[3]+((v[6] or 0)*scale)
		end
		return flat.array_predraw({fmt="xyz",data=t,array=gl.LINES,vb=true})
	end

-- make a predraw line buffer containing tangents for each vertex, scale controls its length.
	geom.predraw_tangents=function(it,scale)
		scale=scale or it.normal_scale or 1
		local t={}
		local mask=it.mask and it.mask.verts or {}
		for i,v in ipairs(it.verts) do
			t[#t+1]=v[1]
			t[#t+1]=v[2]
			t[#t+1]=v[3]

			t[#t+1]=v[1]+((v[9] or 0)*scale)
			t[#t+1]=v[2]+((v[10] or 0)*scale)
			t[#t+1]=v[3]+((v[11] or 0)*scale)
		end
		return flat.array_predraw({fmt="xyz",data=t,array=gl.LINES,vb=true})
	end

-- make a predraw line buffer containing tangents for each vertex, scale controls its length.
	geom.predraw_bitangents=function(it,scale)
		scale=scale or it.normal_scale or 1
		local t={}
		local mask=it.mask and it.mask.verts or {}
		for i,v in ipairs(it.verts) do
			t[#t+1]=v[1]
			t[#t+1]=v[2]
			t[#t+1]=v[3]
			
			local b={ -- bitangent is cross product of normal and tangent
						(((v[5] or 0)*(v[11] or 0))-((v[6] or 0)*(v[10] or 0))) ,
						(((v[6] or 0)*(v[9]  or 0))-((v[4] or 0)*(v[11] or 0))) ,
						(((v[4] or 0)*(v[10] or 0))-((v[5] or 0)*(v[9]  or 0)))
					}

			t[#t+1]=v[1]+(b[1]*(v[12] or 1)*scale)
			t[#t+1]=v[2]+(b[2]*(v[12] or 1)*scale)
			t[#t+1]=v[3]+(b[3]*(v[12] or 1)*scale)
		end
		return flat.array_predraw({fmt="xyz",data=t,array=gl.LINES,vb=true})
	end

	geom.draw_polys=function(it,progname,cb)
		local pd="pd_polys"
		if not it[pd] then it[pd]=geom.predraw_polys(it) end
		it[pd].draw(cb,progname)
		return it
	end	
	
	geom.draw_flatpolys=function(it,progname,cb)
		local pd="pd_flatpolys"
		if not it[pd] then it[pd]=geom.predraw_flatpolys(it) end
		it[pd].draw(cb,progname)
		return it
	end	

	geom.draw_polys_mask=function(it,progname,cb)
		local pd="pd_polys_mask"
		if not it[pd] then it[pd]=geom.predraw_mask(it) end
		it[pd].draw(cb,progname)
		return it
	end	

	geom.draw_lines=function(it,progname,cb)
		local pd="pd_lines"
		if not it.lines then geom.build_lines(it) end
		if not it[pd] then it[pd]=geom.predraw_lines(it) end
		it[pd].draw(cb,progname)
		return it
	end	

	geom.draw_lines_mask=function(it,progname,cb)
		local pd="pd_lines_mask"
		if not it.lines then geom.build_lines(it) end
		if not it[pd] then it[pd]=geom.predraw_lines_mask(it) end
		it[pd].draw(cb,progname)
		return it
	end	

	geom.draw_verts_mask=function(it,progname,cb)
		local pd="pd_verts_mask"
		if not it[pd] then it[pd]=geom.predraw_verts_mask(it) end
		it[pd].draw(cb,progname)
		return it
	end	

	geom.draw_normals=function(it,progname,cb)
		local pd="pd_normals"
		if not it[pd] then it[pd]=geom.predraw_normals(it) end
		it[pd].draw(cb,progname)
		return it
	end	

	geom.draw_tangents=function(it,progname,cb)
		local pd="pd_tangents"
		if not it[pd] then it[pd]=geom.predraw_tangents(it) end
		it[pd].draw(cb,progname)
		return it
	end	

	geom.draw_bitangents=function(it,progname,cb)
		local pd="pd_bitangents"
		if not it[pd] then it[pd]=geom.predraw_bitangents(it) end
		it[pd].draw(cb,progname)
		return it
	end	

-- remove all predraw buffers so we will resync base data next time we draw
	geom.clear_predraw=function(it)
		it.pd_polys=nil
		it.pd_flatpolys=nil
		it.pd_lines=nil
		it.pd_polys_mask=nil
		it.pd_flatpolys_mask=nil
		it.pd_lines_mask=nil
		it.pd_verts_mask=nil
		it.pd_normals=nil
		it.pd_tangents=nil
		it.pd_bitangents=nil
	end

-- default draw is polys
	geom.draw=function(it,progname,cb,modes)
		if not modes then
			return geom.draw_polys(it,progname,cb)
		end
		for i,n in ipairs( modes ) do
			geom[n](it,progname,cb)
		end
	end


	return geom
end


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

-- make a predraw buffer to draw triangles
	geom.predraw_polys=function(it)

		local orders={}
		orders[3]={ {3,3,2,1,1}			,	{1,1,2,3,3}		}
		orders[4]={ {3,3,4,2,1,1}		,	{1,1,2,4,3,3}	}
		orders[5]={ {4,4,3,5,2,1,1}		,	{1,1,2,5,3,4,4}	}

		local t={}
		local f=1
		for i,p in ipairs(it.polys) do
			local o=orders[#p][1+f%2]
			for _,i in ipairs(o) do
				local idx=p[i]
				local v=it.verts[idx]
				t[#t+1]=v[1]
				t[#t+1]=v[2]
				t[#t+1]=v[3]

				t[#t+1]=v[4] or 0
				t[#t+1]=v[5] or 0
				t[#t+1]=v[6] or 0

				t[#t+1]=v[7] or 0
				t[#t+1]=v[8] or 0
				t[#t+1]=(p.mat or 1)-1

				t[#t+1]=v[9] or 0
				t[#t+1]=v[10] or 0
				t[#t+1]=v[11] or 0
				t[#t+1]=v[12] or 0

				f=f+1
			end
		end
		return flat.array_predraw({fmt="xyznrmuvmbone",data=t,array=gl.TRIANGLE_STRIP,vb=true})
	end

-- make a predraw buffer to draw triangles
	geom.predraw_flatpolys=function(it)

		local orders={}
		orders[3]={ {3,3,2,1,1}			,	{1,1,2,3,3}		}
		orders[4]={ {3,3,4,2,1,1}		,	{1,1,2,4,3,3}	}
		orders[5]={ {4,4,3,5,2,1,1}		,	{1,1,2,5,3,4,4}	}

		local t={}
		local f=1
		for i,p in ipairs(it.polys) do
			local n=geom.get_poly_normal(it,p)
			local o=orders[#p][1+f%2]
			for _,i in ipairs(o) do
				local idx=p[i]
				local v=it.verts[idx]
				t[#t+1]=v[1]
				t[#t+1]=v[2]
				t[#t+1]=v[3]

				t[#t+1]=n[1] or 0
				t[#t+1]=n[2] or 0
				t[#t+1]=n[3] or 0

				t[#t+1]=v[7] or 0
				t[#t+1]=v[8] or 0
				t[#t+1]=(p.mat or 1)-1

				t[#t+1]=v[9] or 0
				t[#t+1]=v[10] or 0
				t[#t+1]=v[11] or 0
				t[#t+1]=v[12] or 0

				f=f+1
			end
		end
		return flat.array_predraw({fmt="xyznrmuvmbone",data=t,array=gl.TRIANGLE_STRIP,vb=true})
	end

-- make a predraw buffer to draw triangles but with mask instead of material
	geom.predraw_polys_mask=function(it)
	
		local orders={}
		orders[3]={ {3,3,2,1,1}			,	{1,1,2,3,3}		}
		orders[4]={ {3,3,4,2,1,1}		,	{1,1,2,4,3,3}	}
		orders[5]={ {4,4,3,5,2,1,1}		,	{1,1,2,5,3,4,4}	}

		local t={}
		local f=1
		for i,p in ipairs(it.polys) do
			local o=orders[#p][1+f%2]
			for _,i in ipairs(o) do
				local idx=p[i]
				local v=it.verts[idx]
				t[#t+1]=v[1]
				t[#t+1]=v[2]
				t[#t+1]=v[3]

				t[#t+1]=v[4] or 0
				t[#t+1]=v[5] or 0
				t[#t+1]=v[6] or 0

				t[#t+1]=v[7] or 0
				t[#t+1]=v[8] or 0
				t[#t+1]=v.mask or 0

				t[#t+1]=v[9] or 0
				t[#t+1]=v[10] or 0
				t[#t+1]=v[11] or 0
				t[#t+1]=v[12] or 0

				f=f+1
			end
		end
		return flat.array_predraw({fmt="xyznrmuvmbone",data=t,array=gl.TRIANGLE_STRIP,vb=true})
	end

-- make a predraw buffer to draw lines not triangles
	geom.predraw_lines=function(it)
		local t={}
		local f=1
		for i,p in ipairs(it.lines) do
			for i=1,2 do
				local idx=p[i]
				local v=it.verts[idx]
				t[#t+1]=v[1]
				t[#t+1]=v[2]
				t[#t+1]=v[3]

				t[#t+1]=v[4] or 0
				t[#t+1]=v[5] or 0
				t[#t+1]=v[6] or 0

				t[#t+1]=v[7] or 0
				t[#t+1]=v[8] or 0
				t[#t+1]=(p.mat or 1)-1

				t[#t+1]=v[9] or 0
				t[#t+1]=v[10] or 0
				t[#t+1]=v[11] or 0
				t[#t+1]=v[12] or 0

				f=f+1
			end
		end
		return flat.array_predraw({fmt="xyznrmuvmbone",data=t,array=gl.LINES,vb=true})
	end

-- make a predraw buffer to draw lines not triangles and use ask rather than materials
	geom.predraw_lines_mask=function(it)
		local t={}
		local f=1
		for i,p in ipairs(it.lines) do
			for i=1,2 do
				local idx=p[i]
				local v=it.verts[idx]
				t[#t+1]=v[1]
				t[#t+1]=v[2]
				t[#t+1]=v[3]

				t[#t+1]=v[4] or 0
				t[#t+1]=v[5] or 0
				t[#t+1]=v[6] or 0

				t[#t+1]=v[7] or 0
				t[#t+1]=v[8] or 0
				t[#t+1]=v.mask or p.mask or 0

				t[#t+1]=v[9] or 0
				t[#t+1]=v[10] or 0
				t[#t+1]=v[11] or 0
				t[#t+1]=v[12] or 0

				f=f+1
			end
		end
		return flat.array_predraw({fmt="xyznrmuvmbone",data=t,array=gl.LINES,vb=true})
	end

-- make a predraw buffer to draw points not triangles and use mask rather than materials
	geom.predraw_points_mask=function(it)
		local t={}
		local f=1
		for i,v in ipairs(it.verts) do
			t[#t+1]=v[1]
			t[#t+1]=v[2]
			t[#t+1]=v[3]

			t[#t+1]=v[4] or 0
			t[#t+1]=v[5] or 0
			t[#t+1]=v[6] or 0

			t[#t+1]=v[7] or 0
			t[#t+1]=v[8] or 0
			t[#t+1]=v.mask or 0

			t[#t+1]=v[9] or 0
			t[#t+1]=v[10] or 0
			t[#t+1]=v[11] or 0
			t[#t+1]=v[12] or 0

			f=f+1
		end
		return flat.array_predraw({fmt="xyznrmuvmbone",data=t,array=gl.POINTS,vb=true})
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

	geom.draw_points_mask=function(it,progname,cb)
		local pd="pd_points_mask"
		if not it[pd] then it[pd]=geom.predraw_points_mask(it) end
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
		it.pd_points_mask=nil
	end

-- default draw is polys
	geom.draw=geom.draw_polys


	return geom
end


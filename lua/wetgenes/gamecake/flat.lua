--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,flat)

	local canvas=oven.rebake("wetgenes.gamecake.canvas")

	local gl=oven.gl
	local cake=oven.cake
	local buffers=cake.buffers

-- should replace this with tristrip
flat.quad = function(x1,y1,x2,y2,x3,y3,x4,y4)

	if y4 then
		pack.save_array({
			x1,		y1,		0,		0,		0,
			x2,		y2,		0,		1,		0,
			x4,		y4,		0,		0,		1,
			x3,		y3,		0,		1,		1,
		},"f32",0,5*4,canvas.vdat)	
	else
		pack.save_array({
			x1,		y1,		0,		0,		0,
			x2,		y1,		0,		1,		0,
			x1,		y2,		0,		0,		1,
			x2,		y2,		0,		1,		1,
		},"f32",0,5*4,canvas.vdat)	
	end

	local p=gl.program("pos")
	gl.UseProgram( p[0] )
	gl.BindTexture( gl.TEXTURE_2D , 0 )

	local vertexarray
	if gl.GenVertexArray then
		vertexarray=gl.GenVertexArray()
		gl.BindVertexArray(vertexarray)
	end

	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())
	gl.BufferData(gl.ARRAY_BUFFER,5*4*4,canvas.vdat,gl.DYNAMIC_DRAW)

	gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
	gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )
	gl.Uniform4f( p:uniform("color"), gl.cache.color )

	gl.VertexAttribPointer(p:attrib("a_vertex"),3,gl.FLOAT,gl.FALSE,5*4,0)
	gl.EnableVertexAttribArray(p:attrib("a_vertex"))

	gl.DrawArrays(gl.TRIANGLE_STRIP,0,4)

	if gl.GenVertexArray then
		gl.BindVertexArray(0)
		gl.DeleteVertexArray(vertexarray)
	end

end

-- this allows us to prepare vertex buffers, then draw at anytime
-- just call it.draw on the returned table.
flat.array_predraw = function(it) -- pass in fmt,data,progname,vb=-1 in here

-- some basic vertexformats
	local pstride
	local pnrm
	local ptex
	local pcolor
	local pmat
	local ptans
	local pbone
	local p
	local def_progname
	local fmt=it.fmt
	if fmt=="xyz" or fmt=="pos" then -- xyz or pos only
	
		def_progname=fmt:sub(1,3)
		pstride=12
	
	elseif fmt=="xyznrm" or fmt=="posnrm" then -- xyz and normal (so we may light the thing)

		def_progname=fmt:sub(1,3).."_normal"

		pstride=24
		pnrm=12
	
	elseif fmt=="xyznrmuv" or fmt=="posnrmuv" then -- xyz and normal and texture

		def_progname=fmt:sub(1,3).."_normal_tex"

		pstride=32
		pnrm=12
		ptex=24
	
	elseif fmt=="xyznrmuvm" or fmt=="posnrmuvm" then -- xyz and normal and texture and  material id

		def_progname=fmt:sub(1,3).."_normal_tex_mat"

		pstride=36
		pnrm=12
		ptex=24
		pmat=32
	
	elseif fmt=="xyznrmuvmbone" or fmt=="posnrmuvmbone" then -- xyz and normal and texture and  material id and bones

		def_progname=fmt:sub(1,3).."_normal_tex_mat_bone"

		pstride=52
		pnrm=12
		ptex=24
		pmat=32
		pbone=36

	elseif fmt=="xyznrmuvtansbone" or fmt=="posnrmuvtansbone" then -- xyz and normal and texture and  material id and tangent and bones

		def_progname=fmt:sub(1,3).."_normal_tex_tans_bone"

		pstride=64
		pnrm=12
		ptex=24
		ptans=36
		pbone=52

	elseif fmt=="xyznrmuvmtansbone" or fmt=="posnrmuvmtansbone" then -- xyz and normal and texture and  material id and tangent and bones

		def_progname=fmt:sub(1,3).."_normal_tex_mat_tans_bone"

		pstride=68
		pnrm=12
		ptex=24
		pmat=32
		ptans=36
		pbone=52

	elseif fmt=="xyznrmuvmtans" or fmt=="posnrmuvmtans" then -- xyz and normal and texture and  material id and tangent

		def_progname=fmt:sub(1,3).."_normal_tex_mat_tans"

		pstride=52
		pnrm=12
		ptex=24
		pmat=32
		ptans=36

	elseif fmt=="xyzuv" or fmt=="posuv" then -- xyz and texture

		def_progname=fmt:sub(1,3).."_tex"

		pstride=20
		ptex=12
	
	elseif fmt=="xyzrgba" or fmt=="posrgba" or fmt=="rawrgba" then -- xyz and color

		def_progname=fmt:sub(1,3).."_color"

		pstride=28
		pcolor=12
	
	elseif fmt=="xyzuvrgba" or fmt=="posuvrgba" then -- xyz and texture and color
	
		def_progname=fmt:sub(1,3).."_tex_color"

		pstride=36
		ptex=12
		pcolor=20

	elseif fmt=="rawuvrgba" then -- raw-xyz and texture and color
	
		def_progname="raw_tex_color"

		pstride=36
		ptex=12
		pcolor=20

	elseif fmt=="rawuv" then -- raw-xyz and texture and color
	
		def_progname="raw_tex"

		pstride=20
		ptex=12

	end
	
	if canvas.discard_low_alpha then -- try not to break the zbuffer
		def_progname=def_progname.."_discard"
	end

	
	local data=it.data
	local datalen=it.datalen or #data
	local datasize=datalen*4 -- we need this much vdat memory
	canvas.vdat_check(datasize) -- make sure we have space in the buffer
	
	if it.vb then
		if type(it.vb)~="table" then -- need to create
			it.vb=buffers.create({
				start=function(vb)
					vb:bind()
					pack.save_array(data,"f32",0,datalen,canvas.vdat)
					gl.BufferData(gl.ARRAY_BUFFER,datasize,canvas.vdat,gl.STATIC_DRAW)
				end,
			})
		end
	end


	it.draw=function(cb,progname)

		progname=progname or it.progname or def_progname
		local p=gl.program(progname)
		gl.UseProgram( p[0] )

		if gl.GenVertexArray then
			it.va=gl.GenVertexArray()
			gl.BindVertexArray(it.va)
		end

		if it.vb then -- use a precached buffer
			it.vb:bind()
		elseif it.dataraw then -- use prebuilt data
			gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())
			gl.BufferData(gl.ARRAY_BUFFER,datasize,it.dataraw,gl.DYNAMIC_DRAW)
		else
			pack.save_array(data,"f32",0,datalen,canvas.vdat)
			gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())
			gl.BufferData(gl.ARRAY_BUFFER,datasize,canvas.vdat,gl.DYNAMIC_DRAW)
		end

		gl.uniforms_apply(p)

--		gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
--		gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )
--		gl.Uniform4f( p:uniform("color"), gl.cache.color )

		gl.VertexAttribPointer(p:attrib("a_vertex"),3,gl.FLOAT,gl.FALSE,pstride,0)
		gl.EnableVertexAttribArray(p:attrib("a_vertex"))
		
--print( progname , fmt , p:attrib("a_texcoord") )

		if pnrm then
			local a=p:attrib("a_normal")
			if a>=0 then
				gl.VertexAttribPointer(a,3,gl.FLOAT,gl.FALSE,pstride,pnrm)
				gl.EnableVertexAttribArray(a)
			end
		end

		if ptex then
			local a=p:attrib("a_texcoord")
			if a>=0 then
				gl.VertexAttribPointer(a,2,gl.FLOAT,gl.FALSE,pstride,ptex)
				gl.EnableVertexAttribArray(a)
			end
		end

		if pcolor then
			local a=p:attrib("a_color")
			if a>=0 then
				gl.VertexAttribPointer(a,4,gl.FLOAT,gl.FALSE,pstride,pcolor)
				gl.EnableVertexAttribArray(a)
			end
		end

		if pmat then
			local a=p:attrib("a_matidx")
			if a>=0 then
				gl.VertexAttribPointer(a,1,gl.FLOAT,gl.FALSE,pstride,pmat)
				gl.EnableVertexAttribArray(a)
			end
		end
		
		if ptans then
			local a=p:attrib("a_tangent")
			if a>=0 then
				gl.VertexAttribPointer(a,4,gl.FLOAT,gl.FALSE,pstride,ptans)
				gl.EnableVertexAttribArray(a)
			end
		end

		if pbone then
			local a=p:attrib("a_bone")
			if a>=0 then
				gl.VertexAttribPointer(a,4,gl.FLOAT,gl.FALSE,pstride,pbone)
				gl.EnableVertexAttribArray(a)
			end
		end

--	local err=gl.GetError() ; assert( err==0 , gl.numtostring(err) ) -- well that went wrong

		if cb then cb(p) end -- callback to fill in more uniforms before drawing

		local cc=datasize/pstride
		if cc>0 then
			gl.DrawArrays( it.array or gl.TRIANGLE_STRIP,0,cc)
		end

		if gl.GenVertexArray then
			gl.BindVertexArray(0)
			gl.DeleteVertexArray(it.va)
			it.va=nil
		end

	end

	return it
end
flat.tristrip_predraw=flat.array_predraw

-- tristrip is the most useful, 3 points gives us a tri
-- 4 gives us a quad, and of course you can keep going to create a strip
flat.tristrip = function(fmt,data,progname,cb)
	if #data > 0 then
		local it=flat.array_predraw({fmt=fmt,data=data,progname=progname,array=gl.TRIANGLE_STRIP})
		it.draw(cb)
	end
end

flat.lines = function(fmt,data,progname,cb)
	if #data > 0 then
		local it=flat.array_predraw({fmt=fmt,data=data,progname=progname,array=gl.LINES})
		it.draw(cb)
	end
end

	return flat
end

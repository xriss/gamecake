-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- this contains mild emulation of the old gl fixed pipeline, such that we can run some code in gles2 and above

local tardis=require("wetgenes.tardis")

local glesfix={}





-- apply our compatibility fixes into the base gles function table
function glesfix.apply_compat(gles)

	gles.stacks={}
	gles.stacks[gles.MODELVIEW]={ tardis.m4.new() }
	gles.stacks[gles.PROJECTION]={ tardis.m4.new() }
	gles.stacks[gles.TEXTURE]={ tardis.m4.new() }
	
	gles.stack_mode=gles.MODELVIEW
	gles.stack=gles.stacks[gles.stack_mode]
	gles.stack_matrix=gles.stack[#gles.stack]


--debug test function, push into old gl
	function uploadmatrix()
		gles.core.MatrixMode(gles.stack_mode)
		gles.core.LoadMatrix(gles.stack_matrix)
	end
	
	function gles.MatrixMode(mode)
		gles.stack_mode=mode
		gles.stack=assert(gles.stacks[gles.stack_mode])
		gles.stack_matrix=assert(gles.stack[#gles.stack])
		uploadmatrix()
	end

	function gles.LoadMatrix(...)
		gles.stack_matrix:set(...)
		uploadmatrix()
	end

	function gles.MultMatrix(a)
		tardis.m4_product_m4(gles.stack_matrix,a,gles.stack_matrix)
		uploadmatrix()
	end

	function gles.Frustum(...)
		error("frustrum not suported")
	end

	function gles.LoadIdentity()
		gles.stack_matrix:identity()
		uploadmatrix()
	end

	function gles.Translate(vx,vy,vz)
		gles.stack_matrix:translate({vx,vy,vz})
		uploadmatrix()
	end

	function gles.Rotate(d,vx,vy,vz)
		gles.stack_matrix:rotate(d,{vx,vy,vz})
		uploadmatrix()
	end

	function gles.Scale(vx,vy,vz)
		gles.stack_matrix:scale_v3({vx,vy,vz})
		uploadmatrix()
	end

	function gles.PushMatrix()		
		gles.stack[#gles.stack+1]=tardis.m4.new(gles.stack_matrix) -- duplicate to new top
		gles.stack_matrix=assert(gles.stack[#gles.stack])
		uploadmatrix()
	end

	function gles.PopMatrix()
		gles.stack[#gles.stack]=nil -- remove topmost
		gles.stack_matrix=assert(gles.stack[#gles.stack]) -- this will assert on too many pops
		uploadmatrix()
	end


end

return glesfix

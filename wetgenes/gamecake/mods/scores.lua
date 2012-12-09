-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ partname=(...) } ; package.loaded[M.partname]=M

M.bake=function(state,scores)

	scores=scores or {} 
	scores.partname=M.partname
	

	function scores.setup(state)
	end

	function scores.clean(state)
	end

	function scores.update(state)	
	end
	
	function scores.draw(state)
	end
		
	function scores.msg(state,m)
	end

	return scores
end

--
-- (C) 2013 Kriss@XIXs.com
--

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local M={ modname=(...) } ; package.loaded[M.modname]=M

local wbgrd=M

local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local wbake=require("wetgenes.bake")

local dprint=function(...) print(wstr.dump(...)) end


M.auto_copyfile=function(from,too)

	wbake.create_dir_for_file(too)

	local ext=from:sub(-4)
	if ext==".png" or ext==".jpg" or ext==".gif" then -- auto mip
		wbgrd.copymips({from=from,too=too})
	else
		wbake.copyfile(from,too)
	end
end

M.auto_copysave=function(g,too)
	wbgrd.copymips({g=g,too=too})
end

M.find_image=function(filename)

	for i,v in ipairs( { ".png" , ".jpg" , ".gif" } ) do
		local fext=v
		local fname=filename..fext -- hardcode
		if wbake.file_exists(fname) then  return fname ,fext end -- found it
	end

end

--
-- build mipped versions of an image and copy them into data (give full path from,too)
--
function M.copymips(args)

	local it={}

	local g=args.g -- pass in g
	
	if not g then -- load
	
		local from=args.from:sub(1,-5)
		local fext=args.from:sub(-4)
		g=assert(wgrd.create(from..fext)) -- load it
		assert(g:convert(wgrd.FMT_U8_RGBA_PREMULT)) -- premult default

	end
	
	local too=args.too:sub(1,-5)
	local text=args.too:sub(-4)
	

-- force powah of 2 sizes, so we can mipmap

	it.ext=text

	it.width=g.width
	it.height=g.height
	it.texture_width=2^math.ceil(math.log(it.width)/math.log(2))
	it.texture_height=2^math.ceil(math.log(it.height)/math.log(2))
	

	local mip1=math.ceil(math.log(it.width)/math.log(2))
	local mip2=math.ceil(math.log(it.height)/math.log(2))
	
	it.mip=(mip1 > mip2) and mip1 or mip2 -- pick biggest mip

-- HACK force square?
--	it.texture_width=2^it.mip
--	it.texture_height=2^it.mip

	if it.texture_width~=g.width or it.texture_height~=g.height then 
		g:resize(it.texture_width,it.texture_height,1) -- resize keeping the image in the topleft corner
	end
	
	for i=1,it.mip do it[i]=false end
	for i=it.mip,4,-1 do --build mips
		local ms=string.format("%02d",i)
		local t={}
		it[i]=t
		
		t.mip=i
		t.width=g.width
		t.height=g.height
		
		local fn=too.."."..ms..text
		g:save(fn)

-- next mip
		g:scale(math.ceil(g.width/2),math.ceil(g.height/2),1)
	end
	
	local fn=too..".00.lua"

	wbake.writefile(fn,wstr.serialize(it)) -- save mipmap data as lua file

--	dprint(it)
--os.exit()

end

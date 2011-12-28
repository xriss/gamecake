-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local lfs=require("lfs")

module(...)

function build(tab)

	local bake=require("wetgenes.bake")
	
	-- where we are building from
	bake.cd_base	=	bake.cd_base or bake.get_cd()
	-- where we are building to
	bake.cd_out		=	bake.cd_out or '.ngx'
	-- where we are building from
	bake.cd_root	=	bake.cd_base .. "/../.."

-- we need this one
	lfs.mkdir(bake.cd_out)


-- combine all possible lua files into one lua dir in the .ngx output dir

	local opts={basedir=bake.cd_root.."/bin",dir="lua",filter="%.lua$"}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

	local opts={basedir=bake.cd_base.."/html",dir="lua",filter="%.lua$"}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

	local opts={basedir=bake.cd_base,dir="lua",filter="%.lua$"}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

end

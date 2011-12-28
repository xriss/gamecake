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

	local opts={basedir=bake.cd_root.."/bin",dir="lua",filter=""}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

	local opts={basedir=bake.cd_base.."/html",dir="lua",filter=""}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

	local opts={basedir=bake.cd_base,dir="lua",filter=""}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end


-- now do the same with the modules datas

	local modnames={
		"admin",
		"base",
		"blog",
		"chan",
		"comic",
		"console",
		"data",
		"dice",
		"dimeload",
		"dumid",
		"forum",
		"mirror",
		"note",
		"port",
		"profile",
		"score",
		"shoop",
		"thumbcache",
		"todo",
		"waka",
	}
	for i,n in ipairs(modnames) do
		for i,s in ipairs{"art","css","js"} do
			local opts={basedir=bake.cd_root.."/mods/"..n.."/"..s,dir="",filter=""}
			local r=bake.findfiles(opts)
			for i,v in ipairs(r.ret) do
				local fname=bake.cd_out.."/html/"..s.."/"..n.."/"..v
				bake.create_dir_for_file(fname)
				bake.copyfile(opts.basedir.."/"..v,fname)
			end
		end
		
		local opts={basedir=bake.cd_root.."/mods/"..n.."/lua",dir="",filter=""}
		local r=bake.findfiles(opts)
		for i,v in ipairs(r.ret) do
			local fname=bake.cd_out.."/lua/"..n.."/"..v
			bake.create_dir_for_file(fname)
			bake.copyfile(opts.basedir.."/"..v,fname)
		end
	end

	local opts={basedir=bake.cd_base,dir="html",filter=""}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

local ngx_config=[[
#worker_processes  4;

events {
    worker_connections  64;
}

http {

lua_package_path  './lua/?.lua;./lua/?/init.lua;;';
lua_package_cpath ';;';

  server {
#      access_log  access.log;
#      error_log   error.log;
      listen      127.0.0.1:8888;
      root        www;
      server_name host.local;

#try existing files
	location  / {
		try_files $uri @serv;
	}
	
#call into lua to handle anything else	
	location  @serv {
		content_by_lua_file lua/ngx_serv.lua;
	}
	
  }
}
]]
	local fname=bake.cd_out.."/conf/nginx.conf"
	bake.create_dir_for_file(fname)
	bake.writefile(fname,ngx_config)
	
	local fname=bake.cd_out.."/logs/test" --nginx wont make its logs dir...
	bake.create_dir_for_file(fname)


	if (tab.arg[1] or "")=="serv" then
	
		print("Starting anlua on nginx")
		
		bake.execute(bake.cd_out,"../../../bin/exe/nginx","-p. -sstop")
		bake.execute(bake.cd_out,"../../../bin/exe/nginx","-p.")
		bake.execute(bake.cd_out,"tail","-f logs/error.log")
	end
	
end

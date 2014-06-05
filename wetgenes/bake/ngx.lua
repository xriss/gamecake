--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local lfs=require("lfs")
local wstr=require("wetgenes.string")
local dprint=function(...) print(wstr.dump(...)) end

module(...)

function build(tab)

	local bake=require("wetgenes.bake")
	
	-- where we are building from
	bake.cd_base	=	tab.cd_base or bake.get_cd()
	-- where we are building to
	bake.cd_out		=	tab.cd_out or '.ngx'
	-- where we are building from
	bake.cd_root	=	bake.cd_base .. "/../.."

-- we need this one
	lfs.mkdir(bake.cd_out)
-- and these
	lfs.mkdir(bake.cd_out.."/conf")
	lfs.mkdir(bake.cd_out.."/logs")
	lfs.mkdir(bake.cd_out.."/sqlite")


-- combine all possible lua files into one lua dir in the .ngx output dir

	local opts={basedir=bake.cd_root.."/../bin",dir="lua",filter=""}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

	local opts={basedir=bake.cd_root,dir="lua",filter=""}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end
	
	local opts={basedir=bake.cd_base.."/public",dir="lua",filter=""}
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

	local modnames={}
	for v in lfs.dir(bake.cd_root.."/mods") do
		local a=lfs.attributes(bake.cd_root.."/mods/"..v)
		if a.mode=="directory" then
			if v:sub(1,1)~="." then
				modnames[#modnames+1]=v
			end
		end
	end

	for i,n in ipairs(modnames) do
		for i,s in ipairs{"art","css","js"} do
			local opts={basedir=bake.cd_root.."/mods/"..n.."/"..s,dir="",filter=""}
			local r=bake.findfiles(opts)
			for i,v in ipairs(r.ret) do
				local fname=bake.cd_out.."/public/"..s.."/"..n.."/"..v
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

	local opts={basedir=bake.cd_base,dir="public",filter=""}
	local r=bake.findfiles(opts)
	for i,v in ipairs(r.ret) do
		local fname=bake.cd_out.."/"..v
		bake.create_dir_for_file(fname)
		bake.copyfile(opts.basedir.."/"..v,fname)
	end

tab.ngx_listen	=tab.ngx_listen or "127.0.0.1:8888"
tab.ngx_user	=tab.ngx_user	or "kriss"
tab.ngx_debug	=tab.ngx_debug	or "debug"


	local ngx_config=bake.readfile(bake.cd_base.."/nginx.conf")
	ngx_config=wstr.replace(ngx_config,tab)

	local fname=bake.cd_out.."/conf/nginx.conf"
	bake.writefile(fname,ngx_config)

	if (tab.arg[1] or "")=="serv" then
	
		print("Starting anlua on nginx\n\n")
		
		bake.execute(bake.cd_out,"../../../../bin/dbg/nginx","-p. -sstop")
		bake.execute(bake.cd_out,"../../../../bin/dbg/nginx","-p.")
--		bake.execute(bake.cd_out,"tail","-n0 -f logs/error.log")
		
		local fp=io.popen("tail -n0 -f logs/error.log","r") -- should probably just open the file myself...
		local finished
		while true do
			local l=fp:read("*l")
			if l then
				local s=l
				s=s:gsub(", client: .*$"," .")
				s=s:gsub("^.*: %*%d* ",". ")
				print(s)
			else break end
		end
	end
	
end

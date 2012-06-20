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
-- and these
	lfs.mkdir(bake.cd_out.."/conf")
	lfs.mkdir(bake.cd_out.."/logs")
	lfs.mkdir(bake.cd_out.."/sqlite")


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

#This forces us to have only one lua state, which can make some nginx speed hacks possible
#Since we can leave values lying around for future use.
#I figure we can just use lanes to offload big cpu tasks onto other threads...

worker_processes  16;



events {
    worker_connections  512;
}

http {

# do not merge slashes fool
merge_slashes off ;

types {
    text/html                             html htm shtml;
    text/css                              css;
    text/xml                              xml;
    image/gif                             gif;
    image/jpeg                            jpeg jpg;
    application/x-javascript              js;
    application/atom+xml                  atom;
    application/rss+xml                   rss;

    text/mathml                           mml;
    text/plain                            txt;
    text/vnd.sun.j2me.app-descriptor      jad;
    text/vnd.wap.wml                      wml;
    text/x-component                      htc;

    image/png                             png;
    image/tiff                            tif tiff;
    image/vnd.wap.wbmp                    wbmp;
    image/x-icon                          ico;
    image/x-jng                           jng;
    image/x-ms-bmp                        bmp;
    image/svg+xml                         svg svgz;
    image/webp                            webp;

    application/java-archive              jar war ear;
    application/mac-binhex40              hqx;
    application/msword                    doc;
    application/pdf                       pdf;
    application/postscript                ps eps ai;
    application/rtf                       rtf;
    application/vnd.ms-excel              xls;
    application/vnd.ms-powerpoint         ppt;
    application/vnd.wap.wmlc              wmlc;
    application/vnd.google-earth.kml+xml  kml;
    application/vnd.google-earth.kmz      kmz;
    application/x-7z-compressed           7z;
    application/x-cocoa                   cco;
    application/x-java-archive-diff       jardiff;
    application/x-java-jnlp-file          jnlp;
    application/x-makeself                run;
    application/x-perl                    pl pm;
    application/x-pilot                   prc pdb;
    application/x-rar-compressed          rar;
    application/x-redhat-package-manager  rpm;
    application/x-sea                     sea;
    application/x-shockwave-flash         swf;
    application/x-stuffit                 sit;
    application/x-tcl                     tcl tk;
    application/x-x509-ca-cert            der pem crt;
    application/x-xpinstall               xpi;
    application/xhtml+xml                 xhtml;
    application/zip                       zip;

    application/octet-stream              bin exe dll;
    application/octet-stream              deb;
    application/octet-stream              dmg;
    application/octet-stream              eot;
    application/octet-stream              iso img;
    application/octet-stream              msi msp msm;

    audio/midi                            mid midi kar;
    audio/mpeg                            mp3;
    audio/ogg                             ogg;
    audio/x-m4a                           m4a;
    audio/x-realaudio                     ra;

    video/3gpp                            3gpp 3gp;
    video/mp4                             mp4;
    video/mpeg                            mpeg mpg;
    video/quicktime                       mov;
    video/webm                            webm;
    video/x-flv                           flv;
    video/x-m4v                           m4v;
    video/x-mng                           mng;
    video/x-ms-asf                        asx asf;
    video/x-ms-wmv                        wmv;
    video/x-msvideo                       avi;
}

lua_package_path  './lua/?.lua;./lua/?/init.lua;;';
lua_package_cpath ';;';

  server {

      access_log  logs/access.log;
      error_log   logs/error.log debug;
      listen      127.0.0.1:8888;
      root        html;
      server_name $host;

#do the fetching here...
	location ~ /@fetch/(.*)$ {
		internal;
		set $a $1;
		resolver 8.8.8.8;
		rewrite (.*) $a break;
		proxy_pass_request_headers off;
		proxy_pass '$a?$args';
	}

#is this the best way to sleep?
	location ~ /@sleep/(.*)$ {
		internal;
		echo_sleep $1;
	}

#try existing files
	location  / {
		try_files $uri @serv;
	}
	
#call into lua to handle anything else	
	location  @serv {
		content_by_lua "require(\"wetgenes.www.ngx.serv\").serv()";
	}
	
  }
}
]]
	local fname=bake.cd_out.."/conf/nginx.conf"
	bake.writefile(fname,ngx_config)

	if (tab.arg[1] or "")=="serv" then
	
		print("Starting anlua on nginx\n\n")
		
		bake.execute(bake.cd_out,"../../../bin/dbg/nginx","-p. -sstop")
		bake.execute(bake.cd_out,"../../../bin/dbg/nginx","-p.")
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

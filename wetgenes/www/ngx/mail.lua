-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

local wstr=require("wetgenes.string")

local socket=require("socket")
local http=require("socket.http")
local smtp = require("socket.smtp") 
local mime = require("mime") 
local ltn12 = require("ltn12") 

local ngx = require("ngx") 

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.mail"]=_M



local function create()

--log("CREATE")
	local p={}
	local tcp=ngx.socket.tcp()

	p.accept=function(_,...)
--log("accept")
--log(...)
		return tcp:accept(...)
	end

	p.bind=function(_,...)
--log("bind")
--log(...)
		return tcp:bind(...)
	end

	p.close=function(_,...)
--log("close")
--log(...)
		return tcp:close(...)
	end
	
	p.connect=function(_,...)
--log("connect")
--log(...)
		tcp:connect(...)
		return true
	end

	p.getpeername=function(_,...)
--log("getpeername")
--log(...)
		return tcp:getpeername(...)
	end

	p.getsockname=function(_,...)
--log("getsockname")
--log(...)
		return tcp:getsockname(...)
	end

	p.getstats=function(_,...)
--log("getstats")
--log(...)
		return tcp:getstats(...)
	end

	p.listen=function(_,...)
--log("listen")
--log(...)
		return tcp:listen(...)
	end

	p.receive=function(_,...)
--log("receive")
--log(...)
		local r=tcp:receive(...)
		return r
	end

	p.send=function(_,...)
--log("send")
--log(...)
		return tcp:send(...)
	end

	p.setoption=function(_,...)
--log("setoption")
--log(...)
		return tcp:setoption(...)
	end

	p.setstats=function(_,...)
--log("setstats")
--log(...)
		return tcp:setstats(...)
	end

	p.settimeout=function(_,n)
--log("settimeout")
--log(n)
		tcp:settimeout(n*1000)
		return true
	end

	p.shutdown=function(_,...)
--log("shutdown")
--log(...)
		return tcp:shutdown(...)
	end

--log(wstr.dump(p))

	return p
end


function send(tab)
	log("mail.send:")
--	return core.send(...)



	local source = smtp.message{ 
      headers = { 
         from = "<"..tab.from..">", 
         to = "<"..tab.to..">", 
         subject = tab.subject,
      }, 
      body = { 
        preamble = "preamble", 
        [1] = { body = mime.eol(0, tab.body) }, 
      } 
    } 

    r, e = smtp.send{ 
        from = "<"..tab.from..">", 
        rcpt = "<"..tab.to..">", 
        source = source, 
        server = "127.0.0.1", 
        port = 25, 
        create=create,
    } 

    if not r then 
        log("failed to send: ", e) 
    end 
    
    log(wstr.dump(r))

    log("done!") 
    
end

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- An attempt to turn youtube IDs into links to video files.
-- This may break if youtube decides to prevent this sort of activity


local wstr=require("wetgenes.string")

module("wetgenes.tube.youtube")


-- overload this function if we are running in a strange environment
-- notice the require, which means socket.http is not a forced dependency
function get_url(url)

local http=require("socket.http")

local body, headers, code = http.request(url)

local ret={}

	ret.body=body
	ret.headers=headers
	ret.code=code
	
	return ret

end


function url_split(s)
	local ands=wstr.split(s,"&")
	local vals={}
	for i,v in ipairs(ands) do
		local aa=wstr.split(v,"=")
		vals[ wstr.url_decode(aa[1]) ]=wstr.url_decode(aa[2])
	end
	return vals
end

function get_video_info(id)

	local url="http://www.youtube.com/get_video_info?video_id="..id.."&ps=default&eurl=&gl=US&hl=en"
	local ret=get_url(url)
	
	local vals=url_split(ret.body)
	
	vals.fmts={}
	local ps=wstr.split(vals.url_encoded_fmt_stream_map,",")
	for i,v in ipairs(ps) do
		vals.fmts[i]=url_split(v)
	end
	
	vals.url_encoded_fmt_stream_map=nil -- kill the original fmt string
	
	return vals
end



-- return info about a youtube ID, most importantly a URL to stream it from
-- this cleans up the data from get_video_info
function info(id)
	local tab
	
	tab=get_video_info(id)
	
	return tab
end


-- Test a file?
--[[
local t=info("SmI9-fQKOIA")
print(wstr.dump(t))
local url
for i,v in ipairs(t.fmts) do
	if v.itag=="43" then url=v.url end
end

print(url)

local fp=io.open("dumb.web","w")
fp:write( get_url(url).body )
fp:close()
]]

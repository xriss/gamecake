--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

--[[#lua.wetgenes.gists

	local wgists=require("wetgenes.gists")

Functions that query the gists API 

https://docs.github.com/en/rest/gists

This module requires wire http tasks to be up and running as that is 
how they send http requests. Also be aware that these busy wait on 
results from a webserver and anything can and will go wrong in that 
situation.

]]

local wire=require("wetgenes.wire")
local djon=require("djon")

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local wgists=M


wgists.baseurl="https://api.github.com"

local get_timezone=function()
    local now = os.time()
    return os.difftime(now, os.time(os.date("!*t", now)))
end
local epoch = get_timezone()

--[[#lua.wgists.parse_iso8601

	seconds = wgists.parse_iso8601(text)

Parse a string in iso8601 format into a linux time ( seconds since 1970 )

]]
wgists.parse_iso8601=function(str)
    local year, month, day, hour, minute, seconds, offsetsign, offsethour, offsetmin =
		str:match("(%d+)%-(%d+)%-(%d+)%a(%d+)%:(%d+)%:([%d%.]+)([Z%+%- ])(%d?%d?)%:?(%d?%d?)")
    local timestamp = os.time{year = year, month = month, day = day, hour = hour, min = minute, sec = math.floor(seconds)} + epoch
    local offset = 0
    if offsetsign ~= 'Z' then
        offset = tonumber(offsethour) * 60 + tonumber(offsetmin)
        if offsetsign == "-" then offset = -offset end
    end
    return timestamp - offset * 60
end


--[[#lua.wgists.list

	local list=gist.list(opts)

The http task must be running and this will block waiting on results.

List some gists from github using options found in opts, see 
https://docs.github.com/en/rest/gists

	opts.baseurl

Can be used to overide the default of "https://api.github.com" which is 
set in wgists.baseurl and can also be changed globally there.

	opts.token

Should be set to a github access token and is required for writing data 
or accessing non public information.

	opts.username
	
Is the name of a github user we wish to list gists for

	opts.filter

Can be "public" to request only public gists or "starred" to request 
only starred gists.

	opts.since

An ISO_8601 timestamp in a string and should be set if we are only 
interested in gists since this time.

	opts.per_page

The number of results we want, the api limits this to 100 but defaults 
to 30. Probably a good idea to set this to 100 unless you want to deal 
with paging code and even then you can only get the first 3000 results.

	opts.page

The page of results we are requesting, It defaults to 1 so can be 
omitted unless you are going to request multiple pages.

]]
wgists.list=function(opts)
	
	local baseurl=opts.baseurl or wgists.baseurl
	local username=""
	if opts.username then -- request gists from this username (does not have to match token)
		username="/users/"..opts.username
	end
	local filter=""
	if opts.filter=="public" then -- ask for public gists of tokens user
		filter="/public"
	elseif opts.filter=="starred" then -- ask for starred gists of tokens user
		filter="/starred"
	end
		
	local q=""
	do
		local t={}
		if opts.since    then t[#t+1]="since="..    opts.since    end
		if opts.per_page then t[#t+1]="per_page=".. opts.per_page end
		if opts.page     then t[#t+1]="page="..     opts.page     end
		if #t>0 then
			q="?"..table.concat(t,"&")
		end
	end

	local headers={}
	headers["Accept"]="application/vnd.github+json"
	headers["X-GitHub-Api-Version"]="2022-11-28"
	if opts.token then
		headers["Authorization"]="Bearer "..opts.token
	end
	
	local data={headers=headers,method="GET",url=baseurl..username.."/gists"..filter..q}
	local result=wire.memo({
		fifo=wire.fifo("http"),
		data=data,
	}):resolve()
	local body=djon.load(result.body or "{}")

	return body
end

--[[#lua.wgists.get


The http task must be running and this will block waiting on results.


Get a gist, see https://docs.github.com/en/rest/gists

	opts.gid

Must be set to the gist id

	opts.token

Optional token to read this gist

	opts.baseurl

Can be used to overide the default of "https://api.github.com" 
which is set in wgists.baseurl and can also be changed globally 
there.

]]
wgists.get=function(opts)

	local baseurl=opts.baseurl or wgists.baseurl

	local headers={}
	headers["Accept"]="application/vnd.github+json"
	headers["X-GitHub-Api-Version"]="2022-11-28"
	if opts.token then
		headers["Authorization"]="Bearer "..opts.token
	end
	
	local data={headers=headers,method="GET",url=baseurl.."/gists/"..opts.gid}
	local result=wire.memo({
		fifo=wire.fifo("http"),
		data=data,
	}):resolve()
	local body=djon.load(result.body or "{}")

	return body
end

--[[#lua.wgists.set


The http task must be running and this will block waiting on results.


Set a gist, see https://docs.github.com/en/rest/gists

	opts.gid

Must be set to the gist id

	opts.token

Token to write this gist

	opts.body

Gist data

	opts.baseurl

Can be used to overide the default of "https://api.github.com" 
which is set in wgists.baseurl and can also be changed globally 
there.

]]
wgists.set=function(opts)

	local baseurl=opts.baseurl or wgists.baseurl

	local headers={}
	headers["Accept"]="application/vnd.github+json"
	headers["X-GitHub-Api-Version"]="2022-11-28"
	headers["Authorization"]="Bearer "..assert(opts.token)
	
	local data={headers=headers,method="PATCH",url=baseurl.."/gists/"..opts.gid,body=djon.save(opts.body)}
	local result=wire.memo({
		fifo=wire.fifo("http"),
		data=data,
	}):resolve()
	local body=djon.load(result.body or "{}")

	return body
end

--
-- (C) 2025 Kriss@XIXs.com
--

-- module
local M={ modname = (...) } package.loaded[M.modname] = M

--[[#lua.wetgenes.tasks_gist

	local gist=require("wetgenes.tasks_gist")

Access github gists using the github api via the http tasks.

]]

local djon=require("djon")

M.functions={}
M.metatable={__index=M.functions}
setmetatable(M,M.metatable)

M.baseurl="https://api.github.com"

local get_timezone=function()
    local now = os.time()
    return os.difftime(now, os.time(os.date("!*t", now)))
end
local epoch = get_timezone()

M.functions.parse_iso8601=function(str)
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


--[[#lua.wetgenes.tasks_gist.list

	local tasks=require("wetgenes.tasks").create()
	local gist=require("wetgenes.tasks_gist")
	local opts={
		tasks=tasks, -- required
	}
	local list=gist.list(opts)

List some gists from github using options found in opts, valid options are...

baseurl can be used to overide the default of "https://api.github.com"

tasks is always *required* as we use it for http access.

token should be set to a github access token and is required for 
writting data or accessing non public information.

username is the name of a github user we wish to list gists for

filter can be "public" to request only public gists or "starred" to 
request only starred gists.

since is an ISO_8601 timestamp in a string and should be set if we are 
only interested in gists since this time.

per_page is the number of results we want, the api limits this to 100 
but defaults to 30. Probably a good idea to set this to 100 unless you 
want to deal with paging code and even then you can only get the first 
3000 results.

page is the page of results we are requesting, It defaults to 1 so can 
be omitted unless you are going to request multiple pages.

]]
M.functions.list=function(opts)
	local tasks=opts.tasks -- must be running tasks as returned from wetgenes.tasks.create()
	
	local baseurl=opts.baseurl or M.baseurl
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
	
	local memo={headers=headers,method="GET",url=baseurl..username.."/gists"..filter..q}
	local result=tasks:http(memo)
	local body=djon.load(result.body or "{}")

	return body
end

M.functions.get=function(opts)
	local tasks=opts.tasks -- must be running tasks as returned from wetgenes.tasks.create()
	
	local baseurl=opts.baseurl or M.baseurl

	local headers={}
	headers["Accept"]="application/vnd.github+json"
	headers["X-GitHub-Api-Version"]="2022-11-28"
	if opts.token then
		headers["Authorization"]="Bearer "..opts.token
	end
	
	local memo={headers=headers,method="GET",url=baseurl.."/gists/"..opts.gid}
	local result=tasks:http(memo)
	local body=djon.load(result.body or "{}")

	return body
end

M.functions.set=function(opts)
	local tasks=opts.tasks -- must be running tasks as returned from wetgenes.tasks.create()
	
	local baseurl=opts.baseurl or M.baseurl

	local headers={}
	headers["Accept"]="application/vnd.github+json"
	headers["X-GitHub-Api-Version"]="2022-11-28"
	headers["Authorization"]="Bearer "..assert(opts.token)
	
	local memo={headers=headers,method="PATCH",url=baseurl.."/gists/"..opts.gid,body=djon.save(opts.body)}
	local result=tasks:http(memo)
	local body=djon.load(result.body or "{}")

	return body
end


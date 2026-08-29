
local wstr=require("wetgenes.string")

local M={}

-- { name , default , help , description }


M.doublewrap=function(tab,width,maxleft,append)
	local append=append or " "
	local appendblank=string.rep(" ",#append)
	local lines={}
	local push=function(...) for i,v in ipairs({...}) do lines[#lines+1]=v end end

	local w1=0
	for i,v in ipairs(tab) do -- find how much space the left needs
		local w=#(v[1] or "")
		if w>w1 then w1=w end
	end
	if w1 > maxleft-#append then w1=maxleft-#append end --left hand width maximum
	local w2=width-w1 -- right hand width

	for i,v in ipairs(tab) do -- wrap each column and merge
		if not v[1] and v[2] then -- fullwrap
			for line in v[2]:gmatch("[^\n]*\n?") do
				if line~="" then
					lines[#lines+1]=line:match("[^\n]*")
				end
			end
		else
			local ls1=wstr.smart_wrap(v[1] or "",w1)
			local ls2=wstr.smart_wrap(v[2] or "",w2)
			local idx=1
			while ls1[idx] or ls2[idx] do -- merge
				
				local line=(ls1[idx] or "")
				if #line<w1 then -- pad
					line=line..string.rep(" ",w1-#line)
				end
				if idx==1 then
					line=line..append
				else
					line=line..appendblank
				end
				line=line..(ls2[idx] or "")
				
				lines[#lines+1]=line
			
				idx=idx+1
			end
		end
	end


	return lines
end

-- the parts may be rejoined eg the following is always true
-- tab.path == tab.dirname..tab.basename..tab.extension == tab.dirname..tab.namename
-- tab.filename == tab.basename..tab.extension
-- so dirname will contain a trailing / and extension will begin with a .
M.splitpath=function(path)
	local tab={}
	tab.path=path:gsub("\\","/") -- remove windows \
	tab.dirname,tab.filename=tab.path:match("^(.-)([^\\/]*)$")
	tab.basename,tab.extension=tab.filename:match("^(.+)(%.[^%.]+)$")
	tab.filename=tab.filename or "" -- make sure we have empty strings rather than nil
	tab.basename=tab.basename or tab.filename -- fix case of no "." in filename
	tab.dirname=tab.dirname or ""
	tab.extension=tab.extension or ""
	return tab
end


M.bake=function(args)

	args=args or {} -- bound state
	
	args.doublewrap=M.doublewrap
	args.splitpath=M.splitpath
	
	args.new_inputs=function(args,_inputs)
		args.inputs=args.inputs or {}
		for i,v in ipairs(_inputs or {}) do
			local it={}
			it.name=v[1]
			it.default=v[2]
			it.type=type(v[2])
			if it.type=="table" then -- can be multiple values
				it.default=v[2][1]
				it.type=type(v[2][1])
				it.list=v[2]
			end
			it.help=v[3] or ""
			it.description=v[4] -- optional preformated text
			args.inputs[it.name]=it
		end
		args.list={}
		for n,v in pairs(args.inputs) do
			args.list[#args.list+1]=v
		end
		table.sort(args.list,function(a,b)
			local ta,tb=type(a.name),type(b.name)
			if ta=="string" and tb=="string" then
				return a.name<b.name
			elseif ta=="number" and tb=="number" then
				return a.name<b.name
			elseif ta=="number" and tb=="string" then
				return true
			else
				return false
			end
		end)
		return args
	end
	do
		local cmdline=args.inputs
		args.inputs={}
		args:new_inputs( cmdline or {})
	end
	
	args.help=function(args)
		local lines={}
		lines[#lines+1] = ""
		local tab={}
--		for i,v in ipairs(args.list) do
--			if type(v.name)=="number" then -- example
--				lines[#lines+1] = v.default
--				lines[#lines+1] = ""
--				for _,l in ipairs( wstr.smart_wrap( v.help:gsub("\n"," ") , 60 ) ) do
--					lines[#lines+1] = "                "..l
--				end
--				lines[#lines+1] = ""
--				if v.description then
--					tab[#tab+1]={nil,v.description:match("^%s+(.-)%s+$")}
--				end
--			end
--		end
		
		for i,v in ipairs(args.list) do
			if type(v.name)=="string" then -- option
				local secs={ ("--"..v.name) , "" }
				tab[#tab+1]=secs
				if v.default then
					secs[1]=secs[1].."="..tostring(v.default).." "
				end
				if v.list then
					secs[2]="("..table.concat(v.list,"|")..") "..v.help:gsub("\n"," ")
				elseif v.type=="nil" then
					secs[2]=v.help:gsub("\n"," ")
				else
					secs[2]="("..tostring(v.type)..") "..v.help:gsub("\n"," ")
				end
			end
			if v.description then
				tab[#tab+1]={nil,v.description:match("^%s+(.-)%s+$")}
			end
		end
		for i,v in ipairs( args.doublewrap(tab,78,30," : ") ) do lines[#lines+1]=" "..v end
		
		lines[#lines+1] = ""
		return lines
	end
	

-- exit if we do not understand all the opts
	args.sanity=function(args)
		for n,v in pairs(args.data) do --check for bad args
			if type(n)=="string" then -- not the numbers
				local v=args.inputs[n] -- lookup
				if not v then
					print("")
					print("Unknown option --"..n.." , aborting.")
					print("")
					os.exit(0)
				end
			end
		end
		return args
	end

	args.set=function(args,n,v)
		local n=n:lower()
		local p=args.inputs[n]
		args.data[n]=v
		if p and type(v)=="string" then -- convert input strings
			if     p.type=="number" then	args.data[n]=tonumber(v)
			elseif p.type=="boolean" then	args.data[n]=((v~="false") and (v~="off") and (v~="no"))
			end
		end
	end

	args.parse=function(args,cmds)
		arg=arg or {}
		for i=0,#cmds do arg[i]=cmds[i] end
		local data={}
		args.raw=arg
		args.data=data
		args.inputs=args.inputs or {}
		
		for i,v in pairs(args.inputs) do
			if type(v.name)=="string" then data[v.name:lower()]=v.default end
		end
		
		-- perform very simple processing of args to be passed into the command
		local state=false
		for i,v in ipairs(arg) do
			if state=="--" then
				data[#data+1]=v
			elseif v=="--" then state="--" -- we are done stop messing with the rest of the args
			elseif v:sub(1,2)=="--" then -- found a greedy opt, expects to assign a value
				local s,e=v:find("=",1,true)
				if s then -- found a "=" so split and assign
					local a=v:sub(3,s-1)
					local b=v:sub(e+1)
					args:set(a,b)
					state=false
				else -- set to true and try to grab the next value
					state=v:sub(3)
					if state:sub(1,3)=="no-" then
						state=state:sub(4) -- remove "no-" from start of string
						args:set(state,false) -- this is a false flag
						state=false -- and we do not wish to grab the next arg
					else
						args:set(state,true)
						local input=args.inputs[state]
						if input and input.type=="boolean" then state=nil end -- got our bool value already do not grab next arg
					end
				end
			else
				if state then -- want next value
					args:set(state,v)
					state=false
				else
					data[#data+1]=v
				end
			end
		end

		return args
	end
	
	return args
end

return M

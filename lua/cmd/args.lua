
local wstr=require("wetgenes.string")

local M={}

M.doublewrap=function(tab,width,maxleft,append)
	local append=append or " "
	local appendblank=string.rep(" ",#append)
	local lines={}
	local push=function(...) for i,v in ipairs({...}) do lines[#lines+1]=v end end

	local w1=0
	for i,v in ipairs(tab) do -- find how much space the left needs
		local w=#v[1]
		if w>w1 then w1=w end
	end
	if w1 > maxleft-#append then w1=maxleft-#append end --left hand width maximum
	local w2=width-w1 -- right hand width

	for i,v in ipairs(tab) do -- wrap each column and merge
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
		args.inputs=_inputs or args.inputs
		for i,v in ipairs(args.inputs) do
			v.name=v[1]
			v.default=v[2]
			v.type=type(v[2])
			v.help=v[3]
			if type(v.name)=="string" then
				args.inputs[v.name]=v
			end
		end
		table.sort(args.inputs,function(a,b)
			local ta,tb=type(a),type(b)
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
	args:new_inputs()

	args.help=function()
		local lines={}
		lines[#lines+1] = ""
		local tab={}
		for i,v in ipairs(args.inputs) do
			if type(v.name)=="number" then -- example
				lines[#lines+1] = v.default
				lines[#lines+1] = ""
				for _,l in ipairs( wstr.smart_wrap( v.help:gsub("\n"," ") , 60 ) ) do
					lines[#lines+1] = "                "..l
				end
				lines[#lines+1] = ""
			end
		end
		
		for i,v in ipairs(args.inputs) do
			if type(v.name)=="string" then -- option
				tab[#tab+1]={ ("--"..v.name.." ("..tostring(v.type)..")") , v.help:gsub("\n"," ").."\n --"..v.name.."="..tostring(v.default).."" }
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

	args.set=function(n,v)
		local n=n:lower()
		local p=args.inputs[n]
		args.data[n]=v
		if p and type(v)=="string" then -- convert input strings
			if     p.type=="number" then	args.data[n]=tonumber(v)
			elseif p.type=="boolean" then	args.data[n]=((v~="false") and (v~="off") and (v~="no"))
			end
		end
	end

	args.parse=function(args,arg)
		arg=arg or {}
		local data={}
		args.raw=arg
		args.data=data
		args.inputs=args.inputs or {}
		
		for i,v in ipairs(args.inputs) do
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
					args.set(a,b)
					state=false
				else -- set to true and try to grab the next value
					state=v:sub(3)
					if state:sub(1,3)=="no-" then
						state=state:sub(4) -- remove "no-" from start of string
						args.set(state,false) -- this is a false flag
						state=false -- and we do not wish to grab the next arg
					else
						args.set(state,true)
						local input=args.inputs[state]
						if input and input.type=="boolean" then state=nil end -- got our bool value already do not grab next arg
					end
				end
			else
				if state then -- want next value
					args.set(state,v)
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

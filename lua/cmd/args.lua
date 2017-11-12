
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
			
			line=(ls1[idx] or "")
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


M.bake=function(args)

	args=args or {} -- bound state
	
	args.doublewrap=M.doublewrap
	
	args.new_inputs=function(args,_inputs)
		args.inputs=_inputs or args.inputs
		for i,v in ipairs(args.inputs) do
			v.name=v[1]
			v.default=v[2]
			v.type=type(v[2])
			v.help=v[3]
			args.inputs[v.name]=v
		end
		table.sort(args.inputs,function(a,b) return a.name<b.name end)
		return args
	end
	args:new_inputs()

	args.help=function()
		local tab={}
		for i,v in ipairs(args.inputs) do
			tab[i]={ ("--"..v.name.." ("..tostring(v.type)..")") , v.help.." ("..tostring(v.default)..")" }
		end
		local lines=args.doublewrap(tab,78,30," : ")
		for i,v in ipairs(lines) do lines[i]="  "..v end -- pad
		return lines
	end
	
	

	args.parse=function(args,arg)
		local data={}
		args.raw=arg
		args.data=data
		
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
					args.data[a]=b
					state=false
				else -- set to true and try to grab the next value
					state=v:sub(3)
					if state:sub(1,3)=="no-" then
						state=state:sub(4) -- remove "no-" from start of string
						args.data[state]=false -- this is a false flag
						state=false -- and we do not wish to grab the next arg
					else
						args.data[state]=true
					end
				end
--[[
			elseif v:sub(1,1)=="+" then -- a non greedy bool, set to true
				arg[v:sub(2)]=true
				state=false
			elseif v:sub(1,1)=="-" then -- a non greedy bool, set to false
				arg[v:sub(2)]=false
				state=false
]]
			else
				if state then -- want next value
					args.data[state]=v
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

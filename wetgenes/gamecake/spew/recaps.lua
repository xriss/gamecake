-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(state,recaps)

	recaps=recaps or {} 
	
	local cake=state.cake
	local canvas=cake.canvas
	
	function recaps.setup()
	end

	function recaps.clean()
	end

	function recaps.update()	
	end
	
	function recaps.draw()
	end
		
	function recaps.msg(m)
	end

-- the above are just stubs "incase", most of the meat happens in the recap table


-- create a new recap table, then we can load or save this data to or from our server
	function recaps.create()
		local recap={}
		

		function recap.reset()
			recap.flow="record" -- record by default
			recap.last={}
			recap.now={}
			recap.stream={} -- a stream of change "table"s or "number" frame skips
			recap.frame=0
			recap.read=0
			recap.wait=0
		end
		
		function recap.set(nam,dat)
			recap.now[nam]=dat
		end
		function recap.now(nam) -- return now or last frame data whatever we have
			local v=recap.now[nam]
			if type(v)=="nil" then v=recap.last[nam] end -- now probably only contains recent changes
			return v
		end
		function recap.get(nam) -- return "last" frame data not this frame data
			return recap.last[nam]
		end
		
-- use this to set button flags, that may trigger a set/clr extra state
-- also call once a frame with no value to clear these extra states
		function recap.but(nam,v)
			if (type(v)=="nil") then -- call with no value to clear the set/clr change states
				recap.set(nam.."_set",false)
				recap.set(nam.."_clr",false)
			end
			local l=recap.now(nam)
			if v then -- set
				if not l then -- change?
					recap.set(nam,true)
					recap.set(nam.."_set",true)
				end
			else -- clr
				if l then -- change?
					recap.set(nam,false)
					recap.set(nam.."_clr",true)
				end
			end
		end


		function recap.step()
			
			if recap.flow=="record" then
				local change
				for n,v in pairs(recap.now) do
					if recap.last[n]~=v then -- changes
						change=change or {}
						change[n]=v
						recap.last[n]=v
						recap.now[n]=nil -- from now on we get the value from the last table
					end
				end
				if change then
					table.insert(recap.stream,change) -- change something
				else
					if type(recap.stream[#recap.stream])=="number" then
						recap.stream[#recap.stream] = recap.stream[#recap.stream] + 1 -- keep on changing nothing
					else
						table.insert(recap.stream,1) -- change nothing
					end
				end
				
			elseif recap.flow=="play" then -- grab from the stream
			
				if recap.wait>0 then
				
					recap.wait=recap.wait-1
					
				else
				
					recap.read=recap.read+1
					
					local t=recap.stream[recap.read]
					local tt=type(t)
					
					if tt=="number" then
					
						recap.wait=t-1

					elseif tt=="table"then
					
						for n,v in pairs(t) do
							recap.last[n]=v
							recap.now[n]=v
						end
					
					end
				end
			
			else -- default of do not record do not play just be
			
				for n,v in pairs(recap.now) do
					if recap.last[n]~=v then
						recap.last[n]=v
						recap.now[n]=nil
					end
				end
				
			end
			
			recap.frame=recap.frame+1 -- advance frame counter
		end
		
		recap.reset()

		return recap
	end


	return recaps
end

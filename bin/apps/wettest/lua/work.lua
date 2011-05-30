
local work_lanes_version=16 -- bump this version number to live update running threads
local work_lanes_disable=false -- do not use lanes if set, block the main thread for easier debuging



local table=table
local ipairs=ipairs
local math=math
local string=string
local print=print
local coroutine=coroutine
local require=require

local lanes=require("lanes")

local worker=require("worker")

local wetlua=wetlua

module(...)


-----------------------------------------------------------------------------
--
-- create data.lanes table and threads if it isnt already setup
--
-----------------------------------------------------------------------------
function lanes_setup()

	msgid=msgid or 1 -- msg id, use and inc on each send
	rets=rets or {} -- table of return values, slot filed is msgid, just busy wait for results
	threads=threads or {} -- our threads
	
	local need_new_workers=false
	
	if work_lanes_disable then -- stop here, we are not using lanes
		state="working"
		return
	end

	
local function create_new_thread(i)
	threads[i]={}
	threads[i].linda=lanes.linda()
	threads[i].version=work_lanes_version
	local worker=lanes.gen("*",{["globals"]={wetlua=wetlua}}, worker.lanes_worker )
	threads[i].worker=worker(threads[i].linda,i)

end
	
	for i=1,1 do
	
		if not threads[i] then -- create new
		
			create_new_thread(i)
		
		else -- check old
				
			if	threads[i].worker.status~="done" and 
				threads[i].worker.status~="error" and 
				threads[i].worker.status~="cancelled" then
						
				if threads[i].version~=work_lanes_version then -- this old running thread is a bad version
				
					need_new_workers=true
					
				end
			
			else -- replace dead thread
			
				create_new_thread(i)
			
			end
		end
		
	end
		
--	update=lanes_update
	
	if need_new_workers then -- kill all old threads and start new threads
	
		lanes_refresh()
			
	else
	
		state="working"
	
	end
end


-----------------------------------------------------------------------------
--
-- tell worker threads to shutdown and start again
--
-----------------------------------------------------------------------------
function lanes_refresh()

		lanes_clean()		
		state="refreshing"
end

-----------------------------------------------------------------------------
--
-- tell worker threads to shutdown
--
-----------------------------------------------------------------------------
function lanes_clean()

	state="cleaning"

	if not work_lanes_disable then
	
		for i,v in ipairs(threads) do
		
			lanes_send({cmd="end"},i)
			
		end
	end
end

-----------------------------------------------------------------------------
--
-- get a uniqueish msg id for comms
--
-----------------------------------------------------------------------------
function lanes_get_new_msgid()

	msgid=msgid+1
	if msgid > 65535 then msgid=0 end -- do not let numbers spiral out of control
	rets[msgid]=nil -- maybe something got stuck
	
	return msgid
	
end

-----------------------------------------------------------------------------
--
-- send a msg to a worker
--
-----------------------------------------------------------------------------
function lanes_send(msg,idx)

	msg.id=lanes_get_new_msgid() -- where to store a reply, 
	
	if not work_lanes_disable then
		threads[idx].linda:send(nil,idx,msg)
	
	end
	
end

-----------------------------------------------------------------------------
--
-- check the state of workers
--
-----------------------------------------------------------------------------
function lanes_update()

--dbg("searching for msg\n")
	if state=="working" then
		local msg
		
		if not work_lanes_disable then

			for i,v in ipairs(threads) do -- we shouldnt share lindas amongst worker threads, it seems to confuse things, a bug no doubt
			
				repeat
				
					msg=threads[i].linda:receive( 0 , 0 ) -- check for returned msgs, but do not block
					
					if msg then
						rets[msg.id]=msg -- store msg in rets table, there are polling waits on these
					end
					
				until not msg
				
			end
		
		end
	
	elseif state=="cleaning" or state=="refreshing" then
	
-- check threads for ending

	local finished=true

		if not work_lanes_disable then
		
			for i,v in ipairs(threads) do
			
				if	v.worker.status~="done" and 
					v.worker.status~="error" and 
					v.worker.status~="cancelled" then
				
					finished=false
				
				end
			
			end
		
		end
		
		if finished then
		
			if state=="cleaning" then -- just shutting down
			
				state="cleaned"
				
			elseif state=="refreshing" then -- reloading
			
				state="cleaned"
				
				lanes_setup()
			end
			
		end
	
	end

end


-----------------------------------------------------------------------------
--
-- send a url request to a worker thread and return the result
-- use yield to wait and poll, this should probably be upgraded to use a wakeup signal
--
-----------------------------------------------------------------------------
function get_url(url)

local msg={}
local ret

	msg.cmd="url"
	msg.url=url
	if spew_lanes_disable then
	
		return lanes_url_worker(nil,msg)
	
	else	
	
		lanes_send(msg,1)
		
--print("waiting for "..msg.id.."\n")
		repeat coroutine.yield() until rets[msg.id] -- poll wait for return value
--print("got "..msg.id.."\n")

		ret=rets[msg.id]
		rets[msg.id]=nil
	
		return ret.ret
	end
	
	
end

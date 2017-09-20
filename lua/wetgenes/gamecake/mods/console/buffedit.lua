--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- a 1 line buffer edit, how you display it is up to you
-- has a simple history, just pass in key presses
-- this is intended for commandline style editing


module("wetgenes.gamecake.mods.console.buffedit")

function keypress(it,ascii,key,act)

	if ascii then 
		local c=string.byte(ascii)
		
		if c>=32 and c<128 then -- ascii sanity
		
			if it.line_idx >= #it.line then -- put at end
			
				it.line=it.line..ascii
				it.line_idx=#it.line
				
			elseif it.line_idx == 0 then -- put at start
			
				it.line=ascii..it.line
				it.line_idx=it.line_idx+1
				
			else -- need to insert into line
			
				it.line=it.line:sub(1,it.line_idx) .. ascii .. it.line:sub(it.line_idx+1)
				it.line_idx=it.line_idx+1
				
			end
			
			it.throb=255
			
		end
	elseif act==1 or act==0 then
	
		if key=="left" then

			it.line_idx=it.line_idx-1
			if it.line_idx<0 then it.line_idx=0 end
			
			it.throb=255
						
		elseif key=="right" then
	
			it.line_idx=it.line_idx+1
			if it.line_idx>#it.line then it.line_idx=#it.line end
			
			it.throb=255
			
		elseif key=="home" then
		
			it.line_idx=0
		
		elseif key=="end" then
		
			it.line_idx=#it.line
		
		elseif key=="back" then
	
			if it.line_idx >= #it.line then -- at end
			
				it.line=it.line:sub(1,-2)
				it.line_idx=#it.line
			
			elseif it.line_idx == 0 then -- at start
			
			elseif it.line_idx == 1 then -- near start
			
				it.line=it.line:sub(2)
				it.line_idx=it.line_idx-1
			
			else -- somewhere in the line
			
				it.line=it.line:sub(1,it.line_idx-1) .. it.line:sub(it.line_idx+1)
				it.line_idx=it.line_idx-1
				
			end
			
			it.throb=255
			
		elseif key=="delete" then
	
			if it.line_idx >= #it.line then -- at end
			

			elseif it.line_idx == 0 then -- at start
			
				it.line=it.line:sub(2)
				it.line_idx=0
			
			else -- somewhere in the line
			
				it.line=it.line:sub(1,it.line_idx) .. it.line:sub(it.line_idx+2)
				it.line_idx=it.line_idx
				
			end
			
			it.throb=255
			
		elseif key=="enter" or key=="return" then
		
			if act==1 then -- ignore repeats on enter key
			
				local f=it.line
--				fenestra._g.print(">"..f)
				
				table.insert(it.history,it.line)
				
				while #it.history > it.history_max do
					table.remove(it.history,1)
				end
		
				it.history_idx=#it.history+1
			
				it.line=""
				it.line_idx=0
				
				if f and it.enter then -- callback?
				
					it:enter(f)
					
				end
				
			end
			
		elseif key=="up" then
		
			it.history_idx=it.history_idx-1
			if it.history_idx<0 then it.history_idx=#it.history end
			it.line=it.history[it.history_idx] or ""
			it.line_idx=#it.line
		
		elseif key=="down" then
		
			it.history_idx=it.history_idx+1
			if it.history_idx>#it.history then it.history_idx=0 end
			it.line=it.history[it.history_idx] or ""
			it.line_idx=#it.line
			
		end
	end
	
	return true

end


function update(it)

	it.throb=it.throb-4
	if it.throb<0 then it.throb=255 end

end


function create()

local it={}

	it.history={}
	it.history_idx=0
	it.history_max=64
	it.line=""
	it.line_idx=0

	it.throb=255
	
	it.keypress=keypress
	it.update=update

	return it
end

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- widget class string
-- a one line string buffer that can be edited




module("fenestra.widget.textedit")

local widget_data=require("fenestra.widget.data")


function mouse(widget,act,x,y,key)

--	local it=widget.string

-- call here so we can use any state changes immediatly	
	local ret=widget.meta.mouse(widget,act,x,y,key)
	
	if widget.master.active==widget then
	
		widget.master.focus=widget
		
		if act=="down" then
			local dx=x-((widget.pxd or 0)+(widget.text_x or 0))
--print(dx)
			if dx<0 then -- catch lessthan
				widget.data.str_idx=0
			else
				widget.data.str_idx=widget.master.font.which(dx,widget.data.str)
				if widget.data.str_idx<0 then widget.data.str_idx=#widget.data.str end -- catch morethan
			end

			widget.master.throb=255
			widget:set_dirty()

		end
	end
	
	return ret
end


function key(widget,ascii,key,act)
--	local it=widget.string
	local master=widget.master
	
	local changed=false

--print("gotkey",ascii)
	
	if act=="down" or act=="repeat" then
	
		if key=="left" then

			widget.data.str_idx=widget.data.str_idx-1
			if widget.data.str_idx<0 then widget.data.str_idx=0 end
			
			master.throb=255
			changed=true
						
		elseif key=="right" then
	
			widget.data.str_idx=widget.data.str_idx+1
			if widget.data.str_idx>#widget.data.str then widget.data.str_idx=#widget.data.str end
			
			master.throb=255
			changed=true
			
		elseif key=="home" then
		
			widget.data.str_idx=0
			changed=true
		
		elseif key=="end" then
		
			widget.data.str_idx=#widget.data.str
			changed=true
		
		elseif key=="backspace" then
	
			if widget.data.str_idx >= #widget.data.str then -- at end
			
				widget.data.str=widget.data.str:sub(1,-2)
				widget.data.str_idx=#widget.data.str
				
				changed=true
			
			elseif widget.data.str_idx < 1 then -- at start
			
			elseif widget.data.str_idx == 1 then -- near start
			
				widget.data.str=widget.data.str:sub(2)
				widget.data.str_idx=widget.data.str_idx-1
			
				changed=true

			else -- somewhere in the line
			
				widget.data.str=widget.data.str:sub(1,widget.data.str_idx-1) .. widget.data.str:sub(widget.data.str_idx+1)
				widget.data.str_idx=widget.data.str_idx-1
				
				changed=true

			end
			
			master.throb=255
			
		elseif key=="delete" then
	
			if widget.data.str_idx >= #widget.data.str then -- at end
			
			elseif widget.data.str_idx < 1 then -- at start
			
				widget.data.str=widget.data.str:sub(2)
				widget.data.str_idx=0
			
				changed=true

			else -- somewhere in the line
			
				widget.data.str=widget.data.str:sub(1,widget.data.str_idx) .. widget.data.str:sub(widget.data.str_idx+2)
				widget.data.str_idx=widget.data.str_idx
				
				changed=true

			end
			
			master.throb=255
			
		elseif key=="enter" or key=="return" then
		
			if act=="down" then -- ignore repeats on enter key
			
				if widget.data.str and widget.onenter then -- callback?
				
					widget:call_hook("click")
					
				end
				
				changed=true
			end
			
--		elseif key=="up" then
--		elseif key=="down" then
		
		elseif ascii~="" then -- not a blank string
			local c=string.byte(ascii)
			
			if c>=32 and c<128 then
			
				if widget.data.str_idx >= #widget.data.str then -- put at end
				
					widget.data.str=widget.data.str..ascii
					widget.data.str_idx=#widget.data.str
					
				elseif widget.data.str_idx < 1 then -- put at start
				
					widget.data.str=ascii..widget.data.str
					widget.data.str_idx=1
					
				else -- need to insert into line
				
					widget.data.str=widget.data.str:sub(1,widget.data.str_idx) .. ascii .. widget.data.str:sub(widget.data.str_idx+1)
					widget.data.str_idx=widget.data.str_idx+1
					
				end
				
				master.throb=255
				
				changed=true

			end
		end
	end
	
	if changed then
		widget.text=widget.data.str
		
		widget:call_hook("update")
		widget:set_dirty()
	end
	
	return true

end


function update(widget)

	if widget.text ~= widget.data.str then
		widget.text = widget.data.str
		widget:set_dirty()
	end
end


function setup(widget,def)
--	local it={}
--	widget.string=it
	widget.class="textedit"
	
	widget.data=widget.data or widget_data.new_data({})
	
--	widget.data.str=""
--	widget.data.str_idx=0
	
--	widget.key=key
	widget.update=update

	widget.key=key
	widget.mouse=mouse

	return widget
end

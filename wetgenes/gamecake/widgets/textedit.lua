--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- widget class string
-- a one line string buffer that can be edited




--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wtextedit)
wtextedit=wtextedit or {}

local cake=oven.cake

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")


function wtextedit.mouse(widget,act,x,y,key)

--	local it=widget.string

-- call here so we can use any state changes immediatly	
	local ret=widget.meta.mouse(widget,act,x,y,key)
	
	if widget.master.active==widget then
	
		widget.master.focus=widget
		
		if act==1 then
			local dx=x-((widget.pxd or 0)+(widget.text_x or 0))
--print(dx)
			if dx<0 then -- catch lessthan
				widget.data.str_idx=0
			else
				local f=widget.font or widget.master.font or 1
				local font=cake.fonts.get(f)

				widget.data.str_idx=0 -- font.which(dx,widget.data.str) // TODO: fix position under mouse
				
				if widget.data.str_idx<0 then widget.data.str_idx=#widget.data.str end -- catch morethan
			end

			widget.master.throb=255
			widget:set_dirty()

		end
	end
	
	return ret
end


function wtextedit.key(widget,ascii,key,act)
--	local it=widget.string
	local master=widget.master
	
	local changed=false

--print("gotkey",ascii,act)
	
	
	if act==-1 then

		if key=="enter" or key=="return" then
		
			if widget.data.str and widget.onenter then -- callback?
			
				widget:call_hook("click")
				
			end
			
			master.focus=nil

			changed=true
		end

	elseif act==1 or act==0 then
	
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
		
		elseif key=="back" then
	
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
	
		if widget.data.class=="number" then
			local num=widget.data.num
			if widget.data.str=="" then num=0 end
			num=tonumber(widget.data.str) or num
			widget.data:value(num)
			widget.data.str=widget.data:get_string()
		end
	
	
		widget.text=widget.data.str
		
		widget:call_hook("update")
		widget:set_dirty()
	end
	
	return true

end


function wtextedit.update(widget)

	if widget.text ~= widget.data.str then
		widget.text = widget.data.str
		widget:set_dirty()
	end

	return widget.meta.update(widget)
end


function wtextedit.setup(widget,def)
--	local it={}
--	widget.string=it
	widget.class="textedit"
	
	widget.data=widget.data or widget_data.new_data({})
	
--	widget.data.str=""
--	widget.data.str_idx=0
	
--	widget.key=key
	widget.update=wtextedit.update

	widget.key=wtextedit.key
	widget.mouse=wtextedit.mouse
	
	widget.solid=true

	widget.can_focus=true

	return widget
end

return wtextedit
end

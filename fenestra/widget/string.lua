-- widget class string
-- a one line string buffer that can be edited



local require=require
local print=print

module("fenestra.widget.string")

local string=require("string")
local table=require("table")

function mouse(widget,act,x,y,key)

	local it=widget.string

-- call here so we can use any state changes immediatly	
	local ret=widget.meta.mouse(widget,act,x,y,key)
	
	if widget.master.active==widget then
	
		widget.master.focus=widget
		
		if act=="down" then
			local dx=x-(widget.px+widget.text_x)
--print(dx)
			if dx<0 then -- catch lessthan
				it.line_idx=0
			else
				it.line_idx=widget.master.font.which(dx,it.line)
				if it.line_idx<0 then it.line_idx=#it.line end -- catch morethan
			end
		end
	end
	
	return ret
end


function key(widget,ascii,key,act)
	local it=widget.string
	local master=widget.master
	
	local changed=false

--print("gotkey",ascii)
	
	if act=="down" or act=="repeat" then
	
		if key=="left" then

			it.line_idx=it.line_idx-1
			if it.line_idx<0 then it.line_idx=0 end
			
			master.throb=255
						
		elseif key=="right" then
	
			it.line_idx=it.line_idx+1
			if it.line_idx>#it.line then it.line_idx=#it.line end
			
			master.throb=255
			
		elseif key=="home" then
		
			it.line_idx=0
		
		elseif key=="end" then
		
			it.line_idx=#it.line
		
		elseif key=="backspace" then
	
			if it.line_idx >= #it.line then -- at end
			
				it.line=it.line:sub(1,-2)
				it.line_idx=#it.line
				
				changed=true
			
			elseif it.line_idx < 1 then -- at start
			
			elseif it.line_idx == 1 then -- near start
			
				it.line=it.line:sub(2)
				it.line_idx=it.line_idx-1
			
				changed=true

			else -- somewhere in the line
			
				it.line=it.line:sub(1,it.line_idx-1) .. it.line:sub(it.line_idx+1)
				it.line_idx=it.line_idx-1
				
				changed=true

			end
			
			master.throb=255
			
		elseif key=="delete" then
	
			if it.line_idx >= #it.line then -- at end
			
			elseif it.line_idx < 1 then -- at start
			
				it.line=it.line:sub(2)
				it.line_idx=0
			
				changed=true

			else -- somewhere in the line
			
				it.line=it.line:sub(1,it.line_idx) .. it.line:sub(it.line_idx+2)
				it.line_idx=it.line_idx
				
				changed=true

			end
			
			master.throb=255
			
		elseif key=="enter" or key=="return" then
		
			if act=="down" then -- ignore repeats on enter key
			
				if it.line and it.onenter then -- callback?
				
					widget:call_hook("click")
					
				end
				
			end
			
--		elseif key=="up" then
--		elseif key=="down" then
		
		elseif ascii~="" then -- not a blank string
			local c=string.byte(ascii)
			
			if c>=32 and c<128 then
			
				if it.line_idx >= #it.line then -- put at end
				
					it.line=it.line..ascii
					it.line_idx=#it.line
					
				elseif it.line_idx < 1 then -- put at start
				
					it.line=ascii..it.line
					it.line_idx=1
					
				else -- need to insert into line
				
					it.line=it.line:sub(1,it.line_idx) .. ascii .. it.line:sub(it.line_idx+1)
					it.line_idx=it.line_idx+1
					
				end
				
				master.throb=255
				
				changed=true

			end
		end
	end
	
	if changed then
		widget.text=it.line
		
		widget:call_hook("update")
	end
	
	return true

end


function update(widget)
end


function setup(widget,def)
	local it={}
	widget.string=it
	widget.class="string"
	
	it.line=""
	it.line_idx=0
	
--	it.key=key
	it.update=update

	widget.key=key
	widget.mouse=mouse

	return widget
end

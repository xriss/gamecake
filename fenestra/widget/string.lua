-- widget class string
-- a one line string buffer that can be edited



local require=require


module("fenestra.widget.string")

local string=require("string")
local table=require("table")



function keypress(widget,ascii,key,act)
	local it=widget.string
	
	if act=="down" or act=="repeat" then
	
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
		
		elseif key=="backspace" then
	
			if it.line_idx >= #it.line then -- at end
			
				it.line=it.line:sub(1,-2)
				it.line_idx=#it.line
			
			elseif it.line_idx < 1 then -- at start
			
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
			
			elseif it.line_idx < 1 then -- at start
			
				it.line=it.line:sub(2)
				it.line_idx=0
			
			else -- somewhere in the line
			
				it.line=it.line:sub(1,it.line_idx) .. it.line:sub(it.line_idx+2)
				it.line_idx=it.line_idx
				
			end
			
			it.throb=255
			
		elseif key=="enter" or key=="return" then
		
			if act=="down" then -- ignore repeats on enter key
			
				if it.line and it.enter then -- callback?
				
					it:enter(it.line)
					
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
				
				it.throb=255
				
			end
		end
	end
	
	return true

end


function update(widget)
	local it=widget.string
	
	it.throb=it.throb-4
	if it.throb<0 then it.throb=255 end

end




function setup(widget,def)
	local it={}
	widget.string=it
	widget.class="string"
	
	it.line=""
	it.line_idx=0
	it.throb=255
	
	it.keypress=keypress
	it.update=update


	return widget
end

-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- widget class string
-- a one line string buffer that can be edited




module("fenestra.widget.textedit")



function keypress(widget,ascii,key,act)
--	local it=widget.textedit
	
	if act=="down" or act=="repeat" then
	
		if key=="left" then

			widget.line_idx=widget.line_idx-1
			if widget.line_idx<0 then widget.line_idx=0 end
			
			widget.throb=255
						
		elseif key=="right" then
	
			widget.line_idx=widget.line_idx+1
			if widget.line_idx>#widget.line then widget.line_idx=#widget.line end
			
			widget.throb=255
			
		elseif key=="home" then
		
			widget.line_idx=0
		
		elseif key=="end" then
		
			widget.line_idx=#widget.line
		
		elseif key=="backspace" then
	
			if widget.line_idx >= #widget.line then -- at end
			
				widget.line=widget.line:sub(1,-2)
				widget.line_idx=#widget.line
			
			elseif widget.line_idx < 1 then -- at start
			
			elseif widget.line_idx == 1 then -- near start
			
				widget.line=widget.line:sub(2)
				widget.line_idx=widget.line_idx-1
			
			else -- somewhere in the line
			
				widget.line=widget.line:sub(1,widget.line_idx-1) .. widget.line:sub(widget.line_idx+1)
				widget.line_idx=widget.line_idx-1
				
			end
			
			widget.throb=255
			
		elseif key=="delete" then
	
			if widget.line_idx >= #widget.line then -- at end
			
			elseif widget.line_idx < 1 then -- at start
			
				widget.line=widget.line:sub(2)
				widget.line_idx=0
			
			else -- somewhere in the line
			
				widget.line=widget.line:sub(1,widget.line_idx) .. widget.line:sub(widget.line_idx+2)
				widget.line_idx=widget.line_idx
				
			end
			
			widget.throb=255
			
		elseif key=="enter" or key=="return" then
		
			if act=="down" then -- ignore repeats on enter key
			
				if widget.line and widget.enter then -- callback?
				
					widget:enter(widget.line)
					
				end
				
			end
			
--		elseif key=="up" then
--		elseif key=="down" then
		
		elseif ascii~="" then -- not a blank string
			local c=string.byte(ascii)
			
			if c>=32 and c<128 then
			
				if widget.line_idx >= #widget.line then -- put at end
				
					widget.line=widget.line..ascii
					widget.line_idx=#widget.line
					
				elseif widget.line_idx < 1 then -- put at start
				
					widget.line=ascii..widget.line
					widget.line_idx=1
					
				else -- need to insert into line
				
					widget.line=widget.line:sub(1,widget.line_idx) .. ascii .. widget.line:sub(widget.line_idx+1)
					widget.line_idx=widget.line_idx+1
					
				end
				
				widget.throb=255
				
			end
		end
	end
	
	return true

end


function update(widget)
	local widget=widget.textedwidget
	
	widget.throb=widget.throb-4
	if widget.throb<0 then widget.throb=255 end

end




function setup(widget,def)
--	local it={}
--	widget.textedit=it
	widget.class="textedit"
	
	it.lines=""
	it.line_idx=0
	it.lines_idx=0
	it.throb=255
	
	it.keypress=keypress
	it.update=update


	return widget
end

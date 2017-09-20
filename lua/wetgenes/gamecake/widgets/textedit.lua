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

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

function wtextedit.mouse(widget,act,_x,_y,key)

	local x,y=widget:mousexy(_x,_y)

--print(act,x,y,key)

	widget.meta.mouse(widget,act,_x,_y,key)

--	local it=widget.string

	if widget.master.over==widget or act==-1 then

--		widget.master.set_focus(widget)
		
		local function get_idx()
			local idx=0
			local dx=x-(widget.text_x or 0)
			if dx<0 then -- catch lessthan
				idx=0
			else
				local f=widget.font or widget.master.font or 1
				local dat=cake.fonts.get(f)
				local size=widget:bubble("text_size") or 16

--				widget.data.str_idx=0 -- font.which(dx,widget.data.str) // TODO: fix position under mouse
				
				idx=cake.canvas.font.xindex(widget.data.str,dx,dat,size,0)
				
			end
			
			if idx<0 then idx=0 end
			if idx>#widget.data.str then idx=#widget.data.str end

			return idx
		end
		
		if act==1 then
			widget.mouse_down=true
			
			widget.data.str_select=0
			widget.data.str_idx=get_idx()
			widget.data.str_select_click=widget.data.str_idx

			widget.master.throb=255
			widget:set_dirty()

		elseif act==0 then -- drag

			if widget.mouse_down and widget.data.str_select_click then
			
				widget.data.str_idx=get_idx()
			
				widget.data.str_select = widget.data.str_select_click - widget.data.str_idx
			
				widget:set_dirty()
				
			end
		
		elseif act==-1 then
			widget.mouse_down=false
		end
	end

--print("","OVER",widget.data.str_select,widget.data.str_idx,widget.data.str_select_click)


end


function wtextedit.key(widget,ascii,key,act)
--	local it=widget.string
	local master=widget.master
	
	local changed=false

--print("gotkey",ascii,act)
	
	
	local function clear_selected_chars()
		if widget.data.str_select~=0 then
			if  widget.data.str_select<0 then
				local s1=widget.data.str:sub(1,widget.data.str_idx+widget.data.str_select)
				local s2=widget.data.str:sub(widget.data.str_idx+1)
				widget.data.str=s1..s2
				widget.data.str_idx=widget.data.str_idx + widget.data.str_select
			elseif  widget.data.str_select>0 then
				local s1=widget.data.str:sub(1,widget.data.str_idx)
				local s2=widget.data.str:sub(widget.data.str_idx+widget.data.str_select+1)
				widget.data.str=s1..s2
			end
			widget.data.str_select=0
		end
	end
	
	if act==1 then
	
		if key=="enter" or key=="return" or key=="tab" then
		
			if widget.data.str and widget.onenter then -- callback?
			
				widget:call_hook_later("click")
				
			end
			
			if key=="tab" then -- tab to the next textedit we can find
			
				local n,nf,nn
				local f
				f=function(w)
					if nn and n then return end
					if w.class=="textedit" and not w.hidden then
						nf=nf or w
						if nn then n=w end
					end
					for i,v in ipairs(w) do
						f(v)
					end
					if w==master.focus then nn=true end
				end
				f(master)
--				print("found",n,nf,nn)
				n=n or nf
				master.set_focus(n)
				master.set_focus_edit(n)
				
				if n then wtextedit.select_all(n) end

			else
				master.set_focus(nil)
				master.set_focus_edit(nil)
			end
			
			changed=true
			
		end
		
	end
	
	if ascii and ascii~="" then -- not a blank string

		local c=string.byte(ascii)
		
		if c>=32 and c<128 then
		
			clear_selected_chars()

			if widget.data.str_idx >= #widget.data.str then -- put at end
			
				widget.data.str=widget.data.str..ascii
				widget.data.str_idx=#widget.data.str
				widget.data.str_select=0
				
			elseif widget.data.str_idx < 1 then -- put at start
			
				widget.data.str=ascii..widget.data.str
				widget.data.str_idx=1
				widget.data.str_select=0
				
			else -- need to insert into line
			
				widget.data.str=widget.data.str:sub(1,widget.data.str_idx) .. ascii .. widget.data.str:sub(widget.data.str_idx+1)
				widget.data.str_idx=widget.data.str_idx+1
				widget.data.str_select=0
				
			end
			
			master.throb=255
			
			changed=true

		end

	elseif act==1 or act==0 then
	
		if key=="left" then

			widget.data.str_select=0
			widget.data.str_idx=widget.data.str_idx-1
			if widget.data.str_idx<0 then widget.data.str_idx=0 end
			
			master.throb=255
			changed=true
						
		elseif key=="right" then
	
			widget.data.str_select=0
			widget.data.str_idx=widget.data.str_idx+1
			if widget.data.str_idx>#widget.data.str then widget.data.str_idx=#widget.data.str end
			
			master.throb=255
			changed=true
			
		elseif key=="home" then
		
			widget.data.str_select=0
			widget.data.str_idx=0
			changed=true
		
		elseif key=="end" then
		
			widget.data.str_select=0
			widget.data.str_idx=#widget.data.str
			changed=true
		
		elseif key=="back" then
			
			if 	widget.data.str_select~=0 then

				clear_selected_chars()
				changed=true
				
			elseif widget.data.str_idx >= #widget.data.str then -- at end
			
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
	
			if 	widget.data.str_select~=0 then

				clear_selected_chars()
				changed=true
				
			elseif widget.data.str_idx >= #widget.data.str then -- at end
			
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
		end
		
	end
	
	if changed then
	
		widget.master.timehooks[widget]=os.time()+2

--		widget.text=widget.data.str
		
		widget:call_hook_later("changed")
		widget:set_dirty()
	end
	
	return true

end

-- a delayed update action
function wtextedit.timedelay(widget)
--print("timedelay")
	if widget.data then
		if widget.data.class=="number" then
			local num=widget.data:tonumber(widget.data.str)
			widget.data:value(num)
		end
		widget.text=widget.data:tostring(widget.data.num)
	end
	widget:set_dirty()
end


function wtextedit.select_all(widget)
	widget.data.str_idx=0
	widget.data.str_select=#widget.data.str
end

function wtextedit.unfocus(widget)
	if widget.data.class=="number" then
		local num=widget.data:tonumber(widget.data.str)
		widget.data:value(num)
	end

	widget.text=widget.data:tostring(widget.data.num)
	widget:set_dirty()
end

function wtextedit.update(widget)

	if widget.text ~= widget.data.str then
		widget.text = widget.data.str
		widget:set_dirty()
	end

	return widget.meta.update(widget)
end

function wtextedit.class_hooks(hook,widget,dat)

	if hook=="timedelay" then return wtextedit.timedelay(widget) end
	if hook=="unfocus" then return wtextedit.unfocus(widget) end
	
end


function wtextedit.setup(widget,def)
--	local it={}
--	widget.string=it
	widget.class="textedit"
	widget.class_funcs=wtextedit
	widget.class_hooks=wtextedit.class_hooks

	widget.data=widget.data or widget_data.new_data({master=widget.master})
	
--	widget.data.str=""
--	widget.data.str_idx=0
	
--	widget.key=key
	widget.update=wtextedit.update

	widget.key=wtextedit.key
	widget.mouse=wtextedit.mouse

	
--	widget.timedelay=wtextedit.timedelay
--	widget.unfocus=wtextedit.unfocus
		
	widget.solid=true

	widget.can_focus=true
	
	widget.fbo=framebuffers.create(0,0,0) -- need framebuffer for clipping

	return widget
end

return wtextedit
end

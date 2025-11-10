--[[#lua.wetgenes.txt.edit

(C) 2023 Kriss Blank under the https://opensource.org/licenses/MIT

Generic text modifying functions.
  
]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstring=require("wetgenes.string")

local wtsv = require("wetgenes.tsv")
local wutf=require("wetgenes.txt.utf")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local split = function(text,separator)
	separator = separator or "%s+" -- white space default
	
	local idx = 1	-- parts idx
	local parts = {}
	local start = 1
	
	local split_start, split_end = text:find(separator, start)
	
	while split_start do
		if split_start>1 then
			parts[idx]=text:sub(start, split_start-1)
			idx=idx+1
		end
		start = split_end + 1
		split_start, split_end = text:find(separator, start)
	end
	
	if text:sub(start)~="" then
		parts[idx]=text:sub(start)
	end
	
	return parts
end


-- bind an edit state to a txt state
M.construct=function(edit,txt)
	edit = edit or {}
	
	edit.txt=txt
	txt.edit=edit
	
	edit.width=80 -- should be editable


-- select paragraph under cursor , spread up and down until we find a blank line or change of indention
	edit.select_paragraph=function()
	
		local fy=txt.cy
		local ty=fy

		local trim=function(s) return (s:match("^%s*(.-)%s*$")) end
		local pfix=function(s) return (s:match("^%s*")) end

		local l=txt.get_string(fy)
		if l and trim(l)=="" then return txt.mark(fy,1,ty+1,1) end  -- so nothing to select
		local w=pfix(l) -- get white space prefix

-- search up
		repeat
			fy=fy-1
			local done=true
			local l=txt.get_string(fy)
			if l then
				if trim(l)~="" then done=false end
				if pfix(l)~=w  then done=true  end -- not matching white space prefix
			end
		until done
		fy=fy+1

-- search down
		repeat
			ty=ty+1
			local done=true
			local l=txt.get_string(ty)
			if l then
				if trim(l)~="" then done=false end
				if pfix(l)~=w  then done=true  end -- not matching white space prefix
			end
		until done
		ty=ty-1

		txt.mark(fy,1,ty+1,1) -- full lines
	end

-- copy current selection
	edit.copy=function()
		return txt.undo.copy()
	end

-- paste over current selection
	edit.paste=function(s)
		txt.undo.replace(s)
	end

-- wrap paragraph
	edit.justify=function()
		local pfix=function(s) return (s:match("^%s*")) end
		if not txt.marked() then edit.select_paragraph() end
		local s=edit.copy() or ""
		local w=pfix(s) -- get white space prefix
		local line=""
		local t=split(s)
		local a={}
		local ww=0
		for i=1,#w do -- dumb whitespace width
			local a=w:sub(i,i)
			if a=="\t" then
				ww=ww+4 -- tab
			else
				ww=ww+1 -- space
			end
		end
		if ww>40 then ww=40 end -- 50% max
		local width=edit.width-ww
		for _,w in ipairs(t) do
			if #line+#w+1 >= width then -- last part of line
				if line=="" then -- full overflow
					a[#a+1]=w
					line=""
				else
					a[#a+1]=line
					line=w
				end
			else -- just append
				if line~="" then line=line.." " end
				line=line..w
			end
		end
		if line~="" then a[#a+1]=line end
		s=w..table.concat(a,txt.endline..w)..txt.endline
		if s then edit.paste(s) end
		txt.cy,txt.cx=txt.clip_left(txt.cy,txt.cx)
	end

-- align paragraph vertically ( useful for code )
	edit.align=function()
		if not txt.marked() then edit.select_paragraph() end
		local s=edit.copy() or ""
		local ls=split(s,txt.endline)
		local ws={}
		for _,l in ipairs(ls) do -- find widest of each word
			for i,w in ipairs( split(l) ) do
				if not  ws[i] then ws[i]=#w end
				if #w > ws[i] then ws[i]=#w end
			end
		end
		local indent=(ls[1] or ""):sub( 1 , ((ls[1] or ""):find("%S") or 1)-1 )
		local a={}
		for _,l in ipairs(ls) do -- find widest of each word
			a[#a+1]=indent
			for i,w in ipairs( split(l) ) do
				a[#a+1]=w
				a[#a+1]=string.rep(" ",1+ws[i]-#w)
			end
			a[#a+1]=txt.endline
		end
		s=table.concat(a)
		if not ls[1] then s=txt.endline end
		if s then edit.paste(s) end
	end

	return edit
end

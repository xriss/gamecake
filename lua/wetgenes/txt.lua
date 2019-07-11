--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstring=require("wetgenes.string")


-- manage the text data part of a text editor


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.construct=function(txt)
	txt = txt or {}

	txt.hooks={} -- user call backs	
	local hook=function(name) local f=txt.hooks[name] if f then return f(txt) end end
	
	txt.strings={} -- a table of strings per line inclusive of any \n at the end
	
	txt.cx=1 -- cursor location
	txt.cy=1

	txt.hx=0 -- widest string
	txt.hy=0 -- number of strings

	txt.get_string=function(idx)
		return txt.strings[idx]
	end

	txt.set_string=function(idx,str)
		txt.strings[idx]=str
	end

	txt.add_string=function(idx,str)
		table.insert( txt.strings , idx , str)
		txt.hy=txt.hy+1
	end

	txt.del_string=function(idx)
		table.remove( txt.strings , idx )
		txt.hy=txt.hy-1
	end

	txt.set_text=function(text)

		if text then -- set new text
			txt.strings=wstring.split_lines(text)
		end
		
		txt.hx=0
		txt.hy=#txt.strings
		for i,v in ipairs(txt.strings) do
			local lv=#v
			if lv > txt.hx then txt.hx=lv end
		end
		
		hook("changed")
		
	end

	txt.get_text=function()
		return table.concat(txt.strings) or ""
	end


	txt.mark=function(fx,fy,tx,ty)
		if not fx then -- unmark
			txt.fx=nil
			txt.fy=nil
			txt.tx=nil
			txt.ty=nil
			return
		end
		txt.fx,txt.fy=txt.clip(fx,fy)
		txt.tx,txt.ty=txt.clip(tx,ty)
		
		txt.cx,txt.cy=txt.tx,txt.ty
		
		local flip=false
		if txt.fy==txt.ty and txt.fx>txt.tx then flip=true
		elseif                  txt.fy>txt.ty then flip=true end
		if flip then
			txt.fx,txt.tx=txt.tx,txt.fx
			txt.fy,txt.ty=txt.ty,txt.fy
		end

-- print( txt.fx , txt.fy , txt.tx , txt.ty )

	end

	-- cut out the text in the marked area (if marked) and set the cursor to this location
	txt.cut=function()
		
		if txt.fx and txt.fy then
		
			local s

			if txt.tx and txt.ty then
			end
			
			txt.cx=txt.fx
			txt.cy=txt.fy
			
			
			txt.mark()

			return s
		end

	end

	
	txt.get_hx=function(y)
		y=y or txt.cy
		local s=txt.get_string(y)
		local hx=s and #s or 0
		while hx>0 do
			local endswith=s:byte(hx)
			if endswith==10 or endswith==13 then hx=hx-1
			else break end -- ignore any combination of CR or LF at end of line
		end
		if hx > txt.hx then txt.hx=hx end -- fix max
		return hx
	end

	txt.clip=function(x,y)
	
		if not x then
			txt.cx,txt.cy=txt.clip(txt.cx,txt.cy)
			return
		end

		if y>txt.hy then y=txt.hy end
		if y<1 then y=1 end

		local hx=txt.get_hx(y)

		if x>hx+1 then x=hx+1 end
		if x<1 then x=1 end

		return x,y
	end

	txt.insert_char=function(s)
	
		txt.cut()

		local sa=txt.get_string(txt.cy) or ""
		local sb=sa:sub(0,txt.cx-1)
		local sc=sa:sub(txt.cx)
		
		txt.set_string(txt.cy,sb..s..sc)
	
		txt.cx=txt.cx+1
		txt.clip()
	
		hook("changed")
	end

	txt.insert_newline=function()
	
		txt.cut()

		local sa=txt.get_string(txt.cy) or ""
		local sb=sa:sub(0,txt.cx-1) or ""
		local sc=sa:sub(txt.cx) or ""
		
		txt.set_string(txt.cy,sb.."\n")

		txt.cy=txt.cy+1
		txt.cx=1

		txt.add_string(txt.cy,sc)
		
		txt.clip()
	
		hook("changed")
	end

	local merge_lines=function()

		local sa=txt.get_string(txt.cy) or ""
		txt.cy=txt.cy-1
		local hx=txt.get_hx()
		local sb=txt.get_string(txt.cy) or ""
		txt.cx=hx+1
		txt.set_string(txt.cy,sb:sub(1,hx)..sa)
		txt.del_string(txt.cy+1)
		txt.clip()

	end

	txt.backspace=function()

		txt.cut()

		if txt.cx==1 and txt.cy>1 then
			merge_lines()
			return
		end

		local sa=txt.get_string(txt.cy) or ""
		local sb=sa:sub(0,txt.cx-2)
		local sc=sa:sub(txt.cx)
		
		txt.set_string(txt.cy,sb..sc)
	
		txt.cx=txt.cx-1
		txt.clip()
	
		hook("changed")
	end

	txt.delete=function()
	
		txt.cut()
	
		local hx=txt.get_hx()
		if txt.cx==hx+1 and txt.cy<txt.hy then
			txt.cy=txt.cy+1
			merge_lines()
			return
		end

		local sa=txt.get_string(txt.cy) or ""
		local sb=sa:sub(0,txt.cx-1)
		local sc=sa:sub(txt.cx+1)
		
		txt.set_string(txt.cy,sb..sc)
	
		txt.clip()
	
		hook("changed")
	end

	return txt
end

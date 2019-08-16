--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstring=require("wetgenes.string")
local wtxtutf=require("wetgenes.txtutf")


-- manage the text data part of a text editor


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.construct=function(txt)
	txt = txt or {}
	
	txt.tabsize=8

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

-- code based string sub
	txt.get_string_sub=function(idx,i,j)
		local cache=txt.get_cache(idx)
		if not cache then return "" end
		i=i or  1
		j=j or -1
		local l=#cache.codes -- length of codes
		if i<0 then i=l+i end -- deal with negative values
		if j<0 then j=l+1+j end
		if i<1 then i=1 end -- clamp values
		if j<0 then j=0 end
		if i>l then i=l end
		if j>l then j=l end
		if j==0 then return "" end
		i=cache.cb[i] or 0 -- convert to byte offset
		j=(cache.cb[j+1] or 1)-1
		return cache.string:sub(i,j) -- finally return string
	end

	txt.set_string=function(idx,str)
		txt.strings[idx]=str
		txt.clear_caches()
	end

	txt.add_string=function(idx,str)
		table.insert( txt.strings , idx , str)
		txt.hy=txt.hy+1
		txt.clear_caches()
	end

	txt.del_string=function(idx)
		table.remove( txt.strings , idx )
		txt.hy=txt.hy-1
		txt.clear_caches()
	end

	txt.del_cache=function(idx)
		txt.caches[idx]=nil
	end

	txt.get_cache=function(idx)
		local cache=txt.caches[idx]
		if cache then return cache end
		cache=txt.build_cache(idx)
		txt.caches[idx]=cache
		return cache
	end

-- just one meta for the caches
	txt.caches_meta={}
	txt.caches_meta.__mode="v"

	txt.clear_caches=function()
		txt.caches={}
		setmetatable(txt.caches, txt.caches_meta)
	end
	txt.clear_caches()
	
	txt.set_text=function(text)

		if text then -- set new text
			txt.strings=wstring.split_lines(text)
			txt.clear_caches()
		
			txt.hx=0
			txt.hy=#txt.strings
			for i,v in ipairs(txt.strings) do
				local lv=wtxtutf.length(v)
				if lv > txt.hx then txt.hx=lv end
			end
			
			hook("changed")
		
		end

	end

	txt.get_text=function()
		return table.concat(txt.strings) or ""
	end


-- mostly this deals with the visible width of a character, eg tab,
-- compared to its byte width, but it is also necesary for utf8 encoding
-- or japanese double glyphs. Its a complicated mapping so it is precalculated

	txt.build_cache=function(idx)
		
		local s=txt.get_string(idx)
		if not s then return nil end

		local cache={}
		
		cache.string=s
		
		cache.codes={}

-- x xpos is the screen space offset, so a tab would be 8 and a space 1.
-- b byte is the byte offset into the string
-- c code is the code offset into the string (unicode character)

-- these arrays map one space to another

		cache.bx={} -- map byte to xpos
		cache.bc={} -- map byte to code

		cache.xb={} -- map xpos to byte
		cache.xc={} -- map xpos to code

		cache.cx={} -- map code to xpos
		cache.cb={} -- map code to byte

		local b=1
		local x=0
		local c=1
		
		for char in s:gmatch(wtxtutf.charpattern) do
			local code=wtxtutf.code(char)
			local size=#char
			local width=1
			
			cache.codes[c]=code
			
			if code==9 then --tab
				width=math.ceil((x+1)/txt.tabsize)*txt.tabsize-x
			end

			cache.cb[c]=b
			cache.cx[c]=x

			for i=0,width-1 do
				cache.xb[x+i]=b
				cache.xc[x+i]=c
			end

			for i=0,size-1 do
				cache.bx[b+i]=x
				cache.bc[b+i]=c
			end

			c=c+1
			x=x+width
			b=b+size

			cache.cb[c]=b
			cache.cx[c]=x
		end

		return cache
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
		
			local s=""

			if txt.tx and txt.ty then
			
				for idx=txt.fy,txt.ty do
				
					if idx==txt.fy then -- first line
					
						if txt.ty==txt.fy then -- single line

--							local sa=txt.get_string(txt.fy) or ""
							local sb=txt.get_string_sub(txt.fy,1,txt.fx-1)
							local sc=txt.get_string_sub(txt.fy,txt.fx,txt.tx-1)
							local sd=txt.get_string_sub(txt.fy,txt.tx)
							
							s=s..sc
							txt.set_string(txt.fy,sb..sd)

						else -- multiple lines

--							local sa=txt.get_string(txt.fy) or ""
							local sb=txt.get_string_sub(txt.fy,1,txt.fx-1)
							local sc=txt.get_string_sub(txt.fy,txt.fx)
							
							s=s..sc
							txt.set_string(txt.fy,sb)

						end

					elseif idx==txt.ty then -- last line

--						local sa=txt.get_string(txt.fy+1) or ""
						local sb=txt.get_string_sub(txt.fy+1,1,txt.tx-1)
						local sc=txt.get_string_sub(txt.fy+1,txt.tx)
						
						s=s..sb
						txt.set_string(txt.fy+1,sc)

						local sa=txt.get_string(txt.fy) or ""
						local sb=txt.get_string(txt.fy+1) or ""
						txt.set_string(txt.fy,sa..sb)
						txt.del_string(txt.fy+1)

					else -- middle line
					
						local sa=txt.get_string(txt.fy+1) or ""
						s=s..sa
						txt.del_string(txt.fy+1)

					end

				end
			
			end
			
			txt.cx=txt.fx
			txt.cy=txt.fy
			txt.tx=txt.fx
			txt.ty=txt.fy
			
			txt.mark()
			
			if s=="" then s=nil end

			return s
		end

	end

	
	txt.get_hx=function(y)
		y=y or txt.cy
--		local s=txt.get_string(y)
		local cache=txt.get_cache(y)
		if not cache then return 0 end
		local hx=#cache.codes
		while hx>0 do
			local endswith=cache.codes[hx]
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

-- move a cursor one character left/right/up/down and clip it
	txt.clip_left=function(x,y)
		if x<=1 and y>1 then
			y=y-1
			x=txt.get_hx(y)+1
		else
			x=x-1
		end
		return txt.clip(x,y)
	end

	txt.clip_right=function(x,y)
		local hx=txt.get_hx(y)+1
		if x>=hx and y<txt.hy then
			y=y+1
			x=1
		else
			x=x+1
		end
		return txt.clip(x,y)
	end

	txt.clip_up=function(x,y)
		return txt.clip(x,y-1)
	end

	txt.clip_down=function(x,y)
		return txt.clip(x,y+1)
	end

	txt.insert_char=function(s)
	
		txt.cut()

--		local sa=txt.get_string(txt.cy) or ""
		local sb=txt.get_string_sub(txt.cy,1,txt.cx-1)
		local sc=txt.get_string_sub(txt.cy,txt.cx)
		
		txt.set_string(txt.cy,sb..s..sc)
	
		txt.cx=txt.cx+1
		txt.clip()
	
		hook("changed")
	end

	txt.insert_newline=function()
	
		txt.cut()

--		local sa=txt.get_string(txt.cy) or ""
		local sb=txt.get_string_sub(txt.cy,1,txt.cx-1)
		local sc=txt.get_string_sub(txt.cy,txt.cx)
		
		txt.set_string(txt.cy,sb.."\n")

		txt.cy=txt.cy+1
		txt.cx=1

		txt.add_string(txt.cy,sc)
		
		txt.clip()
	
		hook("changed")
	end

	txt.insert=function(s)

		txt.cut()
	
		local lines=wstring.split_lines(s,"\n")
		
		for idx,line in ipairs(lines) do

			if idx==1 then -- first line
			
				if #lines>1 then -- inserting multiple lines

--					local sa=txt.get_string(txt.cy) or ""
					local sb=txt.get_string_sub(txt.cy,0,txt.cx-1)
					local sc=txt.get_string_sub(txt.cy,txt.cx)
					
					txt.set_string(txt.cy,sb..line)
				
					txt.cy=txt.cy+1
					txt.cx=1

					txt.add_string(txt.cy,sc) -- put remainder on new line


				else -- inserting a single line

--					local sa=txt.get_string(txt.cy) or ""
					local sb=txt.get_string_sub(txt.cy,0,txt.cx-1)
					local sc=txt.get_string_sub(txt.cy,txt.cx)
					
					txt.set_string(txt.cy,sb..line..sc)
				
					txt.cx=txt.cx+wtxtutf.length(line)

				end


			elseif idx==#lines then -- last line

				txt.set_string(txt.cy,line..(txt.get_string(txt.cy) or ""))
				txt.cx=wtxtutf.length(line)

			else -- middle

				txt.add_string(txt.cy,s)
				txt.cy=txt.cy+1
				txt.cx=1

			end
		end

		txt.clip()
	
		hook("changed")

	end

	local merge_lines=function()

		local sa=txt.get_string(txt.cy) or ""
		txt.cy=txt.cy-1
		local hx=txt.get_hx()
--		local sb=txt.get_string(txt.cy) or ""
		txt.cx=hx+1
		txt.set_string(txt.cy,txt.get_string_sub(txt.cy,1,hx)..sa)
		txt.del_string(txt.cy+1)
		txt.clip()

	end

	txt.backspace=function()

		if txt.cut() then return end -- just delete selection

		if txt.cx==1 and txt.cy>1 then
			merge_lines()
			return
		end

--		local sa=txt.get_string(txt.cy) or ""
		local sb=txt.get_string_sub(txt.cy,1,txt.cx-2)
		local sc=txt.get_string_sub(txt.cy,txt.cx)
		
		txt.set_string(txt.cy,sb..sc)
	
		txt.cx=txt.cx-1
		txt.clip()
	
		hook("changed")
	end

	txt.delete=function()
	
		if txt.cut() then return end -- just delete selection
	
		local hx=txt.get_hx()
		if txt.cx==hx+1 and txt.cy<txt.hy then
			txt.cy=txt.cy+1
			merge_lines()
			return
		end

--		local sa=txt.get_string(txt.cy) or ""
		local sb=txt.get_string_sub(txt.cy,1,txt.cx-1)
		local sc=txt.get_string_sub(txt.cy,txt.cx+1)
		
		txt.set_string(txt.cy,sb..sc)
	
		txt.clip()
	
		hook("changed")
	end

	return txt
end

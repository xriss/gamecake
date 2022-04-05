--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstring=require("wetgenes.string")
local wutf=require("wetgenes.txt.utf")

local wtxtundo=require("wetgenes.txt.undo")
local wtxtdiff=require("wetgenes.txt.diff")
local wtxtlex =require("wetgenes.txt.lex")

local wpath=require("wetgenes.path")

-- manage the text data part of a text editor


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.construct=function(txt)
	txt = txt or {}
	
	txt.lexer="text"
	txt.tabsize=4

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
		if (idx-1)%txt.permacache_ratio == 0 then
			txt.permacaches[1+math.floor((idx-1)/txt.permacache_ratio)]=cache -- do not forget every X caches
		end
		return cache
	end

-- just one meta for the caches
	txt.caches_meta={}
	txt.caches_meta.__mode="v"

	txt.permacache_ratio=128
	txt.clear_caches=function()
		txt.permacaches={}
		txt.caches={}
		setmetatable(txt.caches, txt.caches_meta)
		txt.permastart={} -- recalculate start indexs of each perma string
		local start=0
		for i,v in ipairs(txt.strings or {} ) do
			if (i-1)%txt.permacache_ratio == 0 then -- a perma string so remember start
				txt.permastart[1+math.floor((i-1)/txt.permacache_ratio)]=start
			end
			start=start+#v -- next index
		end
		txt.permastart[#txt.permastart+1]=start -- end of text
	end
	txt.clear_caches()
--[[

Find byte offset into the text from a given line and character.

]]
	txt.location_to_ptr=function(y,x)

		local cache=txt.get_cache(y)
		local ptr=cache.cb[x] or 0 -- convert to byte offset
		ptr=cache.start+ptr-1

		return ptr
	end

--[[

Find the line number and column number of the given byte offset into the text.

]]
	txt.ptr_to_location=function(ptr)

		local mh=#txt.permastart
		local c=math.ceil(mh/2)
		local y=1
		
		repeat

			local a=txt.permastart[y]
			local b=txt.permastart[y+1] or a
			
			if not a or not b then y=nil break end -- failed
			
			if     a>ptr then y=y-c
			elseif b<ptr then y=y+c end
			
			if y<1    then y=1    end -- clamp
			if y>mh-1 then y=mh-1 end
			
			c=math.ceil(c/2) -- half the jump
			
		until a<=ptr and b>ptr -- in range
		
		if not y then return end -- failed to find line
		
		local a=txt.permastart[y]

		y=1+(y-1)*txt.permacache_ratio -- y back into full range

		local b=a+#txt.strings[y]

		while b<=ptr do -- step forward
			y=y+1
			local s=txt.strings[y]
			if not s then break end -- fail
			a=b
			b=a+#s
		end

		if a<=ptr and b>ptr then
			local cache=txt.get_cache(y)
			local x=cache.bc[1+ptr-a] or #cache.string+1
			return y,x	-- found line
		end
	end
	
--[[

Fill the editor up with text, the filename is used to set the lexer used for highlighting

]]
	txt.set_text=function(text,filename)

		if text then -- set new text
			txt.strings=wstring.split_lines(text)
			txt.clear_caches()
		
			txt.hx=0
			txt.hy=#txt.strings
			for i,v in ipairs(txt.strings) do
				local cache=txt.get_cache(i)
				local lv=#cache.codes -- wutf.length(v)
				if lv > txt.hx then txt.hx=lv end
			end
			
			if filename then -- set new filename ( can help us guess the format )
				txt.filename=filename
			end
			
			hook("changed")
		
		end

	end

	txt.append_text=function(text)
		txt.clip(txt.hy+1,0)
		txt.insert(text)
	end

	txt.get_text=function()
		return table.concat(txt.strings) or ""
	end

--[[

mostly this deals with the visible width of a character, eg tab,
compared to its byte width, but it is also necessary for utf8 encoding
or japanese double glyphs. Its a complicated mapping so it is precalculated

]]
	txt.build_cache=function(idx)
		
		local s=txt.get_string(idx)
		if not s then return nil end

		local cache={}
		
		cache.string=s
		
		local perma=math.floor((idx-1)/txt.permacache_ratio)
		cache.start=txt.permastart[1+perma] -- get start from sparse array
		perma=1+(perma*txt.permacache_ratio) -- find start for this chunk
		while perma<idx do -- find precise start
			cache.start=cache.start+#txt.strings[perma]
			perma=perma+1
		end
		

		
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
		
		for char in s:gmatch(wutf.charpattern) do
			local code=wutf.code(char)
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
	
--[[

auto select an entire word or line depending on number of clicks

]]
	txt.markauto=function(fy,fx,clicks)

		txt.mark(fy,fx,fy,fx)

		local s=txt.get_string(txt.cy) or ""

		if clicks==2 then -- select word
		
			local sl=wutf.length(s)
			local lx=txt.cx-1
			local hx=txt.cx-1

			local c = wutf.ncode( s , lx )

			if c and c > 32 then -- solid
				
				while ( (wutf.ncode( s , lx-1 ) or 0) > 32 ) do lx=lx-1 end
				while ( (wutf.ncode( s , hx+1 ) or 0) > 32 ) do hx=hx+1 end

				txt.mark(fy,lx,fy,hx+1)
			
			elseif c and c <= 32 then -- White

				while ( (wutf.ncode( s , lx-1 ) or 33) <= 32 ) do lx=lx-1 end
				while ( (wutf.ncode( s , hx+1 ) or 33) <= 32 ) do hx=hx+1 end

				txt.mark(fy,lx,fy,hx+1)

			end

		elseif clicks>=3 then -- select line
			txt.mark(fy,0,fy+1,0)
		end

	end
	

--[[

get current selected area / cursor position

]]
	txt.markget=function()
		return txt.fy,txt.fx,txt.ty,txt.tx
	end

--[[

are one or more glyphs currently selected, returns true or false

]]
	txt.marked=function()
		if txt.fy and txt.fx and txt.ty and txt.tx then
			if txt.fy==txt.ty and txt.fx==txt.tx then
				return false
			end
			return true
		end
		return false
	end


--[[

merge two marked areas into one

]]
	txt.markmerge=function( fya,fxa,tya,txa, fyb,fxb,tyb,txb )

		if not fxa and fxb then return txt.mark(fyb,fxb,tyb,txb) end -- nothing to merge
		if not fxb and fxa then return txt.mark(fya,fxa,tya,txa) end -- nothing to merge
		if not fxb and not fxa then return end -- nothing to do

		local fx,fy,tx,ty

		if     fya < fyb then fy=fya fx=fxa
		elseif fya > fyb then fy=fyb fx=fxb
		else
			if fxa < fxb then fy=fya fx=fxa
			else              fy=fyb fx=fxb
			end
		end

		if     tya > tyb then ty=tya tx=txa
		elseif tya < tyb then ty=tyb tx=txb
		else
			if txa > txb then ty=tya tx=txa
			else              ty=tyb tx=txb
			end
		end
		
		txt.mark(fy,fx,ty,tx)

	end
	
--[[

mark or unmark an area

]]
	txt.mark=function(fy,fx,ty,tx)
		if not fx then -- unmark
			txt.fx=nil
			txt.fy=nil
			txt.tx=nil
			txt.ty=nil
			return
		end
		txt.fy,txt.fx=txt.clip(fy,fx)
		txt.ty,txt.tx=txt.clip(ty,tx)
		
		txt.cy,txt.cx=txt.ty,txt.tx
		
		local flip=false
		if txt.fy==txt.ty and txt.fx>txt.tx then flip=true
		elseif                txt.fy>txt.ty then flip=true end
		if flip then
			txt.fx,txt.tx=txt.tx,txt.fx
			txt.fy,txt.ty=txt.ty,txt.fy
		end

	end

--[[

get fy,fx,ty,tx range given a start and a glyph offset ( probably +1 or -1 ) 

]]
	txt.rangeget=function(fy,fx,length)

		local tx=fx
		local ty=fy

		if length<0 then -- backward search
		
			local cache=txt.get_cache(fy)
			
			while length<0 do

				if fx <= -length then -- full line

					length=length+1+fx
					fy=fy-1
					cache=txt.get_cache(fy)
					if not cache then return fy+1,fx,ty,tx end -- start of file
					fx=#cache.codes -- end of line
				
				else -- partial line
				
					fx=fx+length
					length=0
				end
				
			end

		elseif length>0 then -- forward search

			local cache=txt.get_cache(ty)
			
			while length>0 and cache do
				
				if ( #cache.codes - tx ) < length then -- full line

					length=length-( #cache.codes + 1 - tx ) -- include line end
					ty=ty+1
					cache=txt.get_cache(ty)
					if not cache then return fy,fx,ty-1,tx end -- end of file
					tx=1 -- start of line
				
				else -- partial line
				
					tx=tx+length
					length=0
				end
				
			end

		end

		return fy,fx,ty,tx
	end

--[[

cut out the text in the marked area (if marked) and set the cursor to this location

]]
	txt.cut=function(fy,fx,ty,tx)
	
		local setcursor=not fx
	
		fx=fx or txt.fx
		fy=fy or txt.fy
		tx=tx or txt.tx
		ty=ty or txt.ty
		
		if fx and fy then
		
			local s=""

			if tx and ty then
			
				for idx=fy,ty do
				
					if idx==fy then -- first line
					
						if ty==fy then -- single line

--							local sa=txt.get_string(fy) or ""
							local sb=txt.get_string_sub(fy,1,fx-1)
							local sc=txt.get_string_sub(fy,fx,tx-1)
							local sd=txt.get_string_sub(fy,tx)
							
							s=s..sc
							txt.set_string(fy,sb..sd)

						else -- multiple lines

--							local sa=txt.get_string(fy) or ""
							local sb=txt.get_string_sub(fy,1,fx-1)
							local sc=txt.get_string_sub(fy,fx)
							
							s=s..sc
							txt.set_string(fy,sb)

						end

					elseif idx==ty then -- last line

--						local sa=txt.get_string(fy+1) or ""
						local sb=txt.get_string_sub(fy+1,1,tx-1)
						local sc=txt.get_string_sub(fy+1,tx)
						
						s=s..sb
						txt.set_string(fy+1,sc)

						local sa=txt.get_string(fy) or ""
						local sb=txt.get_string(fy+1) or ""
						txt.set_string(fy,sa..sb)
						txt.del_string(fy+1)

					else -- middle line
					
						local sa=txt.get_string(fy+1) or ""
						s=s..sa
						txt.del_string(fy+1)

					end

				end
			
			end
			
			if setcursor then

				txt.cx=txt.fx
				txt.cy=txt.fy
				txt.tx=txt.fx
				txt.ty=txt.fy
				
				txt.mark()

			end
			
			if s=="" then
				s=nil
			else
				hook("changed")
			end

			return s
		end

	end
--[[

copy text from the marked area (if marked) or return nil

]]
	txt.copy=function(fy,fx,ty,tx)
		
		fx=fx or txt.fx
		fy=fy or txt.fy
		tx=tx or txt.tx
		ty=ty or txt.ty

		if fx and fy then
		
			local s=""

			if tx and ty then
			
				for idx=fy,ty do
				
					if idx==fy then -- first line
					
						if ty==fy then -- single line

							s=s..txt.get_string_sub(idx,fx,tx-1)

						else -- multiple lines

							s=s..txt.get_string_sub(idx,fx)

						end

					elseif idx==ty then -- last line

						s=s..txt.get_string_sub(idx,1,tx-1)

					else -- middle line
					
						s=s..txt.get_string(idx) or ""

					end

				end
			
			end
			
			if s=="" then s=nil end

			return s
		end

	end
		
--[[

get length of currently selected text in bytes

	txt.copy_length=function()
		local s=txt.copy() -- expensive hax, we should not bother building a string
		if s then return #s end
		return 0
	end
]]


--[[

get length of line in glyphs

]]
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

--[[

clip this location so it is within the text.

eg too the end of a line

]]
	txt.clip=function(y,x)
	
		if not x then
			txt.cy,txt.cx=txt.clip(txt.cy,txt.cx)
			return
		end

		if y>txt.hy then y=txt.hy x=txt.get_hx(y)+1 end
		if y<1 then y=1 end

		local hx=txt.get_hx(y)

		if x>hx+1 then x=hx+1 end
		if x<1 then x=1 end

		return y,x
	end

--[[

move a cursor one character left and clip it

]]
	txt.clip_left=function(y,x)
		if x<=1 and y>1 then
			y=y-1
			x=txt.get_hx(y)+1
		else
			x=x-1
		end
		return txt.clip(y,x)
	end

--[[

move a cursor one character right and clip it

]]
	txt.clip_right=function(y,x)
		local hx=txt.get_hx(y)+1
		if x>=hx and y<txt.hy then
			y=y+1
			x=1
		else
			x=x+1
		end
		return txt.clip(y,x)
	end

--[[

move a cursor one character up and clip it

]]
	txt.clip_up=function(y,x)
		return txt.clip(y-1,x)
	end

--[[

move a cursor one character down and clip it

]]
	txt.clip_down=function(y,x)
		return txt.clip(y+1,x)
	end

--[[

insert a character, eg user typing

]]
	txt.insert_char=function(s)
	
		local sb=txt.get_string_sub(txt.cy,1,txt.cx-1)
		local sc=txt.get_string_sub(txt.cy,txt.cx)
		
		txt.set_string(txt.cy,sb..s..sc)
	
		txt.cx=txt.cx+1
		txt.clip()
	
		hook("changed")
	end

--[[

insert a new line

]]
	txt.insert_newline=function()
	
		local sb=txt.get_string_sub(txt.cy,1,txt.cx-1)
		local sc=txt.get_string_sub(txt.cy,txt.cx)
		
		txt.set_string(txt.cy,sb.."\n")

		txt.cy=txt.cy+1
		txt.cx=1

		txt.add_string(txt.cy,sc)
		
		txt.clip()
	
		hook("changed")
	end

--[[

insert any string, which will be broken down by newlines

]]
	txt.insert=function(s)

		local split=function(s,d)
			d=d or "\n"
			local ss={} -- output table
			local ti=1  -- table index
			local si=1  -- string index
			while true do
				local fa,fb=string.find(s,d,si) -- find delimiter
				if fa then
					ss[ti]=string.sub(s,si,fb) -- add string to table, including delimiter
					ti=ti+1
					si=fb+1
				else break end -- no more delimiters
			end
			ss[ti]=string.sub(s,si) -- we want empty string after a final new line
			return ss
		end

		local lines=split(s,"\n")
		
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
				
					txt.cx=txt.cx+wutf.length(line)

				end


			elseif idx==#lines then -- last line

				txt.set_string(txt.cy,line..(txt.get_string(txt.cy) or ""))
				txt.cx=wutf.length(line)+1

			else -- middle

				txt.add_string(txt.cy,line)
				txt.cy=txt.cy+1
				txt.cx=1

			end
		end

		txt.clip()
	
		hook("changed")

	end

--[[

remove the new line delimiter between this line and the next

eg delete the \n at the end of the line

]]
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

--[[

delete one glyph from the left of the cursor

]]
	txt.backspace=function()

		if txt.cx==1 then
			if txt.cy==1 then return end
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

--[[

delete one glyph from the right of the cursor

]]
	txt.delete=function()
	
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
	
--[[

assign the lexxer code in charge of highlights

]]
	txt.set_lexer=function(lexer)

		if lexer then -- force

			txt.lexer=lexer

		else -- guess
		
			txt.lexer="text" -- generic
			
			if txt.filename then
				local p=wpath.parse(txt.filename)
				
				if p.ext then
					txt.lexer=string.lower( string.sub(p.ext,2) )-- skip the . at the start
				end
			end

		end
		
		txt.lex=wtxtlex.list[txt.lexer] -- the lex to use (if any)
		
		txt.clear_caches()
		
	end

--[[

get the lexxer cache for the given line

]]
	txt.get_cache_lex=function(idx)
	
		if not txt.lex then return txt.get_cache(idx) end -- no lex available

		local cache=txt.get_cache(idx)
		if not cache then return cache end -- no cache

		if cache.lex then -- already done
			return cache
		end
	
--print("newlex",idx)

		local state
		
		if idx>1 then -- get last
			local last_cache=txt.get_cache(idx-1)
			if not last_cache.lex then -- build
				for i=1,idx-1 do -- dumb auto loop from start, should be moved
--print("lex",i)
					txt.get_cache_lex(i)
				end
			end
			if last_cache.lex then
				state=wtxtlex.load(last_cache.lex)
			end
		end
		
		if not state then
			state=txt.lex.create()
		end

--print("tokens",cache.string)
		local tokens=txt.lex.parse(state,cache.string,{})
		cache.lex=wtxtlex.save(state)
		cache.tokens=table.concat(tokens)
--print("tokens",cache.tokens)

		return cache
	end


-- bind to an undo state
	wtxtundo.construct({},txt)


	txt.set_text("\n","")
	txt.set_lexer()
	return txt
end



local hex=function(str) return tonumber(str,16) end

local string=string
local table=table
local ipairs=ipairs
local math=math
local loadstring=loadstring
local pcall=pcall

-- imported global functions
local sub = string.sub
local match = string.match
local find = string.find
local push = table.insert
local pop = table.remove
local append = table.insert
local concat = table.concat
local floor = math.floor
local write = io.write
local read = io.read
local type = type
local setfenv = setfenv
local tostring=tostring
local pairs=pairs
local ipairs=ipairs
local unpack=unpack
local require=require

local _G = _G

module("fenestra.console")
local fenestra_buffedit=require("fenestra.buffedit")

-----------------------------------------------------------------------------
--
-- split a string into a table
--
-----------------------------------------------------------------------------
local function split(div,str)

  if (div=='') or not div then error("div expected", 2) end
  if (str=='') or not str then error("str expected", 2) end
  
  local pos,arr = 0,{}
  
  -- for each divider found
  for st,sp in function() return string.find(str,div,pos,false) end do
	table.insert(arr,sub(str,pos,st-1)) -- Attach chars left of current divider
	pos = sp + 1 -- Jump past current divider
  end
  
  if pos~=0 then
	table.insert(arr,sub(str,pos)) -- Attach chars right of last divider
  else
	table.insert(arr,str) -- return entire string
  end
  
  
  return arr
end




function setup(fenestra)

	local function print(...)
		fenestra._g.print(...)
	end

	local ogl=fenestra.ogl

	local it={}
	
	it.buff=fenestra_buffedit.create() -- create buff edit
	it.buff.enter=function(_,line) it.dump_eval(line) end
	
	it.lines={}
	it.lines_display={}
	
	it.x=0
	it.y=0
	it.y_show=8*8
	
	it.show=false

	function it.clean()

	end
	
	-- print out lua data in a somewhat sensible way, returns a string
	it.dump_limit = 20
	it.dump_depth = 7
	it.dump_stack = {}

	it.call = {} -- name -> function : functions that should be easily to call on the console command line
	
	it.call.help=function()
		local t={}
		for n,f in pairs(it.call) do
			t[#t+1]=n
		end
		return table.concat(t," ")
	end
	
	function it.dump_table(tbl,delim)
		local n = #tbl
		local res = ''
		local k = 0
		-- very important to avoid disgracing ourselves with circular referencs...
		if #it.dump_stack > it.dump_depth then
			return "..."
		end
		for i,t in ipairs(it.dump_stack) do
			if tbl == t then
				return "<self>"
			end
		end
		push(it.dump_stack,tbl)
		
		for key,v in pairs(tbl) do
			if type(key) == 'number' then
				key = '['..tostring(key)..']'
			else
				key = tostring(key)
			end
			res = res..delim..key..'='..it.dump_string(v)
			k = k + 1
			if k > it.dump_limit then
				res = res.." ... "
				break
			end
		end
		
		pop(it.dump_stack)
		return sub(res,2)
	end



	function it.dump_string(val)
		local tp = type(val)
		if tp == 'function' then
			return tostring(val)
		elseif tp == 'table' then
			if val.__tostring  then
				return tostring(val)
			else
				return '{'..it.dump_table(val,',\n')..'}'
			end
		elseif tp == 'string' then
			return val--"'"..val.."'"
		elseif tp == 'number' then
			return tostring(val)
		else
			return tostring(val)
		end
	end
	
-- based on ilua.lua
	function it.dump_eval(line)
	
		local function compile(line)
			local f,err = loadstring(line,'local')
			return err,f
		end
		
		
		local err,chunk
		local ret={}
		local args={}
		
		
		if line~="" then args=split("%s",line) end -- split input on whitespace
		
		if args[1] then
		
			function lookup(tab,name)
				local names=split("%.",name)
				for i,v in ipairs(names) do
--				print(i.." "..v)
					if type(tab)=="table" then
						tab=tab[v]
					else
						tab=nil
					end
				end
				return tab
			end
			
			chunk=lookup(it.call,args[1]) -- check special console functions
			
			if chunk and type(chunk)=="function" then -- must be a function
			
				table.remove(args,1) -- remove the function name
			
				setfenv(chunk,fenestra._g) -- call with master environment?
			else
			
				chunk=lookup(fenestra._g,args[1]) -- check for functions in master environment
				
				if chunk and type(chunk)=="function" then -- must be a function
				
					table.remove(args,1) -- remove the function name
				
				else
					chunk=nil
				end
				
				-- do not try and change the fenv of a function in the main envronment...
			
			end
			
		end
		
		if not chunk then
		
			args={} -- no arguments
			
			-- is it an expression?
			err,chunk = compile('print('..line..')')
			if err then
				-- otherwise, a statement?
				err,chunk = compile(line)
			end
			
			if chunk then
				setfenv(chunk,fenestra._g) -- compile in master environment will have an overloaded print
			end
		end

		-- if compiled ok, then evaluate the chunk
		if not err and chunk then
		
			ret = { pcall(chunk,unpack(args)) }
			
			if not ret[1] then
				err=ret[2]
				ret={}
			else
				table.remove(ret,1)
			end
			
		end
		
		-- if there was any error, print it out
		if err then
			fenestra._g.print(err)
		else
			for i,v in ipairs(ret) do
				fenestra._g.print(v)
			end
		end
	end

	function it.update()
	
		it.buff:update()
		
		if it.show then
			if it.y~=it.y_show then
			
				local d=(it.y_show-it.y)/4
				if d>0 then d=math.ceil(d) else d=math.floor(d) end
				it.y= math.floor( it.y + d )
			
			end
		else
			if it.y~=0 then
			
				local d=(0-it.y)/4
				if d>0 then d=math.ceil(d) else d=math.floor(d) end
				it.y= math.floor( it.y + d )
			
			end
		end
		
	end
	
	function it.draw()
	
		fenestra.debug_begin()
		
		local w=fenestra.get("width")
		local h=it.y
		fenestra.debug_polygon_begin()
		fenestra.debug_polygon_vertex(0,0,hex"ee00cc00")
		fenestra.debug_polygon_vertex(w,0,hex"ee00cc00")
		fenestra.debug_polygon_vertex(w,h,hex"ee004400")
		fenestra.debug_polygon_vertex(0,h,hex"ee004400")
		fenestra.debug_polygon_end()
		
--		fenestra.debug_rect(0,0,fenestra.get("width"),it.y,hex"8800ff00")
		
		local i=#it.lines
		local y=it.y-16
		while y>-8 and i>0 do
		
			fenestra.debug_print({x=0,y=y,size=8,color=hex"ff00ff00",s=it.lines[i]})
			
			y=y-8
			i=i-1
		end
		
		if it.show_hud then
			for i,v in ipairs(it.lines_display) do
			
				fenestra.debug_print({x=0,y=it.y+i*8-8,size=8,color=hex"ffffffff",s=v})
			
			end
		end
		
		fenestra.debug_print({x=0,y=it.y-8,size=8,color=hex"ff00ff00",s=">"..it.buff.line})

		fenestra.debug_rect((it.buff.line_idx+1)*8,it.y-8,(it.buff.line_idx+2)*8,it.y,hex"00ff00"+it.buff.throb*256*256*256)

		fenestra.debug_end()

		it.lines_display={}

	end
	
	function it.print(s)
	
		s=it.dump_string(s)
	
		table.insert(it.lines,s)
		
		while #it.lines > 64 do
		
			table.remove(it.lines,1)
		
		end
		
	end
	
	function it.display(s)
	
		s=it.dump_string(s)
		
		table.insert(it.lines_display,s)
	
	end
	
	function it.mouse(act,x,y,key)
--		print(act.." "..x..","..y.." "..key)
	end
	
	function it.keypress(ascii,key,act)

		if act=="down" then
--			fenestra._g.print(ascii.." "..(key or ""))
		end

		if act=="down" and ascii=="`" then
		
			if it.show then
			
				it.show=false
				it.show_hud=false
				
			elseif it.show_hud then
			
				it.show=true			
				throb=255
			else
				it.show_hud=true
			end

			return true
		end
			
		if it.show then
		
			if act=="down" or act=="repeat" then
					
				if key=="page up" or key=="prior" then
				
					it.y_show=it.y_show-8
				
				elseif key=="page down" or key=="next" then
				
					it.y_show=it.y_show+8

				end
				
			end
			
			return it.buff:keypress(ascii,key,act)
			
		end
		
	end
	
-- overload print function in the given (global) tab
-- returns a function to undo this act (however this function may fail...)
	function it.replace_print(g)
	
		local print_old=g.print
		it.print_old=g.print
		local print_new=function(...)
		
			local t={}
			for i,v in ipairs(arg) do
				table.insert(t, it.dump_string(v) )
			end
			if not t[1] then t[1]="nil" end
			
			it.print( unpack(t) )
			if print_old then
				print_old( unpack(t) )
			end
		end
		g.print=print_new
		
		return function()
			if g.print==print_new then -- only change back if noone else changed it
				g.print=print_old
				return true
			end
		end
	end

	
	return it
end

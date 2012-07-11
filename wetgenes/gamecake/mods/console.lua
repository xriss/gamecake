-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local hex=function(str) return tonumber(str,16) end

local wetstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local gl=require("gles").gles1


module("wetgenes.gamecake.mods.console")
local buffedit=require("wetgenes.gamecake.mods.console.buffedit")

function bake(opts)

	local console={}
	
	-- print out lua data in a somewhat sensible way, returns a string
	console.dump_limit = 20
	console.dump_depth = 7
	console.dump_stack = {}

	console.call = {} -- name -> function : functions that should be easily to call on the console command line


	function console.setup(state)
	
		console.buff=buffedit.create() -- create buff edit
		console.buff.enter=function(_,line) console.dump_eval(line) end
		
		console.lines={}
		console.lines_display={}
		
		console.x=0
		console.y=0
		console.y_show=8*8
		
		console.show=false
		
		console.setup_done=true
print("console setup")
	end

	function console.clean(state)
	
		console.setup_done=false

	end
	
	
	console.call.help=function()
		local t={}
		for n,f in pairs(console.call) do
			t[#t+1]=n
		end
		return table.concat(t," ")
	end
	
	function console.dump_table(tbl,delim)
		local n = #tbl
		local res = ''
		local k = 0
		-- very important to avoid disgracing ourselves with circular referencs...
		if #console.dump_stack > console.dump_depth then
			return "..."
		end
		for i,t in ipairs(console.dump_stack) do
			if tbl == t then
				return "<self>"
			end
		end
		push(console.dump_stack,tbl)
		
		for key,v in pairs(tbl) do
			if type(key) == 'number' then
				key = '['..tostring(key)..']'
			else
				key = tostring(key)
			end
			res = res..delim..key..'='..console.dump_string(v)
			k = k + 1
			if k > console.dump_limit then
				res = res.." ... "
				break
			end
		end
		
		pop(console.dump_stack)
		return sub(res,2)
	end



	function console.dump_string(val)
		local tp = type(val)
		if tp == 'function' then
			return tostring(val)
		elseif tp == 'table' then
			if val.__tostring  then
				return tostring(val)
			else
				return '{'..console.dump_table(val,',\n')..'}'
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
	function console.dump_eval(line)
	
		local function compile(line)
			local f,err = loadstring(line,'local')
			return err,f
		end
		
		
		local err,chunk
		local ret={}
		local args={}
		
		
		if line~="" then args=wetstr.split("%s",line) end -- split input on whitespace
		
		if args[1] then
		
			function lookup(tab,name)
				local names=wetstr.split("%.",name)
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
			
			chunk=lookup(console.call,args[1]) -- check special console functions
			
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

	function console.update(state)
	
		if not console.setup_done then console.setup(state) end -- modules do not have their setup called...
	
		console.buff:update()
		
		if console.show then
			if console.y~=console.y_show then
			
				local d=(console.y_show-console.y)/4
				if d>0 then d=math.ceil(d) else d=math.floor(d) end
				console.y= math.floor( console.y + d )
			
			end
		else
			if console.y~=0 then
			
				local d=(0-console.y)/4
				if d>0 then d=math.ceil(d) else d=math.floor(d) end
				console.y= math.floor( console.y + d )
			
			end
		end
		
	end
	
	function console.draw(state)
	
		state.win:info()
		local w,h=state.win.width,state.win.height
		gl.Viewport(0,0,w,h)

		gl.ClearColor(0,0,0,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		gl.MatrixMode(gl.PROJECTION)
		gl.LoadMatrix( tardis.m4_project23d(w,h,w,h,1,h*2) )

		gl.MatrixMode(gl.MODELVIEW)
		gl.LoadIdentity()

		gl.PushMatrix()


		gl.PopMatrix()

--[[
		fenestra.debug_begin()
		
		local w=fenestra.get("width")
		local h=console.y
		fenestra.debug_polygon_begin()
		fenestra.debug_polygon_vertex(0,0,hex"ee00cc00")
		fenestra.debug_polygon_vertex(w,0,hex"ee00cc00")
		fenestra.debug_polygon_vertex(w,h,hex"ee004400")
		fenestra.debug_polygon_vertex(0,h,hex"ee004400")
		fenestra.debug_polygon_end()
		
--		fenestra.debug_rect(0,0,fenestra.get("width"),console.y,hex"8800ff00")
		
		local i=#console.lines
		local y=console.y-16
		while y>-8 and i>0 do
		
			fenestra.debug_print({x=0,y=y,size=8,color=hex"ff00ff00",s=console.lines[i]})
			
			y=y-8
			i=i-1
		end
		
		if console.show_hud then
			for i,v in ipairs(console.lines_display) do
			
				fenestra.debug_print({x=0,y=console.y+i*8-8,size=8,color=hex"ffffffff",s=v})
			
			end
		end
		
		fenestra.debug_print({x=0,y=console.y-8,size=8,color=hex"ff00ff00",s=">"..console.buff.line})

		fenestra.debug_rect((console.buff.line_idx+1)*8,console.y-8,(console.buff.line_idx+2)*8,console.y,hex"00ff00"+console.buff.throb*256*256*256)

		fenestra.debug_end()

		console.lines_display={}
]]

	end
	
	function console.print(s)
	
		s=console.dump_string(s)
	
		table.insert(console.lines,s)
		
		while #console.lines > 64 do
		
			table.remove(console.lines,1)
		
		end
		
	end
	
	function console.display(s)
	
		s=console.dump_string(s)
		
		table.insert(console.lines_display,s)
	
	end
	
	function console.mouse(act,x,y,key)
--		print(act.." "..x..","..y.." "..key)
	end
	
	function console.keypress(ascii,key,act)

		if act=="down" then
--			fenestra._g.print(ascii.." "..(key or ""))
		end

		if act=="down" and ascii=="`" then
		
			if console.show then
			
				console.show=false
				console.show_hud=false
				
			elseif console.show_hud then
			
				console.show=true			
				throb=255
			else
				console.show_hud=true
			end

			return true
		end
			
		if console.show then
		
			if act=="down" or act=="repeat" then
					
				if key=="page up" or key=="prior" then
				
					console.y_show=console.y_show-8
				
				elseif key=="page down" or key=="next" then
				
					console.y_show=console.y_show+8

				end
				
			end
			
			return console.buff:keypress(ascii,key,act)
			
		end
		
	end
	
-- overload print function in the given (global) tab
-- returns a function to undo this act (however this function may fail...)
	function console.replace_print(g)
	
		local print_old=g.print
		console.print_old=g.print
		local print_new=function(...)
		
			local t={}
			for i,v in ipairs({...}) do
				table.insert(t, console.dump_string(v) )
			end
			if not t[1] then t[1]="nil" end
			
			console.print( unpack(t) )
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

	
	return console
end

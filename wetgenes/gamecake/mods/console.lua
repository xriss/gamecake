-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local wetstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


module("wetgenes.gamecake.mods.console")
local buffedit=require("wetgenes.gamecake.mods.console.buffedit")

name="console" -- by this name shall ye know me

function bake(opts)

	local console={}
	
	-- print out lua data in a somewhat sensible way, returns a string
	console.dump_limit = 20
	console.dump_depth = 7
	console.dump_stack = {}

	console.call = {} -- name -> function : functions that should be easily to call on the console command line


	function console.setup(state)
	
		console.replace_print(_G)
	
		state.cake.fonts:loads({1}) -- load builtin font number 1 a basic 8x8 font

		console.buff=buffedit.create() -- create buff edit
		console.buff.enter=function(_,line) console.dump_eval(line) end
		
		console.lines={}
		console.lines_display={}
		
		console.x=0
		console.y=0
		console.y_show=8*8
		
		console.show=false

--		console.show=true
--		console.show_hud=true
		
		console.setup_done=true
--print("console setup")
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
			
				setfenv(chunk,_G) -- call with master environment?
			else
			
				chunk=lookup(_G,args[1]) -- check for functions in master environment
				
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
			err,chunk = compile('return '..line..' ')
			if err then
				-- otherwise, a statement?
				err,chunk = compile(line)
			end
			
			if chunk then
				setfenv(chunk,_G) -- compile in master environment will have an overloaded print
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
			_G.print(err)
		else
			if ret[1] then
				_G.print(unpack(ret))
			end
		end
	end

	function console.update(state)
	
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
	
		local win=state.win
		local cake=state.cake
		local canvas=cake.canvas
		local font=canvas.font
		local gl=cake.gl

		if state.times and state.win then -- simple benchmarking
		
			local t=state.win.time()

		-- count frames	
			if (not console.fps) or t-console.fps_last >= 1 then -- update with average value once a sec
			
				console.fps=console.fps_count or 0
				console.fps_count=0
				console.fps_last=t
			
				state.times.update.done()
				state.times.draw.done()
			end

			local gci=gcinfo()
			console.display(string.format("fps=%02.0f t=%03.0f u=%03.0f d=%03.0f gc=%0.0fk",
				console.fps,
				math.floor(0.5+(10000/console.fps)),
				math.floor(0.5+state.times.update.time*10000),
				math.floor(0.5+state.times.draw.time*10000/state.times.draw.hash),
				math.floor(gci) ))
				
			console.fps_count=console.fps_count+1
		end


		state.win:info()
		local w,h=state.win.width,state.win.height
		gl.Viewport(0,0,w,h)

--		gl.ClearColor(0,0,0,0)
--		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		gl.MatrixMode(gl.PROJECTION)
		gl.LoadMatrix( tardis.m4_project23d(w,h,w,h,0.5,0) )

		gl.MatrixMode(gl.MODELVIEW)
		gl.LoadIdentity()
		gl.Translate(-w/2,-h/2,-h) -- top/left 1unit==1pixel
		gl.PushMatrix()

		font:set(cake.fonts:get(1))
		font:set_size(8,0)

		local i=#console.lines
		local y=console.y-16
		while y>-8 and i>0 do
		
			font:set_xy(0,y)
			font:draw(console.lines[i])
			
			y=y-8
			i=i-1
		end
		
		if console.show_hud then
			for i,v in ipairs(console.lines_display) do
			
				font:set_xy(0,console.y+i*8-8)
				font:draw(v)
			
			end
		end
		
		font:set_xy(0,console.y-8)
		font:draw(">"..console.buff.line)

		if console.buff.throb > 128 then
			font:set_xy((console.buff.line_idx+1)*8,console.y-8)
			font:draw("_")
		end

		console.lines_display={}


		gl.PopMatrix()

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
	
	function console.msg(m)
		if m[1]=="key" then
			console.keypress(m[2],string.lower(m[5]),m[3])
		end
	end

	function console.keypress(ascii,key,act)
		if act==1 then
--			_G.print(ascii.." "..(key or ""))
		end

		if act==1 and ascii=="`" then
		
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
		
			if act==1 or act==0 then
					
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
--			if not t[1] then t[1]="nil" end
			
			console.print( table.concat(t,"\t") )
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

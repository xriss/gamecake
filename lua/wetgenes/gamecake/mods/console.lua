--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local log,dump=require("wetgenes.logs"):export("log","dump")

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")

local wpackage=require("wetgenes.package")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local buffedit=require("wetgenes.gamecake.mods.console.buffedit")


local function assert_resume(co,...)

	local t={coroutine.resume(co,...)}

	if t[1] then

		table.remove(t,1) -- remove status code

		return unpack(t) -- no error
	end

	log("console" , t[2].."\nin coroutine\n"..debug.traceback(co) ) -- error

end

function M.bake(oven,console)

	console=console or {}

	-- print out lua data in a somewhat sensible way, returns a string
	console.dump_limit = 20
	console.dump_depth = 7
	console.dump_stack = {}

	console.data={}
	console.data_meta={__index=_G}
	console.call = {} -- name -> function : functions that should be easily to call on the console command line
	console.help={} -- simple help text for callable functions

	console.tasks={} -- running tasks

	setmetatable(console.data,console.data_meta)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas

	local win=oven.win
	local font=canvas.font
	local flat=canvas.flat


	function console.setup()
		console.replace_print(_G)

		oven.cake.fonts.loads({4}) -- load builtin font number 4 a basic 8x16 font

		console.buff=buffedit.create() -- create buff edit
		console.buff.enter=function(_,line) console.dump_eval(line) end

		console.lines={}
		console.lines_display={}
		console.line_width=80
		console.x=0
		console.y=0
		console.y_show=8*16

		console.fps_updates=0

		console.show=false
--		console.show_hud=true
--		console.show_hud=false

--		console.show=true
--		console.show_hud=true

		console.setup_done=true
--print("console setup")

		console.view=cake.views.create({
			parent=cake.views.peek(),
			mode="raw",
		})

	end

	function console.clean()

		console.setup_done=false

	end

	local function lookup(tab,name)
		local names=wstr.split(name,".")
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

	console.help.help="List available commands or a specific commands help string."
	console.call.help=function(name)

		if name then
			local s=lookup( console.help , name )
			if s then return "\n"..s.."\n" end
		end

		local t={"\n"}
		for n,f in pairs(console.call) do
			t[#t+1]=n
		end

		local s="\n\nFor more info about the above commands, try -> help [name] "

		return table.concat(t," ") .. s
	end

	console.help.reload="Force reload a module."
	console.call.reload=wpackage.reload

	console.help.rebake="Force reload a baked module using global oven."
	console.call.rebake=function(n) if OVEN then return OVEN.reload(n) end end

	console.help.ls="List data currently set in the console environment."
	console.call.ls=function(n)
		local d=console.data
		if n then d=lookup(d,n) end
		local t={"\n"}
		local count=0
		for n,f in pairs(d) do
			t[#t+1]=tostring(n)
			count=count+1
		end
		local s="\n\n"..count.." values found"
		if n then s=s.." in "..n end
		return table.concat(t," ")..s
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

		if line~="" then args=wstr.split(line,"%s",true) end -- split input on whitespace

		if args[1] then

			chunk=lookup(console.call,args[1]) -- check special console functions

			if chunk and type(chunk)=="function" then -- must be a function

				setfenv(chunk,console.data) -- call with master environment?
				table.remove(args,1) -- remove the function name

			else

				chunk=lookup(console.data,args[1]) -- check for functions in data or master environment

				if chunk and type(chunk)=="function" then -- must be a function
					table.remove(args,1) -- remove the function name
				else
					chunk=nil	-- not found
				end

			end

		end


		if not chunk then -- nothing found above

			args={} -- no arguments

			-- is it an expression?
			err,chunk = compile('return '..line..' ')
			if err then
				-- otherwise, a statement?
				err,chunk = compile(line)
			end

			if chunk then
				setfenv(chunk,console.data) -- compile in master environment will have an overloaded print
			end

		end

		-- if there was a compile error, print it out
		if err then

			_G.print(err)

		elseif chunk then -- begin to run the chunk, it may end or yield

			local co=coroutine.create(chunk)
			console.tasks[#console.tasks+1]=co
			local ret={assert_resume(co,unpack(args))}

			if #ret>0 then _G.print(unpack(ret)) end

		end

	end

	function console.update()

		for idx=#console.tasks,1,-1 do -- backwards
			local co=console.tasks[idx]
			if coroutine.status(co)~="dead" then
				assert_resume(co) -- keep running
			else
				table.remove(console.tasks,idx) -- remove
			end
		end

		console.fps_updates=console.fps_updates+1

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

		console.line_width=oven.view.hx/8
		console.data.main=oven.main and oven.main.console or oven.main	-- update best table of app function?

		if not console.display_disable then
			console.display_clear()
		end
	end

console.gci_last=0
	function console.draw()

font.vbs_idx=1



		if oven.times and oven.win then -- simple benchmarking

			local t=oven.win.time()

		-- count frames
			if (not console.fps) or t-console.fps_last >= 1 then -- update with average value once a sec

				console.fps=console.fps_count or 0
				console.fps_count=0
				console.fps_last=t

			end

			oven.times.update.done()
			oven.times.draw.done()

-- we average the last 60 frames
-- this assumes we are running at 60 frames and not dropping any or the numbers will be a bit off
			local gci=gcinfo()
			local mem=(gci-console.gci_last)
			console.gci_last=gci

			local s=string.format("fps=%2d %s %2d /%3d %4.0fm%s%4.0fk vb=%d tx=%d fb=%d gl=%d vbi=%d gm=%d",
				console.fps,
				string.rep("x",console.fps_updates)..string.rep(" ",8-(console.fps_updates)), -- ideally we only want 1 x
				(oven.times.update.time*1000),
				(oven.times.draw.time*1000),
				gci/1024,
				mem<0 and "-" or "+",math.abs(mem),
				gl.counts.buffers,
				gl.counts.textures,
				gl.counts.framebuffers,
				gl.counts.calls,
				#canvas.vbs,
				cake.images.gl_mem/(1024*1024)
				)
			gl.counts.calls=0 -- reset number of gl calls, so we display number of calls per frame.
			console.fps_updates=0

-- disabled means we can not see anything so need to print info here
			if gl.patch_functions_method=="disable" then
				print(s)
			end

			console.lines_display[1]=s
--			console.display(s)

if oven.times.update.time+oven.times.draw.time > 1/16 then -- log a slow frame
		LOG( "oven","SLOW "..s)
end

			console.fps_count=console.fps_count+1
		end

		cake.views.push_and_apply(console.view)
		gl.PushMatrix()


		font.set(cake.fonts.get(4))
		font.set_size(16,0)

		if console.y > 0 then

			gl.Color(pack.argb4_pmf4(0xc040))
			flat.quad(0,0,oven.view.hx,console.y)

			gl.Color(pack.argb4_pmf4(0xf4f4))


			local i=#console.lines
			local y=console.y-32
			while y>-8 and i>0 do

				font.set_xy(0,y)
				font.draw(console.lines[i])

				y=y-16
				i=i-1
			end

			font.set_xy(0,console.y-16)
			font.draw(">"..console.buff.line)

			if console.buff.throb > 128 then
				font.set_xy((console.buff.line_idx+1)*8,console.y-16)
				font.draw("_")
			end

		end


		if console.show_hud then
			if console.flick then
--				console.flick=false
				gl.Color(pack.argb4_pmf4(0xffff))
			else
				console.flick=true
				gl.Color(pack.argb4_pmf4(0xf000))
			end
			for i,v in ipairs(console.lines_display) do

				font.set_xy(1,1+console.y+i*16-16)
				gl.Color(pack.argb4_pmf4(0xf000))
				font.draw(v)

				font.set_xy(0,console.y+i*16-16)
				gl.Color(pack.argb4_pmf4(0xffff))
				font.draw(v)

			end
		end


		gl.PopMatrix()
		cake.views.pop_and_apply()

	end

-- print to the console
	function console.print(...)

		local t={...}
		for i=1,#t do t[i]=tostring(t[i]) end
		local s=table.concat(t,"\t").."\n"

		if console.linehook then console.linehook(s) end -- send print data here

		if not console.lines then return end -- not setup yet

		for _,l in ipairs( wstr.smart_wrap( s , console.line_width) ) do
			table.insert(console.lines,l)

			while #console.lines > 64 do
				table.remove(console.lines,1)
			end
		end

	end

-- print to the display overlay, this must be done *every* frame draw cycle
	function console.display(...)

		if not console.lines then return end -- not setup yet
		if console.display_disable then return end -- do not print right now

		local t={...}
		for i=1,#t do t[i]=tostring(t[i]) end
		local s=table.concat(t,"\t").."\n"

		for _,l in ipairs( wstr.smart_wrap( s , console.line_width) ) do
			table.insert(console.lines_display,l)
		end

	end

	function console.display_clear()
		console.lines_display={""}
	end

	function console.mouse(act,x,y,key)
	end

	function console.screen_mode(mode)

		if type(mode)=="boolean" and mode==true then

			if wwin.flavour=="emcc" then-- emcc should only ever switch to full as the browser will force us back
				win:show("win")
				win:show("full")
				win:show("win")
				win:show("full")
			elseif win.view then -- we can do smart toggles but this does not read the real state from the system so may be out of sync
				if     win.view=="win" then
					win:show("max")
				elseif win.view=="max" then
					win:show("full")
				elseif win.view=="full" then
					win:show("win")
				else
					win:show("full")
				end
			else -- just try full
				win:show("full")
			end

		end
		if console.screen_mode_data then
			console.screen_mode_data:value(win.view)
		end
		return win.view
	end

	function console.msg(m)
		

		if     m.class=="key" and ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) and m.action==1 then
			console.shift_key=true
		elseif m.class=="key" and ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) and m.action==-1 then
			console.shift_key=false
		end

		if ( m.class=="key" and m.keyname=="f11" and m.action==1 ) then -- fullscreen toggle
			if console.shift_key then
				oven.view.stretch=not oven.view.stretch
			else
				console.screen_mode(true)
			end
			return nil
		end

		if console.input_disable then return m end -- can turn off key intercepts

		if m.class=="key" then
			if console.keypress(nil,m.keyname,m.action) then return nil end
		end

		if m.class=="text" then
			if console.keypress(m.text) then return nil end
		end

		return m
	end

	function console.keypress(ascii,key,act)

--print("conkey",key)
		if key=="grave" then

			if act==-1 then
				if console.show then

					console.show=false
					console.show_hud=false

				elseif console.show_hud then

					console.show=true
					console.buff.throb=255
				else
					console.show_hud=true
				end
			end

			return true
		end

		if console.show then

			if act==1 or act==0 then

				if key=="pageup" then

					console.y_show=console.y_show-16

				elseif key=="pagedown" then

					console.y_show=console.y_show+16

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
			console.print( ... )
			if print_old then
				print_old( ... )
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

--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- handle command line arguments

M.bake=function(oven,cmd)
	local cmd=cmd or {}
	cmd.oven=oven
	
	cmd.modname=M.modname
	
	local gui=oven.rebake(oven.modname..".gui")

	-- custom options or default values from the oven
	cmd.start=function(opts)
		opts=opts or oven.opts
		
--		dprint(opts)

--		gui.texteditor.txt.set_text("Hello")




		local args=require("cmd.args").bake({inputs={

			{	"swanky-edit",	false,	"force swanky.edit app. When running a combined swanky app this forces edit mode.", },
			{	"help",			false,	"Print help and exit.", },
			{	"console",		false,	"Keep console open.", },
			{	"logs",			false,	[[

Choose log verbosity. Set to true to enable all logs or use a string of 
+- flags eg --logs=+this-that to enable and disable the given log 
prefixes.

]], },
			{	"show",			"win",	[[

Choose window mode. win|max|full for normal maximized or fullscreen.

]], },
			{	1,			"swanky.edit file.txt",	[[

Load file.txt for editing.

			]], },

		}}):parse(arg):sanity()
			
		if args.data.help then
			print(table.concat(args:help(),"\n"))
			oven.next=true
			return
		end
		
		cmd.args=args
	end
	
	return cmd
end

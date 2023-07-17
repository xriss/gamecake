
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local wsandbox=require("wetgenes.sandbox")



-- generic options for all commands
local default_inputs=function(inputs)
	for i,v in ipairs{

		{	"help",			false,	"Print help and exit.", },

	} do
		inputs[#inputs+1]=v
	end
	return inputs
end




local args=require("cmd.args").bake({inputs=default_inputs{

	{	1,			arg[0].." DIR",	[[

Dump all internal lua files into the given DIR/lua/* Use . to dump 
all files into ./lua/* but beware this will overwrite existing files.

Note that all the files found in lua/* will override our internal 
files so this can be used to export all the internal scripts and make 
slight tweaks to them.

An exception to this are the very low level workings such as the 
lua/fun.lua library. These are simply loaded too early in the startup 
process and this functionality is not available.

	]], },

}}):parse(arg):sanity()
	
if not ( arg[1] ) then args.data.help=true end 

if args.data.help then
	print(table.concat(args:help(),"\n"))
else


	print( "DUMPING INTO "..arg[1] )
	require("wetgenes").savescripts(arg[1])

end


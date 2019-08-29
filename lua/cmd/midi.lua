
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local wgrd=require("wetgenes.grd")

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local wmidi=require("wetgenes.midi")

local cmds={
	{ "list",		"List available midi connections."},
}
for i,v in ipairs(cmds) do
	v.name=v[1]
	v.help=v[2]
end
table.sort(cmds,function(a,b) return a.name<b.name end)


-- generic options for all commands
local default_inputs=function(inputs)
	for i,v in ipairs{

		{	"help",			false,	"Print help and exit.", },

	} do
		inputs[#inputs+1]=v
	end
	return inputs
end



local cmd=table.remove(arg,1)       -- check what cmd is asked for
local cmd=cmd and string.lower(cmd) -- force lowercase

if cmd=="list" then

	local args=require("cmd.args").bake({inputs=default_inputs{

--		{	"in",		false,	"Input chanels only.", },
--		{	"out",		false,	"Output chanels only.", },

	}}):parse(arg):sanity()
	
	if args.data.help then
		print("\n"..arg[0].." list --OPTIONS {in|out} \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end
	
	local m=wmidi.create()

	ls( m:clients() )

--	midialsa.start()
	
--[[
	ls( midialsa.listclients() )

	ls( midialsa.listnumports() )
	
	ls( midialsa.listconnectedto() )
	
	ls( midialsa.listconnectedfrom() )
	
	repeat

		local m=midialsa.input()

		ls(m)

	until false
]]
	

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

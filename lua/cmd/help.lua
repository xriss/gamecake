
local wstr=require("wetgenes.string")
local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local cmds={
	{ "help",		"List available commands."},
	{ "http",		"Static web server."},
	{ "gltf",		"Process a 3d thing."},
	{ "grd",		"Process an image."},
	{ "midi",		"Midi connections."},
}
for i,v in ipairs(cmds) do
	v.name=v[1]
	v.help=v[2]
end
table.sort(cmds,function(a,b) return a.name<b.name end)



print("\ngamecake -lcmd -- COMMAND ".."...\n")
print("Where COMMAND is one of the following :\n")
print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
os.exit(0)
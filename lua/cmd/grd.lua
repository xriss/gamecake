
local doublewrap=require("cmd.args").doublewrap

local cmds={
	{ "quant",		"Quantize to an automatically generated 256 colors or less palette based image."},
	{ "resize",		"Resize image, to smaller or larger."},
	{ "convert",	"Convert image to a new graphics file format."},
}
for i,v in ipairs(cmds) do
	v.name=v[1]
	v.help=v[2]
end
table.sort(cmds,function(a,b) return a.name<b.name end)


local cmd=table.remove(arg,1)       -- check what cmd is asked for
local cmd=cmd and string.lower(cmd) -- force lowercase

if cmd=="quant" then

	local args=require("cmd.args").bake({inputs={

		{	"colors",	0,		"Number of colors between 2 and 256 that we wish to reduce image to, set to 0 for no change.",	},
		{	"dither",	4,		"Amount of dither between 0 and 6 where 0 is none, defaults to a middle value of 4 which gives pleasant noticeable patterns.",		},
		{	"help",		false,	"Print help and exit.",							},

	}}):parse(arg)
--	for i,v in pairs(args.data) do print(i,v) end

	if args.data.help or not args[1] then
		print("\n"..arg[0].." quant OPTIONS ...\n")
		print( "where OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end

	

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

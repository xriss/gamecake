
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local wgrd=require("wetgenes.grd")

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local cmds={
	{ "quant",		"Quantize to an automatically generated 256 colors or less palette based image."},
--	{ "resize",		"Resize image, to smaller or larger."},
--	{ "convert",	"Convert image to a new graphics file format."},
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

		{	"colors",		256,	"Number of colors between 2 and 256 that we wish to reduce image to.", },

		{	"dither",		4,		"Amount of dither between 0 and 6 where 0 is none, defaults to a middle value of 4 which gives pleasant noticeable patterns.", },

		{	"overwrite",	false,	"Will refuse to overwrite a file unless this flag is set.", },

		{	"output",		"{dirname}{basename}.quant{extension}",
									"Output filename, {dirname}{basename}{extension} are replaced with the input path components.", },

		{	"help",			false,	"Print help and exit.", },

	}}):parse(arg)
--	for i,v in pairs(args.data) do print(i,v) end

	if args.data.help or not args.data[1] then
		print("\n"..arg[0].." quant --OPTIONS input {input2} {input3} {input...} \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end
	
	for i,v in ipairs(args.data) do

		local path=splitpath( args.data[i] )
		local input=path.path
		local output=wstr.replace( args.data.output , path )

		print( input.." -> "..output )
		
		if not args.data.overwrite then
			local fp=io.open(output,"r")
			if fp then
				fp:close()
				print("File exists, aborting, rerun with --overwrite to force.")
				os.exit(20)
			end
		end
		
		local g=assert( wgrd.create(input) )
		g:convert( wgrd.FMT_U8_RGBA_PREMULT )
		g:quant( args.data.colors , args.data.dither )
		assert( g:save( output ) )

	end

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

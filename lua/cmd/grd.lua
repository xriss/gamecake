
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local wgrd=require("wetgenes.grd")

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local cmds={
	{ "quant",		"Quantize to an automatically generated 256 colors or less palette based image."},
	{ "convert",	"Convert image to a new graphics file format."},
}
for i,v in ipairs(cmds) do
	v.name=v[1]
	v.help=v[2]
end
table.sort(cmds,function(a,b) return a.name<b.name end)


-- generic options for all commands
local default_inputs=function(inputs)
	for i,v in ipairs{

		{	"overwrite",	false,	"Will refuse to overwrite a file unless this flag is set.", },

		{	"output",		"{dirname}{basename}.output{extension}",
									"Output filename, {dirname}{basename}{extension} are replaced with the input path components, {idx} will be replaced with the file number in three digits, eg 001 and {filename} may be used in place of {basename}{extension}.", },

		{	"help",			false,	"Print help and exit.", },

	} do
		inputs[#inputs+1]=v
	end
	return inputs
end

local overwrite_abort=function(args,output)
	if not args.data.overwrite then
		local fp=io.open(output,"r")
		if fp then
			fp:close()
			print("")
			print("File exists, aborting, rerun with --overwrite to force.")
			print("")
			os.exit(20)
		end
	end
end

local check_load=function(input)
	local g=assert( wgrd.create() )
	local suc,err=g:load(input)
	if not suc then
		print("Failed to read from "..input.."\n"..err)
		os.exit(20)
	end
	return g
end

local check_save=function(g,output)
	local suc,err=g:save(output)
	if not suc then
		print("Failed to write to "..output.."\n"..err)
		os.exit(20)
	end
	return g
end


local cmd=table.remove(arg,1)       -- check what cmd is asked for
local cmd=cmd and string.lower(cmd) -- force lowercase

if cmd=="quant" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	"colors",		256,	"Number of colors between 2 and 256 that we wish to reduce image to.", },

		{	"dither",		4,		"Amount of dither between 0 and 6 where 0 is none, defaults to a middle value of 4 which gives pleasant noticeable patterns.", },

	}}):parse(arg):sanity()
	
	if args.data.help or not args.data[1] then
		print("\n"..arg[0].." quant --OPTIONS input {input2} {input3} {input...} \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end
	
	for i,v in ipairs(args.data) do

		local path=splitpath( args.data[i] )
		path.idx=string.format("%03d",i)
		local input=path.path
		local output=wstr.replace( args.data.output , path )
		

		print( input.." -> "..output )
		
		overwrite_abort(args,output)
		
		local g=check_load(input)

		assert( g:convert( wgrd.FMT_U8_RGBA ) )
		assert( g:quant( args.data.colors , args.data.dither ) )

		local g=check_save(g,output)

	end

elseif cmd=="convert" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	"fmt",	"U8_RGBA",	"The type of pixel data to convert the image into, many of these are not supported by all file formats. See the grd documentation for details but this is a list of possible values : U8_RGBA, U8_ARGB, U8_RGB, U8_INDEXED, U8_LUMINANCE, U8_ALPHA, U16_RGBA_5551, U16_RGBA_4444, U16_RGBA_5650 and each can have _PREMULT appended to the end for premultiplied alpha.", },

		}}):parse(arg):sanity()
	
	if args.data.help or not args.data[1] then
		print("\n"..arg[0].." convert --OPTIONS input {input2} {input3} {input...} \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end
	
	for i,v in ipairs(args.data) do

		local path=splitpath( args.data[i] )
		path.idx=string.format("%03d",i)
		local input=path.path
		local output=wstr.replace( args.data.output , path )
		
		print( input.." -> "..output )
		overwrite_abort(args,output)
		
		local g=check_load(input)

		assert( g:convert( args.data.fmt ) )

		local g=check_save(g,output)

	end

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

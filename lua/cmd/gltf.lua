
local wpath=require("wetgenes.path")
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local geom_gltf=require("wetgenes.gamecake.spew.geom_gltf")

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local cmds={
	{ "info",		"Display information about a gltf file."},
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

if cmd=="info" then

	local args=require("cmd.args").bake({inputs=default_inputs{

	}}):parse(arg):sanity()
	
	if args.data.help or not args.data[1] then
		print("\n"..arg[0].." info --OPTIONS input.gltf \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end
	
	for i,v in ipairs(args.data) do

		local path=wpath.parse( args.data[i] )
		path.idx=string.format("%03d",i)
		
		local input=path[1]..path[2]..path[3]..path[4]

		print( "Loading gltf "..input )
		local gltf
		do
			local fp=io.open(input,"rb")
			gltf=fp:read("*a")
			fp:close()
		end
		gltf=geom_gltf.parse(gltf)
		
		for i=1,#gltf.buffers do
			local v=gltf.buffers[i]
			if type(v)=="table" then -- try and convert data to strings
				if v.uri then -- try and load
					local scary=false
					if string.find(v.uri,"..",1,true) then scary=true end
					if string.find(v.uri,":",1,true)  then scary=true end
					if string.find(v.uri,"/",1,true)  then scary=true end
					if string.find(v.uri,"\\",1,true) then scary=true end
					if not scary then
						local bname=path[1]..path[2]..v.uri
						print("Loading buffer "..bname)
						local fp=io.open(bname,"rb")
						gltf.buffers[i]=fp:read("*a")
						fp:close()
					else
						print("Ignoring scary buffer "..v.uri)
					end
				end
			end
		end

		geom_gltf.info(gltf)

	end

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

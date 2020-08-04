
local wpath=require("wetgenes.path")
local wgrd=require("wetgenes.grd")
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


local load_gltf=function(fname)

	local it_is_scary=function(uri)
		local scary=false
		if string.find(uri,"..",1,true) then scary=true end
		if string.find(uri,":",1,true)  then scary=true end
		if string.find(uri,"/",1,true)  then scary=true end
		if string.find(uri,"\\",1,true) then scary=true end
		return scary
	end

	local path=wpath.parse( fname )
	
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
			if v.uri then -- try and load file

				if it_is_scary(v.uri) then
					print("Ignoring scary buffer "..v.uri)
				else
					local bname=path[1]..path[2]..v.uri
					print("Loading buffer "..bname)
					local fp=io.open(bname,"rb")
					gltf.buffers[i]=fp:read("*a")
					fp:close()
				end

			end
		end
	end

	if gltf.images then
		for i=1,#gltf.images do
			local v=gltf.images[i]
			if type(v)=="table" then -- try and convert data to strings
				if v.uri then -- try and load file

					if it_is_scary(v.uri) then
						print("Ignoring scary buffer "..v.uri)
					else
						local iname=path[1]..path[2]..v.uri
						print("Loading image "..iname)
						gltf.images[i]=wgrd.create():load_file(iname)
					end

				elseif v.bufferView then -- try and load data

					local view=gltf.bufferViews[v.bufferView+1]
					local buffer=gltf.buffers[view.buffer+1]
					local data=string.sub(buffer,(view.byteOffset or 0)+1,(view.byteOffset or 0)+view.byteLength+1)
					gltf.images[i]=wgrd.create():load_data(data)

				end
			end
		end
	end

	return gltf
end


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
	
	for i=1,#args.data do

		local gltf=load_gltf( args.data[i] )

		geom_gltf.info(gltf)

	end

elseif cmd=="view" then

	local args=require("cmd.args").bake({inputs=default_inputs{

	}}):parse(arg):sanity()
	
	if args.data.help or not args.data[1] then
		print("\n"..arg[0].." info --OPTIONS input.gltf \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end


	local gltf=load_gltf( args.data[1] )

	geom_gltf.view(gltf)


else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

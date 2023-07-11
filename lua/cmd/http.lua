
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local cmds={
	{ "server",		"Start a simple http server to statically expose all the files in the current directory on localhost."},
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

if cmd=="server" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	"policy",		"none",	[[
		
Use --policy=wasm to send CORS headers that make wasm happy.

]], },

		{	"port",		12211,	[[
		
Port to listen on.

]], },

		{	"host",		"*",		[[
		
Host to listen on.

]], },

		{	"location",		".",		[[
		
Location of files to expose.

]], },

	}}):parse(arg):sanity()
	
	if args.data.help then
		print("\n"..arg[0].." server --OPTIONS \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end

	local pegasus = require "pegasus"
	
	local plugin={}
	if args.data.policy=="wasm" then -- include headers needed for wasm
		plugin.newRequestResponse=function(self,request,response)
			response:addHeader(	"Cross-Origin-Opener-Policy"	,	"same-origin"	)
			response:addHeader(	"Cross-Origin-Embedder-Policy"	,	"require-corp"	)
		end
	end
	
	local server = pegasus:new({
	  host=args.data.host,
	  port=args.data.port,
	  location=args.data.location,
	  plugins={plugin},
	})

	server:start()

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

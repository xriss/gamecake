
print("testing")

print("0",arg[0])
for i,v in ipairs(arg) do print(i,v) end

print("finishing")

--[[

local cli = require "cliargs"

local cli_name="gamecake.grd"
local cli_version="0.0"

-- this is called when the flag -v or --version is set
local function print_version()
  print(cli_name..": version "..cli_version)
  print("lua_cliargs: version " .. cli.VERSION)
  os.exit(0)
end

cli:set_name("run.grd")

cli:argument(	"INPUT",			"Path to the input file."														)
cli:option(		"--colors=0-256",	"Number of colors 2-256 to reduce image to.",					"0"				)
cli:option(		"--dither=0-6",		"Amount of dither 0 for non 6 for lots.",						"4"				)
cli:flag(		"--version",		"Prints the program's version and exits.",						print_version	)


-- remove original args upto -- if invoked with a -lcmd.something
if arg[1] and arg[1]:sub(1,6)=="-lrun." then
	while true do
		local check=table.remove(arg,1)
		if not arg[1] or check=="--" then break end
	end
end

-- Parses from _G['arg']
local args, err = cli:parse(arg)

if not args and err then
  -- something wrong happened and an error was printed
  print(string.format('%s: %s; re-run with help for usage', cli.name, err))
  os.exit(1)
end

print("Input file: " .. args["INPUT"])
print("colors: " .. args["colors"])
print("dither: " .. args["dither"])

os.exit(0)
]]

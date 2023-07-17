
-- start by removing the args that got us here
-- which should have been "gamecake -lcmd --"

if arg[1] and arg[1]=="-lcmd" then 
	table.remove(arg,1)
	if arg[1] and arg[1]=="--" then table.remove(arg,1) end
end

require("apps").default_paths()     -- set search paths to smarter defaults

local cmd
for i=1,#arg do
	cmd=table.remove(arg,1)       -- check what cmd is asked for
	if cmd and cmd:sub(1,1)=="-" then cmd=nil end -- ignore args
	if cmd then break end -- found
end

if cmd then
	arg[0]="gamecake."..cmd           -- remember cmd name	
	require("cmd."..cmd)              -- try and run the actual cmd
else
	require("cmd.help")              -- print help
end

-- this file has been required as a module on the command line
-- so we need to explicitly exit at the end to stop anything else running

os.exit(0)

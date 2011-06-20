-- make sure we have the package path setup and working to find everything, this should be made smrtr
-- i mean you still have to find this file in the first place...

package.path =package.path.. ";../bin/lua/?.lua;../bin/lua/?/init.lua"

if os.getenv("SHELL") and string.sub(os.getenv("SHELL"),1,5)== "/bin/" then
	package.cpath=package.cpath..";../bin/exe/?.so"
else
	package.cpath=package.cpath..";../bin/exe/?.dll"
end



local wpath=require("wetgenes.path")
local lfs=require("lfs")

print( "wpath.root" , wpath.root )
print( "wpath.separator" , wpath.separator )
print( "wpath.delimiter" , wpath.delimiter )
print( "wpath.winhax" , wpath.winhax )
print( "wpath.home" , wpath.home )

local dir=wpath.currentdir()

print( "currentdir" , dir )

local dd=wpath.parse( dir )
for n,v in pairs( dd ) do
	print( "currentdir" , n , v )
end

dir="/."
print("dir",dir)
for nam in lfs.dir(dir) do
	if nam~="." and nam~=".." then
print("dir",dir,nam)
local dd=wpath.parse( wpath.resolve(dir,nam) )
for n,v in pairs( dd ) do
	if type(n)~="number" then
		print( "dir",dir,nam , n , v )
	end
end
	end
end

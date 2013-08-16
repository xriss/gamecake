
project "lua_lanes"
language "C++"
files { "src/**.cpp" , "src/**.c" , "src/**.h" }

links { "lib_lua" }


defines("LUA_PRELOADLIBS=lua_preloadlibs")

if ANDROID then
	defines("pthread_yield=sched_yield")
	defines("pthread_cancel=0")
end

KIND{kind="lua",name="lua51-lanes",luaname="lua51-lanes",luaopen="lanes"}


local function ldump(fni,fno)

	local out_f   -- file to output to (stdout if nil)
	
	local function OUT( ... )
		(out_f): write( ... );     -- ; actually needed by Lua
		(out_f): write "\n"
	end

	local HEAD= "{ "
	local START= '  '
	local FILL= '%3d,'
	local STOP= ""
	local TAIL= "};\n"

	--
	local function dump( f )
		--
		OUT [[
	/* bin2c.lua generated code -- DO NOT EDIT
	 *
	 * To use from C source: 
	 *    char my_chunk[]=
	 *    #include "my.lch"
	 */]]

		local str= HEAD..'\n'..START
		local len= 0
		
		while true do
			for n=1,20 do
				local c= f:read(1)
				if c then
					str= str..string.format( FILL, string.byte(c) )
					len= len+1
				else
					OUT( str..STOP.. string.format( TAIL, len ) )
					return  -- the end
				end
			end
			OUT(str..STOP)
			str= START
		end
	end

	--
	local function fdump( fni,fno )
	
		out_f=assert( io.open(fno,"w") )
		
		local f= io.open( fni, "rb" )    -- must open as binary
		
		if not f then
			error( "bin2c: cannot open "..fni )
		else
			dump( f )
			f:close()
		end
		
		out_f:close()
	end

-- call the above functions that we grabbed from bin2c.lua

	fdump(fni,fno)

end

-- build this needed source file
ldump("src/keeper.lua","src/keeper.lch")


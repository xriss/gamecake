
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local wgrd=require("wetgenes.grd")

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local SDL=require("SDL")


local cmds={
	{ "screen",		"List or test screen video modes with a window."},
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


local mode_to_string=function(m)
	for n,v in pairs( SDL.pixelFormat ) do
		if v==m.format then m.format_name=n end
	end
	return m.w.."x"..m.h.."."..m.format_name.."/"..m.refreshRate
end

local mode_from_string=function(str)
	str=tostring(str) -- force bools to string 
	local mode={
		w=1280,
		h=720,
		refreshRate=0,
		format=SDL.pixelFormat.RGB888,
	}
	local idx=1
	local names={"w","h","refreshRate"}
	for s in str:gmatch("([%u%d]+)") do
		local n=tonumber(s)
		if tostring(n)==s then -- got a number
			if names[idx] then -- got a place to put it
				mode[ names[idx] ]=n
				idx=idx+1
			end
		else
			n=SDL.pixelFormat[s]
			if n then -- got a pixel format
				mode.format=n
			end
		end
	end
	return mode
end


local cmd=table.remove(arg,1)       -- check what cmd is asked for
local cmd=cmd and string.lower(cmd) -- force lowercase

if cmd=="screen" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	"list",			false,	"List all video modes.", },
		{	"test",			"",		"Test this specific video mode by opening a window.", },

	}}):parse(arg):sanity()
	
	if args.data.help or ( not ( args.data.list or (args.data.test and args.data.test~="") ) ) then
		print("\n"..arg[0].." screen --OPTIONS\n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end

	SDL.init{ SDL.flags.Video }

	if args.data.list then

		local num_video_displays=SDL.getNumVideoDisplays()
		for video_display_idx=0,num_video_displays-1 do
			local num_display_modes=SDL.getNumDisplayModes(video_display_idx)
			print("display : "..video_display_idx)
			for display_mode_idx=0,num_display_modes-1 do
				local mode=SDL.getDisplayMode(video_display_idx,display_mode_idx)
				for n,v in pairs( SDL.pixelFormat ) do
					if v==mode.format then mode.format_name=n end
				end
				print("   mode : "..mode_to_string(mode))
			end
		end	

--	it.screen_mode=t.screen_mode -- try something like "640x480x60.RGB888"

	end
	
	if args.data.test and args.data.test~="" then
	
		local mode=mode_from_string(args.data.test)

		print("Asking for display mode : "..mode_to_string(mode))

		local win= assert(SDL.createWindow {
			width   = mode.w,
			height  = mode.h,
		})
		assert( win:setDisplayMode(mode) )
		win:show()
		assert( win:setFullscreen(SDL.window.Fullscreen) )
		
		local gotmode=win:getDisplayMode()

		win:setFullscreen(0)
		win:restore()

		print("Actual display mode was : "..mode_to_string(gotmode))
	
	end

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end

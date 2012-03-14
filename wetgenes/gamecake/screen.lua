-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.screen")

base=require(...)
meta={}
meta.__index=base


function bake(opts)

	local screen={}
	setmetatable(screen,meta)
	
	screen.cake=opts.cake
	
	screen.width=opts.width or 320
	screen.height=opts.height or 240
	
	screen:project23d(opts.width,opts.height,0.5,1024) -- some dumb defaults
	
	return screen
end




--
-- build a simple field of view projection matrix designed to work in 2d or 3d and keep the numbers
-- easy for 2d positioning.
--
-- setting aspect to 640,480 and fov of 1 would mean at a z depth of 240 (which is y/2) then your view area would be
-- -320 to +320 in the x and -240 to +240 in the y.
--
-- fov is a tan like value (a view size inverse scalar) so 1 would be 90deg, 0.5 would be 45deg and so on
--
-- the depth parameter is only used to limit the range of the zbuffer so it covers 0 to depth
--
-- The following would be a reasonable default for a 640x480 screen.
--
-- build_project23d(640,480,0.5,1024)
--
-- then at z=((480/2)/0.5)=480 we would have one to one pixel scale...
-- the total view area volume from there would be -320 +320 , -240 +240 , -480 +(1024-480)
--
-- screen needs to contain width and height of the display which we use to work
-- out where to place our view such that it is always visible and keeps its aspect.
--
base.project23d = function(screen,width,height,fov,depth)

	local aspect=height/width
	
--	screen.width=screen.width or 100 -- just in case it is missing?
--	screen.height=screen.height or 100 
	
	screen.view_width=width
	screen.view_height=height
	screen.view_fov=fov
	screen.view_depth=depth

	local m={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} -- defaults
	screen.pmtx=m
	
	local f=depth
	local n=1

	local screen_aspect=(screen.height/screen.width)
		
	if (screen_aspect > (aspect) ) 	then 	-- fit width to screen
	
		m[1] = ((aspect)*1)/fov
		m[6] = -((aspect)/screen_aspect)/fov
		
		screen.xs=1
		screen.ys=screen_aspect/aspect
	else									-- fit height to screen
	
		m[1] = screen_aspect/fov
		m[6] = -1/fov
		
		screen.xs=aspect/screen_aspect
		screen.ys=1
	end
	
	
	m[11] = -(f+n)/(f-n)
	m[12] = -1

	m[15] = -2*f*n/(f-n)
	
	return m -- return the matrix but we also updated the screen
end

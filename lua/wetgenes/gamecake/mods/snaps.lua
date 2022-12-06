--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local log,dump=require("wetgenes.logs"):export("log","dump")

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local wwin=require("wetgenes.win")

local wgrd   =require("wetgenes.grd")
local lfs ; (function() pcall( function() lfs=require("lfs") end ) end)()

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,snaps)

	snaps=snaps or {}
	
	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas

	function snaps.setup()
		if not lfs then return end

		lfs.mkdir(wwin.files_prefix.."snaps")
	end

	function snaps.clean()
	end
	
	function snaps.update()

		if snaps.auto and snaps.frame_rate and snaps.frame_time then --  a nonskip frame rate limiter for animation capture
			if wwin.hardcore.sleep then
				while snaps.frame_time>oven.win:time() do wwin.hardcore.sleep(0.0001) end -- sleep here until we need to update
			end
			snaps.frame_time=snaps.frame_time+snaps.frame_rate -- step a frame forward
		end

	end
	


	function snaps.get_grd()
		local g
		if snaps.fbo and snaps.auto then -- use this fbo for animations
			g=snaps.fbo:download()
		else -- full screen
			g=wgrd.create( wgrd.FMT_U8_RGBA_PREMULT , oven.win.width , oven.win.height , 1 )
			gl.ReadPixels(0,0,oven.win.width,oven.win.height,gl.RGBA,gl.UNSIGNED_BYTE,g.data)
			g:flipy() -- open gl is upside down
		end
		return g
	end
	
	function snaps.save()
		local name=os.date("%Y%m%d_%H%M%S")
log("snap",wwin.files_prefix.."snaps/"..name..".png")
		local g=snaps.get_grd()
		g:save(wwin.files_prefix.."snaps/"..name..".png")
	end

	function snaps.begin_record(frame_max,frame_skip)

		snaps.frame_max=frame_max or snaps.frame_max
		snaps.frame_skip=frame_skip or snaps.frame_skip

		snaps.auto=os.date("%Y%m%d_%H%M%S")
		snaps.idx=0
		snaps.frame=0
		snaps.list={}
		snaps.frame_rate=oven.frame_rate -- time per frame
		snaps.frame_time=oven.frame_time -- start time
		oven.frame_rate=nil -- disable limiter
	end

	function snaps.draw()


		if snaps.auto then
			snaps.frame=snaps.frame+1
			
			if snaps.frame%snaps.frame_skip == 0 then
				snaps.idx=snaps.idx+1

				local name=snaps.auto.."_"..(("%04d"):format(snaps.idx))

if lfs then -- shove files in dir
		lfs.mkdir(wwin.files_prefix.."snaps/"..snaps.auto)
		name=snaps.auto.."/"..(("%04d"):format(snaps.idx))
end



				local g=snaps.get_grd()

--				assert(g:save(path)) -- this is slow, so lets hack in a simple PAM output

				g:convert("FMT_U8_RGB") -- drop the alpha
				
				local path=wwin.files_prefix.."snaps/"..name..".pam"
log("snap","Auto "..path)

				local fp=assert(io.open(path,"wb"))
				fp:write(string.format([[
P7
WIDTH %.f
HEIGHT %.f
DEPTH 3
MAXVAL 255
TUPLTYPE RGB
ENDHDR
]],g.width,g.height))
				fp:write(g:pixels(0,0,g.width,g.height,""))
				fp:close()

				snaps.list[#snaps.list+1]=path

				if snaps.frame>=snaps.frame_max then -- finished

				local fname=wwin.files_prefix.."snaps/"..snaps.auto..".sh"

log("snap","run this script "..fname)
log("snap","to create this "..wwin.files_prefix.."snaps/"..snaps.auto..".mp4")

				local f=io.open(fname,"w")
				if f then
					f:write([[
cd `dirname $0`
ffmpeg -framerate 30 -i "]]..snaps.auto..[[/%04d.pam" -vf "pad=ceil(iw/2)*2:ceil(ih/2)*2" -pix_fmt yuv420p -y ]]..snaps.auto..[[.mp4
]])
					f:close()
				end

					snaps.auto=false
					oven.frame_rate=snaps.frame_rate

				end
				
			end

			return nil
		end

	end
		
	snaps.list={}
	snaps.shift_key=false
	snaps.auto=false
	snaps.idx=0
	snaps.frame=0
	snaps.frame_max=60*20  -- x seconds, at 60fps
	snaps.frame_skip=2     -- only record every other frame, so 30 fps output.
	function snaps.msg(m)
		if not lfs then return m end

--print(wstr.dump(m))

		if     m.class=="key" and ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) and m.action==1 then
			snaps.shift_key=true
		elseif m.class=="key" and ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) and m.action==-1 then
			snaps.shift_key=false
		end

		if ( m.class=="key" and m.keyname=="f12" and m.action==1 ) then

			if snaps.shift_key then
				if snaps.auto then
					snaps.auto=nil
				else
					snaps.begin_record()
				end
				return nil
			end

			snaps.save()

			return nil
		end
		return m
	end

	return snaps
end

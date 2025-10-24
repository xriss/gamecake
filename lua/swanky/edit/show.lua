--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")
local function ls(...) print(wstr.dump({...})) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- keep track of all the open documents

M.bake=function(oven,show)
	local show=show or {}
	show.oven=oven
	
	show.modname=M.modname

	show.stoy=oven.rebake(M.modname.."_stoy")
	show.vtoy=oven.rebake(M.modname.."_vtoy")
	show.fun64=oven.rebake(M.modname.."_fun64")

	local gui=oven.rebake(oven.modname..".gui")

	local gl=oven.gl

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local docs=oven.rebake(oven.modname..".docs")

--	show.enabled="glsl"

	show.setup=function()
--		show.fbo=framebuffers.create( 0,0 , 0 , { no_uptwopow=true } )
	end
	
	show.clean=function()
	end
	
	show.cam=M4()
	show.rot=V3()
	show.pos=V3()
	show.siz=V3(1,1,1)

	show.cam2d=M4()
	show.pos2d=V3()
	show.siz2d=V3(1,1,1)

	show.cam3d=M4()
	show.orbit=V3()
	show.dolly=-256

	show.mouse=V4(0,0,0,0) -- shadertoy style mouse values
	
	show.cam_reset=function()
		show.mouse_start=nil
		
		show.cam:identity()
		show.rot:set(0,0,0)
		show.pos:set(0,0,0)

		show.mouse_start2d=nil

		show.cam2d:identity()
		show.pos2d:set(0,0,0)
		show.siz2d:set(1,1,1)
		
		show.mouse_start3d=nil

		show.cam3d:identity()
		show.orbit:set(0,0,0)
		show.dolly=256
	end
	show.cam_reset()

	show.cam_build=function()
		local m=show.cam:identity()
		m:translate(show.pos)
		m:rotate( show.rot[1] , {0,1,0} )
		m:rotate( show.rot[2] , {1,0,0} )

		local m=show.cam2d:identity()
		m:translate(show.pos2d)
		m:scale(show.siz2d)

		-- This is a camera so we are applying reverse transforms...
		local m=show.cam3d
		m:identity()
--			m:translate( camera.pos[1] , camera.pos[2] , camera.pos[3] )
		m:translate( 0,0, 0.0 - show.dolly )
		m:rotate( show.orbit[1] ,  0, 1, 0 )
		m:rotate( show.orbit[2] ,  1, 0, 0 )

--print( "dolly" , show.orbit[1] , show.orbit[2] , show.dolly )

	end
	
	show.cam_build()
	
	show.mode=nil
	show.mode_height=nil
	show.update=function()
	
		local mode=show.get_mode()
		if mode~=show.mode then
			show.mode=mode
			if mode=="stoy" or mode=="vtoy" or mode=="fun64" then

				local ss=math.pow(2,gui.master.datas.get_value("run_scale")-1)
				
				gui.master.ids.runscale.sx=ss
				gui.master.ids.runscale.sy=ss

				if gui.master.ids.dock2.hy > 0 then
					show.mode_height=gui.master.ids.dock2.hy
				end
				if not show.mode_height or show.mode_height<ss or show.mode_height>gui.master.ids.dock2.parent.hy-ss then
					show.mode_height = math.floor(gui.master.ids.dock2.parent.hy/2)
				end
				gui.master.ids.dock2.hy=show.mode_height
				gui.master:layout()

			else

				gui.master.ids.runscale.sx=1
				gui.master.ids.runscale.sy=1

				if gui.master.ids.dock2.hy > 0 then
					show.mode_height=gui.master.ids.dock2.hy
				end
				gui.master.ids.dock2.hy=0
				gui.master:layout()
			
			end
		end

	end
	
	show.get_mode=function()
		local d=gui.datas.get("run_mode")
		local mode=d.str
		if mode=="auto" then -- pick from file extension
			if docs.doc and docs.doc.filename then
				local s=docs.doc.filename
				local endswith=function(a,b)
					if a:sub(-#b)==b then return true end
					return false
				end
				if     endswith(s,".stoy.glsl") then mode="stoy"
				elseif endswith(s,".vtoy.glsl") then mode="vtoy"
				elseif endswith(s,".fun.lua")   then mode="fun64"
				end
			end
		end
		return mode
	end
		
		
	show.widget_msg=function(widget,m)
		local buildcam

		do
			local mode=show.get_mode()
			local it=show[mode]
			if it and it.widget_msg then
				it.widget_msg(widget,m)
			end
		end

		if m.class~="mouse" then return end

		if m.keyname=="middle" and m.action==-1 then -- toggle 3d 2d camerea mode
		
			show.cam_reset()
			buildcam=true

		end
	
		if m.keyname=="left" then
			if m.action ==  1 then show.mouse_start={"left",m.x,m.y,V3(show.rot)} end
			if m.action == -1 then show.mouse_start=nil end
		elseif m.keyname=="right" then
			if m.action ==  1 then show.mouse_start={"right",m.x,m.y,V3(show.pos)} end
			if m.action == -1 then show.mouse_start=nil end
		elseif m.keyname=="middle" then
			if m.action ==  1 then show.mouse_start={"middle",m.x,m.y,V3(show.pos)} end
			if m.action == -1 then show.mouse_start=nil end
		elseif m.keyname=="wheel_add" and m.action == -1 then
			show.pos=show.pos + ( V3( show.cam[ 9],show.cam[10],show.cam[11]  ) * 100 )
			buildcam=true
		elseif m.keyname=="wheel_sub" and m.action == -1 then
			show.pos=show.pos + ( V3( show.cam[ 9],show.cam[10],show.cam[11]  ) *-100 )
			buildcam=true
		end
		
		if m.keyname=="left" then
			if m.action ==  1 then show.mouse_start2d={"left",m.x,m.y,V3(show.pos)} end
			if m.action == -1 then show.mouse_start2d=nil end
		elseif m.keyname=="right" then
			if m.action ==  1 then show.mouse_start2d={"right",m.x,m.y,V3(show.siz)} end
			if m.action == -1 then show.mouse_start2d=nil end
		elseif m.keyname=="middle" then
			if m.action ==  1 then show.mouse_start2d={"middle",m.x,m.y,V3(show.rot)} end
			if m.action == -1 then show.mouse_start2d=nil end
		elseif m.keyname=="wheel_add" and m.action == -1 then
			show.siz2d:scale(7/8)
			buildcam=true
		elseif m.keyname=="wheel_sub" and m.action == -1 then
			show.siz2d:scale(8/7)
			buildcam=true
		end
		
		if m.keyname=="left" then
			if m.action ==  1 then show.mouse_start3d={"left",m.x,m.y,V3(show.orbit)} end
			if m.action == -1 then show.mouse_start3d=nil end
		elseif m.keyname=="right" then
--			if m.action ==  1 then show.mouse_start3d={"right",m.x,m.y,V3(show.siz)} end
--			if m.action == -1 then show.mouse_start3d=nil end
		elseif m.keyname=="middle" then
--			if m.action ==  1 then show.mouse_start3d={"middle",m.x,m.y,V3(show.rot)} end
--			if m.action == -1 then show.mouse_start3d=nil end
		elseif m.keyname=="wheel_add" and m.action == -1 then
			show.dolly=show.dolly+8
			buildcam=true
		elseif m.keyname=="wheel_sub" and m.action == -1 then
			show.dolly=show.dolly-8
			buildcam=true
		end

--		ls(m)
		local x,y=widget:mousexy(m.x,m.y)
		if m.keyname=="left" and m.action ==  1 then -- click remember
			show.mouse=V4(x,widget.hy-y,-x,-(widget.hy-y)) -- shadertoy style mouse values
		end
		if show.mouse then
			if show.mouse_start and show.mouse_start[1]=="left" then
				show.mouse[1]= x
				show.mouse[2]= widget.hy-y
				show.mouse[3]= (math.abs(show.mouse[3]))
			else
				show.mouse[3]=-(math.abs(show.mouse[3]))
			end
		end

		if show.mouse_start and ( widget.master.active==widget or widget.master==widget ) then
		
			if show.mouse_start[1]=="left" then
				local ax=(m.x-show.mouse_start[2])/widget.hy
				local ay=(m.y-show.mouse_start[3])/widget.hy
				show.rot[1]=show.mouse_start[4][1]+(ax*90)
				show.rot[2]=show.mouse_start[4][2]-(ay*90)
				
				show.rot[1]=show.rot[1]%360 -- wrap x
				if show.rot[2]<-90 then show.rot[2]=-90 end -- clamp y
				if show.rot[2]> 90 then show.rot[2]= 90 end

			elseif show.mouse_start[1]=="right" then
				local ax=(m.x-show.mouse_start[2])/widget.hy
				local ay=(m.y-show.mouse_start[3])/widget.hy
				show.mouse_start[2]=m.x
				show.mouse_start[3]=m.y

				show.pos=show.pos + ( V3( show.cam[ 1],show.cam[ 2],show.cam[ 3]  ) * ax*-1000 )
								  + ( V3( show.cam[ 5],show.cam[ 6],show.cam[ 7]  ) * ay*-1000 )

			elseif show.mouse_start[1]=="middle" then
				local ax=(m.x-show.mouse_start[2])/widget.hy
				local ay=(m.y-show.mouse_start[3])/widget.hy
				show.mouse_start[2]=m.x
				show.mouse_start[3]=m.y

--				show.pos=show.pos + ( V3( show.cam[ 9],show.cam[10],show.cam[11]  ) * ay*-1000 )

				local res=1.0-( math.abs(ax) + math.abs(ay) )
				if res<0 then res=0 end
				if res>1 then res=1 end
				show.pos=show.pos * res

			else
				show.mouse_start=nil
			end
		
		end
			
		if show.mouse_start2d and( widget.master.active==widget or widget.master==widget ) then

			if show.mouse_start2d[1]=="left" then
				local ax=(m.x-show.mouse_start2d[2])/widget.hy
				local ay=(m.y-show.mouse_start2d[3])/widget.hy
				show.mouse_start2d[2]=m.x
				show.mouse_start2d[3]=m.y
				show.pos2d=show.pos2d + ( V3( ax*-1 , ay*-1 , 0) * (show.siz2d) )

			elseif show.mouse_start2d[1]=="right" then

			else
				show.mouse_start2d=nil
			end
			
			buildcam=true
		end
		
		if show.mouse_start3d and ( widget.master.active==widget or widget.master==widget ) then

			if show.mouse_start3d[1]=="left" then
				local ax=(m.x-show.mouse_start[2])/widget.hy
				local ay=(m.y-show.mouse_start[3])/widget.hy
				show.orbit[1]=show.mouse_start[4][1]+(ax*90)
				show.orbit[2]=show.mouse_start[4][2]-(ay*90)
				
				show.orbit[1]=show.orbit[1]%360 -- wrap x
				show.orbit[2]=show.orbit[2]%360 -- wrap x
--				if show.orbit[1]<-90 then show.orbit[1]=-90 end -- clamp y
--				if show.orbit[1]> 90 then show.orbit[1]= 90 end

			elseif show.mouse_start3d[1]=="right" then

			else
				show.mouse_start3d=nil
			end
			
			buildcam=true
		end

		if buildcam then
			show.cam_build()
		end
	end

	show.widget_draw=function(px,py,hx,hy) -- draw a widget of this size using opengl

		local mode=show.get_mode()
		local it=show[mode]

		if it then

			it.widget_draw(px,py,hx,hy)

		end
		
	end

	return show
end

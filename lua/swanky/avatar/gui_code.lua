--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wgrd=require("wetgenes.grd")
local wzips=require("wetgenes.zips")
local bitdown=require("wetgenes.gamecake.fun.bitdown")
local _,lfs=pcall( function() return require("lfs") end )

local function dprint(a) print(wstr.dump(a)) end


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.fill=function(gui)

	local oven=gui.oven

	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=oven.gl

	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local avatar=oven.rebake(oven.modname..".avatar")

	local zgui=oven.rebake("wetgenes.gamecake.zone.gui")

gui.widgets_of_dat_id=function(id)
	local its={}
	local idx=0
	gui.master:call_descendents(function(w)
		if	( w.data and w.data.id==id ) or
			( w.datx and w.datx.id==id ) or
			( w.daty and w.daty.id==id ) then
			its[w]=w
		end
	end)
	return pairs(its)
end




function gui.hooks(act,w,dat)

--print(act)

	if act=="click" then

--print(w.id)

		if w.id=="quit"  then

			oven.next=true
			
		elseif w.id=="file_save" then
		
			local filename=gui.datas.get_value("file_dir") .."/".. gui.datas.get_value("file_name")
			avatar.save(filename)

		elseif w.id=="file_load" then

			local filename=gui.datas.get_value("file_dir") .."/".. gui.datas.get_value("file_name")
			avatar.load(filename)
		

		elseif w.id=="layout" then

			if     w.user=="load" then gui.plan_windows_load()
			elseif w.user=="save" then gui.plan_windows_save()
			elseif w.user=="reset" then gui.plan_windows_reset()
			end
		
		elseif w.id=="theme" then
		
			if     w.user=="bright" then	gui.master.color_theme=gui.master.color_theme_bright
			elseif w.user=="dark"   then	gui.master.color_theme=gui.master.color_theme_dark
			end
			
			gui.master:call_descendents(function(w) w:set_dirty() end)


		elseif w.id=="window" then

			local name="window_"..(w.user or "")
			w.master:call_descendents(function(w)
				if w.id==name then
					w.hidden=not w.hidden
				end
			end)
		
		elseif w.id=="random" then
		
			avatar.random()

			gui.data_save("tweaks")

		elseif w.id=="tweak_reset" then
		
			gui.datas.get("tweak_scale"):value(100,true)
			gui.datas.set_value("tweak_scale_x",    100)
			gui.datas.set_value("tweak_scale_y",    100)
			gui.datas.set_value("tweak_scale_z",    100)
			gui.datas.set_value("tweak_rotate_x",   0)
			gui.datas.set_value("tweak_rotate_y",   0)
			gui.datas.set_value("tweak_rotate_z",   0)
			gui.datas.set_value("tweak_translate_x",0)
			gui.datas.set_value("tweak_translate_y",0)
			gui.datas.set_value("tweak_translate_z",0)

			gui.data_save("tweaks")
			
		elseif w.id=="ramp_add" then   
		
			if gui.datas.get_string("ramp_mode")=="keys" then
				local material_name=gui.datas.get_string("material")
				local material=avatar.soul.materials[material_name]
				local ramp=material and material.ramp or {0xffffffff}
				local idx=gui.datas.get_value("ramp_index")
				if idx<1 then idx=1 end
				if idx>#ramp then idx=#ramp end
				local argb=ramp[idx]
				table.insert(ramp,idx,argb)

				gui.data_load("materials")
			end

		elseif w.id=="ramp_del" then   

			if gui.datas.get_string("ramp_mode")=="keys" then
				local material_name=gui.datas.get_string("material")
				local material=avatar.soul.materials[material_name]
				local ramp=material and material.ramp or {0xffffffff}
				local idx=gui.datas.get_value("ramp_index")
				if idx<1 then idx=1 end
				if idx>#ramp then idx=#ramp end

				if #ramp>1 then table.remove(ramp,idx) end

				gui.data_load("materials")
			end

		elseif w.id=="ramp_sort" then   

			if gui.datas.get_string("ramp_mode")=="keys" then
				local material_name=gui.datas.get_string("material")
				local material=avatar.soul.materials[material_name]
				local ramp=material and material.ramp or {0xffffffff}

				gui.data_load("materials")
			end
			
		end
	
	end


	if act=="value" then

		if w.id=="ramp_mode" then

			if gui.datas.get_string("ramp_mode")=="keys" then
			
				gui.master.ids["ramp_index"].hidden=false
				gui.master.ids["ramp_add"].hidden=false
				gui.master.ids["ramp_del"].hidden=false
				gui.master.ids["ramp_sort"].hidden=false

			else

				gui.master.ids["ramp_index"].hidden=true
				gui.master.ids["ramp_add"].hidden=true
				gui.master.ids["ramp_del"].hidden=true
				gui.master.ids["ramp_sort"].hidden=true

			end
		
			gui.data_load("materials")

		elseif w.id=="tweak" or w.id=="material" then
		
			if w.id=="material" then -- check ramp or simp mode
				local material_name=gui.datas.get_string("material")
				local material=avatar.soul.materials[material_name]
				local ramp=material.ramp or {0xffffffff}
				if avatar.ramp_is_simp(ramp) then
					gui.datas.set_string("ramp_mode","simple")
				else
					gui.datas.set_string("ramp_mode","keys")
				end
			end

			gui.data_load("all")

		elseif w.id=="ramp_index" then
			
			if gui.datas.get_string("ramp_mode")=="keys" then
			
				gui.data_load("materials")
				
			end
			
		elseif w.id:sub(1,5)=="part_" then   

			gui.data_save("parts")
			gui.data_load("all")

		elseif w.id=="tweak_scale" then
		
			local v=gui.datas.get_value("tweak_scale" )
			gui.datas.set_value("tweak_scale_x", v )
			gui.datas.set_value("tweak_scale_y", v )
			gui.datas.set_value("tweak_scale_z", v )

		elseif w.id:sub(1,6)=="tweak_" then   
		
			gui.data_save("tweaks")

		elseif w.id=="ramp_red" or w.id=="ramp_grn" or w.id=="ramp_blu" or
			w.id=="ramp_pos" then

			gui.data_save("materials")

		end

	end

end


-- save the gui data from gui values

function gui.data_save(...)

	if not gui.master then return end
	if not gui.datas then return end

	for _,name in ipairs{...} do
	 
		if name=="all" or name=="materials" then

			local material_name=gui.datas.get_string("material")
			local material=avatar.soul.materials[material_name]

			if material then
			
				material.ramp=material.ramp or {0xffffffff}

				if gui.datas.get_string("ramp_mode")=="keys" then

					local idx=gui.datas.get_value("ramp_index")

					local ramp=material.ramp
					local idx=gui.datas.get_value("ramp_index")
					if idx<1 then idx=1 end
					if idx>#ramp then idx=#ramp end
					
					local argb=pack.b4_argb8(
						gui.datas.get_value("ramp_red"),
						gui.datas.get_value("ramp_grn"),
						gui.datas.get_value("ramp_blu"),
						gui.datas.get_value("ramp_pos")
					)
					ramp[idx]=argb

				else -- simple mode

					material.ramp=avatar.simp_to_ramp(
						gui.datas.get_value("ramp_red"),
						gui.datas.get_value("ramp_grn"),
						gui.datas.get_value("ramp_blu"),
						gui.datas.get_value("ramp_pos"))

				end
				
			end
			
--			avatar.gs.grd_texmat=nil
			avatar.rebuild("materials")
			
		end
	
		if name=="all" or name=="tweaks" then

			local tweak_name=gui.datas.get_string("tweak")
			if not avatar.soul.tweaks then avatar.soul.tweaks={} end
			local tweak=avatar.soul.tweaks[tweak_name]
			
			if not tweak then
				tweak={}
				avatar.soul.tweaks[tweak_name]=tweak
			end

			tweak.scale=tweak.scale or {}
			tweak.scale[1]    =gui.datas.get_value("tweak_scale_x"    )/100
			tweak.scale[2]    =gui.datas.get_value("tweak_scale_y"    )/100
			tweak.scale[3]    =gui.datas.get_value("tweak_scale_z"    )/100
			tweak.rotate=tweak.rotate or {}
			tweak.rotate[1]   =gui.datas.get_value("tweak_rotate_x"   )
			tweak.rotate[2]   =gui.datas.get_value("tweak_rotate_y"   )
			tweak.rotate[3]   =gui.datas.get_value("tweak_rotate_z"   )
			tweak.translate=tweak.translate or {}
			tweak.translate[1]=gui.datas.get_value("tweak_translate_x")/1000
			tweak.translate[2]=gui.datas.get_value("tweak_translate_y")/1000
			tweak.translate[3]=gui.datas.get_value("tweak_translate_z")/1000

			avatar.rebuild("tweaks")

		end

		if name=="all" or name=="parts" then


			avatar.soul.parts={}
			for i,v in ipairs(avatar.part_names) do
				local newpart=function(name,flags)
					local r={
						name=name or "" ,
						flags=flags or 0
					}
					avatar.soul.parts[v]=avatar.soul.parts[v] or {}
					if name and name~="" then
						avatar.soul.parts[v][ #avatar.soul.parts[v]+1 ]=r
					end
				end
			
				local count=avatar.part_counts[v] or 1
				for i=1,count do
					local s= (i==1) and "" or ("_"..i)
					newpart( gui.datas.get_string("part_"..v..s) , gui.datas.get_value("part_flags_"..v..s) )
				end
			end


			avatar.rebuild("parts")
			
		end

	end

end

-- load the gui data into gui values

function gui.data_load(...)

	if not gui.master then return end
	if not gui.datas then return end

	for _,name in ipairs{...} do
	
		if ( name=="all" or name=="materials" ) and avatar.soul.materials then

			local material_name=gui.datas.get_string("material")
			local material=avatar.soul.materials[material_name]

			if material then
			
				if not material.ramp then
					if material.diffuse then
						local argb=0x80000000
						argb=argb+material.diffuse[  1]*255*256*256
						argb=argb+material.diffuse[  2]*255*256
						argb=argb+material.diffuse[  3]*255
						material.ramp={argb}
					end
				end
				if not material.ramp then
					material.ramp={0xffffffff}
				end
				local ramp=material and material.ramp or {0xffffffff}
				
				local idx=1

				if gui.datas.get_string("ramp_mode")=="keys" then
				
					gui.datas.get("ramp_index"):set( gui.datas.get_value("ramp_index") ,1,#ramp,true)
					
					idx=gui.datas.get_value("ramp_index")
					
				else -- simple mode
				
					idx=math.ceil(#ramp/2)
				end
				
				if idx<1 then idx=1 end
				if idx>#ramp then idx=#ramp end
				local cr,cg,cb,ca=pack.argb8_b4(ramp[idx])

				if gui.datas.get_string("ramp_mode")=="simple" then
				
					cr,cg,cb,ca=avatar.ramp_to_simp(ramp)
				
				end


				gui.datas.set_value("ramp_red",cr)
				gui.datas.set_value("ramp_grn",cg)
				gui.datas.set_value("ramp_blu",cb)
				gui.datas.set_value("ramp_pos",ca)

			end

		end
	
		if name=="all" or name=="tweaks" then

			local tweak_name=gui.datas.get_string("tweak")
			local tweak=avatar.soul.tweaks[tweak_name]
			
			if tweak then

				gui.datas.get("tweak_scale"):value( math.floor((tweak.scale[1]+tweak.scale[2]+tweak.scale[3])*100/3) , true )
				
				gui.datas.set_value("tweak_scale_x",    tweak.scale[1]*100     )
				gui.datas.set_value("tweak_scale_y",    tweak.scale[2]*100     )
				gui.datas.set_value("tweak_scale_z",    tweak.scale[3]*100     )
				gui.datas.set_value("tweak_rotate_x",   tweak.rotate[1]        )
				gui.datas.set_value("tweak_rotate_y",   tweak.rotate[2]        )
				gui.datas.set_value("tweak_rotate_z",   tweak.rotate[3]        )
				gui.datas.set_value("tweak_translate_x",tweak.translate[1]*1000)
				gui.datas.set_value("tweak_translate_y",tweak.translate[2]*1000)
				gui.datas.set_value("tweak_translate_z",tweak.translate[3]*1000)
				
			else
				
				gui.datas.get("tweak_scale"):value( 100 , true )

				gui.datas.set_value("tweak_scale_x",    100     )
				gui.datas.set_value("tweak_scale_y",    100     )
				gui.datas.set_value("tweak_scale_z",    100     )
				gui.datas.set_value("tweak_rotate_x",   0        )
				gui.datas.set_value("tweak_rotate_y",   0        )
				gui.datas.set_value("tweak_rotate_z",   0        )
				gui.datas.set_value("tweak_translate_x",0)
				gui.datas.set_value("tweak_translate_y",0)
				gui.datas.set_value("tweak_translate_z",0)

			end

		end

		if name=="all" or name=="parts" then


			for i,v in ipairs(avatar.part_names) do
				if avatar.soul.parts[v] and avatar.soul.parts[v][1] then
					local count=avatar.part_counts[v] or 1
					for i=1,count do
						local s= (i==1) and "" or ("_"..i)
						if avatar.soul.parts[v][i] then
							gui.datas.set_string("part_"..v..s,avatar.soul.parts[v][i].name)
							gui.datas.set_value("part_flags_"..v..s,avatar.soul.parts[v][i].flags)
						else
							gui.datas.set_value("part_"..v..s,1)
							gui.datas.set_value("part_flags_"..v..s,3)
						end
					end
				else
					local count=avatar.part_counts[v] or 1
					for i=1,count do
						local s= (i==1) and "" or ("_"..i)
						gui.datas.set_value("part_"..v..s,1)
							gui.datas.set_value("part_flags_"..v..s,3)
					end
				end
			end


		end

	end


	gui.master:call_descendents(function(w)
		w.dirty=true
	end)
	
end



	return gui
end

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

--	local datas=gui.master.datas

--[[
datas.set_infos({

	{
		id="test",
		help="This is a test.",
	},
	
})
]]

-- create data
gui.data_setup=function()

--print("DATA SETUP")

	local datas=zgui.master.datas

	if not gui.datas then -- only setup once
	
		gui.datas=datas


		local hex_tostring=function(dat,num) return string.format("%02x",num) end
		local hex_tonumber=function(dat,str) return tonumber(str,16) end

		datas.new({id="bloom_pick",class="number",hooks=gui.hooks,num=4.00,min=0,max=16,step=0.1})
		datas.new({id="bloom_add", class="number",hooks=gui.hooks,num=0.25,min=0,max=2,step=0.01})

		datas.new({id="ramp_index",class="number",hooks=gui.hooks,num=1,min=1,max=1,step=1})

		datas.new({id="ramp_red",class="number",hooks=gui.hooks,num=0,min=0,max=255,step=1,
			tonumber=hex_tonumber,tostring=hex_tostring})
		datas.new({id="ramp_grn",class="number",hooks=gui.hooks,num=0,min=0,max=255,step=1,
			tonumber=hex_tonumber,tostring=hex_tostring})
		datas.new({id="ramp_blu",class="number",hooks=gui.hooks,num=0,min=0,max=255,step=1,
			tonumber=hex_tonumber,tostring=hex_tostring})
		datas.new({id="ramp_pos",class="number",hooks=gui.hooks,num=0,min=0,max=255,step=1,
			tonumber=hex_tonumber,tostring=hex_tostring})


		datas.new({id="rotate",class="number",hooks=gui.hooks,num=180,min=0,max=360,step=1})
		datas.new({id="zoom",class="number",hooks=gui.hooks,num=100,min=25,max=400,step=1})

		datas.new({id="tweak_scale",class="number",hooks=gui.hooks,num=100,min=50,max=200,step=1})
		datas.new({id="tweak_scale_x",class="number",hooks=gui.hooks,num=100,min=50,max=200,step=1})
		datas.new({id="tweak_scale_y",class="number",hooks=gui.hooks,num=100,min=50,max=200,step=1})
		datas.new({id="tweak_scale_z",class="number",hooks=gui.hooks,num=100,min=50,max=200,step=1})
		datas.new({id="tweak_rotate_x",class="number",hooks=gui.hooks,num=0,min=-90,max=90,step=1})
		datas.new({id="tweak_rotate_y",class="number",hooks=gui.hooks,num=0,min=-90,max=90,step=1})
		datas.new({id="tweak_rotate_z",class="number",hooks=gui.hooks,num=0,min=-90,max=90,step=1})
		datas.new({id="tweak_translate_x",class="number",hooks=gui.hooks,num=0,min=-1000,max=1000,step=1})
		datas.new({id="tweak_translate_y",class="number",hooks=gui.hooks,num=0,min=-1000,max=1000,step=1})
		datas.new({id="tweak_translate_z",class="number",hooks=gui.hooks,num=0,min=-1000,max=1000,step=1})

		datas.new({id="file_dir",class="string",hooks=gui.hooks,str="art/souls"})
		datas.new({id="file_name",class="string",hooks=gui.hooks,str=string.format("test.soul.json",os.time())})

		local list={} ; for i,name in ipairs({
				"keys",
				"simple",
		}) do list[#list+1]={str=name} end
		datas.new({id="ramp_mode" ,class="list",  hooks=gui.hooks,num=1,list=list})

		local list={} ; for i,name in ipairs(avatar.material_names) do list[#list+1]={str=name} end
		datas.new({id="material"  ,class="list",  hooks=gui.hooks,num=4,list=list})

		local list={} ; for i,name in ipairs(avatar.pose_names) do list[#list+1]={str=name} end
		datas.new({id="pose"  ,class="list",  hooks=gui.hooks,num=1,list=list})


		local list={} ; for i,name in ipairs(avatar.tweak_names) do list[#list+1]={str=name} end
		datas.new({id="tweak"  ,class="list",  hooks=gui.hooks,num=1,list=list})

		for i,name in ipairs(avatar.part_names) do
			local list={}
				list[#list+1]={
					str=""
				}
			for i,v in ipairs(avatar.mesh_names) do
				local meshname=v[1]
				if meshname:sub(1,#name+1)==name.."_" then
					list[#list+1]={
						str=meshname
					}
				end
			end
			local count=avatar.part_counts[name] or 1
			for i=1,count do
				local s= (i==1) and "" or ("_"..i)
				datas.new({id="part_"..name..s  ,class="list",  hooks=gui.hooks,num=1,list=list})
				datas.new({id="part_flags_"..name..s,class="number",hooks=gui.hooks,num=3})
			end
		end

	end
end



	return gui
end

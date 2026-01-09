--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.widgets.datas

Handle a collection of data (IE in the master widget)

]]


-- module bake
local M={ modname = (...) } package.loaded[M.modname] = M function M.bake(oven,B) B = B or {}

local wdatas=B

local wdata=oven.rebake("wetgenes.gamecake.widgets.data")



--[[#lua.wetgenes.gamecake.widgets.datas.new_datas

]]
function wdatas.new_datas(datas)

	local datas=datas or {} -- probably use what is passed in only fill in more values

-- wrapper for wdata.new_data that remembers the master
	datas.new_data=function(dat)
		dat.master=datas.master or dat.master -- set master of each data we create
		return wdata.new_data(dat)
	end


	datas.ids=datas.ids or {}
	
--[[#lua.wetgenes.gamecake.widgets.datas.set

]]
	datas.set=function(v)
		if type(v)=="table" then datas.ids[v.id]=v return v else error("must set data") end
	end

--[[#lua.wetgenes.gamecake.widgets.datas.del

]]
	datas.del=function(v)
		if type(v)=="table" then datas.ids[v.id]=nil else datas.ids[v]=nil end
	end

--[[#lua.wetgenes.gamecake.widgets.datas.new

]]
	datas.new=function(v)
		return datas.set( datas.new_data(v) )
	end

--[[#lua.wetgenes.gamecake.widgets.datas.get

]]
	datas.get=function(n)
		return datas.ids[n]
	end

--[[#lua.wetgenes.gamecake.widgets.datas.get_value

]]
	datas.get_value=function(n)
		local v=datas.get(n)
		return v and v:value()
	end

--[[#lua.wetgenes.gamecake.widgets.datas.get_string

]]
	datas.get_string=function(n)
		local v=datas.get(n)
		return v and v:tostring()
	end

--[[#lua.wetgenes.gamecake.widgets.datas.set_string

]]
	datas.set_string=function(n,val)
		local v=datas.get(n)
		if v.class=="list" then
			return v and v:value( v:tonumber(val) )
		else
			return v and v:value(val)
		end
	end

--[[#lua.wetgenes.gamecake.widgets.datas.get_number

]]
	datas.get_number=function(n)
		local v=datas.get(n)
		return v and v:tonumber()
	end

--[[#lua.wetgenes.gamecake.widgets.datas.set_value

]]
	datas.set_value=function(n,val)
		local v=datas.get(n)
		return v and v:value(val)
	end


-- set infos and build lookups

	datas.infos={}
	datas.lookup_infos={}
	datas.lookup_keys={}
		
--[[#lua.wetgenes.gamecake.widgets.datas.set_infos

]]
	datas.set_infos=function(infos)
	
		datas.infos=infos

		-- build lookup
		datas.lookup_infos={}
		for i,v in ipairs(datas.infos) do
			if v.user then
				datas.lookup_infos[v.id]=datas.lookup_infos[v.id] or {}
				datas.lookup_infos[v.id][v.user]=v
			else
				datas.lookup_infos[v.id]=v
			end
		end

		-- build lookup by keys
		datas.lookup_keys={}
		for i,v in ipairs(datas.infos) do
			if v.key then
				local t=(type(v.key)=="table") and v.key or {v.key}
				for _,key in ipairs(t) do
					datas.lookup_keys[key]=v
				end
			end
		end
		
	end


--print("DATA SETUP")	

	return datas
	
end

return wdatas
end


--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets data values
--


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wdatas)
wdatas=wdatas or {}

local wdata=oven.rebake("wetgenes.gamecake.widgets.data")

function wdatas.new_datas(datas)

	local datas=datas or {} -- probably use what is passed in only fill in more values

-- wrapper for wdata.new_data that remembers the master
	datas.new_data=function(dat)
		dat.master=datas.master or dat.master -- set master of each data we create
		return wdata.new_data(dat)
	end


	datas.ids=datas.ids or {}
	
	datas.set=function(n,v)
		if type(n)=="table" then datas.ids[n.id]=n return n else datas.ids[n]=v return v end
	end

	datas.del=function(v)
		if type(v)=="table" then datas.ids[v.id]=nil else datas.ids[v]=nil end
	end

	datas.new=function(v)
		datas.set( datas.new_data(v) )
	end

	datas.get=function(n)
		return datas.ids[n]
	end

	datas.get_value=function(n)
		local v=datas.get(n)
		return v and v:value()
	end

	datas.get_string=function(n)
		local v=datas.get(n)
		return v and v:tostring()
	end

	datas.set_string=function(n,val)
		local v=datas.get(n)
		if v.class=="list" then
			return v and v:value( v:tonumber(val) )
		else
			return v and v:value(val)
		end
	end

	datas.get_number=function(n)
		local v=datas.get(n)
		return v and v:tonumber()
	end

	datas.set_value=function(n,val)
		local v=datas.get(n)
		return v and v:value(val)
	end


-- set infos and build lookups

	datas.infos={}
	datas.lookup_infos={}
	datas.lookup_keys={}
		
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


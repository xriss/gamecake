
--[[


Turn tables of numbers too and from strings of floats or doubles etc


]]


local fats={ modname=(...) } ; package.loaded[fats.modname]=fats


fats.functions=function(fats,use_ffi)

	if use_ffi then


local ffi = require("ffi")

fats.table_to_floats=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("float[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len*4)
	return str
end

fats.floats_to_table=function(str,s,e)
	local lenstr=#str/4
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("float*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end


fats.table_to_doubles=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("double[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len*8)
	return str
end

fats.doubles_to_table=function(str,s,e)
	local lenstr=#str/8
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("double*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end


fats.table_to_uint32s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("uint32_t[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len*4)
	return str
end

fats.uint32s_to_table=function(str,s,e)
	local lenstr=#str/4
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("uint32_t*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end

fats.table_to_int32s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("int32_t[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len*4)
	return str
end

fats.int32s_to_table=function(str,s,e)
	local lenstr=#str/4
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("int32_t*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end


fats.table_to_uint16s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("uint16_t[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len*2)
	return str
end


fats.uint16s_to_table=function(str,s,e)
	local lenstr=#str/2
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("uint16_t*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end

fats.table_to_int16s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("int16_t[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len*2)
	return str
end

fats.int16s_to_table=function(str,s,e)
	local lenstr=#str/2
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("int16_t*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end


fats.table_to_uint8s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("uint8_t[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len)
	return str
end

fats.uint8s_to_table=function(str,s,e)
	local lenstr=#str
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("uint8_t*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end

fats.table_to_int8s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	local len=1+e-s
	local buf=ffi.new("int8_t[?]",len)
	for i=s,e do buf[i-s]=tab[i] end
	local str=ffi.string(buf,len)
	return str
end

fats.int8s_to_table=function(str,s,e)
	local lenstr=#str
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	local len=1+e-s
	local buf=ffi.cast("int8_t*",str)
	local tab={}
	for i=1,len do tab[i]=buf[s+i-2] end
	return tab
end


	else -- need to use C core code


local core=require("wetgenes.fats.core")

fats.table_to_floats=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_floats(tab,s,e)
end

fats.floats_to_table=function(str,s,e)
	local lenstr=#str/4
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.floats_to_table(str,s,e)
end


fats.table_to_doubles=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_doubles(tab,s,e)
end

fats.doubles_to_table=function(str,s,e)
	local lenstr=#str/8
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.doubles_to_table(str,s,e)
end


fats.table_to_uint32s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_uint32s(tab,s,e)
end

fats.uint32s_to_table=function(str,s,e)
	local lenstr=#str/4
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.uint32s_to_table(str,s,e)
end

fats.table_to_int32s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_int32s(tab,s,e)
end

fats.int32s_to_table=function(str,s,e)
	local lenstr=#str/4
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.int32s_to_table(str,s,e)
end


fats.table_to_uint16s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_uint16s(tab,s,e)
end

fats.uint16s_to_table=function(str,s,e)
	local lenstr=#str/2
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.uint16s_to_table(str,s,e)
end

fats.table_to_int16s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_int16s(tab,s,e)
end

fats.int16s_to_table=function(str,s,e)
	local lenstr=#str/2
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.int16s_to_table(str,s,e)
end


fats.table_to_uint8s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_uint8s(tab,s,e)
end

fats.uint8s_to_table=function(str,s,e)
	local lenstr=#str
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.uint8s_to_table(str,s,e)
end

fats.table_to_int8s=function(tab,s,e)
	local lentab=#tab
	if not s then s=1      elseif s<0 then s=1+lentab+s end
	if not e then e=lentab elseif e<0 then e=1+lentab+e end
	return core.table_to_int8s(tab,s,e)
end

fats.int8s_to_table=function(str,s,e)
	local lenstr=#str
	if not s then s=1      elseif s<0 then s=1+lenstr+s end
	if not e then e=lenstr elseif e<0 then e=1+lenstr+e end
	return core.int8s_to_table(str,s,e)
end


	end

	return fats
end

 -- use ffi if we have it (luajit test) or fall back on the c library
fats:functions(jit)

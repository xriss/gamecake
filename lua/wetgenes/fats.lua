
--[[


Turn tables of numbers too and from strings of floats or doubles etc


]]


local fats={ modname=(...) } ; package.loaded[fats.modname]=fats



if jit then -- use luajit ffi


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


end

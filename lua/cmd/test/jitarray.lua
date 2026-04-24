

  local ffi = require "ffi"
  ffi.cdef[[
  typedef struct {
	double d[2];
  } V2;
  ]]


local meta={}

-- cache constructor
meta.ffi_type=ffi.typeof("V2")
meta.new=function(x,y)
	local t=meta.ffi_type()
	t[1]=x
	t[2]=y
	return t
end

meta.add=function(a,b)
	local r=meta.new(0,0)

	r[1]=a[1]+b[1]
	r[2]=a[2]+b[2]

	return r
end

meta.__len=function(t)
	return 2
end
meta.__index=function(t, n)
--print("__index",type(t),type(n),t,n)
	return meta[n] or t.d[n-1] -- if its not in the meta table then it must be a number
end

meta.__newindex=function(t,n,v)
--print("__newindex",type(t),type(n),type(v),t,n,v)
	t.d[n-1]=v -- only numbers can be set
end

meta.__tostring=function(t)
	return string.format("V2{%.4f, %.4f}",t[1],t[2])
end

meta.__gc=function(t)
--print("__gc",type(t),t)
end

ffi.metatype("V2",meta)


local va=meta.new(1,2)
local vb=meta.new(3,4)
local vc=va:add(vb)

print( type(va) , va , "+" , vb , "=" , vc )

--
-- (C) 2013 Kriss@XIXs.com
--
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--



local math=require("math")
local table=require("table")
local string=require("string")

local unpack=unpack
local getmetatable=getmetatable
local setmetatable=setmetatable
local type=type
local pairs=pairs
local ipairs=ipairs
local tostring=tostring
local tonumber=tonumber
local require=require
local error=error

--[[#lua.wetgenes.tardis

Time And Relative Dimensions In Space is of course the perfect name for
a library of matrix based math functions.

	local tardis=require("wetgenes.tardis")

This tardis is a lua library for manipulating time and space with numbers.
Designed to work as pure lua but with a faster, but less accurate, f32 core by
default. ( this core seems to be slightly faster/same speed as vanilla lua but
slower than luajit , so is currently disabled )

Recoil in terror as we use two glyph names for classes whilst typing in
random strings of numbers and operators that may or may not contain
tyops.

	v# vector [#]
	m# matrix [#][#]
	q4 quaternion

Each class is a table of # values [1] to [#] the 2d number streams are
formatted the same as opengl (row-major) and metatables are used to
provide methods.

The easy way of remembering the opengl 4x4 matrix layout is that the
translate x,y,z values sit at 13,14,15 and 4,8,12,16 is normally set
to the constant 0,0,0,1 for most transforms.

		 | 1  5  9  13 |
		 | 2  6  10 14 |
	m4 = | 3  7  11 15 |
		 | 4  8  12 16 |

This seems to be the simplest (programmer orientated) description of
most of the maths used here so go read it if you want to know what the
funny words mean.

http://www.j3d.org/matrix_faq/matrfaq_latest.html

]]

--module
local tardis={ modname=(...) } ; package.loaded[tardis.modname]=tardis


tardis.export=function(env,...)
	local tab={...} ; for i=1,#tab do tab[i]=env[ tab[i] ] end
	return unpack(tab)
end


-- mix two numbers when all values should be in the range 0<= to <max
tardis.mixwrap=function(a,b,m,t)
	local tfix=function(n) -- clamp to within range, 0 to m
		if n<0 then return m-((-n)%m)
		else        return n%m        end
	end
	local mo2=(m/2)
	a=tfix(a)
	b=tfix(b)
	local d=a-b
	if     d < -mo2 then a=a+m     -- long way down
	elseif d >  mo2 then b=b+m end -- long way up
	return tfix( b*t + a*(1-t) )
end



--[[#lua.wetgenes.tardis.type

	name=tardis.type(object)

This will return the type of an object previously registered with class

]]
function tardis.type(it) local t=type(it) return t=="table" and it.__type or t end

--[[#lua.wetgenes.tardis.class

	metatable=tardis.class(name,class,...)

Create a new metatable for an object class, optionally inheriting from other previously
declared classes.

]]
function tardis.class(name,...)

	if tardis[name] then return tardis[name] end

	local meta={} -- create new
	local sub={...} -- possibly multiple sub classes

	if #sub>0 then -- inherit?
		for idx=#sub,1,-1 do -- reverse sub class order, so the ones to the left overwrite the ones on the right
			for n,v in pairs(sub[idx]) do meta[n]=v end -- each subclass overwrites all values
		end
	end

	meta.__index=meta -- this metatable is its own index
	meta.__type=name -- class name

	tardis[name]=meta -- save in using name and return table
	return meta
end



--[[#lua.wetgenes.tardis.array


Array is the base class for all other tardis classes, it is just a
stream of numbers, probably in a table but possibly a chunk of f32
values in a userdata.

]]
local array=tardis.class("array")
-- yank locals up here
local v1,v2,v3,v4,q4,m2,m3,m4


--[[#lua.wetgenes.tardis.array.__tostring

	string = array.__tostring(it)

Convert an array to a string this is called by the lua tostring() function,

]]
function array.__tostring(it) -- these classes are all just 1d arrays of numbers
	local t={}
	t[#t+1]=tardis.type(it)
	t[#t+1]="={"
	for i=1,#it do
		t[#t+1]=string.format("%.5f",it[i]) -- dumps are unreadable unless we disable exponents
		if i~=#it then t[#t+1]=", " end
	end
	t[#t+1]="}"
	return table.concat(t)
end

--[[#lua.wetgenes.tardis.array.__add

	r = array.__add(a,b)

Add a to b and return a a.new(result) so the class returned will match the
input class of a and neither a or b will be modified.

]]
function array.__add(a,b)
	return a.new(a):add(b)
end

--[[#lua.wetgenes.tardis.array.__sub

	r = array.__sub(a,b)

Subtract b from a and return a a.new(result) so the class returned will match the
input class of a and neither a or b will be modified.

]]
function array.__sub(a,b)
	return a.new(a):sub(b)
end

--[[#lua.wetgenes.tardis.array.__unm

	r = array.__unm(a)

Subtract b from 0 and return a a.new(result) so the class returned will match the
input class of a but a will not be modified

]]
function array.__unm(a)
	return a.new(0):sub(a)
end

--[[#lua.wetgenes.tardis.array.__eq

	r = array.__eq(a,b)

meta to call a:compare(b) and return the result

]]
function array.__eq(a,b)
	return a:compare(b)
end

--[[#lua.wetgenes.tardis.array.__mul

	r = array.__mul(a,b)

Call the appropriate product function depending on the argument classes. Always
creates and returns a.new() value.

]]
function array.__mul(a,b)
	if type(a)=="number" then a,b=b,a end -- deal with number first product
	return a:product(b,a.new())
end

--[[#lua.wetgenes.tardis.array.__div

	r = array.__div(a,b)

Replace b with 1/b and then call the appropriate product function depending on
the argument classes. Always creates and returns a.new() value.

]]
function array.__div(a,b)
	return a:product(1/b,a.new())
end

--[[#lua.wetgenes.tardis.array.new

	a = tardis.array.new()
	a = tardis.array.new(1,2,3)

Create a new array and optionally set it to the given values which will
also control its base length.

]]
function array.new(...) return setmetatable({},array):set(...) end

--[[#lua.wetgenes.tardis.array.nset

	a=a:nset(1,2,3,4)

Dumber version of set that must be number arguments and does not repeat 
the last value or stop if we provide too many values.

]]
function array.nset(it,...)
	local a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17=... -- try and avoid table allocation
	if not a1  then return it end ; it[1]=a1
	if not a2  then return it end ; it[2]=a2
	if not a3  then return it end ; it[3]=a3
	if not a4  then return it end ; it[4]=a4
	if not a5  then return it end ; it[5]=a5
	if not a6  then return it end ; it[6]=a6
	if not a7  then return it end ; it[7]=a7
	if not a8  then return it end ; it[8]=a8
	if not a9  then return it end ; it[9]=a9
	if not a10 then return it end ; it[10]=a10
	if not a11 then return it end ; it[11]=a11
	if not a12 then return it end ; it[12]=a12
	if not a13 then return it end ; it[13]=a13
	if not a14 then return it end ; it[14]=a14
	if not a15 then return it end ; it[15]=a15
	if not a16 then return it end ; it[16]=a16
	if a17 then
		local aa={...} -- try the rest of the vargs
		for i=17,#aa do
			it[i]=aa[i]
		end
	end
	return it
end

--[[#lua.wetgenes.tardis.array.aset

	a=a:aset({1,2,3,4})

Dumber version of set that must be an array of numbers and does not repeat 
the last value or stop if we provide too many values.

]]
function array.aset(it,a)
	if not a[1] then return it end -- nothing to do
	for i=1,#a do
		it[i]=a[i]
	end
	return it
end

--[[#lua.wetgenes.tardis.array.set

	a=a:set(1,2,3,4)
	a=a:set({1,2,3,4})
	a=a:set({1,2},{3,4})
	a=a:set(function(i) return i end)

Assign some numbers to an array, all the above examples will assign 1,2,3,4 to
the first four slots in the given array, as you can see we allow one level of
tables. Any class that is based on this array class can be used instead of an
explicit table. So we can use a v2 or v3 or m4 etc etc.

if more numbers are given than the size of the array then they will be
ignored.

if less numbers are given than the size of the array then the last number will
be repeated.

if no numbers are given then nothing will be done

if a function is given it will be called with the index and should
return a number.

]]
function array.set(it,a1,...)
	if not a1 then return it end -- nothing to do, probably fastest short circuit?
	
	local itlen=#it -- need this many values
	local a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17=... -- try and avoid table allocation

	if type(a1)=="function" then
		for i=1,itlen do it[i]=a1(i) end -- fill
		return it
	end
	
	local last
	local n=1
	local donum;donum=function(v)
		if not v then return true end
		if itlen>0 and n>itlen then return true end
		if type(v)=="number" then
			last=v
			it[n]=v
			n=n+1
		elseif v then -- must be number or table like thing
			for i=1,#v do
				local vi=v[i]
				if itlen>0 and n>itlen then return true end
				if not vi then vi=last end
				last=vi
				it[n]=vi
				n=n+1
			end
		end
	end
	local dofill=function()
		if last and n<=itlen then
			while n<=itlen do -- fill in any more repeats
				if donum(last) then return it end
			end
		end
		return it
	end
	if donum(a1) then return dofill() end
	if donum(a2) then return dofill() end
	if donum(a3) then return dofill() end
	if donum(a4) then return dofill() end
	if donum(a5) then return dofill() end
	if donum(a6) then return dofill() end
	if donum(a7) then return dofill() end
	if donum(a8) then return dofill() end
	if donum(a9) then return dofill() end
	if donum(a10) then return dofill() end
	if donum(a11) then return dofill() end
	if donum(a12) then return dofill() end
	if donum(a13) then return dofill() end
	if donum(a14) then return dofill() end
	if donum(a15) then return dofill() end
	if donum(a16) then return dofill() end
	if a17 and n<=itlen then
		local aa={...} -- try the rest of the vargs
		for i=17-1,#aa do -- note that ... does not contain the first one so we sub 1
			if donum(aa[i]) then return dofill() end
		end
	end
	return dofill()
end

--[[#lua.wetgenes.tardis.array.unpack

	a1,a2=v2:unpack()
	a1,a2,a3=v3:unpack()
	a1,a2,a3,a4=v4:unpack()

Return all values in this array. Note this should be used instead of
the unpack function for future optimisation safety.

]]
function array.unpack(it)
	return unpack(it)
end

--[[#lua.wetgenes.tardis.array.zero

	a=a:zero()

Set all values in this array to zero.

]]
function array.zero(it)
	for i=1,#it do it[i]=0 end
	return it
end

--[[#lua.wetgenes.tardis.array.min

	r=it:min()

Return a single number value that is the minimum of all values in this array.

]]
function array.min(it)
	return math.min(it:unpack())
end

--[[#lua.wetgenes.tardis.array.max

	r=it:max()

Return a single number value that is the maximum of all values in this array.

]]
function array.max(it)
	return math.max(it:unpack())
end


--[[#lua.wetgenes.tardis.array.add

	r=it:add(b,r)
	r=it:add(b,it.new())

Add b to it. b may be a number or another array of the same size as this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.add(it,b,r)
	r=r or it
	if type(b)=="number" then
		for i=1,#it do r[i]= it[i] + b end
	else
		for i=1,#it do r[i]= it[i] + b[i] end
	end
	return r
end

--[[#lua.wetgenes.tardis.array.sub

	r=it:sub(b,r)
	r=it:sub(b,it.new())

Subtract b from it. b may be a number or another array of the same size as this
array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.sub(it,b,r)
	r=r or it
	if type(b)=="number" then
		for i=1,#it do r[i]= it[i] - b end
	else
		for i=1,#it do r[i]= it[i] - b[i] end
	end
	return r
end

--[[#lua.wetgenes.tardis.array.fract

	r=it:fract(r)
	r=it:fract(it.new())

Return the fractional part of each value using math.modf.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.fract(it,r)
	r=r or it
	for i=1,#it do local a,b=math.modf( it[i] ) ; r[i]=b end
	return r
end

--[[#lua.wetgenes.tardis.array.pow

	r=it:pow(p,r)
	r=it:pow(p,it.new())

Perform math.pow(it,p) on all values of this array. p may be a number or
another array of the same size as this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.pow(it,p,r)
	r=r or it
	if type(p)=="number" then
		for i=1,#it do r[i]= it[i] ^ p end
	else
		for i=1,#it do r[i]= it[i] ^ p[i] end
	end
	return r
end

--[[#lua.wetgenes.tardis.array.log

	r=it:log(r)
	r=it:log(it.new())

Perform math.log on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.log(it,p,r)
	r=r or it
	for i=1,#it do r[i]=math.log(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.exp

	r=it:exp(r)
	r=it:exp(it.new())

Perform math.exp on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.exp(it,p,r)
	r=r or it
	for i=1,#it do r[i]=math.exp(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.abs

	r=it:abs(r)
	r=it:abs(it.new())

Perform math.abs on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.abs(it,r)
	r=r or it
	for i=1,#it do r[i]=math.abs(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.floor

	r=it:floor(r)
	r=it:floor(it.new())

Perform math.floor on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.floor(it,r)
	r=r or it
	for i=1,#it do r[i]=math.floor(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.ceil

	r=it:ceil(r)
	r=it:ceil(it.new())

Perform math.ceil on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.ceil(it,r)
	r=r or it
	for i=1,#it do r[i]=math.ceil(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.round

	r=it:round(r)
	r=it:round(it.new())

Perform math.floor( v+0.5 ) on all values of this array. Which will
round up or down to the nearest integer.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.round(it,r)
	r=r or it
	for i=1,#it do r[i]=math.floor(it[i]+0.5) end
	return r
end

--[[#lua.wetgenes.tardis.array.trunc

	r=it:trunc(r)
	r=it:trunc(it.new())

Perform math.floor on positive values and math ceil on negative values
for all values of this array. So a trunication that will always error
towards 0.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.trunc(it,r)
	r=r or it
	for i=1,#it do
		if it[i]<0 then  r[i]=math.ceil(it[i])
		else             r[i]=math.floor(it[i])
		end
	end
	return r
end

--[[#lua.wetgenes.tardis.array.quantize

	r=it:quantize(1/1024,r)
	r=it:quantize(s,it.new())

Perform a trunc(v/s)*s on all values of this array. We recomended the
use of a power of two, eg 1/1024 rather than 1/1000 if you wanted 3
decimal digits.

If r is provided then the result is written into r and returned
otherwise it is modified and returned.

]]
function array.quantize(it,s,r)
	r=r or it
	s=s or 1
	for i=1,#it do
		if it[i]<0 then  r[i]=math.ceil(it[i]/s)*s
		else             r[i]=math.floor(it[i]/s)*s
		end
	end
	return r
end

--[[#lua.wetgenes.tardis.array.sin

	r=it:sin(r)
	r=it:sin(it.new())

Perform math.sin on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.sin(it,r)
	r=r or it
	for i=1,#it do r[i]=math.sin(it[i]) end
	return r
end
--[[#lua.wetgenes.tardis.array.asin

	r=it:asin(r)
	r=it:asin(it.new())

Perform math.asin on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.asin(it,r)
	r=r or it
	for i=1,#it do r[i]=math.asin(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.cos

	r=it:cos(r)
	r=it:cos(it.new())

Perform math.cos on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.cos(it,r)
	r=r or it
	for i=1,#it do r[i]=math.cos(it[i]) end
	return r
end
--[[#lua.wetgenes.tardis.array.acos

	r=it:acos(r)
	r=it:acos(it.new())

Perform math.acos on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.acos(it,r)
	r=r or it
	for i=1,#it do r[i]=math.acos(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.tan

	r=it:tan(r)
	r=it:tan(it.new())

Perform math.tan on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.tan(it,r)
	r=r or it
	for i=1,#it do r[i]=math.tan(it[i]) end
	return r
end
--[[#lua.wetgenes.tardis.array.atan

	r=it:atan(r)
	r=it:atan(it.new())

Perform math.atan on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.

]]
function array.atan(it,r)
	r=r or it
	for i=1,#it do r[i]=math.atan(it[i]) end
	return r
end

--[[#lua.wetgenes.tardis.array.scalar

	r=a:scalar(s,r)

Multiply all value in array by number.

If r is provided then the result is written into r and returned otherwise a is
modified and returned.

]]
function array.scalar(it,s,r)
	r=r or it
	for i=1,#it do r[i]=it[i]*s end
	return r
end

--[[#lua.wetgenes.tardis.array.mix

	r=a:mix(b,s,r)

Mix values between a and b where a is returned if s=0 and b is returned if s=1

If r is provided then the result is written into r and returned otherwise a is
modified and returned.

]]
function array.mix(a,b,s,r)
	r=r or a
	s=( s<=0 and 0 ) or (s>=1 and 1) or s
	local t=1-s
	for i=1,#a do r[i]=a[i]*t + b[i]*s end
	return r
end

--[[#lua.wetgenes.tardis.array.compare

	a=a:compare(b)
	a=a:compare(1,2,3,4)

Are the numbers in b the same as the numbers in a, this function will
return true if they are and false if they are not.

If the arrays are of different lengths then this will return false.

Numbers to test for can be given explicitly in the arguments and we
follow the same one level of tables rule as we do with array.set so any
class derived from array can be used.

]]
function array.compare(it,...)
	local n=1
	for i,v in ipairs{...} do
		if type(v)=="number" then
			if it[n]~=v then return false end
			n=n+1
		else
			for ii=1,#v do local vv=v[ii] -- allow one depth of tables
				if it[n]~=vv then return false end
				n=n+1
			end
		end
	end
	if n<#it then return false end -- array has more numbers than provided for test
	return true
end

--[[#lua.wetgenes.tardis.array.product

	ma = ma:product(mb)
	ma = ma:product(mb,r)

Look at the type and call the appropriate product function, to produce

	mb x ma

Note the right to left application and default returning of the
leftmost term for chaining. This seems to make the most sense.

If r is provided then the result is written into r and returned
otherwise ma is modified and returned.

]]
function array.product(a,b,r)

	local product=a.product_map[ tardis.type(b) ]
	if product then
		return product(a,b,r)
	end

	product=a.product_map[ "*" ] -- try wildcard before we give up
	if product then
		return product(a,b,r)
	end

	error("tardis : "..tardis.type(a).." product "..tardis.type(b).." not supported",2)
end


--[[#lua.wetgenes.tardis.m2

The metatable for a 2x2 matrix class, use the new function to actually create an object.

We also inherit all the functions from tardis.array

]]
m2=tardis.class("m2",array)

--[[#lua.wetgenes.tardis.m2.new

	m2 = tardis.m2.new()

Create a new m2 and optionally set it to the given values, m2 methods
usually return the input m2 for easy function chaining.

]]
function m2.new(...) return setmetatable({1,0, 0,1},m2):set(...) end

--[[#lua.wetgenes.tardis.m2.nset

	m2 = m2:nset(1,2,3,4)

Set exactly 4 numbers into the array.

]]
function m2.nset(it,a1,a2,a3,a4)
	it[1]=a1
	it[2]=a2
	it[3]=a3
	it[4]=a4
	return it
end

--[[#lua.wetgenes.tardis.m2.identity

	m2 = m2:identity()

Set this m2 to the identity matrix.

]]
function m2.identity(it) return m2.nset(it,1,0, 0,1) end

--[[#lua.wetgenes.tardis.m2.determinant

	value = m2:determinant()

Return the determinant value of this m2.

]]
function m2.determinant(it)
	return	 ( it[ 1 ]*it[ 2+2 ] )
			+( it[ 2 ]*it[ 2+1 ] )
			-( it[ 1 ]*it[ 2+1 ] )
			-( it[ 2 ]*it[ 2+1 ] )
end

--[[#lua.wetgenes.tardis.m2.minor_xy

	value = m2:minor_xy()

Return the minor_xy value of this m2.

]]
function m2.minor_xy(it,x,y)
	return it[1+(2-(x-1))+((2-(y-1))*2)]
end

--[[#lua.wetgenes.tardis.m2.transpose

	m2 = m2:transpose()
	m2 = m2:transpose(r)

Transpose this m2.

If r is provided then the result is written into r and returned
otherwise m2 is modified and returned.

]]
function m2.transpose(it,r)
	r=r or it
	return	 m2.nset(r,it[1],it[2+1], it[2],it[2+2])
end

--[[#lua.wetgenes.tardis.m2.scale

	m2 = m2:scale(s)
	m2 = m2:scale(s,r)

Scale this m2 by s.

If r is provided then the result is written into r and returned
otherwise m2 is modified and returned.

]]
function m2.scale(it,s,r)
	r=r or it
	return m2.nset(r,it[1]*s,it[2]*s, it[2+1]*s,it[2+2]*s)
end

--[[#lua.wetgenes.tardis.m2.cofactor

	m2 = m2:cofactor()
	m2 = m2:cofactor(r)

Cofactor this m2.

If r is provided then the result is written into r and returned
otherwise m2 is modified and returned.

]]
function m2.cofactor(it,r)
	r=r or it
	local t={}
	for iy=1,2 do
		for ix=1,2 do
			local idx=iy*2+ix-2
			if ((ix+iy)%2)==1 then
				t[idx]=-m2.minor_xy(it,ix,iy)
			else
				t[idx]=m2.minor_xy(it,ix,iy)
			end
		end
	end
	return r
end

--[[#lua.wetgenes.tardis.m2.adjugate

	m2 = m2:adjugate()
	m2 = m2:adjugate(r)

Adjugate this m2.

If r is provided then the result is written into r and returned
otherwise m2 is modified and returned.

]]
function m2.adjugate(it,r)
	r=r or it
	return m2.cofactor(m2.transpose(it,m2.new()),r)
end

--[[#lua.wetgenes.tardis.m2.inverse

	m2 = m2:inverse()
	m2 = m2:inverse(r)

Inverse this m2.

If r is provided then the result is written into r and returned
otherwise m2 is modified and returned.

]]
function m2.inverse(it,r)
	r=r or it
	local ood=1/m2.determinant(it)
	return m2.scale(m2.cofactor(m2.transpose(it,m2.new())),ood,r)
end



--[[#lua.wetgenes.tardis.m3

The metatable for a 3x3 matrix class, use the new function to actually
create an object.

We also inherit all the functions from tardis.array

]]
m3=tardis.class("m3",array)

--[[#lua.wetgenes.tardis.m3.new

	m3 = tardis.m3.new()

Create a new m3 and optionally set it to the given values, m3 methods
usually return the input m3 for easy function chaining.

]]
function m3.new(...) return setmetatable({1,0,0, 0,1,0, 0,0,1},m3):set(...) end

--[[#lua.wetgenes.tardis.m3.nset

	m3 = m3:nset(1,2,3,4,5,6,7,8,9)

Set exactly 9 numbers into the array.

]]
function m3.nset(it,a1,a2,a3,a4,a5,a6,a7,a8,a9)
	it[1]=a1
	it[2]=a2
	it[3]=a3
	it[4]=a4
	it[5]=a5
	it[6]=a6
	it[7]=a7
	it[8]=a8
	it[9]=a9
	return it
end

--[[#lua.wetgenes.tardis.m3.identity

	m3 = m3:identity()

Set this m3 to the identity matrix.

]]
function m3.identity(it) return m3.nset(it,1,0,0, 0,1,0, 0,0,1) end

--[[#lua.wetgenes.tardis.m3.m4

	m4 = m3:m4()

Expand an m3 matrix into an m4 matrix.

]]
function m3.m4(it)
	return tardis.M4():nset( it[1],it[2],it[3],0 , it[4],it[5],it[6],0 , it[7],it[8],it[9],0 , 0,0,0,1 )
end

--[[#lua.wetgenes.tardis.m3.v3

	v3 = m3:v3(n)

Extract and return a "useful" v3 from an m3 matrix. The first vector is the x
axis, then y axis , then z axis.


]]
function m3.v3(it,n)
	if     n==1 then
		return tardis.V3():nset(it[1],it[2],it[3])
	elseif n==2 then
		return tardis.V3():nset(it[4],it[5],it[6])
	elseif n==3 then
		return tardis.V3():nset(it[7],it[8],it[9])
	end
end

--[[#lua.wetgenes.tardis.m3.determinant

	value = m3:determinant()

Return the determinant value of this m3.

]]
function m3.determinant(it)
	return	 ( it[ 1 ]*it[ 3+2 ]*it[ 6+3 ] )
			+( it[ 2 ]*it[ 3+3 ]*it[ 6+1 ] )
			+( it[ 3 ]*it[ 3+1 ]*it[ 6+2 ] )
			-( it[ 1 ]*it[ 3+3 ]*it[ 6+2 ] )
			-( it[ 2 ]*it[ 3+1 ]*it[ 6+3 ] )
			-( it[ 3 ]*it[ 3+2 ]*it[ 6+1 ] )
end

--[[#lua.wetgenes.tardis.m3.minor_xy

	value = m3:minor_xy()

Return the minor_xy value of this m3.

]]
function m3.minor_xy(it,x,y)
	local t={}
	for ix=1,3 do
		for iy=1,3 do
			if (ix~=x) and (iy~=y) then
				t[#t+1]=it[ix+((iy-1)*3)]
			end
		end
	end
	return m2.determinant(t)
end

--[[#lua.wetgenes.tardis.m3.transpose

	m3 = m3:transpose()
	m3 = m3:transpose(r)

Transpose this m3.

If r is provided then the result is written into r and returned
otherwise m3 is modified and returned.

]]
function m3.transpose(it,r)
	r=r or r
	return	 m3.nset(r,it[1],it[3+1],it[6+1], it[2],it[3+2],it[6+2], it[3],it[3+3],it[6+3])
end

--[[#lua.wetgenes.tardis.m3.scale

	m3 = m3:scale(s)
	m3 = m3:scale(s,r)

Scale this m3 by s.

If r is provided then the result is written into r and returned
otherwise m3 is modified and returned.

]]
function m3.scale(it,s,r)
	r=r or it
	return m3.nset(r,it[1]*s,it[2]*s,it[3]*s, it[3+1]*s,it[3+2]*s,it[3+3]*s, it[6+1]*s,it[6+2]*s,it[6+3]*s)
end

--[[#lua.wetgenes.tardis.m3.cofactor

	m3 = m3:cofactor()
	m3 = m3:cofactor(r)

Cofactor this m3.

If r is provided then the result is written into r and returned
otherwise m3 is modified and returned.

]]
function m3.cofactor(it,r)
	r=r or it
	local t={}
	for iy=1,3 do
		for ix=1,3 do
			local idx=iy*3+ix-3
			if ((ix+iy)%2)==1 then
				t[idx]=-m3.minor_xy(it,ix,iy)
			else
				t[idx]=m3.minor_xy(it,ix,iy)
			end
		end
	end
	return array.aset(r,t)
end

--[[#lua.wetgenes.tardis.m3.adjugate

	m3 = m3:adjugate()
	m3 = m3:adjugate(r)

Adjugate this m3.

If r is provided then the result is written into r and returned
otherwise m3 is modified and returned.

]]
function m3.adjugate(it,r)
	r=r or it
	return m3.cofactor(m3.transpose(it,m3.new()),r)
end

--[[#lua.wetgenes.tardis.m3.inverse

	m3 = m3:inverse()
	m3 = m3:inverse(r)

Inverse this m3.

If r is provided then the result is written into r and returned
otherwise m3 is modified and returned.

]]
function m3.inverse(it,r)
	r=r or it
	local ood=1/m3.determinant(it)
	return m3.scale(m3.cofactor(m3.transpose(it,m3.new())),ood,r)
end

--[[#lua.wetgenes.tardis.m4

The metatable for a 4x4 matrix class, use the new function to actually
create an object.

We also inherit all the functions from tardis.array

]]
m4=tardis.class("m4",array)

--[[#lua.wetgenes.tardis.m4.new

	m4 = tardis.m4.new()

Create a new m4 and optionally set it to the given values, m4 methods
usually return the input m4 for easy function chaining.

]]
function m4.new(...) return setmetatable({1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},m4):set(...) end

--[[#lua.wetgenes.tardis.m4.nset

	m4 = m4:nset(1,2,3,4,5,6,7,8,9)

Set exactly 16 numbers into the array.

]]
function m4.nset(it,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16)
	it[1]=a1
	it[2]=a2
	it[3]=a3
	it[4]=a4
	it[5]=a5
	it[6]=a6
	it[7]=a7
	it[8]=a8
	it[9]=a9
	it[10]=a10
	it[11]=a11
	it[12]=a12
	it[13]=a13
	it[14]=a14
	it[15]=a15
	it[16]=a16
	return it
end

--[[#lua.wetgenes.tardis.m4.identity

	m4 = m4:identity()

Set this m4 to the identity matrix.

]]
function m4.identity(it) return m4.nset(it,1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1) end

--[[#lua.wetgenes.tardis.m4.m3

	m4 = m4:m3()

Grab tthe top,left parts of the m4 matrix as an m3 matrix.

]]
function m4.m3(it)
	return tardis.M3():nset( it[1],it[2],it[3] , it[5],it[6],it[7] , it[9],it[10],it[11] )
end

--[[#lua.wetgenes.tardis.m4.v3

	v3 = m4:v3(n)

Extract and return a "useful" v3 from an m4 matrix. The first vector is the x
axis, then y axis , then z axis and finally transform.

If n is not given or not a known value then we return the 4th vector which is
the "opengl" transform as that is the most useful v3 part of an m4.

]]
function m4.v3(it,n)
	if     n==1 then
		return tardis.V3():nset(it[1],it[2],it[3])
	elseif n==2 then
		return tardis.V3():nset(it[5],it[6],it[7])
	elseif n==3 then
		return tardis.V3():nset(it[9],it[10],it[11])
	else
		return tardis.V3():nset(it[13],it[14],it[15])
	end
end

--[[#lua.wetgenes.tardis.m4.determinant

	value = m4:determinant()

Return the determinant value of this m4.

]]
function m4.determinant(it)
return	(it[ 4 ] * it[ 4+3 ] * it[ 8+2 ] * it[ 12+1 ])-(it[ 3 ] * it[ 4+4 ] * it[ 8+2 ] * it[ 12+1 ])-
		(it[ 4 ] * it[ 4+2 ] * it[ 8+3 ] * it[ 12+1 ])+(it[ 2 ] * it[ 4+4 ] * it[ 8+3 ] * it[ 12+1 ])+
		(it[ 3 ] * it[ 4+2 ] * it[ 8+4 ] * it[ 12+1 ])-(it[ 2 ] * it[ 4+3 ] * it[ 8+4 ] * it[ 12+1 ])-
		(it[ 4 ] * it[ 4+3 ] * it[ 8+1 ] * it[ 12+2 ])+(it[ 3 ] * it[ 4+4 ] * it[ 8+1 ] * it[ 12+2 ])+
		(it[ 4 ] * it[ 4+1 ] * it[ 8+3 ] * it[ 12+2 ])-(it[ 1 ] * it[ 4+4 ] * it[ 8+3 ] * it[ 12+2 ])-
		(it[ 3 ] * it[ 4+1 ] * it[ 8+4 ] * it[ 12+2 ])+(it[ 1 ] * it[ 4+3 ] * it[ 8+4 ] * it[ 12+2 ])+
		(it[ 4 ] * it[ 4+2 ] * it[ 8+1 ] * it[ 12+3 ])-(it[ 2 ] * it[ 4+4 ] * it[ 8+1 ] * it[ 12+3 ])-
		(it[ 4 ] * it[ 4+1 ] * it[ 8+2 ] * it[ 12+3 ])+(it[ 1 ] * it[ 4+4 ] * it[ 8+2 ] * it[ 12+3 ])+
		(it[ 2 ] * it[ 4+1 ] * it[ 8+4 ] * it[ 12+3 ])-(it[ 1 ] * it[ 4+2 ] * it[ 8+4 ] * it[ 12+3 ])-
		(it[ 3 ] * it[ 4+2 ] * it[ 8+1 ] * it[ 12+4 ])+(it[ 2 ] * it[ 4+3 ] * it[ 8+1 ] * it[ 12+4 ])+
		(it[ 3 ] * it[ 4+1 ] * it[ 8+2 ] * it[ 12+4 ])-(it[ 1 ] * it[ 4+3 ] * it[ 8+2 ] * it[ 12+4 ])-
		(it[ 2 ] * it[ 4+1 ] * it[ 8+3 ] * it[ 12+4 ])+(it[ 1 ] * it[ 4+2 ] * it[ 8+3 ] * it[ 12+4 ])
end

--[[#lua.wetgenes.tardis.m4.minor_xy

	value = m4:minor_xy()

Return the minor_xy value of this m4.

]]
function m4.minor_xy(it,x,y)
	local t={}
	for ix=1,4 do
		for iy=1,4 do
			if (ix~=x) and (iy~=y) then
				t[#t+1]=it[ix+((iy-1)*4)]
			end
		end
	end
	return m3.determinant(t)
end

--[[#lua.wetgenes.tardis.m4.transpose

	m4 = m4:transpose()
	m4 = m4:transpose(r)

Transpose this m4.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.transpose(it,r)
	r=r or it
	return	 m4.nset(r,it[1],it[4+1],it[8+1],it[12+1], it[2],it[4+2],it[8+2],it[12+2], it[3],it[4+3],it[8+3],it[12+3], it[4],it[4+4],it[8+4],it[12+4])
end

--[[#lua.wetgenes.tardis.m4.add

	m4 = m4:add(m4b)
	m4 = m4:add(m4b,r)

Add m4b this m4.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.add(it,m,r)
	r=r or it
	return m4.nset(r,
		it[ 1]+m[ 1],it[ 2]+m[ 2],it[ 3]+m[ 3],it[ 4]+m[ 4],
		it[ 5]+m[ 5],it[ 6]+m[ 6],it[ 7]+m[ 7],it[ 8]+m[ 8],
		it[ 9]+m[ 9],it[10]+m[10],it[11]+m[11],it[12]+m[12],
		it[13]+m[13],it[14]+m[14],it[15]+m[15],it[16]+m[16])
end

--[[#lua.wetgenes.tardis.m4.sub

	m4 = m4:sub(m4b)
	m4 = m4:sub(m4b,r)

Subtract m4b this m4.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.sub(it,m,r)
	r=r or it
	return m4.nset(r,
		it[ 1]-m[ 1],it[ 2]-m[ 2],it[ 3]-m[ 3],it[ 4]-m[ 4],
		it[ 5]-m[ 5],it[ 6]-m[ 6],it[ 7]-m[ 7],it[ 8]-m[ 8],
		it[ 9]-m[ 9],it[10]-m[10],it[11]-m[11],it[12]-m[12],
		it[13]-m[13],it[14]-m[14],it[15]-m[15],it[16]-m[16])
end

--[[#lua.wetgenes.tardis.m4.mix

	m4 = m4:mix(m4b,s)
	m4 = m4:mix(m4b,s,r)

Lerp from m4 to m4b by s.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.mix(it,m,s,r)
	r=r or m4.new()
	r:set(m)
	r:sub(it)
	r:scale(s)
	r:add(it)
	return r
end

--[[#lua.wetgenes.tardis.m4.cofactor

	m4 = m4:cofactor()
	m4 = m4:cofactor(r)

Cofactor this m4.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.cofactor(it,r)
	r=r or it
	local t={}
	for iy=1,4 do
		for ix=1,4 do
			local idx=iy*4+ix-4
			if ((ix+iy)%2)==1 then
				t[idx]=-m4.minor_xy(it,ix,iy)
			else
				t[idx]=m4.minor_xy(it,ix,iy)
			end
		end
	end
	return array.aset(r,t)
end

--[[#lua.wetgenes.tardis.m4.adjugate

	m4 = m4:adjugate()
	m4 = m4:adjugate(r)

Adjugate this m4.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.adjugate(it,r)
	r=r or it
	return 	m4.cofactor(m4.transpose(it,m4.new()),r)
end

--[[#lua.wetgenes.tardis.m4.inverse

	m4 = m4:inverse()
	m4 = m4:inverse(r)

Inverse this m4.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.inverse(it,r)
	r=r or it
	local ood=1/m4.determinant(it)
	local d=m4.scalar(m4.cofactor(m4.transpose(it,m4.new())),ood)
	return array.aset(r,d)
end

--[[#lua.wetgenes.tardis.m4.translate_v3

	m4 = m4:translate_v3(v3a)
	m4 = m4:translate_v3(v3a,r)

Translate this m4 along its local axis by v3a.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.translate_v3(it,v3a,r)
	r=r or it
	local r1=it[12+1]+v3a[1]*it[1]+v3a[2]*it[5]+v3a[3]*it[9]
	local r2=it[12+2]+v3a[1]*it[2]+v3a[2]*it[6]+v3a[3]*it[10]
	local r3=it[12+3]+v3a[1]*it[3]+v3a[2]*it[7]+v3a[3]*it[11]
	local r4=it[12+4]+v3a[1]*it[4]+v3a[2]*it[8]+v3a[3]*it[12]
	return m4.nset(r,it[1],it[2],it[3],it[4], it[5],it[6],it[7],it[8], it[9],it[10],it[11],it[12], r1,r2,r3,r4 )
end

--[[#lua.wetgenes.tardis.m4.pretranslate_v3

	m4 = m4:pretranslate_v3(v3a)
	m4 = m4:pretranslate_v3(v3a,r)

Translate this m4 along its global axis by v3a.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.pretranslate_v3(it,v3a,r)
	r=r or it
	local r1=it[12+1]+v3a[1]
	local r2=it[12+2]+v3a[2]
	local r3=it[12+3]+v3a[3]
	return m4.nset(r,it[1],it[2],it[3],it[4], it[5],it[6],it[7],it[8], it[9],it[10],it[11],it[12], r1,r2,r3,it[16] )
end

--[[#lua.wetgenes.tardis.m4.translate

	m4 = m4:translate(x,y,z)
	m4 = m4:translate(x,y,z,r)
	m4 = m4:translate(v3a)
	m4 = m4:translate(v3a,r)

Translate this m4 along its local axis by {x,y,z} or v3a.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.translate(it,a,b,c,d)
	if type(a)=="table" then
		return m4.translate_v3(it,a,b)
	else
		return m4.translate_v3(it,{a,b,c},d)
	end
end

--[[#lua.wetgenes.tardis.m4.pretranslate

	m4 = m4:pretranslate(x,y,z)
	m4 = m4:pretranslate(x,y,z,r)
	m4 = m4:pretranslate(v3a)
	m4 = m4:pretranslate(v3a,r)

Translate this m4 along its global axis by {x,y,z} or v3a.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.pretranslate(it,a,b,c,d)
	if type(a)=="table" then
		return m4.pretranslate_v3(it,a,b)
	else
		return m4.pretranslate_v3(it,{a,b,c},d)
	end
end

--[[#lua.wetgenes.tardis.m4.scale_v3

	m4 = m4:scale_v3(v3)
	m4 = m4:scale_v3(v3,r)

Scale this m4 by {x,y,z} or v3.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.scale_v3(it,v3a,r)
	r=r or it
	local s1=v3a[1]
	local s2=v3a[2]
	local s3=v3a[3]
	return m4.nset(r,
		s1*it[1],	s1*it[2],	s1*it[3],	s1*it[4],
		s2*it[5],	s2*it[6],	s2*it[7],	s2*it[8],
		s3*it[9],	s3*it[10],	s3*it[11],	s3*it[12],
		it[13],		it[14],		it[15],		it[16]
	)
end

--[[#lua.wetgenes.tardis.m4.scale

	m4 = m4:scale(s)
	m4 = m4:scale(s,r)
	m4 = m4:scale(x,y,z)
	m4 = m4:scale(x,y,z,r)
	m4 = m4:scale(v3)
	m4 = m4:scale(v3,r)

Scale this m4 by {s,s,s} or {x,y,z} or v3.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.scale(it,a,b,c,d)
	if type(a)=="table" then
		return m4.scale_v3(it,a,b)
	elseif c then
		return m4.scale_v3(it,{a,b,c},d)
	else
		return m4.scale_v3(it,{a,a,a},b)
	end
end

--[[#lua.wetgenes.tardis.m4.scale_v3

	m4 = m4:scale_v3(v3)
	m4 = m4:scale_v3(v3,r)

Scale this m4 by {x,y,z} or v3.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.prescale_v3(it,v3a,r)
	r=r or it
	local s1=v3a[1]
	local s2=v3a[2]
	local s3=v3a[3]
	return m4.nset(r,
		s1*it[1],	s2*it[2],	s3*it[3],	it[4],
		s1*it[5],	s2*it[6],	s3*it[7],	it[8],
		s1*it[9],	s2*it[10],	s3*it[11],	it[12],
		s1*it[13],	s2*it[14],	s3*it[15],	it[16]
	)
end

--[[#lua.wetgenes.tardis.m4.prescale

	m4 = m4:scale(s)
	m4 = m4:scale(s,r)
	m4 = m4:scale(x,y,z)
	m4 = m4:scale(x,y,z,r)
	m4 = m4:scale(v3)
	m4 = m4:scale(v3,r)

Pre Scale this m4 by {s,s,s} or {x,y,z} or v3.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.prescale(it,a,b,c,d)
	if type(a)=="table" then
		return m4.prescale_v3(it,a,b)
	elseif c then
		return m4.prescale_v3(it,{a,b,c},d)
	else
		return m4.prescale_v3(it,{a,a,a},b)
	end
end

--[[#lua.wetgenes.tardis.m4.get_translation_v3

	v3 = m4:get_translation_v3(r)

Get v3 translation from a scale/rot/trans matrix

If r is provided then the result is written into r and returned
otherwise a new v3 is created and returned.

]]
function m4.get_translation_v3(it,r)
	r=r or tardis.v3.new()
	return v3.nset(r,it[13],it[14],it[15])
end

--[[#lua.wetgenes.tardis.m4.get_scale_v3

	v3 = m4:get_scale_v3(r)

Get v3 scale from a scale/rot/trans matrix

If r is provided then the result is written into r and returned
otherwise a new v3 is created and returned.

]]
function m4.get_scale_v3(it,r)
	r=r or tardis.v3.new()
	return v3.nset(r,
		math.sqrt(it[1]*it[1]+it[5]*it[5]+it[ 9]*it[ 9]),
		math.sqrt(it[2]*it[2]+it[6]*it[6]+it[10]*it[10]),
		math.sqrt(it[3]*it[3]+it[7]*it[7]+it[11]*it[11])
	)
end

--[[#lua.wetgenes.tardis.m4.get_rotation_q4

	q4 = m4:get_rotation_q4(r)

Get quaternion rotation from a scale/rot/trans matrix. Note that scale
is assumed to be uniform which it usually is. If that is not the case
then remove the scale first.

If r is provided then the result is written into r and returned
otherwise a new q4 is created and returned.

]]
function m4.get_rotation_q4(it,r)
	r=r or tardis.q4.new()
	local q1,q2,q3,q4,t
	if it[11] < 0 then
		if it[ 1] > it[ 6] then
			t = 1 + it[ 1] - it[ 6] - it[11]
			q1=t ; q2=it[ 2]+it[ 5] ; q3=it[ 9]+it[ 3] ; q4=it[ 7]-it[10]
		else
			t = 1 - it[ 1] + it[ 6] - it[11]
			q1=it[ 2]+it[ 5] ; q2=t ; q3=it[ 7]+t[10] ; q4=it[ 9]-it[ 3]
		end
	else
		if it[ 1] < -it[ 6] then
			t = 1 - it[ 1] - it[ 6] + it[11]
			q1=it[ 9]+it[ 3] ; q2=it[ 7]+it[10] ; q3=t ; q4=it[ 2]-it[ 5]
		else
			t = 1 + it[ 1] + it[ 6] + it[11]
			q1=it[ 7]-it[10] ; q2=it[ 9]-it[ 3] ; q3=it[ 2]-it[ 5] ; q4=t
		end
	end
	t=0.5/math.sqrt(t)
	return q4.nset(r,q1*t,q2*t,q3*t,q4*t)
end

--[[#lua.wetgenes.tardis.m4.setrot

	m4 = m4:setrot(degrees,v3a)

Set this matrix to a rotation matrix around the given normal by the given degrees.

we will automatically normalise v3a if necessary.

]]
function m4.setrot(it,degrees,v3a)

	local c=math.cos(-math.pi*degrees/180)
	local cc=1-c
	local s=math.sin(-math.pi*degrees/180)

	local x=v3a[1]
	local y=v3a[2]
	local z=v3a[3]

	local delta=0.001 -- a smallish number
	local dd=( (x*x) + (y*y) + (z*z) )
	if ( dd < (1-delta) ) or ( dd > (1+delta) ) then -- not even close to a unit vector
		local d=math.sqrt(dd)
		x=x/d
		y=y/d
		z=z/d
	end

	return m4.nset(it,
		x*x*cc+c	,	x*y*cc-z*s	,	x*z*cc+y*s	, 	0	,
		x*y*cc+z*s	,	y*y*cc+c	,	y*z*cc-x*s	,	0	,
        x*z*cc-y*s	,	y*z*cc+x*s	,	z*z*cc+c	,	0	,
        0			,	0			,	0			,	1	)

end

--[[#lua.wetgenes.tardis.m4.setrrot

	m4 = m4:setrrot(radians,v3a)

Set this matrix to a rotation matrix around the given normal by the given radians.

we will automatically normalise v3a if necessary.

]]
function m4.setrrot(it,radians,v3a)

	local c=math.cos(-radians)
	local cc=1-c
	local s=math.sin(-radians)

	local x=v3a[1]
	local y=v3a[2]
	local z=v3a[3]

	local delta=0.001 -- a smallish number
	local dd=( (x*x) + (y*y) + (z*z) )
	if ( dd < (1-delta) ) or ( dd > (1+delta) ) then -- not even close to a unit vector
		local d=math.sqrt(dd)
		x=x/d
		y=y/d
		z=z/d
	end

	return m4.nset(it,
		x*x*cc+c	,	x*y*cc-z*s	,	x*z*cc+y*s	, 	0	,
		x*y*cc+z*s	,	y*y*cc+c	,	y*z*cc-x*s	,	0	,
        x*z*cc-y*s	,	y*z*cc+x*s	,	z*z*cc+c	,	0	,
        0			,	0			,	0			,	1	)

end

--[[#lua.wetgenes.tardis.m4.arotate

	m4 = m4:arotate(degrees,v3a)
	m4 = m4:arotate(degrees,v3a,r)

Apply a rotation in degres around the given axis to this matrix.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.arotate(it,degrees,v3a,r)
	local m=m4.new():setrot(degrees,v3a)
	return tardis.m4_product_m4(it,m,r)
end

--[[#lua.wetgenes.tardis.m4.rrotate

	m4 = m4:rrotate(radians,v3a)
	m4 = m4:rrotate(radians,v3a,r)

Apply a rotation in radians around the given axis to this matrix.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.rrotate(it,radians,v3a,r)
	local m=m4.new():setrrot(radians,v3a)
	return tardis.m4_product_m4(it,m,r)
end

--[[#lua.wetgenes.tardis.m4.qrotate

	m4 = m4:qrotate(q)
	m4 = m4:qrotate(q,r)

Apply a quaternion rotation to this matrix.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.qrotate(it,q,r)
	local m=tardis.q4_to_m4(q)
	return tardis.m4_product_m4(it,m,r)
end

--[[#lua.wetgenes.tardis.m4.rotate

	m4 = m4:rotate(degrees,v3a)
	m4 = m4:rotate(degrees,v3a,r)
	m4 = m4:rotate(degrees,x,y,z)
	m4 = m4:rotate(degrees,x,y,z,r)
	m4 = m4:rotate(q)
	m4 = m4:rotate(q,r)

Apply quaternion or angle rotation to this matrix depending on
arguments provided.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.rotate(it,a,b,c,d,e)
	if type(a)=="table" then -- q4
		return m4.qrotate(it,a,b)
	elseif type(b)=="table" then -- v3
		return m4.arotate(it,a,b,c)
	else
		return m4.arotate(it,a,{b,c,d},e)
	end
end


--[[#lua.wetgenes.tardis.m4.prearotate

	m4 = m4:prearotate(degrees,v3a)
	m4 = m4:prearotate(degrees,v3a,r)

Pre apply a rotation in degrees around the given axis to this matrix.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.prearotate(it,degrees,v3a,r)
	local m=m4.new():setrot(degrees,v3a)
	return tardis.m4_product_m4(m,it,r or it)
end

--[[#lua.wetgenes.tardis.m4.prerrotate

	m4 = m4:prerrotate(radians,v3a)
	m4 = m4:prerrotate(radians,v3a,r)

Pre apply a rotation in radians around the given axis to this matrix.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.prerrotate(it,radians,v3a,r)
	local m=m4.new():setrrot(radians,v3a)
	return tardis.m4_product_m4(m,it,r or it)
end

--[[#lua.wetgenes.tardis.m4.preqrotate

	m4 = m4:preqrotate(q)
	m4 = m4:preqrotate(q,r)

Pre apply a quaternion rotation to this matrix.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.preqrotate(it,q,r)
	local m=tardis.q4_to_m4(q)
	return tardis.m4_product_m4(m,it,r or it)
end

--[[#lua.wetgenes.tardis.m4.prerotate

	m4 = m4:prerotate(degrees,v3a)
	m4 = m4:prerotate(degrees,v3a,r)
	m4 = m4:prerotate(degrees,x,y,z)
	m4 = m4:prerotate(degrees,x,y,z,r)
	m4 = m4:prerotate(q)
	m4 = m4:prerotate(q,r)

Pre apply quaternion or angle rotation to this matrix depending on
arguments provided.

If r is provided then the result is written into r and returned
otherwise m4 is modified and returned.

]]
function m4.prerotate(it,a,b,c,d,e)
	if type(a)=="table" then -- q4
		return m4.preqrotate(it,a,b)
	elseif type(b)=="table" then -- v3
		return m4.prearotate(it,a,b,c)
	else
		return m4.prearotate(it,a,{b,c,d},e)
	end
end


--[[#lua.wetgenes.tardis.v1

The metatable for a 2d vector class, use the new function to actually
create an object.

We also inherit all the functions from tardis.array

]]
v1=tardis.class("v1",array)

--[[#lua.wetgenes.tardis.v2.new

	v1 = tardis.v1.new()

Create a new v1 and optionally set it to the given values, v1 methods
usually return the input v1 for easy function chaining.

]]
function v1.new(...) return setmetatable({0},v1):set(...) end

--[[#lua.wetgenes.tardis.v1.nset

	v1 = v1:nset(1)

Set exactly 1 number into the array.

]]
function v1.nset(it,a1)
	it[1]=a1
	return it
end

--[[#lua.wetgenes.tardis.v1.identity

	v1 = v1:identity()

Set this v1 to all zeros.

]]
function v1.identity(it) return v1.nset(it,0) end

--[[#lua.wetgenes.tardis.v1.lenlen

	value = v1:lenlen()

Returns the length of this vector, squared, this is often all you need
for comparisons so lets us skip the sqrt.

]]
function v1.lenlen(it)
	return (it[1]*it[1])
end

--[[#lua.wetgenes.tardis.v1.len

	value = v1:len()

Returns the length of this vector.

]]
function v1.len(it)
	return math.abs( it[1] )
end


--[[#lua.wetgenes.tardis.v1.distance

	value = a:distance(b)

Returns the length of the vector between a and b.

]]
function v1.distance(a,b)
	return math.abs( a[1]-b[1] )
end

--[[#lua.wetgenes.tardis.v1.oo

	v1 = v1:oo()
	v1 = v1:oo(r)

One Over value. Build the reciprocal of all elements.

If r is provided then the result is written into r and returned
otherwise v1 is modified and returned.

]]
function v1.oo(it,r)
	r=r or it
	return v1.nset(r, 1/it[1] )
end

--[[#lua.wetgenes.tardis.v1.scale

	v1 = v1:scale(s)
	v1 = v1:scale(s,r)

Scale this v1 by s.

If r is provided then the result is written into r and returned
otherwise v1 is modified and returned.

]]
function v1.scale(it,s,r)
	r=r or it
	return v1.nset(r, it[1]*s )
end

--[[#lua.wetgenes.tardis.v1.normalize

	v1 = v1:normalize()
	v1 = v1:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned
otherwise v1 is modified and returned.

]]
function v1.normalize(it,r)
	local l=v1.len(it)
	if l>0 then l=1/l end
	return v1.scale(it,l,r)
end

--[[#lua.wetgenes.tardis.v1.add

	v1 = v1:add(v1b)
	v1 = v1:add(v1b,r)

Add v1b to v1.

If r is provided then the result is written into r and returned
otherwise v1 is modified and returned.

]]
function v1.add(va,vb,r)
	r=r or va
	return v1.nset(r, va[1]+vb[1] )
end

--[[#lua.wetgenes.tardis.v1.sub

	v1 = v1:sub(v1b)
	v1 = v1:sub(v1b,r)

Subtract v1b from v1.

If r is provided then the result is written into r and returned
otherwise v1 is modified and returned.

]]
function v1.sub(va,vb,r)
	r=r or va
	return v1.nset(r, va[1]-vb[1] )
end

--[[#lua.wetgenes.tardis.v1.mul

	v1 = v1:mul(v1b)
	v1 = v1:mul(v1b,r)

Multiply v1 by v1b.

If r is provided then the result is written into r and returned
otherwise v1 is modified and returned.

]]
function v1.mul(va,vb,r)
	r=r or va
	return v1.nset(r, (va[1]*vb[1]) )
end

--[[#lua.wetgenes.tardis.v1.dot

	value = v1:dot(v1b)

Return the dot product of these two vectors.

]]
function v1.dot(va,vb)
	return ( (va[1]*vb[1]) )
end

--[[#lua.wetgenes.tardis.v1.cross

	value = v1:cross(v1b)

Extend to 3d then only return z value as x and y are always 0

]]
function v1.cross(va,vb)
	return va[1]*vb[1]
end

--[[#lua.wetgenes.tardis.v2

The metatable for a 2d vector class, use the new function to actually
create an object.

We also inherit all the functions from tardis.array

]]
v2=tardis.class("v2",array)

--[[#lua.wetgenes.tardis.v2.new

	v2 = tardis.v2.new()

Create a new v2 and optionally set it to the given values, v2 methods
usually return the input v2 for easy function chaining.

]]
function v2.new(...) return setmetatable({0,0},v2):set(...) end

--[[#lua.wetgenes.tardis.v2.nset

	v2 = v2:nset(1,2)

Set exactly 2 numbers into the array.

]]
function v2.nset(it,a1,a2)
	it[1]=a1
	it[2]=a2
	return it
end

--[[#lua.wetgenes.tardis.v2.identity

	v2 = v2:identity()

Set this v2 to all zeros.

]]
function v2.identity(it) return v2.nset(it,0,0) end

--[[#lua.wetgenes.tardis.v2.lenlen

	value = v2:lenlen()

Returns the length of this vector, squared, this is often all you need
for comparisons so lets us skip the sqrt.

]]
function v2.lenlen(it)
	return (it[1]*it[1]) + (it[2]*it[2])
end

--[[#lua.wetgenes.tardis.v2.len

	value = v2:len()

Returns the length of this vector.

]]
function v2.len(it)
	return math.sqrt( (it[1]*it[1]) + (it[2]*it[2]) )
end


--[[#lua.wetgenes.tardis.v2.distance

	value = a:distance(b)

Returns the length of the vector between a and b.

]]
function v2.distance(a,b)
	local d1=a[1]-b[1]
	local d2=a[2]-b[2]
	return math.sqrt( d1*d1 + d2*d2 )
end

--[[#lua.wetgenes.tardis.v2.oo

	v2 = v2:oo()
	v2 = v2:oo(r)

One Over value. Build the reciprocal of all elements.

If r is provided then the result is written into r and returned
otherwise v2 is modified and returned.

]]
function v2.oo(it,r)
	r=r or it
	return v2.nset(r, 1/it[1] , 1/it[2] )
end

--[[#lua.wetgenes.tardis.v2.scale

	v2 = v2:scale(s)
	v2 = v2:scale(s,r)

Scale this v2 by s.

If r is provided then the result is written into r and returned
otherwise v2 is modified and returned.

]]
function v2.scale(it,s,r)
	r=r or it
	return v2.nset(r, it[1]*s , it[2]*s )
end

--[[#lua.wetgenes.tardis.v2.normalize

	v2 = v2:normalize()
	v2 = v2:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned
otherwise v2 is modified and returned.

]]
function v2.normalize(it,r)
	local l=v2.len(it)
	if l>0 then l=1/l end
	return v2.scale(it,l,r)
end

--[[#lua.wetgenes.tardis.v2.add

	v2 = v2:add(v2b)
	v2 = v2:add(v2b,r)

Add v2b to v2.

If r is provided then the result is written into r and returned
otherwise v2 is modified and returned.

]]
function v2.add(va,vb,r)
	r=r or va
	return v2.nset(r, va[1]+vb[1] , va[2]+vb[2] )
end

--[[#lua.wetgenes.tardis.v2.sub

	v2 = v2:sub(v2b)
	v2 = v2:sub(v2b,r)

Subtract v2b from v2.

If r is provided then the result is written into r and returned
otherwise v2 is modified and returned.

]]
function v2.sub(va,vb,r)
	r=r or va
	return v2.nset(r, va[1]-vb[1] , va[2]-vb[2] )
end

--[[#lua.wetgenes.tardis.v2.mul

	v2 = v2:mul(v2b)
	v2 = v2:mul(v2b,r)

Multiply v2 by v2b.

If r is provided then the result is written into r and returned
otherwise v2 is modified and returned.

]]
function v2.mul(va,vb,r)
	r=r or va
	return v2.nset(r, (va[1]*vb[1]) , (va[2]*vb[2]) )
end

--[[#lua.wetgenes.tardis.v2.dot

	value = v2:dot(v2b)

Return the dot product of these two vectors.

]]
function v2.dot(va,vb)
	return ( (va[1]*vb[1]) + (va[2]*vb[2]) )
end

--[[#lua.wetgenes.tardis.v2.cross

	value = v2:cross(v2b)

Extend to 3d then only return z value as x and y are always 0

]]
function v2.cross(va,vb)
	return (va[1]*vb[2])-(va[2]*vb[1])
end

--[[#lua.wetgenes.tardis.v3

The metatable for a 3d vector class, use the new function to actually
create an object.

We also inherit all the functions from tardis.array

]]
v3=tardis.class("v3",array)

--[[#lua.wetgenes.tardis.v3.new

	v3 = tardis.v3.new()

Create a new v3 and optionally set it to the given values, v3 methods
usually return the input v3 for easy function chaining.

]]
function v3.new(...) return setmetatable({0,0,0},v3):set(...) end

--[[#lua.wetgenes.tardis.v3.nset

	v3 = v3:nset(1,2,3)

Set exactly 3 numbers into the array.

]]
function v3.nset(it,a1,a2,a3)
	it[1]=a1
	it[2]=a2
	it[3]=a3
	return it
end

--[[#lua.wetgenes.tardis.v3.identity

	v3 = v3:identity()

Set this v3 to all zeros.

]]
function v3.identity(it) return v3.nset(it,0,0,0) end

--[[#lua.wetgenes.tardis.v3.lenlen

	value = v3:lenlen()

Returns the length of this vector, squared, this is often all you need
for comparisons so lets us skip the sqrt.

]]
function v3.lenlen(it)
	return (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3])
end

--[[#lua.wetgenes.tardis.v3.len

	value = v3:len()

Returns the length of this vector.

]]
function v3.len(it)
	return math.sqrt( (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3]) )
end

--[[#lua.wetgenes.tardis.v3.distance

	value = a:distance(b)

Returns the length of the vector between a and b.

]]
function v3.distance(a,b)
	local d1=a[1]-b[1]
	local d2=a[2]-b[2]
	local d3=a[3]-b[3]
	return math.sqrt( d1*d1 + d2*d2 + d3*d3 )
end

--[[#lua.wetgenes.tardis.v3.oo

	v3 = v3:oo()
	v3 = v3:oo(r)

One Over value. Build the reciprocal of all elements.

If r is provided then the result is written into r and returned
otherwise v3 is modified and returned.

]]
function v3.oo(it,r)
	r=r or it
	return v3.nset(r, 1/it[1] , 1/it[2] , 1/it[3] )
end

--[[#lua.wetgenes.tardis.v3.scale

	v3 = v3:scale(s)
	v3 = v3:scale(s,r)

Scale this v3 by s.

If r is provided then the result is written into r and returned
otherwise v3 is modified and returned.

]]
function v3.scale(it,s,r)
	r=r or it
	return v3.nset(r, it[1]*s , it[2]*s , it[3]*s )
end

--[[#lua.wetgenes.tardis.v3.normalize

	v3 = v3:normalize()
	v3 = v3:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned
otherwise v3 is modified and returned.

]]
function v3.normalize(it,r)
	local l=v3.len(it)
	if l>0 then l=1/l end
	return v3.scale(it,l,r)
end

--[[#lua.wetgenes.tardis.v3.add

	v3 = v3:add(v3b)
	v3 = v3:add(v3b,r)

Add v3b to v3.

If r is provided then the result is written into r and returned
otherwise v3 is modified and returned.

]]
function v3.add(va,vb,r)
	r=r or va
	return v3.nset(r, va[1]+vb[1] , va[2]+vb[2] , va[3]+vb[3] )
end

--[[#lua.wetgenes.tardis.v3.sub

	v3 = v3:sub(v3b)
	v3 = v3:sub(v3b,r)

Subtract v3b from v3.

If r is provided then the result is written into r and returned
otherwise v3 is modified and returned.

]]
function v3.sub(va,vb,r)
	r=r or va
	return v3.nset(r, va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] )
end

--[[#lua.wetgenes.tardis.v3.mul

	v3 = v3:mul(v3b)
	v3 = v3:mul(v3b,r)

Multiply v3 by v3b.

If r is provided then the result is written into r and returned
otherwise v3 is modified and returned.

]]
function v3.mul(va,vb,r)
	r=r or va
	return v3.nset(r, (va[1]*vb[1]) , (va[2]*vb[2]) , (va[3]*vb[3]) )
end

--[[#lua.wetgenes.tardis.v3.dot

	value = v3:dot(v3b)

Return the dot product of these two vectors.

]]
function v3.dot(va,vb)
	return ( (va[1]*vb[1]) + (va[2]*vb[2]) + (va[3]*vb[3]) )
end

--[[#lua.wetgenes.tardis.v3.cross

	v3 = v3:cross(v3b)
	v3 = v3:cross(v3b,r)

Return the cross product of these two vectors.

If r is provided then the result is written into r and returned
otherwise v3 is modified and returned.

]]
function v3.cross(va,vb,r)
	r=r or va
	return v3.nset(r, (va[2]*vb[3])-(va[3]*vb[2]) , (va[3]*vb[1])-(va[1]*vb[3]) , (va[1]*vb[2])-(va[2]*vb[1]) )
end


--[[#lua.wetgenes.tardis.v3.angle

	radians,axis = v3a:angle(v3b)
	radians,axis = v3a:angle(v3b,axis)

Return radians and axis of rotation between these two vectors. If axis is given
then it must represent a positive world aligned axis normal. So V3(1,0,0) or
V3(0,1,0) or V3(0,0,1) only. The point of providing an axis allows the returned
angle to be over a 360 degree range rather than flipping the axis after 180
degrees this means the second axis returned value can be ignored as it will
always be the axis that is passed in.

]]
function v3.angle(va,vb,axis)

	local na
	local nb

	if axis then
		local nc=tardis.v3.new( 1-axis[1] , 1-axis[2] , 1-axis[3] )
		na=(va*nc):normalize(tardis.v3.new())
		nb=(vb*nc):normalize(tardis.v3.new())
	else
		na=va:normalize(tardis.v3.new())
		nb=vb:normalize(tardis.v3.new())
	end

	local d=na:dot(nb)
	local r=math.acos( d>1 and 1 or  d<-1 and -1 or d ) -- clamp to avoid nan

	local x=na:cross( nb , tardis.v3.new() ):normalize()

	if axis then
		if x:dot(axis) < 0 then -- flip negative axis so we can ignore it
			r=-r
			x:aset(axis)
		end
	end

	return r,x

end

--[[#lua.wetgenes.tardis.v4

The metatable for a 4d vector class, use the new function to actually
create an object.

We also inherit all the functions from tardis.array

]]
v4=tardis.class("v4",array)

--[[#lua.wetgenes.tardis.v4.new

	v4 = tardis.v4.new()

Create a new v4 and optionally set it to the given values, v4 methods
usually return the input v4 for easy function chaining.

]]
function v4.new(...) return setmetatable({0,0,0,0},v4):set(...) end

--[[#lua.wetgenes.tardis.v4.nset

	v4 = v4:nset(1,2,3,4)

Set exactly 4 numbers into the array.

]]
function v4.nset(it,a1,a2,a3,a4)
	it[1]=a1
	it[2]=a2
	it[3]=a3
	it[4]=a4
	return it
end

--[[#lua.wetgenes.tardis.v4.identity

	v4 = v4:identity()

Set this v4 to all zeros.

]]
function v4.identity(it) return v4.nset(it,0,0,0,0) end

--[[#lua.wetgenes.tardis.v4.to_v3

	v3 = v4:to_v3()
	v3 = v4:to_v3(r)

scale [4] to 1 then throw it away so we have a v3 xyz

If r is provided then the result is written into r and returned
otherwise a new v3 is created and returned.

]]
function v4.to_v3(it,r)
	r=r or v3.new()
	local oow=1/it[4]
	return v4.nset(r, it[1]*oow , it[2]*oow , it[3]*oow )
end

--[[#lua.wetgenes.tardis.v4.lenlen

	value = v4:lenlen()

Returns the length of this vector, squared, this is often all you need
for comparisons so lets us skip the sqrt.

]]
function v4.lenlen(it)
	return (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3]) + (it[4]*it[4])
end

--[[#lua.wetgenes.tardis.v4.len

	value = v4:len()

Returns the length of this vector.

]]
function v4.len(it)
	return math.sqrt( (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3]) + (it[4]*it[4]) )
end

--[[#lua.wetgenes.tardis.v4.distance

	value = a:distance(b)

Returns the length of the vector between a and b.

]]
function v4.distance(a,b)
	local d1=a[1]-b[1]
	local d2=a[2]-b[2]
	local d3=a[3]-b[3]
	local d4=a[3]-b[4]
	return math.sqrt( d1*d1 + d2*d2 + d3*d3 + d4*d4 )
end

--[[#lua.wetgenes.tardis.v4.oo

	v4 = v4:oo()
	v4 = v4:oo(r)

One Over value. Build the reciprocal of all elements.

If r is provided then the result is written into r and returned
otherwise v4 is modified and returned.

]]
function v4.oo(it,r)
	r=r or it
	return v4.nset(r, 1/it[1] , 1/it[2] , 1/it[3] , 1/it[4] )
end

--[[#lua.wetgenes.tardis.v4.scale

	v4 = v4:scale(s)
	v4 = v4:scale(s,r)

Scale this v4 by s.

If r is provided then the result is written into r and returned
otherwise v4 is modified and returned.

]]
function v4.scale(it,s,r)
	r=r or it
	return v4.nset(r, it[1]*s , it[2]*s , it[3]*s , it[4]*s )
end

--[[#lua.wetgenes.tardis.v4.normalize

	v4 = v4:normalize()
	v4 = v4:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned
otherwise v4 is modified and returned.

]]
function v4.normalize(it,r)
	local l=v4.len(it)
	if l>0 then l=1/l end
	return v4.scale(it,l,r)
end

--[[#lua.wetgenes.tardis.v4.add

	v4 = v4:add(v4b)
	v4 = v4:add(v4b,r)

Add v4b to v4.

If r is provided then the result is written into r and returned
otherwise v4 is modified and returned.

]]
function v4.add(va,vb,r)
	r=r or va
	return v4.nset(r, va[1]+vb[1] , va[2]+vb[2] , va[3]+vb[3] , va[4]+vb[4] )
end

--[[#lua.wetgenes.tardis.v4.sub

	v4 = v4:sub(v4b)
	v4 = v4:sub(v4b,r)

Subtract v4b from v4.

If r is provided then the result is written into r and returned
otherwise v4 is modified and returned.

]]
function v4.sub(va,vb,r)
	r=r or va
	return v4.nset(r, va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] , va[4]-vb[4] )
end

--[[#lua.wetgenes.tardis.v4.mul

	v4 = v4:mul(v4b)
	v4 = v4:mul(v4b,r)

Multiply v4 by v4b.

If r is provided then the result is written into r and returned
otherwise v4 is modified and returned.

]]
function v4.mul(va,vb,r)
	r=r or va
	return v4.nset(r, (va[1]*vb[1]) , (va[2]*vb[2]) , (va[3]*vb[3]) , (va[4]*vb[4]) )
end

--[[#lua.wetgenes.tardis.v4.dot

	value = v4:dot(v4b)

Return the dot product of these two vectors.

]]
function v4.dot(va,vb)
	return ( (va[1]*vb[1]) + (va[2]*vb[2]) + (va[3]*vb[3]) + (va[4]*vb[4]) )
end


--[[#lua.wetgenes.tardis.q4

The metatable for a quaternion class, use the new function to actually create an object.

We also inherit all the functions from tardis.v4

]]
q4=tardis.class("q4",v4)

--[[#lua.wetgenes.tardis.q4.set

	q4 = tardis.q4.set(q4,{0,0,0,1})
	q4 = tardis.q4.set(q4,0,0,0,1)
	q4 = tardis.q4.set(q4,{"xyz",0,90,0})
	q4 = tardis.q4.set(q4,"xyz",0,90,0)
	q4 = tardis.q4.set(q4,"xyz",{0,90,0})

If the first item in the stream is not a string then this is just a normal
array.set style.

If first parameter of the stream is a string then initialise the quaternion
using a simple axis rotation notation Where the string is a list of axis. This
string is lower case letters. x y or z and then the following numbers are
amount of rotation to apply around that axis in degrees. You should provide as
many numbers as letters.

Essentially this gives you a way of initialising quaternion rotations in an
easily readable way.

]]
function q4.set(it,...)
	local data={...}
	local key

-- check for key or fall back to array.set

	if type(data[1])=="table" and type(data[1][1])=="string" then
		key=data[1][1]
	elseif type(data[1])=="string" then
		key=data[1]
	else
		return array.set(it,...)
	end

-- use the key to perform rotations

	array.set(it,0,0,0,1)

	local n=1
	local dorot=function(angle)
		local axis=key:sub(n,n)
		n=n+1
		if     axis=="x" then	it:rotate(angle,{1,0,0})
		elseif axis=="y" then	it:rotate(angle,{0,1,0})
		else					it:rotate(angle,{0,0,1})
		end
	end

	for i,v in ipairs(data) do
		if type(v)=="number" then
			dorot(v)
		elseif type(v)=="table" then
			for ii=1,#v do
				local vv=v[ii] -- allow one depth of tables
				if type(vv)=="number" then
					dorot(vv)
				end
			end
		end
	end

	return it
end

--[[#lua.wetgenes.tardis.q4.new

	q4 = tardis.q4.new()

Create a new q4 and optionally set it to the given values, q4 methods
usually return the input q4 for easy function chaining.

]]
function q4.new(...) return setmetatable({0,0,0,1},q4):set(...) end

--[[#lua.wetgenes.tardis.q4.identity

	q4 = q4:identity()

Set this q4 to its 0,0,0,1 identity

]]
function q4.identity(it) return q4.nset(it,0,0,0,1) end

--[[#lua.wetgenes.tardis.q4.reverse

	q4 = q4:reverse()
	q4 = q4:reverse(r)

Reverse the rotation of this quaternion.

If r is provided then the result is written into r and returned
otherwise q4 is modified and returned.

]]
function q4.reverse(it,r)
	r=r or it
	return q4.nset(r , -it[1] , -it[2] , -it[3] , it[4] )
end

--[[#lua.wetgenes.tardis.q4.mix

	q4 = q4:mix(q4b,s)
	q4 = q4:mix(q4b,s,r)

Lerp from q4 to q4b by s.

If r is provided then the result is written into r and returned
otherwise q4 is modified and returned.

]]
function q4.mix(qa,qb,sb,r)
	local sa=1-sb
	if qa:dot(qb) < 0 then sa=-sa end -- shortest fix
	r=r or qa
	q4.nset(r, qa[1]*sa+qb[1]*sb , qa[2]*sa+qb[2]*sb , qa[3]*sa+qb[3]*sb , qa[4]*sa+qb[4]*sb )
	return r:normalize()
end

--[[#lua.wetgenes.tardis.q4.setrot

	q4 = q4:setrot(degrees,v3a)

Set this quaternion to a rotation around the given normal by the given degrees.

]]
function q4.setrot(it,degrees,v3a)
	local ah=degrees * (math.pi/360) -- ah = half the angle in radians
	local sh=math.sin(ah)
	return q4.nset(it , v3a[1]*sh , v3a[2]*sh , v3a[3]*sh , math.cos(ah) )
end

--[[#lua.wetgenes.tardis.q4.setrrot

	q4 = q4:setrrot(radians,v3a)

Set this quaternion to a rotation around the given normal by the given radians.

]]
function q4.setrrot(it,radians,v3a)
	local ah=radians*0.5 -- ah = half the angle in radians
	local sh=math.sin(ah)
	return q4.nset(it , v3a[1]*sh , v3a[2]*sh , v3a[3]*sh , math.cos(ah) )
end

--[[#lua.wetgenes.tardis.q4.rotate

	q4 = q4:rotate(degrees,v3a)
	q4 = q4:rotate(degrees,v3a,r)

Apply a degree rotation to this quaternion.

If r is provided then the result is written into r and returned
otherwise q4 is modified and returned.

]]
function q4.rotate(it,degrees,v3a,r)
	local q4a=q4.new():setrot(degrees,v3a)
	return tardis.q4_product_q4(it,q4a,r)
end


--[[#lua.wetgenes.tardis.q4.prerotate

	q4 = q4:prerotate(degrees,v3a)
	q4 = q4:prerotate(degrees,v3a,r)

Pre apply a degree rotation to this quaternion.

If r is provided then the result is written into r and returned
otherwise q4 is modified and returned.

]]
function q4.prerotate(it,degrees,v3a,r)
	local q4a=q4.new():setrot(degrees,v3a)
	return tardis.q4_product_q4(q4a,it,r or it)
end


--[[#lua.wetgenes.tardis.q4.rrotate

	q4 = q4:rrotate(radians,v3a)
	q4 = q4:rrotate(radians,v3a,r)

Apply a radian rotation to this quaternion.

If r is provided then the result is written into r and returned
otherwise q4 is modified and returned.

]]
function q4.rrotate(it,radians,v3a,r)
	local q4a=q4.new():setrrot(radians,v3a)
	return tardis.q4_product_q4(it,q4a,r)
end


--[[#lua.wetgenes.tardis.q4.prerrotate

	q4 = q4:prerrotate(radians,v3a)
	q4 = q4:prerrotate(radians,v3a,r)

Pre apply a radian rotation to this quaternion.

If r is provided then the result is written into r and returned
otherwise q4 is modified and returned.

]]
function q4.prerrotate(it,radians,v3a,r)
	local q4a=q4.new():setrrot(radians,v3a)
	return tardis.q4_product_q4(q4a,it,r or it)
end


--[[#lua.wetgenes.tardis.q4.set_yaw_pitch_roll

	q4 = q4:set_yaw_pitch_roll(v3)
	q4 = q4:set_yaw_pitch_roll({90,60,30})	-- 30yaw 60pitch 90roll

Set a V3(roll,pitch,yaw) degree rotation into this quaternion

	yaw   v[3] is rotation about the z axis and is applied first
	pitch v[2] is rotation about the y axis and is applied second
	roll  v[1] is rotation about the z axis and is applied last

]]
function q4.set_yaw_pitch_roll(it,v)
	return q4.set_yaw_pitch_roll_in_radians(it,tardis.v3.new(v):scale(math.pi/180))
end

--[[#lua.wetgenes.tardis.q4.get_yaw_pitch_roll

	v3 = q4:get_yaw_pitch_roll()

Get a yaw,pitch,roll degree rotation from this quaternion

If r is provided then the result is written into r and returned
otherwise a new v3 is created and returned.

]]
function q4.get_yaw_pitch_roll(it,r)
	return q4.get_yaw_pitch_roll_in_radians(it,r):scale(180/math.pi)
end

--[[#lua.wetgenes.tardis.q4.set_yaw_pitch_roll_in_radians

	q4 = q4:set_yaw_pitch_roll_in_radians(v)

Set a V3(roll,pitch,yaw) radian rotation into this quaternion

	yaw   v[3] is rotation about the z axis and is applied first
	pitch v[2] is rotation about the y axis and is applied second
	roll  v[1] is rotation about the z axis and is applied last

]]
function q4.set_yaw_pitch_roll_in_radians(q,v)

	local cy = math.cos(v[3]*0.5)
	local sy = math.sin(v[3]*0.5)
	local cp = math.cos(v[2]*0.5)
	local sp = math.sin(v[2]*0.5)
	local cr = math.cos(v[1]*0.5)
	local sr = math.sin(v[1]*0.5)

	q[4] = cr * cp * cy + sr * sp * sy
	q[1] = sr * cp * cy - cr * sp * sy
	q[2] = cr * sp * cy + sr * cp * sy
	q[3] = cr * cp * sy - sr * sp * cy

	return q
end

--[[#lua.wetgenes.tardis.q4.get_yaw_pitch_roll_in_radians

	v3 = q4:get_yaw_pitch_roll_in_radians()

Get a yaw,pitch,roll degree rotation from this quaternion

If r is provided then the result is written into r and returned
otherwise a new v3 is created and returned.

]]
function q4.get_yaw_pitch_roll_in_radians(q,r)
	r=r or tardis.v3.new()

-- roll (x-axis rotation)
	local sinr_cosp =   2*(q[4] * q[1] + q[2] * q[3])
	local cosr_cosp = 1-2*(q[1] * q[1] + q[2] * q[3])
	r[1] = math.atan2(sinr_cosp, cosr_cosp)

-- pitch (y-axis rotation)
	local sinp = 2*(q[4] * q[2] - q[3] * q[1])
	if sinp >= 1 then -- use +90 degrees if out of range
		r[2]= math.pi/2
	elseif sinp <= -1  then -- use -90 degrees if out of range
		r[2]=-math.pi/2
	else
		r[2] = math.asin(sinp)
	end

-- yaw (z-axis rotation)
	local siny_cosp =   2*(q[4] * q[3] + q[1] * q[2])
	local cosy_cosp = 1-2*(q[2] * q[2] + q[3] * q[3])
	r[3] = math.atan2(siny_cosp, cosy_cosp)

	return r
end

--[[#lua.wetgenes.tardis.line

A 3d space line class.

[1]position , [2]normal

We also inherit all the functions from tardis.array

]]
local line=tardis.class("line",array)
line.set=nil -- disable

--[[#lua.wetgenes.tardis.line.new

	line = tardis.line.new(p,n)

Create a new line and optionally set it to the given values.

]]
function line.new(p,n) return setmetatable({p or v3.new(),n or v3.new()},line) end

--[[#lua.wetgenes.tardis.plane

A 3d space plane class.

[1]position , [2]normal

We also inherit all the functions from tardis.array

]]
local plane=tardis.class("plane",array)
plane.set=nil -- disable

--[[#lua.wetgenes.tardis.plane.new

	plane = tardis.plane.new(p,n)

Create a new plane and optionally set it to the given values.

]]
function plane.new(p,n) return setmetatable({p or v3.new(),n or v3.new()},plane) end



--[[#lua.wetgenes.tardis.step

	i = tardis.step(edge,num)

return 0 if num is bellow edge or 1 if num is the same or higher

]]
function tardis.step(edge,num)
	return (num<edge) and 0 or 1
end

--[[#lua.wetgenes.tardis.smoothstep

	f = tardis.step(edge1,edge2,num)

return 0 if num is bellow or equal to edge1. Return 1 if num is the same or
higher as edge2 and smoothly interpolate between 0 and 1 for all other values.

]]
function tardis.smoothstep(e1,e2,num)
	if num<=e1 then return 0 end
	if num>=e2 then return 1 end
	return (num-e1)/(e2-e1)
end



function tardis.line_intersect_plane(l,p,r)
	r=r or v3.new()
	local t=v3.new(p[1]):sub(l[1]) -- the line position relative to the plane
	local d=l[2]:dot(p[2]) -- the length of the line until it hits the plane
	if d~=0 then -- less errors please
		d=t:dot(p[2])/d
	end
	return v3.nset(r, l[1][1]+(l[2][1]*d) , l[1][2]+(l[2][2]*d) , l[1][3]+(l[2][3]*d) ) -- the point of intersection
end

function tardis.q4_to_m4(q,m)
	if not m then m=m4.new() end
	local x,y,z,w=q[1],q[2],q[3],q[4]
    local xx,xy,xz,xw=x*x,x*y,x*z,x*w
    local    yy,yz,yw=    y*y,y*z,y*w
    local       zz,zw=        z*z,z*w

	array.set(m,
					1 - 2 * ( yy + zz ),
						2 * ( xy + zw ),
						2 * ( xz - yw ),
						0,
						2 * ( xy - zw ),
					1 - 2 * ( xx + zz ),
						2 * ( yz + xw ),
						0,
						2 * ( xz + yw ),
						2 * ( yz - xw ),
					1 - 2 * ( xx + yy ),
						0,
						0,0,0,1)

	return m
end

function tardis.v4_product_v4(va,vb,r)
	r=r or va
	return v4.nset(r, va[1]*vb[1] , va[2]*vb[2] , va[3]*vb[3] , va[4]*vb[4] )
end

function tardis.v3_product_v3(va,vb,r)
	r=r or va
	return v3.nset(r, va[1]*vb[1] , va[2]*vb[2] , va[3]*vb[3] )
end

function tardis.v2_product_v2(va,vb,r)
	r=r or va
	return v2.nset(r, va[1]*vb[1] , va[2]*vb[2] )
end

function tardis.v1_product_v1(va,vb,r)
	r=r or va
	return v1.nset(r, va[1]*vb[1] )
end

function tardis.q4_product_q4(q4a,q4b,r)
	r=r or q4a
    local r1 =  q4b[1] * q4a[4] + q4b[2] * q4a[3] - q4b[3] * q4a[2] + q4b[4] * q4a[1]
    local r2 = -q4b[1] * q4a[3] + q4b[2] * q4a[4] + q4b[3] * q4a[1] + q4b[4] * q4a[2]
    local r3 =  q4b[1] * q4a[2] - q4b[2] * q4a[1] + q4b[3] * q4a[4] + q4b[4] * q4a[3]
    local r4 = -q4b[1] * q4a[1] - q4b[2] * q4a[2] - q4b[3] * q4a[3] + q4b[4] * q4a[4]
	return q4.nset(r,r1,r2,r3,r4)
end

function tardis.v3_product_q4(v,q,r)
	r=r or v

	local rx  = q[1] + q[1]
	local ry  = q[2] + q[2]
	local rz  = q[3] + q[3]
	local rwx = q[4] * rx
	local rwy = q[4] * ry
	local rwz = q[4] * rz
	local rxx = q[1] * rx
	local rxy = q[1] * ry
	local rxz = q[1] * rz
	local ryy = q[2] * ry
	local ryz = q[2] * rz
	local rzz = q[3] * rz
	local r1  = ((v[1] * ((1 - ryy) - rzz)) + (v[2] * (rxy - rwz))) + (v[3] * (rxz + rwy))
	local r2  = ((v[1] * (rxy + rwz)) + (v[2] * ((1 - rxx) - rzz))) + (v[3] * (ryz - rwx))
	local r3  = ((v[1] * (rxz - rwy)) + (v[2] * (ryz + rwx))) + (v[3] * ((1 - rxx) - ryy))

	return v3.nset(r,r1,r2,r3)
end

function tardis.v3_product_m4(v3,m4,r)
	r=r or v3
	local oow=1/( (m4[   4]*v3[1]) + (m4[ 4+4]*v3[2]) + (m4[ 8+4]*v3[3]) + (m4[12+4] ) )
	local r1= oow * ( (m4[   1]*v3[1]) + (m4[ 4+1]*v3[2]) + (m4[ 8+1]*v3[3]) + (m4[12+1] ) )
	local r2= oow * ( (m4[   2]*v3[1]) + (m4[ 4+2]*v3[2]) + (m4[ 8+2]*v3[3]) + (m4[12+2] ) )
	local r3= oow * ( (m4[   3]*v3[1]) + (m4[ 4+3]*v3[2]) + (m4[ 8+3]*v3[3]) + (m4[12+3] ) )
	return v3.nset(r,r1,r2,r3)
end

function tardis.v4_product_m4(v4,m4,r)
	r=r or v4
	local r1= ( (m4[   1]*v4[1]) + (m4[ 4+1]*v4[2]) + (m4[ 8+1]*v4[3]) + (m4[12+1]*v4[4]) )
	local r2= ( (m4[   2]*v4[1]) + (m4[ 4+2]*v4[2]) + (m4[ 8+2]*v4[3]) + (m4[12+2]*v4[4]) )
	local r3= ( (m4[   3]*v4[1]) + (m4[ 4+3]*v4[2]) + (m4[ 8+3]*v4[3]) + (m4[12+3]*v4[4]) )
	local r4= ( (m4[   4]*v4[1]) + (m4[ 4+4]*v4[2]) + (m4[ 8+4]*v4[3]) + (m4[12+4]*v4[4]) )
	return v4.nset(r,r1,r2,r3,r4)
end

function tardis.m4_product_m4(m4a,m4b,r)
	r=r or m4a
	local r1 = (m4b[   1]*m4a[   1]) + (m4b[   2]*m4a[ 4+1]) + (m4b[   3]*m4a[ 8+1]) + (m4b[   4]*m4a[12+1])
	local r2 = (m4b[   1]*m4a[   2]) + (m4b[   2]*m4a[ 4+2]) + (m4b[   3]*m4a[ 8+2]) + (m4b[   4]*m4a[12+2])
	local r3 = (m4b[   1]*m4a[   3]) + (m4b[   2]*m4a[ 4+3]) + (m4b[   3]*m4a[ 8+3]) + (m4b[   4]*m4a[12+3])
	local r4 = (m4b[   1]*m4a[   4]) + (m4b[   2]*m4a[ 4+4]) + (m4b[   3]*m4a[ 8+4]) + (m4b[   4]*m4a[12+4])
	local r5 = (m4b[ 4+1]*m4a[   1]) + (m4b[ 4+2]*m4a[ 4+1]) + (m4b[ 4+3]*m4a[ 8+1]) + (m4b[ 4+4]*m4a[12+1])
	local r6 = (m4b[ 4+1]*m4a[   2]) + (m4b[ 4+2]*m4a[ 4+2]) + (m4b[ 4+3]*m4a[ 8+2]) + (m4b[ 4+4]*m4a[12+2])
	local r7 = (m4b[ 4+1]*m4a[   3]) + (m4b[ 4+2]*m4a[ 4+3]) + (m4b[ 4+3]*m4a[ 8+3]) + (m4b[ 4+4]*m4a[12+3])
	local r8 = (m4b[ 4+1]*m4a[   4]) + (m4b[ 4+2]*m4a[ 4+4]) + (m4b[ 4+3]*m4a[ 8+4]) + (m4b[ 4+4]*m4a[12+4])
	local r9 = (m4b[ 8+1]*m4a[   1]) + (m4b[ 8+2]*m4a[ 4+1]) + (m4b[ 8+3]*m4a[ 8+1]) + (m4b[ 8+4]*m4a[12+1])
	local r10= (m4b[ 8+1]*m4a[   2]) + (m4b[ 8+2]*m4a[ 4+2]) + (m4b[ 8+3]*m4a[ 8+2]) + (m4b[ 8+4]*m4a[12+2])
	local r11= (m4b[ 8+1]*m4a[   3]) + (m4b[ 8+2]*m4a[ 4+3]) + (m4b[ 8+3]*m4a[ 8+3]) + (m4b[ 8+4]*m4a[12+3])
	local r12= (m4b[ 8+1]*m4a[   4]) + (m4b[ 8+2]*m4a[ 4+4]) + (m4b[ 8+3]*m4a[ 8+4]) + (m4b[ 8+4]*m4a[12+4])
	local r13= (m4b[12+1]*m4a[   1]) + (m4b[12+2]*m4a[ 4+1]) + (m4b[12+3]*m4a[ 8+1]) + (m4b[12+4]*m4a[12+1])
	local r14= (m4b[12+1]*m4a[   2]) + (m4b[12+2]*m4a[ 4+2]) + (m4b[12+3]*m4a[ 8+2]) + (m4b[12+4]*m4a[12+2])
	local r15= (m4b[12+1]*m4a[   3]) + (m4b[12+2]*m4a[ 4+3]) + (m4b[12+3]*m4a[ 8+3]) + (m4b[12+4]*m4a[12+3])
	local r16= (m4b[12+1]*m4a[   4]) + (m4b[12+2]*m4a[ 4+4]) + (m4b[12+3]*m4a[ 8+4]) + (m4b[12+4]*m4a[12+4])
	return m4.nset(r,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16)
end


--
-- build a simple field of view GL projection matrix designed to work in 2d or 3d and keep the numbers
-- easy for 2d positioning.
--
-- setting aspect to 640,480 and fov of 1 would mean at a z depth of 240 (which is y/2) then your view area would be
-- -320 to +320 in the x and -240 to +240 in the y.
--
-- fov is a tan like value (a view size inverse scalar) so 1 would be 90deg, 0.5 would be 45deg and so on
--
-- the depth parameter is only used to limit the range of the zbuffer so it covers 0 to depth
--
-- The following would be a reasonable default for an assumed 640x480 display.
--
-- m4_project23d(w,h,640,480,0.5,1024)
--
-- then at z=480 we would have one to one pixel scale...
-- the total view area volume from there would be -320 +320 , -240 +240 , -480 +(1024-480)
--
-- view_width and view_height must be the current width and height of the display in pixels or nil
-- we use this to work out where to place our view such that it is always visible and keeps its aspect.
--
function tardis.m4_project23d(view_width,view_height,width,height,fov,depth)

	local aspect=height/width

	local m=m4.new()

	local f=depth
	local n=1

	local win_aspect=((view_height or height)/(view_width or width))

	if (win_aspect > (aspect) ) 	then 	-- fit width to screen

		m[1] = ((aspect)*1)/fov
		m[6] = -((aspect)/win_aspect)/fov

	else									-- fit height to screen

		m[1] = win_aspect/fov
		m[6] = -1/fov

	end


	m[11] = -(f+n)/(f-n)
	m[12] = -1

	m[15] = -2*f*n/(f-n)

	return m
end


--[[#lua.wetgenes.tardis.m4_stack

	stack = tardis.m4_stack()

create an m4 stack that is very similar to an old school opengl transform
stack.

]]
function tardis.m4_stack()
	local stack={}

	stack[1]=tardis.m4.new():identity()

	stack.save=function()
		return tardis.m4.new(stack[#stack])
	end

	stack.load=function(...)
		stack[#stack]:set(...)
		return stack
	end

	stack.product=function(a)
		stack[#stack]:product(a,stack[#stack])
		return stack
	end

	stack.premult=function(a)
		a:product(stack[#stack],stack[#stack])
		return stack
	end

	stack.identity=function()
		stack[#stack]:identity()
		return stack
	end

	stack.translate=function(...)
		stack[#stack]:translate(...)
		return stack
	end

	stack.pretranslate=function(...)
		stack[#stack]:pretranslate(...)
		return stack
	end

	stack.rotate=function(...)
		stack[#stack]:rotate(...)
		return stack
	end

	stack.rrotate=function(...)
		stack[#stack]:rrotate(...)
		return stack
	end

	stack.prerotate=function(...)
		stack[#stack]:prerotate(...)
		return stack
	end

	stack.scale=function(...)
		stack[#stack]:scale(...)
		return stack
	end

	stack.push=function()
		local m4=tardis.m4.new(stack[#stack])
		stack[#stack+1]=m4 -- new topmost
		return stack
	end

	stack.pop=function()
		stack[#stack]=nil -- remove topmost
		return stack
	end

	return stack
end



if false then -- this is not faster than luajit :)

tardis.f32=require("wetgenes.tardis.core") -- use a "faster?" f32 C core

if not DISABLE_WETGENES_TARDIS_CORE then -- set this global to true before first use to disable use of tardis f32 core
--upgrade the above to hopefully faster C versions working on 16byte aligned userdata arrays of floats

	local tcore=tardis.f32

	-- allow read/write with magical [] lookups
	function array.__len(it) return 1 end
	function array.__index(it,n) return array[n] or tcore.read(it,n) end
	function array.__newindex(it,n,v) tcore.write(it,n,v) end

	function m2.new(...) return tcore.alloc(4* 4,m2):identity():set(...) end
	function m3.new(...) return tcore.alloc(4* 9,m3):identity():set(...) end
	function m4.new(...) return tcore.alloc(4*16,m4):identity():set(...) end

	function m2.__len(it) return 4 end
	function m3.__len(it) return 9 end
	function m4.__len(it) return 16 end

	function m2.__index(it,n) return m2[n] or tcore.read(it,n) end
	function m3.__index(it,n) return m3[n] or tcore.read(it,n) end
	function m4.__index(it,n) return m4[n] or tcore.read(it,n) end

	m2.__newindex=array.__newindex
	m3.__newindex=array.__newindex
	m4.__newindex=array.__newindex

	function v2.new(...) return tcore.alloc(4* 2,v2):set(...) end
	function v3.new(...) return tcore.alloc(4* 3,v3):set(...) end
	function v4.new(...) return tcore.alloc(4* 4,v4):set(...) end
	function q4.new(...) return tcore.alloc(4* 4,q4):set(...) end

	function v2.__len(it) return 2 end
	function v3.__len(it) return 3 end
	function v4.__len(it) return 4 end
	function q4.__len(it) return 4 end

	function v2.__index(it,n) return v2[n] or tcore.read(it,n) end
	function v3.__index(it,n) return v3[n] or tcore.read(it,n) end
	function v4.__index(it,n) return v4[n] or tcore.read(it,n) end
	function q4.__index(it,n) return q4[n] or tcore.read(it,n) end

	v2.__newindex=array.__newindex
	v3.__newindex=array.__newindex
	v4.__newindex=array.__newindex
	q4.__newindex=array.__newindex

	-- replace some functions with C code

	tardis.m4_product_m4		=	tcore.m4_product_m4
	tardis.v4_product_m4		=	tcore.v4_product_m4

	m4.identity			=	tcore.m4_identity
	m4.arotate			=	tcore.m4_rotate
	m4.scale_v3			=	tcore.m4_scale_v3
	m4.translate		=	tcore.m4_translate

end

end

m2.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=function(a,b,r) return a:scalar(b[1],r) end,
}
m3.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=function(a,b,r) return a:scalar(b[1],r) end,
}
m4.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=function(a,b,r) return a:scalar(b[1],r) end,
	m4=tardis.m4_product_m4,
	m3=function(a,b,r) return tardis.m4_product_m4(a,b:m4(),r) end,
}
v1.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=tardis.v1_product_v1,
}
v2.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=function(a,b,r) return a:scalar(b[1],r) end,
	v2=tardis.v2_product_v2,
}
v3.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=function(a,b,r) return a:scalar(b[1],r) end,
	v3=tardis.v3_product_v3,
	q4=tardis.v3_product_q4,
	m3=function(a,b,r) return tardis.v3_product_m4(a,b:m4(),r) end,
	m4=tardis.v3_product_m4,
}
v4.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=function(a,b,r) return a:scalar(b[1],r) end,
	v4=tardis.v4_product_v4,
	m3=function(a,b,r) return tardis.v4_product_m4(a,b:m4(),r) end,
	m4=tardis.v4_product_m4,
}
q4.product_map={
-- the product of this with a...
	number=function(a,b,r) return a:scalar(b,r) end,
	v1=function(a,b,r) return a:scalar(b[1],r) end,
	q4=tardis.q4_product_q4,
}
line.product_map={
-- the product of this with a...
}
plane.product_map={
-- the product of this with a...
}

tardis.V0=tardis.array.new

tardis.V1=tardis.v1.new
tardis.V2=tardis.v2.new
tardis.V3=tardis.v3.new
tardis.V4=tardis.v4.new

tardis.M2=tardis.m2.new
tardis.M3=tardis.m3.new
tardis.M4=tardis.m4.new

tardis.Q4=tardis.q4.new

tardis.LINE=tardis.line.new
tardis.PLANE=tardis.plane.new


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

This tardis is a lua library for manipulating time and space with 
numbers. Designed to work as pure lua but with a faster, but less 
accurate, f32 core by default.

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

The Lua code is normally replaced with a hopefully faster f32 based C 
version, Use DISABLE_WETGENES_TARDIS_CORE before requiring this file to 
turn it off and get a pure lua library.

This seems to be the simplest (programmer orientated) description of 
most of the maths used here so go read it if you want to know what the 
funny words mean.

http://www.j3d.org/matrix_faq/matrfaq_latest.html

]]

--module
local tardis={ modname=(...) } ; package.loaded[tardis.modname]=tardis

--[[#lua.wetgenes.tardis.type

	name=tardis.type(object)

This will return the type of an object previously registered with class

]]
function tardis.type(it) return it.__type or type(it) end

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

--[[#lua.wetgenes.tardis.array.__tostring

	string = array.__tostring(it)

Convert an array to a string this is called by the lua tostring() function,

]]
function array.__tostring(it) -- these classes are all just 1d arrays of numbers
	local t={}
	t[#t+1]=tardis.type(it)
	t[#t+1]="={"
	for i=1,#it do
		t[#t+1]=tostring(it[i])
		if i~=#it then t[#t+1]=", " end
	end
	t[#t+1]="}"
	return table.concat(t)
end

--[[#lua.wetgenes.tardis.array.set

	a=a:set(1,2,3,4)
	a=a:set({1,2,3,4})
	a=a:set({1,2},{3,4})

Assign some numbers to an array, all the above examples will assign 
1,2,3,4 to the first four slots in the given array, as you can see we 
allow one level of tables. Any class that is based on this array 
class can be used instead of an explicit table. So we can use a v2 or v3 or m4 etc etc.

if more numbers are given than the size of the array then they will be 
ignored.

]]
function array.set(it,...)
	local n=1
	for i,v in ipairs{...} do
		if not it[n] then return it end -- got all the data we need (#it)
		if type(v)=="number" then
			it[n]=v
			n=n+1
		else
			for ii=1,#v do local vv=v[ii] -- allow one depth of tables
				it[n]=vv
				n=n+1
			end
		end
	end
	return it
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
	local mta=tardis.type(a)
	local mtb=tardis.type(b)

	if     mta=="v3" and mtb=="m4" then return tardis.v3_product_m4(a,b,r)
	elseif mta=="v3" and mtb=="q4" then return tardis.v3_product_q4(a,b,r)
	elseif mta=="v4" and mtb=="m4" then return tardis.v4_product_m4(a,b,r)
	elseif mta=="m4" and mtb=="m4" then return tardis.m4_product_m4(a,b,r)
	elseif mta=="q4" and mtb=="q4" then return tardis.q4_product_q4(a,b,r)
	end

	error("tardis : "..mta.." product "..mtb.." not supported",2)
end


--[[#lua.wetgenes.tardis.m2

The metatable for a 2x2 matrix class, use the new function to actually create an object.

We also inherit all the functions from tardis.array

]]
local m2=tardis.class("m2",array)

--[[#lua.wetgenes.tardis.m2.new

	m2 = tardis.m2.new()

Create a new m2 and optionally set it to the given values, m2 methods 
usually return the input m2 for easy function chaining.

]]
function m2.new(...) return setmetatable({0,0,0,0},m2):set(...) end

--[[#lua.wetgenes.tardis.m2.identity

	m2 = m2:identity()

Set this m2 to the identity matrix.

]]
function m2.identity(it) return it:set(1,0, 0,1) end 

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
	return	 r:set(it[1],it[2+1], it[2],it[2+2])
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
	return r:set(it[1]*s,it[2]*s, it[2+1]*s,it[2+2]*s)
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
local m3=tardis.class("m3",array)

--[[#lua.wetgenes.tardis.m3.new

	m3 = tardis.m3.new()

Create a new m3 and optionally set it to the given values, m3 methods 
usually return the input m3 for easy function chaining.

]]
function m3.new(...) return setmetatable({0,0,0,0,0,0,0,0,0},m3):set(...) end

--[[#lua.wetgenes.tardis.m3.identity

	m3 = m3:identity()

Set this m3 to the identity matrix.

]]
function m3.identity(it) return it:set(1,0,0, 0,1,0, 0,0,1) end 

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
	return	 r:set(it[1],it[3+1],it[6+1], it[2],it[3+2],it[6+2], it[3],it[3+3],it[6+3])
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
	return r:set(it[1]*s,it[2]*s,it[3]*s, it[3+1]*s,it[3+2]*s,it[3+3]*s, it[6+1]*s,it[6+2]*s,it[6+3]*s)
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
	return r:set(t)
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
local m4=tardis.class("m4",array)

--[[#lua.wetgenes.tardis.m4.new

	m4 = tardis.m4.new()

Create a new m4 and optionally set it to the given values, m4 methods 
usually return the input m4 for easy function chaining.

]]
function m4.new(...) return setmetatable({0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},m4):set(...) end

--[[#lua.wetgenes.tardis.m4.identity

	m4 = m4:identity()

Set this m4 to the identity matrix.

]]
function m4.identity(it) return it:set(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1) end 

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
	return	 r:set(it[1],it[4+1],it[8+1],it[12+1], it[2],it[4+2],it[8+2],it[12+2], it[3],it[4+3],it[8+3],it[12+3], it[4],it[4+4],it[8+4],it[12+4])
end

--[[#lua.wetgenes.tardis.m4.scale

	m4 = m4:scale(s)
	m4 = m4:scale(s,r)

Scale this m4 by s.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.

]]
function m4.scale(it,s,r)
	r=r or it
	return r:set(
		it[ 1]*s,it[ 2]*s,it[ 3]*s,it[ 4]*s,
		it[ 5]*s,it[ 6]*s,it[ 7]*s,it[ 8]*s,
		it[ 9]*s,it[10]*s,it[11]*s,it[12]*s,
		it[13]*s,it[14]*s,it[15]*s,it[16]*s)
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
	return r:set(
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
	return r:set(
		it[ 1]-m[ 1],it[ 2]-m[ 2],it[ 3]-m[ 3],it[ 4]-m[ 4],
		it[ 5]-m[ 5],it[ 6]-m[ 6],it[ 7]-m[ 7],it[ 8]-m[ 8],
		it[ 9]-m[ 9],it[10]-m[10],it[11]-m[11],it[12]-m[12],
		it[13]-m[13],it[14]-m[14],it[15]-m[15],it[16]-m[16])
end

--[[#lua.wetgenes.tardis.m4.lerp

	m4 = m4:lerp(m4b,s)
	m4 = m4:lerp(m4b,s,r)

Lerp from m4 to m4b by s.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.

]]
function m4.lerp(it,m,s,r)
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
	return r:set(t)
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
	return m4.scale(m4.cofactor(m4.transpose(it,m4.new())),ood,r)
end

--[[#lua.wetgenes.tardis.m4.translate

	m4 = m4:translate(x,y,z)
	m4 = m4:translate(x,y,z,r)
	m4 = m4:translate(v3)
	m4 = m4:translate(v3,r)

Translate this m4 along its local axis by {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.

]]
function m4.translate(it,a,b,c,d)
	local v3a,r
	if type(a)=="number" then v3a=tardis.v3.new(a,b,c) r=d else v3a=a r=b end
	r=r or it
	local r1=it[12+1]+v3a[1]*it[1]+v3a[2]*it[5]+v3a[3]*it[9]
	local r2=it[12+2]+v3a[1]*it[2]+v3a[2]*it[6]+v3a[3]*it[10]
	local r3=it[12+3]+v3a[1]*it[3]+v3a[2]*it[7]+v3a[3]*it[11]
	local r4=it[12+4]+v3a[1]*it[4]+v3a[2]*it[8]+v3a[3]*it[12]
	return r:set(it[1],it[2],it[3],it[4], it[5],it[6],it[7],it[8], it[9],it[10],it[11],it[12], r1,r2,r3,r4 )
end

--[[#lua.wetgenes.tardis.m4.scale_v3

	m4 = m4:scale_v3(x,y,z)
	m4 = m4:scale_v3(x,y,z,r)
	m4 = m4:scale_v3(v3)
	m4 = m4:scale_v3(v3,r)

Scale this m4 by {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.

]]
function m4.scale_v3(it,a,b,c,d)
	local v3a,r
	if type(a)~="number" then v3a=tardis.v3.new(a,b,c) r=d else v3a=a r=b end
	r=r or it
	local s1=v3a[1]
	local s2=v3a[2]
	local s3=v3a[3]
	return r:set(	s1*it[1],	s1*it[2],	s1*it[3],	s1*it[4],
					s2*it[5],	s2*it[6],	s2*it[7],	s2*it[8],
					s3*it[9],	s3*it[10],	s3*it[11],	s3*it[12],
					it[13],		it[14],		it[15],		it[16] )
end

--[[#lua.wetgenes.tardis.m4.scale_v3

	v3 = m4:scale_v3(x,y,z)
	v3 = m4:scale_v3(x,y,z,r)
	v3 = m4:scale_v3(v3)
	v3 = m4:scale_v3(v3,r)

Get v3 scale from a scale/rot/trans matrix

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.

]]
function m4.get_scale_v3(it,r)
	r=r or tardis.v3.new()
	return r:set(
		math.sqrt(it[1]*it[1]+it[5]*it[5]+it[ 9]*it[ 9]),
		math.sqrt(it[2]*it[2]+it[6]*it[6]+it[10]*it[10]),
		math.sqrt(it[3]*it[3]+it[7]*it[7]+it[11]*it[11])
	)
end

--[[#lua.wetgenes.tardis.m4.setrot

	m4 = m4:setrot(degrees,v3a)

Set this matrix to a rotation matrix around the given normal.

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

	return it:set(
		x*x*cc+c	,	x*y*cc-z*s	,	x*z*cc+y*s	, 	0	,
		x*y*cc+z*s	,	y*y*cc+c	,	y*z*cc-x*s	,	0	,
        x*z*cc-y*s	,	y*z*cc+x*s	,	z*z*cc+c	,	0	,
        0			,	0			,	0			,	1	)

end

--[[#lua.wetgenes.tardis.m4.rotate

	m4 = m4:rotate(degrees,v3a)
	m4 = m4:rotate(degrees,v3a,r)

Apply a rotation to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.

]]
function m4.rotate(it,degrees,v3a,r)
	local m4a=m4.new():setrot(degrees,v3a)
	return tardis.m4_product_m4(it,m4a,r)
end

--[[#lua.wetgenes.tardis.v2

The metatable for a 2d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array

]]
local v2=tardis.class("v2",array)

--[[#lua.wetgenes.tardis.v2.new

	v2 = tardis.v2.new()

Create a new v2 and optionally set it to the given values, v2 methods 
usually return the input v2 for easy function chaining.

]]
function v2.new(...) return setmetatable({0,0},v2):set(...) end

--[[#lua.wetgenes.tardis.v2.identity

	v2 = v2:identity()

Set this v2 to all zeros.

]]
function v2.identity(it) return it:set(0,0) end 

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

--[[#lua.wetgenes.tardis.v2.oo

	v2 = v2:oo()
	v2 = v2:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.

]]
function v2.oo(it,r)
	r=r or it
	return r:set( 1/it[1] , 1/it[2] )
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
	return r:set( it[1]*s , it[2]*s )
end

--[[#lua.wetgenes.tardis.v2.normalize

	v2 = v2:normalize()
	v2 = v2:normalize(r)

Adjust the length of this vector to 1.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.

]]
function v2.normalize(it,r)
	return v2.scale(it,1/v2.len(it),r)
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
	return r:set( va[1]+vb[1] , va[2]+vb[2] )
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
	return r:set( va[1]-vb[1] , va[2]-vb[2] )
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
	return r:set( (va[1]*vb[1]) , (va[2]*vb[2]) )
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
local v3=tardis.class("v3",array)

--[[#lua.wetgenes.tardis.v3.new

	v3 = tardis.v3.new()

Create a new v3 and optionally set it to the given values, v3 methods 
usually return the input v3 for easy function chaining.

]]
function v3.new(...) return setmetatable({0,0,0},v3):set(...) end

--[[#lua.wetgenes.tardis.v3.identity

	v3 = v3:identity()

Set this v3 to all zeros.

]]
function v3.identity(it) return it:set(0,0,0) end 

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

--[[#lua.wetgenes.tardis.v3.oo

	v3 = v3:oo()
	v3 = v3:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.

]]
function v3.oo(it,r)
	r=r or it
	return r:set( 1/it[1] , 1/it[2] , 1/it[3] )
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
	return r:set( it[1]*s , it[2]*s , it[3]*s )
end

--[[#lua.wetgenes.tardis.v3.normalize

	v3 = v3:normalize()
	v3 = v3:normalize(r)

Adjust the length of this vector to 1.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.

]]
function v3.normalize(it,r)
	return v3.scale(it,1/v3.len(it),r)
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
	return r:set( va[1]+vb[1] , va[2]+vb[2] , va[3]+vb[3] )
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
	return r:set( va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] )
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
	return r:set( (va[1]*vb[1]) , (va[2]*vb[2]) , (va[3]*vb[3]) )
end

--[[#lua.wetgenes.tardis.v3.dot

	value = v3:dot(v3b)

Return the dot product of these two vectors.

]]
function v3.dot(va,vb)
	return ( (va[1]*vb[1]) + (va[2]*vb[2]) + (va[3]*vb[3]) )
end

--[[#lua.wetgenes.tardis.v3.cross

	v2 = v2:dot(v2b)
	v2 = v2:dot(v2b,r)

Return the cross product of these two vectors.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.

]]
function v3.cross(va,vb,r)
	r=r or va
	return r:set( (va[2]*vb[3])-(va[3]*vb[2]) , (va[3]*vb[1])-(va[1]*vb[3]) , (va[1]*vb[2])-(va[2]*vb[1]) )
end


--[[#lua.wetgenes.tardis.v4

The metatable for a 4d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array

]]
local v4=tardis.class("v4",array)

--[[#lua.wetgenes.tardis.v4.new

	v4 = tardis.v4.new()

Create a new v4 and optionally set it to the given values, v4 methods 
usually return the input v4 for easy function chaining.

]]
function v4.new(...) return setmetatable({0,0,0,0},v4):set(...) end

--[[#lua.wetgenes.tardis.v4.identity

	v4 = v4:identity()

Set this v4 to all zeros.

]]
function v4.identity(it) return it:set(0,0,0,0) end

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
	return r:set( it[1]*oow , it[2]*oow , it[3]*oow )
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

--[[#lua.wetgenes.tardis.v4.oo

	v4 = v4:oo()
	v4 = v4:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.

]]
function v4.oo(it,r)
	r=r or it
	return r:set( 1/it[1] , 1/it[2] , 1/it[3] , 1/it[4] )
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
	return r:set( it[1]*s , it[2]*s , it[3]*s , it[4]*s )
end

--[[#lua.wetgenes.tardis.v4.normalize

	v4 = v4:normalize()
	v4 = v4:normalize(r)

Adjust the length of this vector to 1.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.

]]
function v4.normalize(it,r)
	return v4.scale(it,1/v4.len(it),r)
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
	return r:set( va[1]+vb[1] , va[2]+vb[2] , va[3]+vb[3] , va[4]+vb[4] )
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
	return r:set( va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] , va[4]-vb[4] )
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
	return r:set( (va[1]*vb[1]) , (va[2]*vb[2]) , (va[3]*vb[3]) , (va[4]*vb[4]) )
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
local q4=tardis.class("q4",v4)

--[[#lua.wetgenes.tardis.q4.new

	q4 = tardis.q4.new()

Create a new q4 and optionally set it to the given values, q4 methods 
usually return the input q4 for easy function chaining.

]]
function q4.new(...) return setmetatable({0,0,0,0},q4):set(...) end

--[[#lua.wetgenes.tardis.q4.identity

	q4 = q4:identity()

Set this q4 to its 0,0,0,1 identity

]]
function q4.identity(it) return it:set(0,0,0,1) end 

--[[#lua.wetgenes.tardis.q4.lerp

	q4 = q4:lerp(q4b,s)
	q4 = q4:lerp(q4b,s,r)

Nlerp from q4 to q4b by s.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.

]]
function q4.nlerp(qa,qb,sa,r)
	local sb=1-sa
	if qa.dot(qb) < 0 then sa=-sa end -- shortest fix
	r=r or va
	r:set( va[1]*sa+vb[1]*sb , va[2]*sa+vb[2]*sb , va[3]*sa+vb[3]*sb , va[4]*sa+vb[4]*sb )
	return r:normalize()
end

--[[#lua.wetgenes.tardis.q4.setrot

	q4 = q4:setrot(degrees,v3a)

Set this matrix to a rotation matrix around the given normal.

]]
function q4.setrot(it,degrees,v3a)
	local ah=degrees * (math.PI/360)
	local sh=math.sin(ah)
	return it:set( math.cos(ah) , v3a[1]*sh , v3a[2]*sh , v3a[3]*sh )
end

--[[#lua.wetgenes.tardis.q4.rotate

	q4 = q4:rotate(degrees,v3a)
	q4 = q4:rotate(degrees,v3a,r)

Apply a rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.

]]
function q4.rotate(it,degrees,v3a,r)
	local q4a=q4.new():setrot(degrees,v3a)
	return tardis.q4_product_q4(it,q4a,r)
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


function tardis.line_intersect_plane(l,p,r)
	r=r or v3.new()
	local t=v3.new(p[1]):sub(l[1]) -- the line position relative to the plane
	local d=l[2]:dot(p[2]) -- the length of the line until it hits the plane
	if d~=0 then -- less errors please
		d=t:dot(p[2])/d
	end
	return r:set( l[1][1]+(l[2][1]*d) , l[1][2]+(l[2][2]*d) , l[1][3]+(l[2][3]*d) ) -- the point of intersection
end

function tardis.q4_to_m4(q,m)
	if not m then m=m4.new() end
	local w,x,y,z=q[1],q[2],q[3],q[4]
    local xx,xy,xz,xw=x*x,x*y,x*z,x*w
    local    yy,yz,yw=    y*y,y*z,y*w
    local       zz,zw=        z*z,z*w

	return m:set(
					1 - 2 * ( yy + zz ),
						2 * ( xy - zw ),
						2 * ( xz + yw ),0,
						2 * ( xy + zw ),
					1 - 2 * ( xx + zz ),
						2 * ( yz - xw ),0,
						2 * ( xz - yw ),
						2 * ( yz + xw ),
					1 - 2 * ( xx + yy ),0,
						0,0,0,1				)
end

function tardis.q4_product_q4(q4a,q4b,r)
	r=r or q4a
    local r1 =  q4b[1] * q4a[4] + q4b[2] * q4a[3] - q4b[3] * q4a[2] + q4b[4] * q4a[1];
    local r2 = -q4b[1] * q4a[3] + q4b[2] * q4a[4] + q4b[3] * q4a[1] + q4b[4] * q4a[2];
    local r3 =  q4b[1] * q4a[2] - q4b[2] * q4a[1] + q4b[3] * q4a[4] + q4b[4] * q4a[3];
    local r4 = -q4b[1] * q4a[1] - q4b[2] * q4a[2] - q4b[3] * q4a[3] + q4b[4] * q4a[4];
	return r:set(r1,r2,r3,r4)
end

function tardis.v3_product_q4(v3,q4,r)
	r=r or v3
    local r1 =                  q4[2] * v3[3] - q4[3] * v3[2] + q4[4] * v3[1];
    local r2 = -q4[1] * v3[3]                 + q4[3] * v3[1] + q4[4] * v3[2];
    local r3 =  q4[1] * v3[2] - q4[2] * v3[1]                 + q4[4] * v3[3];
	return r:set(r1,r2,r3)
end

function tardis.v3_product_m4(v3,m4,r)
	r=r or v3
	local oow=1/( (m4[   4]*v3[1]) + (m4[ 4+4]*v3[2]) + (m4[ 8+4]*v3[3]) + (m4[12+4] ) )
	local r1= oow * ( (m4[   1]*v3[1]) + (m4[ 4+1]*v3[2]) + (m4[ 8+1]*v3[3]) + (m4[12+1] ) )
	local r2= oow * ( (m4[   2]*v3[1]) + (m4[ 4+2]*v3[2]) + (m4[ 8+2]*v3[3]) + (m4[12+2] ) )
	local r3= oow * ( (m4[   3]*v3[1]) + (m4[ 4+3]*v3[2]) + (m4[ 8+3]*v3[3]) + (m4[12+3] ) )
	return r:set(r1,r2,r3)
end

function tardis.v4_product_m4(v4,m4,r)
	r=r or v4
	local r1= ( (m4[   1]*v4[1]) + (m4[ 4+1]*v4[2]) + (m4[ 8+1]*v4[3]) + (m4[12+1]*v4[4]) )
	local r2= ( (m4[   2]*v4[1]) + (m4[ 4+2]*v4[2]) + (m4[ 8+2]*v4[3]) + (m4[12+2]*v4[4]) )
	local r3= ( (m4[   3]*v4[1]) + (m4[ 4+3]*v4[2]) + (m4[ 8+3]*v4[3]) + (m4[12+3]*v4[4]) )
	local r4= ( (m4[   4]*v4[1]) + (m4[ 4+4]*v4[2]) + (m4[ 8+4]*v4[3]) + (m4[12+4]*v4[4]) )
	return r:set(r1,r2,r3,r4)
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
	return r:set(r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16)
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


tardis.f32=require("wetgenes.tardis.core") -- use a "faster?" f32 C core

if not DISABLE_WETGENES_TARDIS_CORE then -- set this global to true before first use to disable use of tardis f32 core
--upgrade the above to hopefully faster C versions working on 16byte aligned userdata arrays of floats

	local tcore=tardis.f32
		
	-- allow read/write with magical [] lookups
	function array.__len(it) return 1 end
	function array.__index(it,n) return array[n] or tcore.read(it,n) end
	function array.__newindex(it,n,v) tcore.write(it,n,v) end

	function m2.new(...) return tcore.alloc(4* 4,m2):set(...) end
	function m3.new(...) return tcore.alloc(4* 9,m3):set(...) end
	function m4.new(...) return tcore.alloc(4*16,m4):set(...) end

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
	m4.rotate			=	tcore.m4_rotate
	m4.scale_v3			=	tcore.m4_scale_v3
	m4.scale			=	tcore.m4_scale
	m4.translate		=	tcore.m4_translate

end

--
-- (C) 2013 Kriss@XIXs.com
--
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- Time And Relative Dimensions In Space
--
-- a lua library for manipulating time and space
-- pure lua by default and opengl in flavour
--
-- recoil in terror as we use two glyph names to describe structures
-- whilst typing in random strings of numbers that may or may not
-- contain tyops
--
-- v# vector [#]
-- m# matrix [#][#]
-- q4 quaternion (yeah its just a repackaged v4)
--
-- each class is a table of # values [1] to [#] , just access them
-- directly they are number streams formated the same way as opengl
-- (row-major) metatables are used to provide advanced functionality
--
-- This code may contain bugs,
-- do not use if you are not prepared to fix them.
--
-- https://bitbucket.org/xixs/bin/src/tip/lua/wetgenes/tardis.lua
--
-- This also needs to be overloaded with a float based C version
-- Which does partially exist but is only used in the GLES lib so far
-- This is now done, but probably should be an optional action.
-- Assuming I havent borked anything I can now improve the C side speed.
--
-- This seems to be the simplest (programmer orientated) description of
-- most of the maths used here so go read it
-- http://www.j3d.org/matrix_faq/matrfaq_latest.html
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

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-- a metatable typeof function
M.mtype_lookup=mtype_lookup or {}
function M.mtype(it)
	return M.mtype_lookup[getmetatable(it) or 0] or type(it)
end

-- dumb class inheritance metatable creation
local function class(name,...)

	if M[name] then return M[name] end
	
	local tab={} -- create new
	local sub={...} -- possibly multiple sub classes

	if #sub>0 then -- inherit?
		for idx=#sub,1,-1 do -- reverse sub class order, so the ones to the left overwrite the ones on the right
			for i,v in pairs(sub[idx]) do tab[i]=v end -- each subclass overwrites all values
		end
	end

	tab.__index=tab -- this metatable is its own index

	M.mtype_lookup[name]=tab -- classtype metatable lookup
	M.mtype_lookup[tab]=name -- tab->name or name->tab

	M[name]=tab
	return tab
end



local array=class("array")

function array.__tostring(it) -- these classes are all just 1d arrays of numbers
	local t={}
	t[#t+1]=M.mtype(it)
	t[#t+1]="={"
	for i=1,#it do
		t[#t+1]=tostring(it[i])
		if i~=#it then t[#t+1]=", " end
	end
	t[#t+1]="}"
	return table.concat(t)
end

function array.set(it,...)
	local n=1
	for i,v in ipairs{...} do
		if not it[n] then return it end -- got all the data we need (#it)
		if type(v)=="number" then
			it[n]=v
			n=n+1
		else
			for ii,vv in ipairs(v) do -- allow one depth of tables
				it[n]=vv
				n=n+1
			end
		end
	end
	return it
end

function array.product(a,b,r)
	local mta=M.mtype(a)
	local mtb=M.mtype(b)
	if mta=="m4" then
		if     mtb=="v3" then
			return M.m4_product_v3(a,b,r)
		elseif mtb=="v4" then
			return M.m4_product_v4(a,b,r)
		elseif mtb=="m4" then
			return M.m4_product_m4(a,b,r)
		end
	elseif mta=="q4" then
		if     mtb=="q4" then
			return M.q4_product_q4(a,b,r)
		elseif mtb=="v3" then
			return M.q4_product_v3(a,b,r)
		end
	end
	error("tardis : "..mta.." product "..mtb.." not supported",2)
end


local m2=class("m2",array)
function m2.new(...) return setmetatable({0,0,0,0},m2):set(...) end
function m2.identity(it) return it:set(1,0, 0,1) end 
function m2.determinant(it)
	return	 ( it[ 1 ]*it[ 2+2 ] )
			+( it[ 2 ]*it[ 2+1 ] )
			-( it[ 1 ]*it[ 2+1 ] )
			-( it[ 2 ]*it[ 2+1 ] )
end
function m2.minor_xy(it,x,y)
	return it[1+(2-(x-1))+((2-(y-1))*2)]
end
function m2.transpose(it,r)
	r=r or it
	return	 r:set(it[1],it[2+1], it[2],it[2+2])
end
function m2.scale(it,s,r)
	r=r or it
	return r:set(it[1]*s,it[2]*s, it[2+1]*s,it[2+2]*s)
end
function m2.cofactor(it,r)
	r=r or it
	local t={}
	for iy=1,2 do
		for ix=1,2 do
			if (ix~=x) and (iy~=y) then
				t[#t+1]=m2.minor_xy(it,ix,iy)
				if ((ix+iy)%2)==1 then t[#t]=-t[#t] end
			end
		end
	end
	return r
end
function m2.adjugate(it,r)
	r=r or it
	return m2.cofactor(m2.transpose(it,m2.new()),r)
end
function m2.inverse(it,r)
	r=r or it
	local ood=1/m2.determinant(it)	
	return m2.scale(m2.cofactor(m2.transpose(it,m2.new())),ood,r)
end

local m3=class("m3",m2)
function m3.new(...) return setmetatable({0,0,0,0,0,0,0,0,0},m3):set(...) end
function m3.identity(it) return it:set(1,0,0, 0,1,0, 0,0,1) end 
function m3.determinant(it)
	return	 ( it[ 1 ]*it[ 3+2 ]*it[ 6+3 ] )
			+( it[ 2 ]*it[ 3+3 ]*it[ 6+1 ] )
			+( it[ 3 ]*it[ 3+1 ]*it[ 6+2 ] )
			-( it[ 1 ]*it[ 3+3 ]*it[ 6+2 ] )
			-( it[ 2 ]*it[ 3+1 ]*it[ 6+3 ] )
			-( it[ 3 ]*it[ 3+2 ]*it[ 6+1 ] )
end
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
function m3.transpose(it,r)
	r=r or it
	return	 r:set(it[1],it[3+1],it[6+1], it[2],it[3+2],it[6+2], it[3],it[3+3],it[6+3])
end
function m3.scale(it,s,r)
	r=r or it
	return r:set(it[1]*s,it[2]*s,it[3]*s, it[3+1]*s,it[3+2]*s,it[3+3]*s, it[6+1]*s,it[6+2]*s,it[6+3]*s)
end
function m3.cofactor(it,r)
	r=r or it
	local t={}
	for iy=1,3 do
		for ix=1,3 do
			if (ix~=x) and (iy~=y) then
				t[#t+1]=m3.minor_xy(it,ix,iy)
				if ((ix+iy)%2)==1 then t[#t]=-t[#t] end
			end
		end
	end
	return r:set(t)
end
function m3.adjugate(it,r)
	r=r or it
	return m3.cofactor(m3.transpose(it,m3.new()),r)
end
function m3.inverse(it,r)
	r=r or it
	local ood=1/m3.determinant(it)	
	return m3.scale(m3.cofactor(m3.transpose(it,m3.new())),ood,r)
end

local m4=class("m4",m3)
function m4.new(...) return setmetatable({0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},m4):set(...) end
function m4.identity(it) return it:set(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1) end 
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
function m4.transpose(it,r)
	r=r or r
	return	 r:set(it[1],it[4+1],it[8+1],it[12+1], it[2],it[4+2],it[8+2],it[12+2], it[3],it[4+3],it[8+3],it[12+3], it[4],it[4+4],it[8+4],it[12+4])
end
function m4.scale(it,s,r)
	r=r or it
	return r:set(it[1]*s,it[2]*s,it[3]*s,it[4]*s, it[4+1]*s,it[4+2]*s,it[4+3]*s,it[4+4]*s, it[8+1]*s,it[8+2]*s,it[8+3]*s,it[8+4]*s, it[12+1]*s,it[12+2]*s,it[12+3]*s,it[12+4]*s)
end
function m4.cofactor(it,r)
	r=r or it
	local t={}
	for iy=1,4 do
		for ix=1,4 do
			if (ix~=x) and (iy~=y) then
				t[#t+1]=m4.minor_xy(it,ix,iy)
				if ((ix+iy)%2)==1 then t[#t]=-t[#t] end
			end
		end
	end
	return r:set(t)
end
function m4.adjugate(it,r)
	r=r or it
	return 	m4.cofactor(m4.transpose(it,m4.new()),r)
end
function m4.inverse(it,r)
	r=r or it
	local ood=1/m4.determinant(it)	
	return m4.scale(m4.cofactor(m4.transpose(it,m4.new())),ood,r)
end
function m4.identity(it)
	return it:set(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1)
end
function m4.translate(it,v3a,r)
	r=r or it
	local r1=it[12+1]+v3a[1]*it[1]+v3a[2]*it[5]+v3a[3]*it[9]
	local r2=it[12+2]+v3a[1]*it[2]+v3a[2]*it[6]+v3a[3]*it[10]
	local r3=it[12+3]+v3a[1]*it[3]+v3a[2]*it[7]+v3a[3]*it[11]
	return r:set(it[1],it[2],it[3],it[4], it[5],it[6],it[7],it[8], it[9],it[10],it[11],it[12], r1,r2,r3,it[16] )
end

function m4.scale_v3(it,v3a,r)
	r=r or it
	local s1=v3a[1]
	local s2=v3a[2]
	local s3=v3a[3]
	return r:set(	s1*it[1],	s1*it[2],	s1*it[3],	s1*it[4],
					s2*it[5],	s2*it[6],	s2*it[7],	s2*it[8],
					s3*it[9],	s3*it[10],	s3*it[11],	s3*it[12],
					it[13],		it[14],		it[15],		it[16] )
end

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

function m4.rotate(it,degrees,v3a,r)
	local m4a=m4.new():setrot(degrees,v3a)
	return M.m4_product_m4(m4a,it,r)
end

local v2=class("v2",array)
function v2.new(...) return setmetatable({0,0},v2):set(...) end
function v2.identity(it) return it:set(0,0) end 
function v2.lenlen(it)
	return (it[1]*it[1]) + (it[2]*it[2])
end
function v2.len(it)
	return math.sqrt( (it[1]*it[1]) + (it[2]*it[2]) )
end
function v2.scale(it,s,r)
	r=r or it
	return r:set( it[1]*s , it[2]*s )
end
function v2.normalize(it,r)
	return v2.scale(it,1/v2.len(it),r)
end

local v3=class("v3",v2)
function v3.new(...) return setmetatable({0,0,0},v3):set(...) end
function v3.identity(it) return it:set(0,0,0) end 
function v3.lenlen(it)
	return (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3])
end
function v3.len(it)
	return math.sqrt( (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3]) )
end
function v3.scale(it,s,r)
	r=r or it
	return r:set( it[1]*s , it[2]*s , it[3]*s )
end
function v3.normalize(it,r)
	return v3.scale(it,1/v3.len(it),r)
end
function v3.add(va,vb,r)
	r=r or va
	return r:set( va[1]+vb[1] , va[2]+vb[2] , va[3]+vb[3] )
end
function v3.sub(va,vb,r)
	r=r or va
	return r:set( va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] )
end
function v3.mul(va,vb,r)
	r=r or va
	return r:set( (va[1]*vb[1]) , (va[2]*vb[2]) , (va[3]*vb[3]) )
end
function v3.dot(va,vb)
	return ( (va[1]*vb[1]) + (va[2]*vb[2]) + (va[3]*vb[3]) )
end
function v3.cross(va,vb,r)
	r=r or va
	return r:set( (va[2]*vb[3])-(va[3]*vb[2]) , (va[3]*vb[1])-(va[1]*vb[3]) , (va[1]*vb[2])-(va[2]*vb[1]) )
end


local v4=class("v4",v3)
function v4.new(...) return setmetatable({0,0,0,0},v4):set(...) end
function v4.identity(it) return it:set(0,0,0,0) end 
function v4.to_v3(it,r) -- scale [4] to 1 then throw it away so we have a v3 xyz
	r=r or v3.new()
	local oow=1/it[4]
	return r:set( it[1]*oow , it[2]*oow , it[3]*oow )
end
function v4.lenlen(it)
	return (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3]) + (it[4]*it[4])
end
function v4.len(it)
	return math.sqrt( (it[1]*it[1]) + (it[2]*it[2]) + (it[3]*it[3]) + (it[4]*it[4]) )
end
function v4.scale(it,s,r)
	r=r or it
	return r:set( it[1]*s , it[2]*s , it[3]*s , it[4]*s )
end
function v4.normalize(it,r)
	return v4.scale(it,1/v4.len(it),r)
end
function v4.add(va,vb,r)
	r=r or va
	return r:set( va[1]+vb[1] , va[2]+vb[2] , va[3]+vb[3] , va[4]+vb[4] )
end
function v4.sub(va,vb,r)
	r=r or va
	return r:set( va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] , va[4]-vb[4] )
end
function v4.dot(va,vb)
	return ( (va[1]*vb[1]) + (va[2]*vb[2]) + (va[3]*vb[3]) + (va[4]*vb[4]) )
end


local q4=class("q4",v4)
function q4.new(...) return setmetatable({0,0,0,1},q4):set(...) end
function q4.identity(it) return it:set(0,0,0,1) end 
function q4.nlerp(qa,qb,sa,r)
	local sb=1-sa
	if qa.dot(qb) < 0 then sa=-sa end -- shortest fix
	r=r or va
	r:set( va[1]*sa+vb[1]*sb , va[2]*sa+vb[2]*sb , va[3]*sa+vb[3]*sb , va[4]*sa+vb[4]*sb )
	return r:normalize()
end

function q4.setrot(it,degrees,v3a)
	local ah=degrees * (math.PI/360)
	local sh=math.sin(ah)
	return it:set( math.cos(ah) , v3a[1]*sh , v3a[2]*sh , v3a[3]*sh )
end

function q4.rotate(it,degrees,v3a,r)
	local q4a=q4.new():setrot(degrees,v3a)
	return M.q4_product_q4(q4a,it,r)
end



local line=class("line",array)
line.set=nil -- disable
function line.new(...) return setmetatable({v3.new(),v3.new()},line) end -- [1]position , [2]normal

local plane=class("plane",line)
function plane.new(...) return setmetatable({v3.new(),v3.new()},plane) end -- [1]position , [2]normal


function M.line_intersect_plane(l,p,r)
	r=r or v3.new()
	local t=v3.new(p[1]):sub(l[1]) -- the line position relative to the plane
	local d=l[2]:dot(p[2]) -- the length of the line until it hits the plane
	if d~=0 then -- less errors please
		d=t:dot(p[2])/d
	end
	return r:set( l[1][1]+(l[2][1]*d) , l[1][2]+(l[2][2]*d) , l[1][3]+(l[2][3]*d) ) -- the point of intersection
end

function M.q4_to_m4(q,m)
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

function M.q4_product_q4(q4a,q4b,r)
	r=r or q4b
    local r1 =  q4a[1] * q4b[4] + q4a[2] * q4b[3] - q4a[3] * q4b[2] + q4a[4] * q4b[1];
    local r2 = -q4a[1] * q4b[3] + q4a[2] * q4b[4] + q4a[3] * q4b[1] + q4a[4] * q4b[2];
    local r3 =  q4a[1] * q4b[2] - q4a[2] * q4b[1] + q4a[3] * q4b[4] + q4a[4] * q4b[3];
    local r4 = -q4a[1] * q4b[1] - q4a[2] * q4b[2] - q4a[3] * q4b[3] + q4a[4] * q4b[4];
	return r:set(r1,r2,r3,r4)
end

function M.q4_product_v3(q4a,v3b,r)
	r=r or v3b
    local r1 =                    q4a[2] * v3b[3] - q4a[3] * v3b[2] + q4a[4] * v3b[1];
    local r2 = -q4a[1] * v3b[3]                   + q4a[3] * v3b[1] + q4a[4] * v3b[2];
    local r3 =  q4a[1] * v3b[2] - q4a[2] * v3b[1]                   + q4a[4] * v3b[3];
	return r:set(r1,r2,r3)
end

function M.m4_product_v3(m4a,v3b,r)
	r=r or v3b
	local oow=1/( (m4a[   4]*v3b[1]) + (m4a[ 4+4]*v3b[2]) + (m4a[ 8+4]*v3b[3]) + (m4a[12+4] ) )
	local r1= oow * ( (m4a[   1]*v3b[1]) + (m4a[ 4+1]*v3b[2]) + (m4a[ 8+1]*v3b[3]) + (m4a[12+1] ) )
	local r2= oow * ( (m4a[   2]*v3b[1]) + (m4a[ 4+2]*v3b[2]) + (m4a[ 8+2]*v3b[3]) + (m4a[12+2] ) )
	local r3= oow * ( (m4a[   3]*v3b[1]) + (m4a[ 4+3]*v3b[2]) + (m4a[ 8+3]*v3b[3]) + (m4a[12+3] ) )
	return r:set(r1,r2,r3)
end

function M.m4_product_m4(m4a,m4b,r)
	r=r or m4b
	local r1 = (m4a[   1]*m4b[   1]) + (m4a[   2]*m4b[ 4+1]) + (m4a[   3]*m4b[ 8+1]) + (m4a[   4]*m4b[12+1])
	local r2 = (m4a[   1]*m4b[   2]) + (m4a[   2]*m4b[ 4+2]) + (m4a[   3]*m4b[ 8+2]) + (m4a[   4]*m4b[12+2])
	local r3 = (m4a[   1]*m4b[   3]) + (m4a[   2]*m4b[ 4+3]) + (m4a[   3]*m4b[ 8+3]) + (m4a[   4]*m4b[12+3])
	local r4 = (m4a[   1]*m4b[   4]) + (m4a[   2]*m4b[ 4+4]) + (m4a[   3]*m4b[ 8+4]) + (m4a[   4]*m4b[12+4])
	local r5 = (m4a[ 4+1]*m4b[   1]) + (m4a[ 4+2]*m4b[ 4+1]) + (m4a[ 4+3]*m4b[ 8+1]) + (m4a[ 4+4]*m4b[12+1])
	local r6 = (m4a[ 4+1]*m4b[   2]) + (m4a[ 4+2]*m4b[ 4+2]) + (m4a[ 4+3]*m4b[ 8+2]) + (m4a[ 4+4]*m4b[12+2])
	local r7 = (m4a[ 4+1]*m4b[   3]) + (m4a[ 4+2]*m4b[ 4+3]) + (m4a[ 4+3]*m4b[ 8+3]) + (m4a[ 4+4]*m4b[12+3])
	local r8 = (m4a[ 4+1]*m4b[   4]) + (m4a[ 4+2]*m4b[ 4+4]) + (m4a[ 4+3]*m4b[ 8+4]) + (m4a[ 4+4]*m4b[12+4])
	local r9 = (m4a[ 8+1]*m4b[   1]) + (m4a[ 8+2]*m4b[ 4+1]) + (m4a[ 8+3]*m4b[ 8+1]) + (m4a[ 8+4]*m4b[12+1])
	local r10= (m4a[ 8+1]*m4b[   2]) + (m4a[ 8+2]*m4b[ 4+2]) + (m4a[ 8+3]*m4b[ 8+2]) + (m4a[ 8+4]*m4b[12+2])
	local r11= (m4a[ 8+1]*m4b[   3]) + (m4a[ 8+2]*m4b[ 4+3]) + (m4a[ 8+3]*m4b[ 8+3]) + (m4a[ 8+4]*m4b[12+3])
	local r12= (m4a[ 8+1]*m4b[   4]) + (m4a[ 8+2]*m4b[ 4+4]) + (m4a[ 8+3]*m4b[ 8+4]) + (m4a[ 8+4]*m4b[12+4])
	local r13= (m4a[12+1]*m4b[   1]) + (m4a[12+2]*m4b[ 4+1]) + (m4a[12+3]*m4b[ 8+1]) + (m4a[12+4]*m4b[12+1])
	local r14= (m4a[12+1]*m4b[   2]) + (m4a[12+2]*m4b[ 4+2]) + (m4a[12+3]*m4b[ 8+2]) + (m4a[12+4]*m4b[12+2])
	local r15= (m4a[12+1]*m4b[   3]) + (m4a[12+2]*m4b[ 4+3]) + (m4a[12+3]*m4b[ 8+3]) + (m4a[12+4]*m4b[12+3])
	local r16= (m4a[12+1]*m4b[   4]) + (m4a[12+2]*m4b[ 4+4]) + (m4a[12+3]*m4b[ 8+4]) + (m4a[12+4]*m4b[12+4])
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
-- we use this to wout out where to place our view such that it is always visible and keeps its aspect.
--
function M.m4_project23d(view_width,view_height,width,height,fov,depth)

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


--upgrade the above to hopefully faster C versions working on userdata arrays of floats

local tcore=require("wetgenes.tardis.core") -- use a "faster?" f32 C core

-- allow read/write with magical [] lookups
function array.__len(it) return 1 end
function array.__index(it,n) return array[n] or tcore.read(it,n) end
function array.__newindex(it,n,v) tcore.write(it,n,v) end

-- initialise an array
function array.set(it,...) tcore.set(it,...) return it end

m2.set=array.set
m3.set=array.set
m4.set=array.set

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

v2.set=array.set
v3.set=array.set
v4.set=array.set
q4.set=array.set

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

M.m4_product_m4		=	tcore.m4_product_m4

m4.identity			=	tcore.m4_identity
m4.rotate			=	tcore.m4_rotate
m4.scale_v3			=	tcore.m4_scale_v3
m4.translate		=	tcore.m4_translate

		
--[[
print("test")
local m=m4.new()--:indentity()
print("m4",m)
m:identity()
print("m4",m)
m:scale_v3(2,2,2)
print("m4",m)
m:translate(2,2,2)
print("m4",m)


print("test")
local m=m2.new()--:indentity()
print("m2",m)
m:identity()
print("m2",m)
m:scale(2)
print("m2",m)

os.exit(0)
]]


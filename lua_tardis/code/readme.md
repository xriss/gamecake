

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.tardis


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



## lua.wetgenes.tardis.array



Array is the base class for all other tardis classes, it is just a 
stream of numbers, probably in a table but possibly a chunk of f32 
values in a userdata.



## lua.wetgenes.tardis.array.__add


	r = array.__add(a,b)

Add a to b and return a a.new(result) so the class returned will match the
input class of a and neither a or b will be modified.



## lua.wetgenes.tardis.array.__div


	r = array.__div(a,b)

Replace b with 1/b and then call the appropriate product function depending on
the argument classes. Always creates and returns a.new() value.



## lua.wetgenes.tardis.array.__eq


	r = array.__eq(a,b)

meta to call a:compare(b) and return the result



## lua.wetgenes.tardis.array.__mul


	r = array.__mul(a,b)

Call the appropriate product function depending on the argument classes. Always
creates and returns a.new() value.



## lua.wetgenes.tardis.array.__sub


	r = array.__sub(a,b)

Subtract b from a and return a a.new(result) so the class returned will match the
input class of a and neither a or b will be modified.



## lua.wetgenes.tardis.array.__tostring


	string = array.__tostring(it)

Convert an array to a string this is called by the lua tostring() function,



## lua.wetgenes.tardis.array.__unm


	r = array.__unm(a)

Subtract b from 0 and return a a.new(result) so the class returned will match the
input class of a but a will not be modified



## lua.wetgenes.tardis.array.abs


	r=it:abs(r)
	r=it:abs(it.new())

Perform math.abs on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.acos


	r=it:acos(r)
	r=it:acos(it.new())

Perform math.acos on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.add


	r=it:add(b,r)
	r=it:add(b,it.new())

Add b to it. b may be a number or another array of the same size as this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.asin


	r=it:asin(r)
	r=it:asin(it.new())

Perform math.asin on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.atan


	r=it:atan(r)
	r=it:atan(it.new())

Perform math.atan on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.ceil


	r=it:ceil(r)
	r=it:ceil(it.new())

Perform math.ceil on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.compare


	a=a:compare(b)
	a=a:compare(1,2,3,4)

Are the numbers in b the same as the numbers in a, this function will 
return true if they are and false if they are not.

If the arrays are of different lengths then this will return false.

Numbers to test for can be given explicitly in the arguments and we 
follow the same one level of tables rule as we do with array.set so any 
class derived from array can be used.



## lua.wetgenes.tardis.array.cos


	r=it:cos(r)
	r=it:cos(it.new())

Perform math.cos on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.exp


	r=it:exp(r)
	r=it:exp(it.new())

Perform math.exp on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.floor


	r=it:floor(r)
	r=it:floor(it.new())

Perform math.floor on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.fract


	r=it:fract(r)
	r=it:fract(it.new())

Return the fractional part of each value using math.modf.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.log


	r=it:log(r)
	r=it:log(it.new())

Perform math.log on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.max


	r=it:max()

Return a single number value that is the maximum of all values in this array.



## lua.wetgenes.tardis.array.min


	r=it:min()

Return a single number value that is the minimum of all values in this array.



## lua.wetgenes.tardis.array.mix


	r=a:mix(b,s,r)

Mix values between a and b where a is returned if s<=0 and b is returned if s>=1

If r is provided then the result is written into r and returned otherwise a is
modified and returned.



## lua.wetgenes.tardis.array.pow


	r=it:pow(p,r)
	r=it:pow(p,it.new())

Perform math.pow(it,p) on all values of this array. p may be a number or
another array of the same size as this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.product


	ma = ma:product(mb)
	ma = ma:product(mb,r)

Look at the type and call the appropriate product function, to produce 

	mb x ma
	
Note the right to left application and default returning of the 
leftmost term for chaining. This seems to make the most sense.

If r is provided then the result is written into r and returned 
otherwise ma is modified and returned.



## lua.wetgenes.tardis.array.quantize


	r=it:quantize(1/1024,r)
	r=it:quantize(s,it.new())

Perform a trunc(v/s)*s on all values of this array. We recomended the 
use of a power of two, eg 1/1024 rather than 1/1000 if you wanted 3 
decimal digits.

If r is provided then the result is written into r and returned 
otherwise it is modified and returned.



## lua.wetgenes.tardis.array.round


	r=it:round(r)
	r=it:round(it.new())

Perform math.floor( v+0.5 ) on all values of this array. Which will 
round up or down to the nearest integer.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.scalar


	r=a:scalar(s,r)

Multiply all value in array by number.

If r is provided then the result is written into r and returned otherwise a is
modified and returned.



## lua.wetgenes.tardis.array.set


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



## lua.wetgenes.tardis.array.sin


	r=it:sin(r)
	r=it:sin(it.new())

Perform math.sin on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.sub


	r=it:sub(b,r)
	r=it:sub(b,it.new())

Subtract b from it. b may be a number or another array of the same size as this
array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.tan


	r=it:tan(r)
	r=it:tan(it.new())

Perform math.tan on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.trunc


	r=it:trunc(r)
	r=it:trunc(it.new())

Perform math.floor on positive values and math ceil on negative values 
for all values of this array. So a trunication that will always error 
towards 0.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.zero


	a=a:zero()

Set all values in this array to zero.



## lua.wetgenes.tardis.class


	metatable=tardis.class(name,class,...)

Create a new metatable for an object class, optionally inheriting from other previously 
declared classes.



## lua.wetgenes.tardis.line


A 3d space line class.

[1]position , [2]normal

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.line.new


	line = tardis.line.new(p,n)

Create a new line and optionally set it to the given values.



## lua.wetgenes.tardis.m2


The metatable for a 2x2 matrix class, use the new function to actually create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.m2.adjugate


	m2 = m2:adjugate()
	m2 = m2:adjugate(r)

Adjugate this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.cofactor


	m2 = m2:cofactor()
	m2 = m2:cofactor(r)

Cofactor this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.determinant


	value = m2:determinant()

Return the determinant value of this m2.



## lua.wetgenes.tardis.m2.identity


	m2 = m2:identity()

Set this m2 to the identity matrix.



## lua.wetgenes.tardis.m2.inverse


	m2 = m2:inverse()
	m2 = m2:inverse(r)

Inverse this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.minor_xy


	value = m2:minor_xy()

Return the minor_xy value of this m2.



## lua.wetgenes.tardis.m2.new


	m2 = tardis.m2.new()

Create a new m2 and optionally set it to the given values, m2 methods 
usually return the input m2 for easy function chaining.



## lua.wetgenes.tardis.m2.scale


	m2 = m2:scale(s)
	m2 = m2:scale(s,r)

Scale this m2 by s.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.transpose


	m2 = m2:transpose()
	m2 = m2:transpose(r)

Transpose this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m3


The metatable for a 3x3 matrix class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.m3.adjugate


	m3 = m3:adjugate()
	m3 = m3:adjugate(r)

Adjugate this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.cofactor


	m3 = m3:cofactor()
	m3 = m3:cofactor(r)

Cofactor this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.determinant


	value = m3:determinant()

Return the determinant value of this m3.



## lua.wetgenes.tardis.m3.identity


	m3 = m3:identity()

Set this m3 to the identity matrix.



## lua.wetgenes.tardis.m3.inverse


	m3 = m3:inverse()
	m3 = m3:inverse(r)

Inverse this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.m4


	m4 = m3:m4()

Expand an m3 matrix into an m4 matrix.



## lua.wetgenes.tardis.m3.minor_xy


	value = m3:minor_xy()

Return the minor_xy value of this m3.



## lua.wetgenes.tardis.m3.new


	m3 = tardis.m3.new()

Create a new m3 and optionally set it to the given values, m3 methods 
usually return the input m3 for easy function chaining.



## lua.wetgenes.tardis.m3.scale


	m3 = m3:scale(s)
	m3 = m3:scale(s,r)

Scale this m3 by s.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.transpose


	m3 = m3:transpose()
	m3 = m3:transpose(r)

Transpose this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.v3


	v3 = m3:v3(n)

Extract and return a "useful" v3 from an m3 matrix. The first vector is the x
axis, then y axis , then z axis.




## lua.wetgenes.tardis.m4


The metatable for a 4x4 matrix class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.m4.add


	m4 = m4:add(m4b)
	m4 = m4:add(m4b,r)

Add m4b this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.adjugate


	m4 = m4:adjugate()
	m4 = m4:adjugate(r)

Adjugate this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.arotate


	m4 = m4:arotate(degrees,v3a)
	m4 = m4:arotate(degrees,v3a,r)

Apply a rotation in degres around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.cofactor


	m4 = m4:cofactor()
	m4 = m4:cofactor(r)

Cofactor this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.determinant


	value = m4:determinant()

Return the determinant value of this m4.



## lua.wetgenes.tardis.m4.get_rotation_q4


	q4 = m4:get_rotation_q4(r)

Get quaternion rotation from a scale/rot/trans matrix. Note that scale 
is assumed to be uniform which it usually is. If that is not the case 
then remove the scale first.

If r is provided then the result is written into r and returned 
otherwise a new q4 is created and returned.



## lua.wetgenes.tardis.m4.get_scale_v3


	v3 = m4:get_scale_v3(r)

Get v3 scale from a scale/rot/trans matrix

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.m4.get_translation_v3


	v3 = m4:get_translation_v3(r)

Get v3 translation from a scale/rot/trans matrix

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.m4.identity


	m4 = m4:identity()

Set this m4 to the identity matrix.



## lua.wetgenes.tardis.m4.inverse


	m4 = m4:inverse()
	m4 = m4:inverse(r)

Inverse this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.lerp


	m4 = m4:lerp(m4b,s)
	m4 = m4:lerp(m4b,s,r)

Lerp from m4 to m4b by s.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.m3


	m4 = m4:m3()

Grab tthe top,left parts of the m4 matrix as an m3 matrix.



## lua.wetgenes.tardis.m4.minor_xy


	value = m4:minor_xy()

Return the minor_xy value of this m4.



## lua.wetgenes.tardis.m4.new


	m4 = tardis.m4.new()

Create a new m4 and optionally set it to the given values, m4 methods 
usually return the input m4 for easy function chaining.



## lua.wetgenes.tardis.m4.prearotate


	m4 = m4:prearotate(degrees,v3a)
	m4 = m4:prearotate(degrees,v3a,r)

Pre apply a rotation in degrees around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.preqrotate


	m4 = m4:preqrotate(q)
	m4 = m4:preqrotate(q,r)

Pre apply a quaternion rotation to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.prerotate


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



## lua.wetgenes.tardis.m4.prerrotate


	m4 = m4:prerrotate(radians,v3a)
	m4 = m4:prerrotate(radians,v3a,r)

Pre apply a rotation in radians around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.prescale


	m4 = m4:scale(s)
	m4 = m4:scale(s,r)
	m4 = m4:scale(x,y,z)
	m4 = m4:scale(x,y,z,r)
	m4 = m4:scale(v3)
	m4 = m4:scale(v3,r)

Pre Scale this m4 by {s,s,s} or {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.pretranslate


	m4 = m4:pretranslate(x,y,z)
	m4 = m4:pretranslate(x,y,z,r)
	m4 = m4:pretranslate(v3a)
	m4 = m4:pretranslate(v3a,r)

Translate this m4 along its global axis by {x,y,z} or v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.pretranslate_v3


	m4 = m4:pretranslate_v3(v3a)
	m4 = m4:pretranslate_v3(v3a,r)

Translate this m4 along its global axis by v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.qrotate


	m4 = m4:qrotate(q)
	m4 = m4:qrotate(q,r)

Apply a quaternion rotation to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.rotate


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



## lua.wetgenes.tardis.m4.rrotate


	m4 = m4:rrotate(radians,v3a)
	m4 = m4:rrotate(radians,v3a,r)

Apply a rotation in radians around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.scale


	m4 = m4:scale(s)
	m4 = m4:scale(s,r)
	m4 = m4:scale(x,y,z)
	m4 = m4:scale(x,y,z,r)
	m4 = m4:scale(v3)
	m4 = m4:scale(v3,r)

Scale this m4 by {s,s,s} or {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.scale_v3


	m4 = m4:scale_v3(v3)
	m4 = m4:scale_v3(v3,r)

Scale this m4 by {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.setrot


	m4 = m4:setrot(degrees,v3a)

Set this matrix to a rotation matrix around the given normal by the given degrees.

we will automatically normalise v3a if necessary.



## lua.wetgenes.tardis.m4.setrrot


	m4 = m4:setrrot(radians,v3a)

Set this matrix to a rotation matrix around the given normal by the given radians.

we will automatically normalise v3a if necessary.



## lua.wetgenes.tardis.m4.sub


	m4 = m4:sub(m4b)
	m4 = m4:sub(m4b,r)

Subtract m4b this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.translate


	m4 = m4:translate(x,y,z)
	m4 = m4:translate(x,y,z,r)
	m4 = m4:translate(v3a)
	m4 = m4:translate(v3a,r)

Translate this m4 along its local axis by {x,y,z} or v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.translate_v3


	m4 = m4:translate_v3(v3a)
	m4 = m4:translate_v3(v3a,r)

Translate this m4 along its local axis by v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.transpose


	m4 = m4:transpose()
	m4 = m4:transpose(r)

Transpose this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.v3


	v3 = m4:v3(n)

Extract and return a "useful" v3 from an m4 matrix. The first vector is the x
axis, then y axis , then z axis and finally transform.

If n is not given or not a known value then we return the 4th vector which is
the "opengl" transform as that is the most useful v3 part of an m4.



## lua.wetgenes.tardis.m4_stack


	stack = tardis.m4_stack()

create an m4 stack that is very similar to an old school opengl transform
stack.



## lua.wetgenes.tardis.plane


A 3d space plane class.

[1]position , [2]normal

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.plane.new


	plane = tardis.plane.new(p,n)

Create a new plane and optionally set it to the given values.



## lua.wetgenes.tardis.q4


The metatable for a quaternion class, use the new function to actually create an object.

We also inherit all the functions from tardis.v4



## lua.wetgenes.tardis.q4.get_yaw_pitch_roll


	v3 = q4:get_yaw_pitch_roll()

Get a yaw,pitch,roll degree rotation from this quaternion

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.q4.get_yaw_pitch_roll_in_radians


	v3 = q4:get_yaw_pitch_roll_in_radians()

Get a yaw,pitch,roll degree rotation from this quaternion

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.q4.identity


	q4 = q4:identity()

Set this q4 to its 0,0,0,1 identity



## lua.wetgenes.tardis.q4.lerp


	q4 = q4:lerp(q4b,s)
	q4 = q4:lerp(q4b,s,r)

Nlerp from q4 to q4b by s.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.new


	q4 = tardis.q4.new()

Create a new q4 and optionally set it to the given values, q4 methods 
usually return the input q4 for easy function chaining.



## lua.wetgenes.tardis.q4.prerotate


	q4 = q4:prerotate(degrees,v3a)
	q4 = q4:prerotate(degrees,v3a,r)

Pre apply a degree rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.prerrotate


	q4 = q4:prerrotate(radians,v3a)
	q4 = q4:prerrotate(radians,v3a,r)

Pre apply a radian rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.reverse


	q4 = q4:reverse()
	q4 = q4:reverse(r)

Reverse the rotation of this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.rotate


	q4 = q4:rotate(degrees,v3a)
	q4 = q4:rotate(degrees,v3a,r)

Apply a degree rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.rrotate


	q4 = q4:rrotate(radians,v3a)
	q4 = q4:rrotate(radians,v3a,r)

Apply a radian rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.set


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



## lua.wetgenes.tardis.q4.set_yaw_pitch_roll


	q4 = q4:set_yaw_pitch_roll(v3)
	q4 = q4:set_yaw_pitch_roll({90,60,30})	-- 30yaw 60pitch 90roll

Set a V3(roll,pitch,yaw) degree rotation into this quaternion

	yaw   v[3] is rotation about the z axis and is applied first
	pitch v[2] is rotation about the y axis and is applied second
	roll  v[1] is rotation about the z axis and is applied last



## lua.wetgenes.tardis.q4.set_yaw_pitch_roll_in_radians


	q4 = q4:set_yaw_pitch_roll_in_radians(v)

Set a V3(roll,pitch,yaw) radian rotation into this quaternion

	yaw   v[3] is rotation about the z axis and is applied first
	pitch v[2] is rotation about the y axis and is applied second
	roll  v[1] is rotation about the z axis and is applied last



## lua.wetgenes.tardis.q4.setrot


	q4 = q4:setrot(degrees,v3a)

Set this quaternion to a rotation around the given normal by the given degrees.



## lua.wetgenes.tardis.q4.setrrot


	q4 = q4:setrrot(radians,v3a)

Set this quaternion to a rotation around the given normal by the given radians.



## lua.wetgenes.tardis.smoothstep


	f = tardis.step(edge1,edge2,num)

return 0 if num is bellow or equal to edge1. Return 1 if num is the same or
higher as edge2 and smoothly interpolate between 0 and 1 for all other values.



## lua.wetgenes.tardis.step


	i = tardis.step(edge,num)

return 0 if num is bellow edge or 1 if num is the same or higher



## lua.wetgenes.tardis.type


	name=tardis.type(object)

This will return the type of an object previously registered with class



## lua.wetgenes.tardis.v1


The metatable for a 2d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.v1.add


	v1 = v1:add(v1b)
	v1 = v1:add(v1b,r)

Add v1b to v1.

If r is provided then the result is written into r and returned 
otherwise v1 is modified and returned.



## lua.wetgenes.tardis.v1.cross


	value = v1:cross(v1b)

Extend to 3d then only return z value as x and y are always 0



## lua.wetgenes.tardis.v1.distance


	value = a:distance(b)

Returns the length of the vector between a and b.



## lua.wetgenes.tardis.v1.dot


	value = v1:dot(v1b)

Return the dot product of these two vectors.



## lua.wetgenes.tardis.v1.identity


	v1 = v1:identity()

Set this v1 to all zeros.



## lua.wetgenes.tardis.v1.len


	value = v1:len()

Returns the length of this vector.



## lua.wetgenes.tardis.v1.lenlen


	value = v1:lenlen()

Returns the length of this vector, squared, this is often all you need 
for comparisons so lets us skip the sqrt.



## lua.wetgenes.tardis.v1.mul


	v1 = v1:mul(v1b)
	v1 = v1:mul(v1b,r)

Multiply v1 by v1b.

If r is provided then the result is written into r and returned 
otherwise v1 is modified and returned.



## lua.wetgenes.tardis.v1.normalize


	v1 = v1:normalize()
	v1 = v1:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned 
otherwise v1 is modified and returned.



## lua.wetgenes.tardis.v1.oo


	v1 = v1:oo()
	v1 = v1:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v1 is modified and returned.



## lua.wetgenes.tardis.v1.scale


	v1 = v1:scale(s)
	v1 = v1:scale(s,r)

Scale this v1 by s.

If r is provided then the result is written into r and returned 
otherwise v1 is modified and returned.



## lua.wetgenes.tardis.v1.sub


	v1 = v1:sub(v1b)
	v1 = v1:sub(v1b,r)

Subtract v1b from v1.

If r is provided then the result is written into r and returned 
otherwise v1 is modified and returned.



## lua.wetgenes.tardis.v2


The metatable for a 2d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.v2.add


	v2 = v2:add(v2b)
	v2 = v2:add(v2b,r)

Add v2b to v2.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.cross


	value = v2:cross(v2b)

Extend to 3d then only return z value as x and y are always 0



## lua.wetgenes.tardis.v2.distance


	value = a:distance(b)

Returns the length of the vector between a and b.



## lua.wetgenes.tardis.v2.dot


	value = v2:dot(v2b)

Return the dot product of these two vectors.



## lua.wetgenes.tardis.v2.identity


	v2 = v2:identity()

Set this v2 to all zeros.



## lua.wetgenes.tardis.v2.len


	value = v2:len()

Returns the length of this vector.



## lua.wetgenes.tardis.v2.lenlen


	value = v2:lenlen()

Returns the length of this vector, squared, this is often all you need 
for comparisons so lets us skip the sqrt.



## lua.wetgenes.tardis.v2.mul


	v2 = v2:mul(v2b)
	v2 = v2:mul(v2b,r)

Multiply v2 by v2b.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.new


	v2 = tardis.v2.new()

Create a new v2 and optionally set it to the given values, v2 methods 
usually return the input v2 for easy function chaining.



## lua.wetgenes.tardis.v2.normalize


	v2 = v2:normalize()
	v2 = v2:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.oo


	v2 = v2:oo()
	v2 = v2:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.scale


	v2 = v2:scale(s)
	v2 = v2:scale(s,r)

Scale this v2 by s.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.sub


	v2 = v2:sub(v2b)
	v2 = v2:sub(v2b,r)

Subtract v2b from v2.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v3


The metatable for a 3d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.v3.add


	v3 = v3:add(v3b)
	v3 = v3:add(v3b,r)

Add v3b to v3.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.angle


	radians,axis = v3a:angle(v3b)
	radians,axis = v3a:angle(v3b,axis)

Return radians and axis of rotation between these two vectors. If axis is given 
then it must represent a positive world aligned axis normal. So V3(1,0,0) or 
V3(0,1,0) or V3(0,0,1) only. The point of providing an axis allows the returned 
angle to be over a 360 degree range rather than flipping the axis after 180 
degrees this means the second axis returned value can be ignored as it will 
always be the axis that is passed in.



## lua.wetgenes.tardis.v3.cross


	v3 = v3:cross(v3b)
	v3 = v3:cross(v3b,r)

Return the cross product of these two vectors.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.distance


	value = a:distance(b)

Returns the length of the vector between a and b.



## lua.wetgenes.tardis.v3.dot


	value = v3:dot(v3b)

Return the dot product of these two vectors.



## lua.wetgenes.tardis.v3.identity


	v3 = v3:identity()

Set this v3 to all zeros.



## lua.wetgenes.tardis.v3.len


	value = v3:len()

Returns the length of this vector.



## lua.wetgenes.tardis.v3.lenlen


	value = v3:lenlen()

Returns the length of this vector, squared, this is often all you need 
for comparisons so lets us skip the sqrt.



## lua.wetgenes.tardis.v3.mul


	v3 = v3:mul(v3b)
	v3 = v3:mul(v3b,r)

Multiply v3 by v3b.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.new


	v3 = tardis.v3.new()

Create a new v3 and optionally set it to the given values, v3 methods 
usually return the input v3 for easy function chaining.



## lua.wetgenes.tardis.v3.normalize


	v3 = v3:normalize()
	v3 = v3:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.oo


	v3 = v3:oo()
	v3 = v3:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.scale


	v3 = v3:scale(s)
	v3 = v3:scale(s,r)

Scale this v3 by s.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.sub


	v3 = v3:sub(v3b)
	v3 = v3:sub(v3b,r)

Subtract v3b from v3.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v4


The metatable for a 4d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.v4.add


	v4 = v4:add(v4b)
	v4 = v4:add(v4b,r)

Add v4b to v4.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.distance


	value = a:distance(b)

Returns the length of the vector between a and b.



## lua.wetgenes.tardis.v4.dot


	value = v4:dot(v4b)

Return the dot product of these two vectors.



## lua.wetgenes.tardis.v4.identity


	v4 = v4:identity()

Set this v4 to all zeros.



## lua.wetgenes.tardis.v4.len


	value = v4:len()

Returns the length of this vector.



## lua.wetgenes.tardis.v4.lenlen


	value = v4:lenlen()

Returns the length of this vector, squared, this is often all you need 
for comparisons so lets us skip the sqrt.



## lua.wetgenes.tardis.v4.mul


	v4 = v4:mul(v4b)
	v4 = v4:mul(v4b,r)

Multiply v4 by v4b.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.new


	v4 = tardis.v4.new()

Create a new v4 and optionally set it to the given values, v4 methods 
usually return the input v4 for easy function chaining.



## lua.wetgenes.tardis.v4.normalize


	v4 = v4:normalize()
	v4 = v4:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.oo


	v4 = v4:oo()
	v4 = v4:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.scale


	v4 = v4:scale(s)
	v4 = v4:scale(s,r)

Scale this v4 by s.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.sub


	v4 = v4:sub(v4b)
	v4 = v4:sub(v4b,r)

Subtract v4b from v4.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.to_v3


	v3 = v4:to_v3()
	v3 = v4:to_v3(r)

scale [4] to 1 then throw it away so we have a v3 xyz
 
If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.

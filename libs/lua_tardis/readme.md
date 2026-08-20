
# gamecake-tardis

- v0.9
	- initial rocks release.

Lua code documentation auto built from source comments can be found at 
https://xriss.github.io/gamecake/docs/

Technically pure lua since luajit has always proven faster than 
crossing C/Lua code boundaries.

Does build a core of C helper functions and may make more use of this 
core in the future if it ever makes sense. However tardis.lua currently 
works without it.

Uses simple table arrays or luajit ffi fixedsize arrays pretending to 
be tables if available.

So compatible with code that expects a 4 dimensional vector to be a 
{1,2,3,4} lua table.

Quats are {x,y,z,w} as eris intended.

Matrixs are opengl (row-major) style.

Overloads operators, so v4+v4 or v4*2 etc just work as expected, 
probably not the fastest, mostly due to small table memory churn.

May contain bugs but is a good decade or two old at this point so 
hopefully not many.


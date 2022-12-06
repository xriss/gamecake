

module(...,package.seeall)

local glslang=require("glslang")


function test_pp()

	local o=assert( glslang.pp([[

#extension GL_GOOGLE_include_directive : enable

#define test "poop"

#include "unit"
	
]]) )

--print( tostring(o) )


end


function test_lint_gles2()

	assert( glslang.lint_gles2([[

#define test poop

print( test );
	
]]) )



end

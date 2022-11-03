

module(...,package.seeall)

local glslang=require("glslang")


function test_pp()

	assert( glslang.pp([[

#define test poop

print( test );
	
]]) )


end


function test_lint_gles2()

	assert( glslang.lint_gles2([[

#define test poop

print( test );
	
]]) )



end

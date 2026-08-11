
project "lib_opus"

language "C"

--defines { "USE_ALLOCA", "inline=__inline", "OPUS_BUILD" , "HAVE_LRINTF" }
defines { "USE_ALLOCA" , "OPUS_BUILD" , "HAVE_LRINTF" }

includedirs {
	'git/src/',
	'git/celt/',
	'git/silk/',
	'git/silk/float/',
	'git/include/',
}

files {
	'git/src/analysis.c',
	'git/src/mlp.c',
	'git/src/mlp_data.c',
	'git/src/opus.c',
	'git/src/opus_decoder.c',
	'git/src/opus_encoder.c',
	'git/src/repacketizer.c',
	'git/src/extensions.c',
	'git/celt/*.c',
	'git/silk/*.c',
	'git/silk/float/*.c',
}




KIND{}


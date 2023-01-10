
project "lib_opus"

language "C"

--defines { "USE_ALLOCA", "inline=__inline", "OPUS_BUILD" , "HAVE_LRINTF" }
defines { "USE_ALLOCA" , "OPUS_BUILD" , "HAVE_LRINTF" }

includedirs {
	'opus/src/',
	'opus/celt/',
	'opus/silk/',
	'opus/silk/float/',
	'opus/include/',
}

files {
	'opus/src/analysis.c',
	'opus/src/mlp.c',
	'opus/src/mlp_data.c',
	'opus/src/opus.c',
	'opus/src/opus_decoder.c',
	'opus/src/opus_encoder.c',
	'opus/src/repacketizer.c',
	'opus/celt/*.c',
	'opus/silk/*.c',
	'opus/silk/float/*.c',
}




KIND{}


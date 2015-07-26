
project "lib_speexdsp"

language "C"

defines { "HAVE_STDINT_H" , "FLOATING_POINT" , "EXPORT=extern" , "USE_KISS_FFT" }

includedirs {
	'./libspeexdsp/',
	'./include/',
}

files {
	'./libspeexdsp/buffer.c',
	'./libspeexdsp/fftwrap.c',
	'./libspeexdsp/filterbank.c',
	'./libspeexdsp/jitter.c',
	'./libspeexdsp/kiss_fft.c',
	'./libspeexdsp/kiss_fftr.c',
	'./libspeexdsp/mdf.c',
	'./libspeexdsp/preprocess.c',
	'./libspeexdsp/resample.c',
	'./libspeexdsp/scal.c',
	'./libspeexdsp/smallft.c',
}




KIND{}



project "lib_speexdsp"

language "C"

defines { "HAVE_STDINT_H" , "FLOATING_POINT" , "EXPORT=extern" , "USE_KISS_FFT" }

includedirs {
	'speexdsp/libspeexdsp/',
	'speexdsp/include/',
}

files {
	'speexdsp/libspeexdsp/buffer.c',
	'speexdsp/libspeexdsp/fftwrap.c',
	'speexdsp/libspeexdsp/filterbank.c',
	'speexdsp/libspeexdsp/jitter.c',
	'speexdsp/libspeexdsp/kiss_fft.c',
	'speexdsp/libspeexdsp/kiss_fftr.c',
	'speexdsp/libspeexdsp/mdf.c',
	'speexdsp/libspeexdsp/preprocess.c',
	'speexdsp/libspeexdsp/resample.c',
	'speexdsp/libspeexdsp/scal.c',
	'speexdsp/libspeexdsp/smallft.c',
}




KIND{}


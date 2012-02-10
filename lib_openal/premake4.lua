
if not WINDOWS then

project "lib_openal"
kind "StaticLib"
language "C"


files { 
		"soft/OpenAL32/alAuxEffectSlot.c",
		"soft/OpenAL32/alBuffer.c",
		"soft/OpenAL32/alEffect.c",
		"soft/OpenAL32/alError.c",
		"soft/OpenAL32/alExtension.c",
		"soft/OpenAL32/alFilter.c",
		"soft/OpenAL32/alListener.c",
		"soft/OpenAL32/alSource.c",
		"soft/OpenAL32/alState.c",
		"soft/OpenAL32/alThunk.c",
}

files { 
		"soft/Alc/ALc.c",
		"soft/Alc/ALu.c",
		"soft/Alc/alcConfig.c",
		"soft/Alc/alcDedicated.c",
		"soft/Alc/alcEcho.c",
		"soft/Alc/alcModulator.c",
		"soft/Alc/alcReverb.c",
		"soft/Alc/alcRing.c",
		"soft/Alc/alcThread.c",
		"soft/Alc/bs2b.c",
		"soft/Alc/helpers.c",
		"soft/Alc/hrtf.c",
		"soft/Alc/mixer.c",
		"soft/Alc/panning.c",

		"soft/Alc/backends/loopback.c",
		"soft/Alc/backends/null.c",
		"soft/Alc/backends/oss.c",
}


includedirs { ".","soft/include","soft/OpenAL32/Include" }

defines{ "AL_ALEXT_PROTOTYPES" }

KIND{}
--buildoptions {"--verbose"}

end

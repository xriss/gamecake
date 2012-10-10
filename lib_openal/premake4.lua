
if not WINDOWS then

project "lib_openal"
kind "StaticLib"
language "C"


local prefix="soft"

if ANDROID then

	prefix="android"
	
	files { 
			prefix.."/Alc/backends/android.c",
	}
	defines("HAVE_ANDROID")


elseif NACL then

	files { 
			prefix.."/Alc/backends/ppapi.c",
	}
	defines("HAVE_PPAPI")


else

	files { 
			prefix.."/Alc/backends/alsa.c",
	}
	defines("HAVE_ALSA")
	
end




files { 
		prefix.."/OpenAL32/alAuxEffectSlot.c",
		prefix.."/OpenAL32/alBuffer.c",
		prefix.."/OpenAL32/alEffect.c",
		prefix.."/OpenAL32/alError.c",
		prefix.."/OpenAL32/alExtension.c",
		prefix.."/OpenAL32/alFilter.c",
		prefix.."/OpenAL32/alListener.c",
		prefix.."/OpenAL32/alSource.c",
		prefix.."/OpenAL32/alState.c",
		prefix.."/OpenAL32/alThunk.c",
}

files { 
		prefix.."/Alc/ALc.c",
		prefix.."/Alc/ALu.c",
		prefix.."/Alc/alcConfig.c",
		prefix.."/Alc/alcDedicated.c",
		prefix.."/Alc/alcEcho.c",
		prefix.."/Alc/alcModulator.c",
		prefix.."/Alc/alcReverb.c",
		prefix.."/Alc/alcRing.c",
		prefix.."/Alc/alcThread.c",
		prefix.."/Alc/bs2b.c",
		prefix.."/Alc/helpers.c",
		prefix.."/Alc/hrtf.c",
		prefix.."/Alc/mixer.c",
		prefix.."/Alc/panning.c",

		prefix.."/Alc/backends/loopback.c",
		prefix.."/Alc/backends/wave.c",
		prefix.."/Alc/backends/null.c",

}


includedirs { ".",prefix.."/include",prefix.."/OpenAL32/Include" }

defines{ "AL_ALEXT_PROTOTYPES" }

KIND{}
--buildoptions {"--verbose"}

end

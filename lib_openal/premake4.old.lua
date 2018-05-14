
project "lib_openal"
kind "StaticLib"
language "C"


local prefix="asoft"


if ANDROID then
	
	files { 
			prefix.."/Alc/backends/opensl.c",
	}
	defines("HAVE_OPENSL")
	defines{"HAVE_FENV_H","HAVE_FESETROUND","HAVE_DLFCN_H","HAVE_PTHREAD_SETSCHEDPARAM"}


elseif WINDOWS then

	files { 
			prefix.."/Alc/backends/winmm.c",
	}
	defines("HAVE_WINMM")

elseif RASPI then

	files { 
--			prefix.."/Alc/backends/alsa.c",
			prefix.."/Alc/backends/pulseaudio.c",
	}
--	defines("HAVE_ALSA")
	defines("HAVE_PULSEAUDIO")

	defines{"HAVE_FENV_H","HAVE_FESETROUND","HAVE_DLFCN_H","HAVE_PTHREAD_SETSCHEDPARAM"}
	includedirs { "./raspi/include" } -- need some extraincludes

elseif OSX then

	files { 
			prefix.."/Alc/backends/CoreAudio.c",
	}
	defines("HAVE_COREAUDIO")
	defines{"HAVE_FENV_H","HAVE_FESETROUND","HAVE_DLFCN_H","HAVE_PTHREAD_SETSCHEDPARAM"}
	
else

	files { 
--			prefix.."/Alc/backends/alsa.c",
			prefix.."/Alc/backends/pulseaudio.c",
	}
--	defines("HAVE_ALSA")
	defines("HAVE_PULSEAUDIO")

	defines{"HAVE_FENV_H","HAVE_FESETROUND","HAVE_DLFCN_H","HAVE_PTHREAD_SETSCHEDPARAM"}

end


if prefix=="asoft" then
	files { 
			prefix.."/Alc/alcDedicated.c",
			prefix.."/Alc/helpers.c",
			prefix.."/Alc/hrtf.c",
			prefix.."/Alc/backends/loopback.c",
			prefix.."/Alc/backends/wave.c",
			prefix.."/Alc/backends/null.c",
	}
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
		prefix.."/Alc/ALc.c",
		prefix.."/Alc/ALu.c",
		prefix.."/Alc/alcConfig.c",
		prefix.."/Alc/alcEcho.c",
		prefix.."/Alc/alcModulator.c",
		prefix.."/Alc/alcReverb.c",
		prefix.."/Alc/alcRing.c",
		prefix.."/Alc/alcThread.c",
		prefix.."/Alc/bs2b.c",
		prefix.."/Alc/mixer.c",
		prefix.."/Alc/panning.c",
}


includedirs { ".",prefix.."/include",prefix.."/OpenAL32/Include" }

defines{ "AL_ALEXT_PROTOTYPES" }
defines( "AL_LIBTYPE_STATIC" )

KIND{}
--buildoptions {"--verbose"}

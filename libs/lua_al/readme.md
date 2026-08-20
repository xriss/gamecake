
# gamecake-al

- v0.9
	- initial rocks release.

Lua code documentation auto built from source comments can be found at 
https://xriss.github.io/gamecake/docs/

Provides a binding to openal and openalc in the modules wetgenes.al and 
wetgenes.alc

OpenAL is the audio equivelent of OpenGL so provides a means of playing 
audio in a cross platform way. Works on linux/windows/android and web 
via emscripten.

Sorry about lack of documentation, this is an old hack and needs work 
but you can expect AL_* functions etc to be available via wetgenes.al.* 
mostly.

rocks version expects a system openal, gamecake internal version uses 
mojoal to wrap SDL as a generic openal provider.


# gamecake-gles

- v0.9
	- initial rocks release.

Lua code documentation auto built from source comments can be found at 
https://xriss.github.io/gamecake/docs/

Provides a binding to opengl in the modules wetgenes.gles and 
wetgenes.glescode using gl3w for maximum compatibility.

gles may be gles1 or gles2/gles3 depending on your opengl. It will 
probably be gles3 but this originally worked on old phones.

glescode wraps gles to provide helper functions.

Sorry about lack of documentation, this is an old hack and needs work 
but you can expect GL_* functions etc to be available via wetgenes.gles.* 
mostly.



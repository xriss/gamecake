


---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.gamecake.framebuffers


Deal with FrameBuffers as render targets and textures. Color and Depth 
buffers need to be allocated and managed.

So we need to be baked within an oven so we will talk about the return 
from the bake function rather than the module function.

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local fbo=framebuffers.create(256,256,0)	-- a 256x256 texture only
	local fbo=framebuffers.create(256,256,-1)	-- a 256x256 depth only
	local fbo=framebuffers.create(256,256,1)	-- a 256x256 texture and depth
	local fbo=framebuffers.create()				-- 0x0 and we will resize later



## lua.wetgenes.gamecake.framebuffers.create


	local fbo=framebuffers.dirty()
	local fbo=framebuffers.dirty(x,y)
	local fbo=framebuffers.dirty(x,y,framebuffers.NEED_TEXTURE_AND_DEPTH)
	local fbo=framebuffers.dirty(0,0,0,{
		depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT,gl.FLOAT},
		texture_format={gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE},
		TEXTURE_MIN_FILTER=gl.LINEAR,
		TEXTURE_MAG_FILTER=gl.LINEAR,
		TEXTURE_WRAP_S=gl.CLAMP_TO_EDGE,
		TEXTURE_WRAP_T=gl.CLAMP_TO_EDGE,
	}) -- note this table will be returned as the fbo

Create a new framebuffer object and optionally provide an inital size 
and depth. The depth can use -1,0,1 or the following verbose flags 
framebuffers.NEED_DEPTH,framebuffers.NEED_TEXTURE or 
framebuffers.NEED_TEXTURE_AND_DEPTH to request a depth buffer(1,-1) or not(0). 

Finally you can pass in a table to be returned as the fbo that contains 
defaults or set defaults in the fbo that is returned.

	fbo.depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT,gl.FLOAT}

Can be used to control exactly how a depth buffer is allocated with 
gl.TexImage2D when you care about that sort of thing, IE hardware 
issues.

	fbo.texture_format={gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE}

Can be used to control exactly how a texture buffer is allocated with 
gl.TexImage2D when you care about that sort of thing, IE hardware 
issues.

	fbo.TEXTURE_MIN_FILTER=gl.LINEAR
	fbo.TEXTURE_MAG_FILTER=gl.LINEAR
	fbo.TEXTURE_WRAP_S=gl.CLAMP_TO_EDGE
	fbo.TEXTURE_WRAP_T=gl.CLAMP_TO_EDGE

These can be used to control default TexParameters in the fbo.

	fbo.no_uptwopow=true
	
By deafult we will use power of 2 sizes for the fbo that fit the 
requested size. This disables that and doing so will of course cause 
problems with some hardware. Generally if you avoid mipmaps it probably 
wont be a problem.

See #lua.wetgenes.gamecake.framebuffers.fbo for all the functions you 
can call on the fbo returned.



## lua.wetgenes.gamecake.framebuffers.dirty


	framebuffers.dirty()

Mark all framebuffer objects as dirty by setting fbo.dirty to be true. We do not 
do anything with this flag but it is used in external code and this is 
a useful helper function to set the flag.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_depth


	fbo:bind_depth()

BindTexture the depth texture part of this fbo.

If there is no texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_depth_snapshot


	fbo:bind_depth_snapshot()

BindTexture the depth snapshot texture part of this fbo.

If there is no snapshot texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_frame


	fbo:bind_depth()

BindFramebuffer the framebuffer of this fbo.

If there is no framebuffer we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_texture


	fbo:bind_texture()

BindTexture the rgba texture part of this fbo.

If there is no texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_texture_snapshot


	fbo:bind_texture_snapshot()

BindTexture the rgba snapshot texture part of this fbo.

If there is no snapshot texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.check


	fbo:check()

Check and allocatie if missing and needed (eg depth texture may not be 
needed) all our openGL buffers.



## lua.wetgenes.gamecake.framebuffers.fbo.clean


	fbo:clean()

Free all the opengl buffers.



## lua.wetgenes.gamecake.framebuffers.fbo.download


	fbo:download()
	fbo:download(w,h,x,y)

Read back color data from a framebuffer and return it in an RGBA 
PREMULT grd object (which is probably what it is).

If a width,height is given then we will read the given pixels only from 
the x,y location.



## lua.wetgenes.gamecake.framebuffers.fbo.free_depth


	fbo:free_depth()

Free the depth texture only, is safe to call if there is no depth 
buffer.



## lua.wetgenes.gamecake.framebuffers.fbo.free_frame


	fbo:free_frame()

Free the frame buffer only, is safe to call if there is no frame 
buffer.



## lua.wetgenes.gamecake.framebuffers.fbo.free_snapshot


	fbo:free_snapshot()

Free the snapshot buffers, fbo.texture_snapshot and fbo.depth_snapshot 
if they exist.



## lua.wetgenes.gamecake.framebuffers.fbo.free_texture


	fbo:free_texture()

Free the rgba texture only, is safe to call if there is no rgba 
buffer.



## lua.wetgenes.gamecake.framebuffers.fbo.mipmap


	fbo:mipmap()

Build mipmaps for all existing texture buffers.



## lua.wetgenes.gamecake.framebuffers.fbo.mipmap_depth


	fbo:mipmap_depth()

Build mipmaps for our depth buffer if it exists and set 
TEXTURE_MIN_FILTER to LINEAR_MIPMAP_LINEAR so it will be used.

It is possible this may fail (hardware issues) and the 
TEXTURE_MIN_FILTER be reset to gl.NEAREST along with a flag to stop us 
even trying in the future,



## lua.wetgenes.gamecake.framebuffers.fbo.mipmap_texture


	fbo:mipmap_texture()

Build mipmaps for our texture buffer if it exists and set 
TEXTURE_MIN_FILTER to LINEAR_MIPMAP_LINEAR so it will be used.



## lua.wetgenes.gamecake.framebuffers.fbo.pingpong


	fbo:pingpong(fbout,shadername,callback)
	framebuffers.pingpong({fbin1,fbin2},fbout,shadername,callback)

Render from one or more fbos into another using a fullscreen shader.

Sometime you need to repeatedly copy a texture back and though applying 
a shader, this is the function for you.

The textures will be bound to tex1,tex2,tex3,etc and the uvs supplied 
in a_texcoord with a_vertex being set to screen coords so can be used 
as is.



## lua.wetgenes.gamecake.framebuffers.fbo.render_start


	fbo:render_start()


Start rendering into this fbo.

Push old matrix and set the matrix mode to MODELVIEW

Set fbo.view and reset the gl state.



## lua.wetgenes.gamecake.framebuffers.fbo.render_stop


	fbo:render_stop()

Stop rendering into this fbo and restore the last fbo so these calls 
can be nested.

Restore the old view and old gl state.

Pop old matrix and set the matrix mode to MODELVIEW



## lua.wetgenes.gamecake.framebuffers.fbo.resize


	fbo:resize(w,h,d)

Change the size of our buffers, which probably means free and then
reallocate them.

The w,h,d use the same rules as framebuffer.create



## lua.wetgenes.gamecake.framebuffers.fbo.snapshot


	fbo:snapshot()

Take a current snapshot copy of the texture and depth if they exist, 
store them in fbo.texture_snapshot and fbo.depth_snapshot for later 
binding.



## lua.wetgenes.gamecake.framebuffers.start


	framebuffers.start()

Called as part of the android life cycle, since we auto reallocate this 
does not actually have to do anything. But may do a forced resize in 
the future if that turns out to be more hardware compatible.



## lua.wetgenes.gamecake.framebuffers.stop


	framebuffers.stop()

Called as part of the android life cycle, we go through and call 
fbo.clean on each fbo to free the opengl resources.



## lua.wetgenes.gamecake.oven


	oven=require("wetgenes.gamecake.oven").bake(opts)

The oven module must be baked so only exposes a bake function.

All the other functions are returned from within the bake function.

possible ENV settings

	gamecake_tongue=english
	gamecake_flavour=sdl



## lua.wetgenes.gamecake.oven.bake



	oven=wetgenes.gamecake.oven.bake(opts)

Bake creates an instance of a lua module bound to a state. Here we 
are creating the main state that other modules will then bind to.

We call each state an OVEN to fit into the gamecake naming scheme 
then we bake a module in this oven to bind them to the common state.

Think of it as a sub version of require, so require gets the global 
pointer for a module and bake is used to get the a module bound to 
an oven.

By using this bound state we reduce the verbosity of connecting 
modules and sharing state between them.




## lua.wetgenes.gamecake.toaster


	oven=require("wetgenes.gamecake.toaster").bake(opts)

A cut down oven without opengl or even file access intended to be used 
in a sub process or task.



## lua.wetgenes.gamecake.toaster.bake


	oven=wetgenes.gamecake.toaster.bake(opts)

Bake creates an instance of a lua module bound to an oven state. Here 
we are creating the main state that other modules will then bind to.

Modules are then bound together using rebake...

	b=oven.rebake(nameb)
	c=oven.rebake(namec)

All of these will be connected by the same oven and circular 
dependencies should work with the caveat that just because you have the 
table for a baked module does not mean that it has all been filled in 
yet.



## lua.wetgenes.gamecake.toaster.newticks


	ticks=require("wetgenes.gamecake.toaster").newticks(rate,jump)

create a ticks time controller for updates.

rate is time between steps , jump is how far behind we should get 
before giving up ( eg callback takes longer than rate allows ) these 
values are in seconds and default to 1/60 and 1 respectively.

Then we perform one or more update steps like so

	ticks.step( callback ) -- callback may be called zero or more times

After that we should wait a bit (maybe draw a frame) and then call 
again.

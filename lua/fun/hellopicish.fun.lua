--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

hardware,main=system.configurator({
	mode="picish", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
})


update=function()

	local canvas=system.components.canvas

	canvas.dirty(true)
	canvas.clear( 9 )
	canvas.color( system.ticks%32 )
	canvas.text( "Hello World!" , (128-(12*4))/2 , (128-8)/2 )

end

--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

coroutine.yield() -- wait for picish system components to exist

local canvas=system.components.canvas

while true do

	canvas.dirty(true)
	canvas.clear( 9 )
	canvas.color( system.ticks%32 )
	canvas.text( "Hello World!" , (128-(12*4))/2 , (128-8)/2 )

	while not coroutine.yield().draw do end -- wait for next frame
end

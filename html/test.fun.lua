--
-- This is fun64 code, you can copypasta into https://xixs.com/wasm/swed.html to run it.
--
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
})


-- we will call this once in the update function
setup=function()

-- remove base screen effects?
--	system.components.screen.bloom=0
--	system.components.screen.filter=nil
--	system.components.screen.shadow=nil
	
	print("Setup complete!")

end


-- updates are run at 60fps
update=function()
	
	if setup then setup() setup=nil end

	local ctext=system.components.text
	local bg=9
	local fg=system.ticks%32 -- cycle the foreground color

	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color
	
	ctext.text_print("Hello World!",34,15,fg,bg) -- (text,x,y,color,background)
	
end

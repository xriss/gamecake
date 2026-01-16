--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

oven.opts.fun="" -- back to menu on reset

sysopts={
	mode="swordstone", -- select a characters+sprites on a 256x128 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
	hx=256,hy=128, -- minimum size
	hxhy="best", -- auto set hx,hy size to available space growing upto 2x bigger
}

hardware,main=system.configurator(sysopts)


update=function()

    if setup then setup() setup=nil end

    local ctext=system.components.text
    local bg=9
    local fg=system.ticks%32 -- cycle the foreground color

	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color
	
	ctext.text_print("Hello World!",(256/4-12)/2,(128/8)/2,fg,bg) -- (text,x,y,color,background)

end

--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
})

local wstr=require("wetgenes.string")


-- we will call this once in the update function
setup=function()

--    system.components.screen.bloom=0
--    system.components.screen.filter=nil
--    system.components.screen.shadow=nil
    
    print("Setup complete!")

end


-- updates are run at 60fps
update=function()
    
    if setup then setup() setup=nil end

    local cmap=system.components.colors.cmap
    local ctext=system.components.text
    local bg=9
    local fg=31

	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color

    for y=0,15 do
        for x=0,1 do
            local n=16*x + y
            local web=math.floor(cmap[n].bgra/16)%16 + math.floor(cmap[n].bgra/(16*256))%16*16 + math.floor(cmap[n].bgra/(16*256*256))%16*256
            local s=string.format("%3d          %2s   %03X   %-16s",n,cmap[n].code or "",web,cmap[n].name or "")
        	ctext.text_print(s,x*40+1,y+2,fg,bg) -- (text,x,y,color,background)
        	ctext.text_print("    ",x*40+1+6,y+2,fg,n) -- (text,x,y,color,background)
        end
    end

	local tx=wstr.trim([[

This is a data dump of the ]]..tostring(cmap.name)..[[ palette.

First column is the color index number.
Second column is the color.
Third column is the color code used in bitdown ascii graphics.
Fourth column is the hex RGB that can be used on the web.
Fifth column is the name of the color.

Colors may be referenced by their index number or name.


]]) -- :gsub("\n"," ")

	local tl=wstr.smart_wrap(tx,system.components.text.text_hx-6)
	for i=0,system.components.text.tilemap_hy-1 do
		local t=tl[i+1]
		if not t then break end
		system.components.text.text_print(t,3,19+i,fg,bg)
	end


end

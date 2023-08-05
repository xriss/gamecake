--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--
oven.opts.fun="" -- back to menu on reset
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update
	draw=function() draw() end, -- called repeatedly to draw
})

local wstr=require("wetgenes.string")


-- we will call this once in the update function
setup=function()

--    system.components.screen.bloom=0
--    system.components.screen.filter=nil
--    system.components.screen.shadow=nil
    
    print("Setup complete!")

end


lines={}

-- updates are run at 60fps
update=function()
    
    if setup then setup() setup=nil end
end


draw=function()
    local ccmap=system.components.colors.cmap
    local cmap=system.components.map
    local ctext=system.components.text
    local bg=9
    local fg=31

    cmap.text_clear(0x01000000*bg) -- clear text forcing a background color
	
	
    local y=1
	
    for i=0,6 do
	local up=ups(i)
	
	local ns={
	    "up","down","left","right","fire",				-- basic joystick, most buttons map to fire
	    "pad_up","pad_down","pad_left","pad_right",			-- the d-pad directions
	    "x","y","a","b",						-- the four face buttons
	    "l1","l2","l3","r1","r2","r3",				-- the triggers and stick clicks
	    "select","start","guide",					-- the menu face buttons
	    "mouse_left","mouse_right","mouse_middle",			-- mouse buttons
	    "touch",							-- touch buttons
	    }
	    
-- left stick and trigger		lx ly lz
-- right stick and trigger		rx ry rz
-- dpad							dx dy
-- mouse absolute				px py
-- mouse relative				mx my
-- touch						tx ty
	local ax={"lx","ly","lz","rx","ry","rz","dx","dy","px","py","mx","my","tx","ty"} -- axis name
	
	local a={}
		
	for i,n in ipairs(ax) do
	    local v=up.axis(n)
	    if v and v~=0 then
		a[#a+1]=n.."="..math.floor(v+0.5)
	    end
	end

	for i,n in ipairs(ns) do
	    if up.button(n.."_set") then a[#a+1]=n.."_set" end -- value was set this frame
	    if up.button(n.."_clr") then a[#a+1]=n.."_clr" end -- value was cleared this frame
	    if up.button(n) then a[#a+1]=n end                 -- current value
	end

	local s=i.."up : "..table.concat(a," ")
	ctext.text_print(s,2,y,fg,0) y=y+1

	y=y+1
    end

	local up=ups(1)
	for i,v in ipairs(up.state_keys) do		-- key presses and repeat key presses
		lines[#lines+1]="key  "..v
    end
	for i,v in ipairs(up.state_text) do		-- and decoded to UTF8 text
		lines[#lines+1]="text "..v
    end
    
    while #lines > 14 do table.remove(lines,1) end
    
    for i=1,#lines do
	ctext.text_print(lines[i],1,y,fg,0)
	y=y+1
    end


end

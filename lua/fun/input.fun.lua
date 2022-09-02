--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update
	draw=function() draw() end, -- called repeatedly to draw
	msg=function(m) msg(m) end, -- handle msgs
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

-- handle raw key press
msg=function(m)


--print(wstr.dump(m))

    local s
			    
    if m.class=="mouse" then
	s=string.format("%6.2f %8s %2d %3d,%3d %s %s",m.time,m.class,m.action,m.x,m.y,tostring(m.keycode or ""),m.keyname or "")

    elseif m.class=="touch" then
	s=string.format("%6.2f %8s %2d %3d,%3d %3d %3d",m.time,m.class,m.action,m.x,m.y,m.id or 0,m.pressure or 0)

    elseif m.class=="padaxis" then
	s=string.format("%6.2f %8s %2d %8s %5d %3d",m.time,m.class,m.id or 0,m.name,m.value,m.code)

    elseif m.class=="padkey" then
	s=string.format("%6.2f %8s %2d %8s %3d %3d",m.time,m.class,m.id or 0,m.name,m.value,m.code)

    elseif m.class=="key" then
	s=string.format("%6.2f %8s %2d %3s %16s",m.time,m.class,m.action,m.ascii,m.keyname)

    else
	s=string.format("%6.2f %8s %2d",m.time or 0,m.class,m.action or 0)
    end

    lines[#lines+1]=s
    
    while #lines > 14 do table.remove(lines,1) end

end


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
	    
	local ax={"lx","ly","lz","rx","ry","rz","dx","dy","mx","my","tx","ty"} -- axis name
	
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

    for i=1,#lines do
	ctext.text_print(lines[i],1,y,fg,0)
	y=y+1
    end


end



local _G=_G

local win=win

local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

-- a rogue like

local unpack=unpack

local tostring=tostring

local require=require

local gl=gl



local function print(...) _G.print(...) end



local _M=module("state.rouge")


yarn=require("yarn")

function setup()

	yarn.setup()
	
	print("setup")

end

function keypress(ascii,key,act)

	if yarn.menu.keypress(ascii,key,act) then -- give the menu first chance to eat this keypress
		yarn.level.key_clear() -- stop level repeats
	else
		yarn.level.keypress(ascii,key,act)
	end
end


function mouse(act,x,y,key)

	if act=="down" then

	end

end


	
function clean()
	yarn.clean()
end



function update()

	yarn.update()
	
end

function asc_print(x,y,s)

	local id=1+x+y*asc_xh
	
	for i=1,#s do
	
		if asc[id] then
		
			asc[id]=string.byte(s,i)
		
		end
		
		id=id+1
	
	end
	
end

function asc_draw_box(x,y,xh,yh)

	local sc=string.rep("*",xh)
	asc_print(x,y,sc)
	for i=1,yh-2 do
		local s="*"..string.rep(" ",xh-2).."*"
		asc_print(x,y+i,s)
	end
	asc_print(x,y+yh-1,sc)
	
	
end


-- wrap a string to a given width
function word_wrap(s,w)

	s=s or ""
	local t={}

	while s~="" do
	
		if not s or s=="" then break end -- end of input
		
		local r
		
		if #s<=w or s:byte(w+1)==32 then -- perfect split
		
			r=s:sub(1,w)
			s=s:sub(w+2)
			
		else
		
			local split_at=1
			
			for i=w,1,-1 do
				if s:byte(i)==32 then -- found last space on this line
					split_at=i
					break
				end
			end
			
			if split_at==1 then -- no space no split
				r=s:sub(1,w)
				s=s:sub(w+1)
			else
				r=s:sub(1,split_at-1)
				s=s:sub(split_at+1)
			end
			
		end
		
		table.insert(t,r) -- building a table of lines each one of w or less length
		
	end
	
	return t
end

function draw()

	yarn.draw()

	gl.ClearColor(0,0,0.25,0)
	win.begin()
	
	win.project23d(480/640,1,1024)
	gl.MatrixMode("MODELVIEW")
	gl.LoadIdentity()
					
local i=0
local t={}
--[[
	
	for y=0,asc_yh-1 do
		for x=0,asc_xh-1 do
			i=1+x+y*asc_xh
			local a=yarn.level.get_asc(x,y)
			if a then asc[i]=a end
		end
	end
	
	function prt(y,s)
		s=tostring(s)
		asc_print(30,y,"----------")
		asc_print(30+math.floor((10-#s)/2),y,s)
	end
	function prt_wide(y,s)
		s=tostring(s or "")
		asc_print(0,y,"                                        ")
		asc_print(math.floor((40-#s)/2),y,s)
	end

	local s=""
	
	s=s.." hp="..yarn.level.player.attr.hp .."/".. yarn.level.player.attr.hpmax 
	s=s.." s="..yarn.level.player.attr.score
	s=s.." t="..yarn.level.time_passed
--[ [
	prt(1,"score")
	prt(2,level.player.attr.score)
	
	prt(4,"time")
	prt(5,level.time_passed)
	
	prt(7,"health")
	prt(8, level.player.attr.hp .."/".. level.player.attr.hpmax )
] ]	
--	prt_wide(0,s)
	
	local wrap=word_wrap(yarn.level.get_msg(),40)
	
	yarn.menu.draw()
	
	if #wrap<1 then
		prt_wide(29,"")
	elseif #wrap<2 then
		prt_wide(28,"")
	end
	for i=30-#wrap,29 do
		prt_wide(i,wrap[i-(29-#wrap)])
	end
	
]]	
	gl.Translate(-320,240, -240)
		
	for y=0,yarn.asc_yh-1 do
	
		win.font_debug.set(0,y*-16,0xffffffff,16)
		
		for x=0,yarn.asc_xh-1 do
		
			i=1+x+y*yarn.asc_xh
			t[x+1]=yarn.asc[i]%256
			
		end
		
		local s=string.char(unpack(t))
		
		win.font_debug.draw(s)
		
	end
	
	
--	win.font_debug.set(0,0,0xffffffff,1.6)
--	win.font_debug.draw("asdasdasdsd")
		
end



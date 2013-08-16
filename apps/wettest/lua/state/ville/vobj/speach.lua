
--
-- This is the relocation part of a vobj, not all vobjs need to move so not all vobjs need this
--

local math=math

local table=table
local ipairs=ipairs
local coroutine=coroutine
local error=error
local require=require
local print=print

local win=win

local gl=require("gl")
local tardis=require("wetgenes.tardis")
local comm=require("spew.client.comm")

module(...)
local vobj=require("state.ville.vobj")



function setup(it,tab)

	it.say=say
	
	it.speach={}
	local sp=it.speach
	
	sp.lines=nil
	
--	say(it,"Hello world, this is a long string that should wrap and stuff like that")
	
	return it
end


function clean(it)
end


function change(it,tab)

end


function update(it)
	local sp=it.speach

	if not sp.lines then return end
	
	sp.time=sp.time-1
	sp.alpha=255*(sp.time/50)
	sp.alpha=math.floor(sp.alpha)
	if sp.alpha>255 then sp.alpha=255 end
	if sp.alpha<0 then
		sp.alpha=0
		sp.lines=nil
	end
	
end

function draw(it)

	local sp=it.speach
	
	if not sp.lines then return end

	gl.PushMatrix()

	local p=it.loc.xyz
	gl.Translate(p[1]-(12*16),p[2]+100+(#sp.lines*16),p[3]+40)
	
	for i,v in ipairs(sp.lines) do
		win.font_debug.set((24-#v)*8,(i-1)*-16,0xffffff+(sp.alpha*0x1000000),16,16)	
		win.font_debug.draw(v)
	end
	
	gl.PopMatrix()
	
end


function say(it,str)

	local sp=it.speach
	
	sp.lines=word_wrap(str,24)
	sp.time=400

end


-- wrap a string to a given character width
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

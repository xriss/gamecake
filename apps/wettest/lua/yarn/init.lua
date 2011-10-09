

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug

module(...)
local yarn=require("yarn")
local yarn_level=require("yarn.level")
local yarn_menu=require("yarn.menu")
local yarn_attrs=require("yarn.attrs")

local strings=require("yarn.strings")

local a_space=string.byte(" ",1)
local a_under=string.byte("_",1)
local a_star=string.byte("*",1)
local a_hash=string.byte("#",1)
local a_dash=string.byte("-",1)
local a_dot=string.byte(".",1)

asc={}
asc_xh=0
asc_yh=0	

level={}
menu={}

soul={}

function setup(sd,arg)


local i

	asc_xh=40
	asc_yh=30
	for y=0,asc_yh-1 do
		for x=0,asc_xh-1 do
		
			i=1+x+y*asc_xh
			
			asc[i]=a_space
		end
	end
	
	menu=yarn_menu.create({},yarn)
	
	if arg[1] then -- debug this level
		local pow=tonumber(arg[2] or 1) or 1
		level=yarn_level.create(yarn_attrs.get("level."..arg[1],pow,{xh=40,yh=28}),yarn)
		
	else
	
		level=yarn_level.create(yarn_attrs.get("level.home",1,{xh=40,yh=28}),yarn)
	
	end

	
	for y=0,asc_yh-1 do
		for x=0,asc_xh-1 do
			i=1+x+y*asc_xh
			local a=level.get_asc(x,y)
			if a then asc[i]=a end
		end
	end
	
--	level.draw_map(_M)
	
	level.set_msg("Welcome to the jungle.")
	
	local f=level.can["read welcome"]
	if f then f(level) end

end

function keypress(ascii,key,act)

	if menu.keypress(ascii,key,act) then -- give the menu first chance to eat this keypress
		level.key_clear() -- stop level repeats
	else
		level.keypress(ascii,key,act)
	end
end


function mouse(act,x,y,key)

	if act=="down" then

	end

end


	
function clean()
end



function update()

	return level.update() + menu.update()
	
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


function draw(charwidth)

local i=0
local t={}

	
	for y=0,asc_yh-1 do
		for x=0,asc_xh-1 do
			i=1+x+y*asc_xh
			local a=level.get_asc(x,y)
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


	local wrap=strings.smart_wrap(level.get_msg(),40)
	
	menu.draw()
	
	if #wrap<1 then
		prt_wide(29,"")
	elseif #wrap<2 then
		prt_wide(28,"")
	end
	for i=30-#wrap,29 do
		prt_wide(i,wrap[i-(29-#wrap)])
	end
	
	
	local ret={}
	for y=0,asc_yh-1 do

		for x=0,asc_xh-1 do
		
			i=1+x+y*asc_xh
--			t[x+1]=asc[i]%256

			if charwidth==2 then
			
				t[x*2+1]=asc[i]%256
				t[x*2+2]=32
			
			else
			
				t[x+1]=asc[i]%256
			
			end
		end
		
		local s=string.char(unpack(t))
		
		ret[#ret+1]=s
		ret[#ret+1]="\n"
--print(s)
		
	end
	
	return table.concat(ret)
	
end


-- create a save state for this data
function save()
	local sd={}
	sd.soul=soul -- our soul
	sd.levels={}
	sd.levels["home"]={}
	sd.levels["home"][1]=level.save()
	return sd
end

-- reload a saved data (create and then load)
function load(sd)
end


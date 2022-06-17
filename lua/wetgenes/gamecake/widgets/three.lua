--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wthree)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wthree=wthree or {}

function wthree.update(widget)
	return widget.meta.update(widget)
end

function wthree.draw(widget)
	return widget.meta.draw(widget)
end

-- this is a magic layout that *assumes* only three children
-- 1st is placed on the left, 3rd is placed on the right and the 2nd(middle) one takes up all the space remaining in the middle
-- useful for left/right aligning content and can also be used vertically

function wthree.layout(widget)
	
	local h1,p1,h2,p2,s1,s2
	if		widget.three_axis=="x"	then	h1="hx" p1="px" h2="hy" p2="py" s1="sx" s2="sy"
	elseif	widget.three_axis=="y"	then	h1="hy" p1="py" h2="hx" p2="px" s1="sy" s2="sx"
	end

	local w1=0
	local w3=0

	local v=widget[1]
	if v then
		
		if v.hidden then
		
			v.px=0
			v.py=0
			v.hy=0
			v.hx=0

		else

			v.px=0
			v.py=0
			v[h2]=widget[h2]
		end
		
		w1=v[h1]
	end

	local v=widget[3]
	if v then
		
		w3=v[h1]

		if v.hidden then
		
			v.px=0
			v.py=0
			v.hy=0
			v.hx=0
			
			w3=v[h1]

		else

			v[p1]=widget[h1]-w3
			v.py=0
			v[h2]=widget[h2]
			
		end

	end

	local v=widget[2]
	if v then
		
		v[p1]=w1
		v[h1]=widget[h1]-(w1+w3)
		v[h2]=widget[h2]

	end

-- also layout any other children
	widget.meta.layout(widget,4)

end

function wthree.setup(widget,def)

	widget.class="three"
	
	widget.three_axis=def.three_axis  or "x" 	-- or "y"		three across x or y axis
	
	widget.update=wthree.update
	widget.draw=wthree.draw
	widget.layout=wthree.layout
	
	return widget
end

return wthree
end

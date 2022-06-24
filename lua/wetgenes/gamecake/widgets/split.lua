--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wsplit)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wsplit=wsplit or {}

function wsplit.update(widget)
	return widget.meta.update(widget)
end

function wsplit.draw(widget)
	return widget.meta.draw(widget)
end

-- this is a magic layout that *assumes* only two children
-- one has a controlled size and the other takes up whats left of the space
-- complicated layouts can be achieved by a split being a child of another split

function wsplit.resizelayout(widget)
	
-- work out the size we want for this split using a number of limiters
-- a,b may be x,y or y,x depending on the split axis
	local gsiz=function(a,b,c)
		local r=c or 0
		
		local fit=widget.split_fit

		if type(fit)=="table" then -- work out the sub widget that needs to fit
			local it=widget
			for i,v in ipairs(fit) do
				if it and it[v] then it=it[v] end 
			end
			fit=widget.split_scale or 1

			if it and it.hx>0 and it.hy>0 then -- sanity checks
				if widget.split_axis=="x" then
					fit=math.ceil(fit*it.hx/it.hy)
				elseif widget.split_axis=="y" then
					fit=math.ceil(fit*it.hy/it.hx)
				end
			end
		end
		
		if		widget.split_num	then	r=widget.split_num
		elseif	fit					then	r=fit*b
		elseif	widget.split_fnum	then	r=widget.split_fnum*a
		elseif	widget.split_aspect	then	r=widget.split_aspect*b
		end
		
		if widget.split_fmin then local n=math.ceil(widget.split_fmin*a) if r<n then r=n end end
		if widget.split_min then if r<widget.split_min then r=widget.split_min end end

		if widget.split_fmax then local n=math.ceil(widget.split_fmax*a) if r>n then r=n end end
		if widget.split_max then if r>widget.split_max then r=widget.split_max end end

		if r<0 then r=0 end -- sanity
		if r>a then r=a end -- never bigger than container
		r=math.ceil(r) -- snap to ints
		
		return r
	end

	local s1=widget.split_order
	local s2=3-widget.split_order
	
	if widget[s1] then
		local v=widget[s1]
		
		if v.hidden then -- when hidden the other one becomes full sizes
		
			v.px=0
			v.py=0
			v.hy=0
			v.hx=0
			if widget[s2] then
				local w=widget[s2]
				w.px=0
				w.py=0
				w.hy=widget.hy or 0
				w.hx=widget.hx or 0
			end

		elseif widget.split_axis=="x" then
		
			v.px=0
			v.py=0
			v.hy=widget.hy
			v.hx=gsiz(widget.hx,widget.hy,v.hx)

			if widget[s2] then
				local w=widget[s2]
				w.px=0
				w.py=0
				w.hy=widget.hy
				w.hx=widget.hx-v.hx
			end

		elseif widget.split_axis=="y" then
		
			v.px=0
			v.py=0
			v.hx=widget.hx
			v.hy=gsiz(widget.hy,widget.hx,v.hy)

			if widget[s2] then
				local w=widget[s2]
				w.px=0
				w.py=0
				w.hx=widget.hx
				w.hy=widget.hy-v.hy
			end

		end

-- first part has already been positioned so here we position the second part at far edge
		if widget[2] then
			local v=widget[2]
			if		widget.split_axis=="x"	then	v.px=widget.hx-v.hx
			elseif	widget.split_axis=="y"	then	v.py=widget.hy-v.hy
			end
		end

	end
end

function wsplit.resize(widget)

	wsplit.resizelayout(widget)

-- also layout any other children
	widget.meta.resize(widget)

end

function wsplit.layout(widget)

	wsplit.resizelayout(widget)

-- also layout any other children
	widget.meta.layout(widget)

end

function wsplit.setup(widget,def)

	widget.class="split"
	
	widget.split_axis =widget.split_axis  or "x" 	-- or "y"		split across x or y axis
	widget.split_order=widget.split_order or 1 	-- or 2			control the size of the 1st or 2nd subwidget
	
--[[
-- size (in order of precedence if they exist otherwise we just use the given size of the child widget
	widget.split_num  =widget.split_num			-- fixed pixel size of split
	widget.split_fit  =widget.split_fit			-- desired aspect ratio of split , width*this == height or height*this=width
	widget.split_fnum =widget.split_fnum			-- fixed fractional size of split
	widget.split_scale=widget.split_scale			-- scale the fit by this amount (must be <=1)
	widget.split_aspect=widget.split_aspect		-- keep the height relative to the width or visa versa

-- limits
	widget.split_min =widget.split_min		-- minimum pixel size of split
	widget.split_max =widget.split_max		-- maximum pixel size of split
	widget.split_fmin=widget.split_fmin	-- minimum fractional size of split
	widget.split_fmax=widget.split_fmax	-- maximum fractional size of split
]]

	widget.update=wsplit.update
	widget.draw=wsplit.draw
	widget.resize=wsplit.resize
	widget.layout=wsplit.layout
	
	return widget
end

return wsplit
end

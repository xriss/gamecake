-- widget class string
-- a one line string buffer that can be edited



local require=require
local print=print
local math=math

module("fenestra.widget.slide")

local string=require("string")
local table=require("table")

function mouse(widget,act,x,y,key)
--	widget.master.focus=widget
	return widget.meta.mouse(widget,act,x,y,key)
end


function key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
end


function update(widget)
	local it=widget.slide


	it:snap()

--	it.drag.text=it.datx:get_string()
	
	return widget.meta.update(widget)
end


-- set dat from def table
local dat_set=function(dat,def)

	if not def then return dat end -- short circuit
	
	if def.min then dat.min=def.min end
	if def.max then dat.max=def.max end
	if def.num then dat.num=def.num end
	if def.size then dat.size=def.size end
	if def.step then dat.step=def.step end
	
	return dat
end


-- set number (may trigger hook)
local dat_value=function(dat,num)
	if num and num~=dat.num then -- change value
		if num then dat.num=num end
		if dat.num<dat.min then dat.num=dat.min end
		if dat.num>dat.max then dat.num=dat.max end
		dat.widget:call_hook("value",dat) -- call value hook, which may choose to mod the num some more...
	end
	return dat.num
end



-- a string to put in the handle
local dat_get_string=function(dat)
	return dat.num.."/"..dat.max
end

-- how wide or tall should the handle be given the size of the parent?
local dat_get_size=function(dat,w)
	if dat.size==0 then
		if dat.min==dat.max then return w end -- fullsize		
		return w/4 -- else some room to scroll
	else
		return w*( dat.size / dat.max-dat.min )
	end
end


-- given the parents size and our relative position/size within it
-- update dat.num and return a new position (for snapping)
local dat_snap=function(dat,psiz,bsiz,bpos)
	if dat.snap==0 then -- no snap
	
		if dat.max==dat.min then dat:value(dat.min) return 0 end -- do not move
		
		local f=bpos/(psiz-bsiz)
		dat:value(dat.min+((dat.max-dat.min)*f))
		
		return bpos -- do not snap
		
	else
	
		if dat.max==dat.min then dat:value(dat.min) return 0 end -- do not move
		
		local f=bpos/(psiz-bsiz)
		local n=math.floor(0.5+(((dat.max-dat.min)*f)/dat.step))

		dat:value(dat.min+(n*dat.step))
		
		return math.floor((psiz-bsiz)*((dat.num-dat.min)/(dat.max-dat.min)))
	end
end


function new_dat(it,id)

	local dat={}
	dat.widget=it.widget
	dat.it=it
	dat.id=id
	dat.min=0
	dat.num=0
	dat.max=1
	dat.size=0 -- if 0 then button is auto sized to some value
	dat.step=0 -- if 0 then there is no quantization
	
	dat.get_string=dat_get_string
	dat.get_size=dat_get_size
	dat.set=dat_set
	dat.value=dat_value
	dat.snap=dat_snap
	
	return dat
	
end

function slide_snap(it)
-- auto snap positions when draged
	it.drag.px=it.datx:snap( it.widget.hx , it.drag.hx , it.drag.px )
-- upside down y so need to twiddle it
	it.drag.py=it.widget.py - it.daty:snap( it.widget.hy , it.drag.hy , it.widget.py - it.drag.py )
end
	
function setup(widget,def)
	local it={} -- our main data so as not to clobber widget values
	it.widget=widget
	it.snap=slide_snap
	widget.slide=it
	widget.class="slide"
	
	widget.key=key
	widget.mouse=mouse
	widget.update=update
	
--setup constraints in x and y 
	it.datx=new_dat(it,"x"):set(def.datx)
	it.daty=new_dat(it,"y"):set(def.daty)

-- auto add the draging button as a child
	it.drag=widget:add({class="drag",color=0xffffffff,hy=it.daty:get_size(widget.hy),hx=it.datx:get_size(widget.hx),pxf=0,pyf=0})

	return widget
end

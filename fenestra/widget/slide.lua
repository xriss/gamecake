-- widget class string
-- a one line string buffer that can be edited



local require=require
local print=print

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

	it.drag.text=it.daty:get_string()
	
	
	return widget.meta.update(widget)
end


-- set dat from def table
local dat_set=function(dat,def)

	if not def then return dat end -- short circuit
	
	if def.min then dat.min=def.min end
	if def.max then dat.max=def.max end
	if def.num then dat.num=def.num end
	if def.size then dat.size=def.size end
	if def.step then dat.size=def.step end
	
	return dat
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
		return w*( dat.max-dat.min / dat_size )
	end
end


-- given the parents size and our relative position within it
-- update dat.num and return a new position (snapping)
local dat_snap=function(dat,psiz,bsiz,bpos)
	if dat.snap==0 then -- no snap
	
		if dat.max==dat.min then dat.num=dat.min return 0 end -- do not move
		
		local f=bpos/(psiz-bsiz)
		dat.num=dat.min+(dat.max-dat.min)*f)

		if dat.num<dat.min then dat.num=dat.min end
		if dat.num>dat.max then dat.num=dat.max end
		
		return bpos -- do not snap
		
	else
	
		if dat.max==dat.min then dat.num=dat.min return 0 end -- do not move
		
		local f=bpos/(psiz-bsiz)
		local n=math.round((dat.max-dat.min)*f)/dat.step)

		dat.num=dat.min+(n*dat.step)
		
		if dat.num<dat.min then dat.num=dat.min end
		if dat.num>dat.max then dat.num=dat.max end
		
		return math.floor((psiz-bsiz)*((dat.num-dat.min)/(dat.max-dat.min)))
	end
end


function new_dat(id)

	local dat={}
	dat.widget=widget
	dat.id=id
	dat.min=0
	dat.num=0
	dat.max=1
	dat.size=0 -- if 0 then it is auto sized
	dat.step=0 -- if 0 then there is no quantization
	
	dat.get_string=dat_get_string
	dat.get_size=dat_get_size
	dat.set=dat_set
	dat.snap=dat_snap
	
	return dat
	
end
	
function setup(widget,def)
	local it={}
	widget.slide=it
	widget.class="slide"
	
	widget.key=key
	widget.mouse=mouse
	widget.update=update
	
	-- guess horiz or vertical slider?
--	local maxx=1,maxy=0 -- horiz scroll?
--	if widget.hx>widget.hy then maxy=1 maxx=0 end -- ver scroll
	
	it.datx=new_dat("x"):set(def.datx)
	it.daty=new_dat("y"):set(def.daty)

-- addthe draging part
	it.drag=widget:add({class="drag",color=0xffffffff,hy=it.daty:get_size(widget.hy),hx=it.datx:get_size(widget.hx),pxf=0,pyf=0})

	return widget
end

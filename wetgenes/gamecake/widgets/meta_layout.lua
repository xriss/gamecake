-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generic default widget layout functions


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmeta)
wmeta=wmeta or {}

--
-- add meta functions
--
function wmeta.setup(def)

--	local master=def.master
	local meta=def.meta
	local win=def.win

--
-- initial layout of widgets, to put them into reasonable positions
--
	function meta.layout(widget)
--print(widget.class)
		if widget.class=="fill" or widget.class=="pan" then
			meta.layout_fill(widget)
--		elseif widget.class=="slide" or widget.class=="pad" then
--			meta.layout_padding(widget)
--		elseif widget.class=="master" or widget.class=="abs" then
--			meta.layout_base(widget)
		else
			meta.layout_base(widget)
		end
	end
	
	function meta.layout_none(widget)
		for i,v in ipairs(widget) do
			v:layout()
		end
	end
	
	function meta.layout_padding(widget)
		for i,v in ipairs(widget) do

			if v.hxf then v.hx=widget.hx*v.hxf end -- generate size as a fraction of parent
			if v.hyf then v.hy=widget.hy*v.hyf end
			
			v.px=(widget.hx-v.hx)*v.pxf -- local position relative to parents size
			v.py=(widget.hy-v.hy)*v.pyf

			v.pxd=widget.pxd+v.px -- local absolute position
			v.pyd=widget.pyd+v.py

		end
		for i,v in ipairs(widget) do
			v:layout()
		end
	end
	

	function meta.layout_base(widget)
		for i,v in ipairs(widget) do
		
			if v.hxf then v.hx=widget.hx*v.hxf end -- generate size as a fraction of parent
			if v.hyf then v.hy=widget.hy*v.hyf end
			
			if v.pxf then v.px=(widget.hx)*v.pxf end -- local position relative to parents size
			if v.pyf then v.py=(widget.hy)*v.pyf end

			v.pxd=widget.pxd+v.px -- absolute position
			v.pyd=widget.pyd+v.py
			
		end
		for i,v in ipairs(widget) do
			v:layout()
		end
	end

-- this is a fixed layout that works kind of like text
-- we do not adjust the hx,hy size of sub widgets
-- we just place them left to right top to bottom
-- finally we resize this widget to fit its content
-- the widgets sx,sy is used as default hx,hy for layout
	function meta.layout_fill(widget)
		
		local hx,hy=0,0
		local my=0
		widget.hx_max=0
		widget.hy_max=0
		local function addone(w)
			w.px=hx
			w.py=hy
			hx=hx+w.hx
			if hx > widget.hx_max then widget.hx_max=hx end -- max x total size
			if w.hy > my then my=w.hy end -- max y size for this line
--print(w.id or "?",w.px,w.py,w.hx,w.hy)
		end
		
		local function endoflines()
		end
		
		local function endofline()
			hx=0
			hy=hy+my
			my=0
			widget.hy_max=hy
		end
		
		if #widget>0 then
		
			for i,w in ipairs(widget) do
			
				if hx+w.hx>widget.hx then
					if hx==0 then -- need one item per line so add it anyway
						addone(w)
						endofline()
					else -- skip this one, push it onto nextline
						endofline()
						addone(w)
					end
				else -- it fits so just add
					addone(w)
				end
			end

			if hx>0 then endofline() end -- final end of line
			
			endoflines()
			
		end
		
		for i,v in ipairs(widget) do
			v.pxd=widget.pxd+v.px
			v.pyd=widget.pyd+v.py
		end

-- layout sub sub widgets	
		for i,v in ipairs(widget) do
			v:layout()
		end
	end


end


return wmeta
end

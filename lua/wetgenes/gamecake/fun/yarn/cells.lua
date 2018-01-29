
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.cells

	cells = require("wetgenes.gamecake.fun.yarn.cells").create(items)

This module contains only one function which can be used to create an 
cells instance and the rest of this documentation concerns the return 
from this create function, not the module itself.


]]
-----------------------------------------------------------------------------
M.create=function(items)

	local cells={} -- a place to store all cells

	cells.metatable={} -- unique meta table everytime we create
	cells.metatable.__index=cells.metatable -- metatable is full of functions
	setmetatable(cells.metatable,items.metatable) -- inherit
		
	cells.create=function(cell)

		cell=cell or {}
		cell.class="cell"
		cell=items.create(cell,cells.metatable)
		
		return cell
	end
	
	cells.metatable.get_cell_relative=function(cell,vx,vy)
		return cell.pages.get_cell(cell.cx+vx,cell.cy+vy)
	end
	
	cells.metatable.iterate_neighbours=function(cell)
		local n_x_look={  0 , -1 , 1 , 0 }
		local n_y_look={ -1 ,  0 , 0 , 1 }
		return function(cell,i)
			if i>4 then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end
	
	cells.metatable.iterate_neighboursplus=function(cell)
		local n_x_look={ 0,  0 , -1 , 1 , 0 }
		local n_y_look={ 0, -1 ,  0 , 0 , 1 }
		return function(cell,i)
			if i>5 then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end

	cells.metatable.iterate_corners=function(cell)
		local n_x_look={ -1 ,  1 , -1 , 1 }
		local n_y_look={ -1 , -1 ,  1 , 1 }
		return function(cell,i)
			if i>4 then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end

	cells.metatable.iterate_borders=function(cell)
		local n_x_look={ -1 ,  0 ,  1 , -1 , 1 , -1 , 0 , 1 }
		local n_y_look={ -1 , -1 , -1 ,  0 , 0 ,  1 , 1 , 1 }
		return function(cell,i)
			if i>8 then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end
	
	cells.metatable.iterate_bordersplus=function(cell)
		local n_x_look={ 0, -1 ,  0 ,  1 , -1 , 1 , -1 , 0 , 1 }
		local n_y_look={ 0, -1 , -1 , -1 ,  0 , 0 ,  1 , 1 , 1 }
		return function(cell,i)
			if i>9 then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end
		
	cells.metatable.iterate_range=function(cell,lx,hx,ly,hy)

		local n_x_look={}
		local n_y_look={}

		for y=ly,hy,1 do
			for x=lx,hx,1 do
				n_x_look[#n_x_look+1]=x
				n_y_look[#n_y_look+1]=y
			end
		end
		
		return function(cell,i)
			if i>#n_x_look then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end

	cells.metatable.iterate_rangebleed=function(cell,lx,hx,ly,hy)

		local n_x_look={}
		local n_y_look={}

		for y=ly,hy,1 do
			for x=lx,hx,1 do
				n_x_look[#n_x_look+1]=x
				n_y_look[#n_y_look+1]=y
			end
		end
		for y=hy,ly,-1 do
			for x=hx,lx,-1 do
				n_x_look[#n_x_look+1]=x
				n_y_look[#n_y_look+1]=y
			end
		end
		for y=ly,hy,1 do
			for x=hx,lx,-1 do
				n_x_look[#n_x_look+1]=x
				n_y_look[#n_y_look+1]=y
			end
		end
		for y=hy,ly,-1 do
			for x=lx,hx,1 do
				n_x_look[#n_x_look+1]=x
				n_y_look[#n_y_look+1]=y
			end
		end
		
		return function(cell,i)
			if i>#n_x_look then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end

	cells.metatable.iterate_hashrange=function(cell,lx,hx,ly,hy)

		local n_x_look={}
		local n_y_look={}

		for y=ly,hy,2 do
			for x=lx+1,hx,2 do
				n_x_look[#n_x_look+1]=x
				n_y_look[#n_y_look+1]=y
			end
			if y+1<hy then
				for x=lx,hx,2 do
					n_x_look[#n_x_look+1]=x
					n_y_look[#n_y_look+1]=y+1
				end
			end
		end
		for y=ly,hy,2 do
			for x=lx,hx,2 do
				n_x_look[#n_x_look+1]=x
				n_y_look[#n_y_look+1]=y
			end
			if y+1<hy then
				for x=lx+1,hx,2 do
					n_x_look[#n_x_look+1]=x
					n_y_look[#n_y_look+1]=y+1
				end
			end
		end
		
		return function(cell,i)
			if i>#n_x_look then return nil,nil end -- no more edges
			return i+1 , cell:get_cell_relative( n_x_look[i] , n_y_look[i] )
		end , cell , 1
	end

	cells.metatable.apply_update=function(cell,dx,dy)
		for i,c in cell:iterate_hashrange(-dx,dx,-dy,dy) do
			c:apply("update")
			for i,item in ipairs(c) do
				item:apply("update")
			end
		end
	end

	return cells

end

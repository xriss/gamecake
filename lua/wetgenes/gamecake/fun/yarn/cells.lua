
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

		cell=items.create(cell)
		setmetatable(cell,cells.metatable)
		cell.class="cell"
		
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
		
	return cells

end

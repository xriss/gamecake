
-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local win=win
local apps=apps

local gl=require("gl")
local grd=require("grd")


local function print(...) _G.print(...) end


module(...)


	
	
	
function setup()

	page()
	
	print("setup")

end


	

mdown=false

function mouse(act,x,y,key)

	local x,y=win.mouse23d(640,480,x,y)

	if act=="down" then
	
		mdown=true

--		table.insert(items, new_item((x)/10,(y)/10,0) )
	
	elseif act=="up" then
	
		mdown=false
		
	end

end


	
function clean()
end



function update()

end


function draw()

	win.begin()
	gl.ClearColor(14/15,14/15,14/15,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
	
	win.project23d(480/640,2,1024)
	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()


end


local this_page=nil
function page(f)

	if f then this_page=f else f=this_page end -- call with no args to reload current page
	
	widgets_main={} -- a place to keep widgets namespace
	widgets_main.master=win.widget
	widgets_main.master:remove_all() -- start afresh, remove all widgets
	widgets_main.master.ids=widgets_main --  lookup widgets by ids here
	widgets_main.master.font=win.font_sans
	
	panels_setup(widgets_main)
		
	if f then
		if type(f)=="string" then
			if widgets_main.panels[f] then
				widgets_main.panels[f].add(hash)
			end
		elseif type(f)=="function" then
			f() -- explicit function
		end
	else
		widgets_main.panels.menu.add()
	end
	
-- dump dbg

--[[
	local count=0
	fenestra_widget_skin.debug_hook=function(name,it)
	
		if it.class=="master" then
			count=count+1
			if count==2 then fenestra_widget_skin.debug_hook=nil return end
		end
		
		print("WGTS:",
			it.class or "?",
			string.format("%-16s",it.id or "?"),
			math.floor(it.pxd or 0) or "?",
			math.floor(it.pyd or 0) or "?",
			math.floor(it.px  or 0) or "?",
			math.floor(it.py  or 0) or "?",
			math.floor(it.hx  or 0) or "?",
			math.floor(it.hy  or 0) or "?")
	end
]]

end



-----------------------------------------------------------------------------
--
-- setup a table of panels functions using the provided widgets table, insert our functions into ws.panels
-- and return the new table
--
-----------------------------------------------------------------------------
function panels_setup(ws)
local env={}
ws.panels=env
setfenv(1,env)

-----------------------------------------------------------------------------
--
-- menu
--
-----------------------------------------------------------------------------
menu={}

-----------------------------------------------------------------------------
function menu.add(hash)
-----------------------------------------------------------------------------

local hooks={}
	function hooks.click(widget)
print(widget.id)
		if widget.id and env[widget.id] then
--			_G.stock_gotopage(widget.id)
		end
	end

--	ws.top=ws.master:add({sx=640,sy=480,mx=1,class="fill",rx=0,ry=0})
	ws.master:add({hx=640,hy=480,mx=1,class="flow",ax=0,ay=0,id="top"})
	
	ws.top:add({class="flow",sy=5,sx=1})
	ws.top:add({class="flow",text="view buys",color=0xffeeeeee,id="buy_list",hooks=hooks})
	ws.top:add({class="flow",text="sold stock",color=0xffeeeeee,id="search",hooks=hooks})
	ws.top:add({class="flow",text="edit items",color=0xffeeeeee,id="item",hooks=hooks})
	ws.top:add({class="flow",sy=5,sx=1})
	
	ws.master:layout()
	
end

return panels
end


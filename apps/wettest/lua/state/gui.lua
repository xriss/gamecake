
-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local win=win
local apps=apps

local gl=require("gl")
local grd=require("grd")


local function print(...) _G.print(...) end


module(...)


	
	
	
function setup()

	goto_page()
	
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
function goto_page(f)

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
local goto_page=goto_page



-- persistant gui data
local datas={}

datas.num1={class="number",val=0,max=10,min=0,step=1,size=1}
datas.num2={class="number",val=0,max=10,min=0,step=0,size=1}

-----------------------------------------------------------------------------
--
-- setup a table of panels functions using the provided widgets table, insert our functions into ws.panels
-- and return the new table
--
-----------------------------------------------------------------------------
function panels_setup(ws)
local env={}
ws.panels=env
ws.datas=datas
setfenv(1,env)

-----------------------------------------------------------------------------
--
-- misc
--
-----------------------------------------------------------------------------
misc={}
misc.hooks={}

-----------------------------------------------------------------------------
function misc.hooks.click(widget)
-----------------------------------------------------------------------------
print(widget.id)

--[[
	local f=hooks_click[widget.id]
	if f then
		f(widget)
	end
]]

	if widget.id=="topbar_menu" then
		goto_page("menu")
	end

end
	

-----------------------------------------------------------------------------
function misc.add(hash)
-----------------------------------------------------------------------------

	if hash=="topbar" then
	
		ws.top:add({hx=640,hy=24,class="fill",id="topbar"})
		ws.topbar:add({class="button",text="menu",color=0xffffffff,id="topbar_menu",hy=24,hx=120,hooks=misc.hooks})
		ws.topbar:add({text="title",id="topbar_title",hy=24,hx=640-120})
	
	end
end

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
			goto_page(widget.id)
		end
	end

	ws.master:add({hx=640,hy=480,mx=1,class="flow",ax=0,ay=0,id="top"})
	
	ws.top:add({class="flow",sy=5,sx=1})
	ws.top:add({class="flow",text="buttons",color=0xffffffff,id="test_buttons",hooks=hooks})
	ws.top:add({class="flow",text="scrolls",color=0xffffffff,id="test_scrolls",hooks=hooks})
	ws.top:add({class="flow",sy=5,sx=1})
	
	ws.master:layout()
	
end

-----------------------------------------------------------------------------
--
-- test_buttons
--
-----------------------------------------------------------------------------
test_buttons={}


-----------------------------------------------------------------------------
function test_buttons.add(hash)
-----------------------------------------------------------------------------

local hooks={}
	function hooks.click(widget)
print(widget.id)
		if widget.id and env[widget.id] then
			goto_page(widget.id)
		end
	end

	ws.master:add({hx=640,hy=480,mx=1,class="fill",ax=0,ay=0,id="top"})
	
	misc.add("topbar")
	
	ws.top:add({class="fill",hx=640,hy=24,text="test buttons",color=0xffffffff,id="test_buttons",hooks=hooks})
	ws.top:add({class="slide",color=0xffffffff,id="test_slide1",hy=24,hx=640,datx=datas.num1,daty={max=0},data=datas.num1,hooks=hooks})
	ws.top:add({class="slide",color=0xffffffff,id="test_slide2",hy=24,hx=640,datx=datas.num2,daty={max=0},data=datas.num2,hooks=hooks})
	
	ws.master:layout()
	
end

-----------------------------------------------------------------------------
--
-- test_scrolls
--
-----------------------------------------------------------------------------
test_scrolls={}

-----------------------------------------------------------------------------
function test_scrolls.add(hash)
-----------------------------------------------------------------------------

local hooks={}
	function hooks.click(widget)
print(widget.id)
		if widget.id and env[widget.id] then
			goto_page(widget.id)
		end
	end

	ws.master:add({hx=640,hy=480,mx=1,class="fill",ax=0,ay=0,id="top"})
	
	misc.add("topbar")
	
	ws.top:add({class="fill",hx=640,hy=24,text="test buttons",color=0xffffffff,id="test_buttons",hooks=hooks})
	
	ws.master:layout()
	
end




-----------------------------------------------------------------------------
return panels
end

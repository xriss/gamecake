

local _G=_G

local win=win

local print=print

module("state.menu")


--	local block=master:add({hx=640,hy=480,color=0x00000000,static=true})

local hooks={}
	function hooks.click(widget)
print(widget.id)
		if widget.id then
			if widget.id=="ville" then
				_G.goto("ville")
			elseif widget.id=="avatar" then
				_G.goto("avatar")
			elseif widget.id=="box2d" then
				_G.goto("box2d")
			elseif widget.id=="rouge" then
				_G.goto("rouge")
--			elseif widget.id=="cruft" then
--				_G.goto("cruft")
			end
		end
	end
	
	
function setup()

	win.widget:remove_all()
	local top=win.widget:add({hx=640,hy=480,mx=1,class="hx",ax=0,ay=0})
	
	top:add({sy=5,sx=1})
	top:add({text="wet ville",color=0x88ff0000,id="ville",hooks=hooks})
	top:add({text="avatar editor",color=0x88ff0000,id="avatar",hooks=hooks})
	top:add({text="test box2d",color=0x88ff0000,id="box2d",hooks=hooks})
	top:add({text="roguyver",color=0x88ff0000,id="rouge",hooks=hooks})
--	top:add({text="old avatar editor",color=0x88ff0000,id="cruft",hooks=hooks})
	top:add({sy=5,sx=1})
	
	win.widget:layout()
	
	win.widget.state="ready"

end

	
function clean()
	win.widget:remove_all()
end







local webcache=require("webcache")

local comm=require("spew.client.comm")

local wetstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")
local buffedit=require("fenestra.buffedit")
local comm=require("spew.client.comm")
local widgets=require("fenestra.widget")
local win=win
local gl=require("gl")

local str_split=wetstr.str_split

local string=string
local math=math
local table=table
local pairs=pairs
local ipairs=ipairs
local coroutine=coroutine
local error=error
local tonumber=tonumber
local print=print
local require=require

local fontsize=8

module(...)
local _M=require(...)

lines={}

function setup(_main)

	main=_main -- the main state that this chat is a part of
	
	total_lines=0

	mtx_proj=tardis.m4.new()
	mtx_view=tardis.m4.new()
	
	mtx_3d_to_2d=tardis.m4.new() -- 3d space to screen space
	mtx_2d_to_3d=tardis.m4.new() -- screen space to 3d space

	
	widget=widgets.setup(win,{font=win.font_debug})
	widget.hx=320
	widget.hy=480
--	local top=widget:add({hx=320,hy=480,mx=1,class="hx",ax=0,ay=0})
	sideslide=widget:add({class="slide",hx=8,hy=480,ax=320-8,ay=0,color=0x44ffffff})
	sideslide_knob=sideslide:add{color=0x88ffffff,text_color=0xffffffff,id="scroll",ax=0,ay=0,fx=1,fy=1/16}
	sideslide:set("slide",0,1)
	widget:layout()
	
	buff=buffedit.create() -- create an editable buffer
	buff.enter=function(it,s)
		comm.send({cmd="cmd",txt=s})
	end
end

function clean()

end

function update()

	buff:update()

	widget:update()

end

function draw(wx,wy)

-- save current mtx for reverse mouse pointer projections
	mtx_proj:set(gl.Get("PROJECTION_MATRIX"))
	mtx_view:set(gl.Get("MODELVIEW_MATRIX"))
	mtx_view:product(mtx_proj,mtx_3d_to_2d) 	-- combine
	mtx_3d_to_2d:inverse(mtx_2d_to_3d)         	-- inverse
	
	gl.PushMatrix()
	gl.Translate(-wx/2,-wy/2, 0)
--	widget:draw()
	gl.PopMatrix()	
--TODO: fix widget scroll thing

--if true then return end

	gl.PushMatrix()
	gl.Translate(-wx/2,-wy/2,0)

local wrap_x=math.floor(wx/(fontsize))

	local dx,dy=sideslide_knob:get("slide")
	local skiplines=total_lines-((wy/(fontsize+2))-2)
--print(dx,dy,total_lines,skiplines)
	skiplines=math.floor(skiplines*(1+dy))
	if skiplines<0 then skiplines=0 end
	local y=0
	if #lines>0 then
		for i=#lines+1,1,-1 do local v=lines[i]
		local noskip=false
		
			if y>wy then break end -- drawing off of the top of the screen
			
			if not v then -- draw edit buffer
			
				v={}
				v.buff=buff
				v.lines=word_wrap(buff.line,wrap_x)
				if not v.lines[1] then v.lines[1]="" end
				noskip=true
				v.rgb=0x00ff00
	
			else
				if v.wrap_x~=wrap_x then
					v.lines=word_wrap(v.txt,wrap_x)
					v.wrap_x=wrap_x
				end
			end
			
			if #v.lines>0 then
			
				local nogap=true
			
				if v.buff then -- pulse
					local y=1
					local x=v.buff.line_idx
					local t=v.lines[y]
					while t and (x>=#t) and v.lines[y+1] do
						y=y+1
						x=x-#t
						t=v.lines[y]
					end
					
					gl.PushMatrix()
					gl.Translate(0,(fontsize)*#v.lines,0)
					win.debug_rect((x+0)*fontsize,(y-1)*-(fontsize+2),(x+1)*fontsize,(y+0)*-(fontsize),0x00ff00+v.buff.throb*0x01000000)
					gl.PopMatrix()
				end
				
				for i=#v.lines,1,-1 do local line=v.lines[i]
					if noskip or skiplines<=0 then
						gl.Translate(0,fontsize,0)
						
						gl.PushMatrix()
						
						win.font_debug.set(0,0,(v.rgb or 0xffffff) + 0xff000000,fontsize,fontsize)
						win.font_debug.draw(line)
						y=y+fontsize
											
						gl.PopMatrix()
						nogap=false
					else
						skiplines=skiplines-1
					end
					
				end				
				if not nogap then -- a gap
					gl.Translate(0,fontsize/2,0)
					y=y+(fontsize/2)
				end				
			end
		end
	end
	
	gl.PopMatrix()
end



function keypress(ascii,key,act)

	buff:keypress(ascii,key,act)
	
end

function mouse(act,x,y,key)

	local p=tardis.v3.new(0,0,0)
	mtx_3d_to_2d:product(p,p) -- get a z value
	p[1]=(x/(win.width/2))-1
	p[2]=(y/(win.height/2))-1
	mtx_2d_to_3d:product(p,p)
	
	widget:mouse(act,p[1]+160,(p[2]+240),key)
	
--	print(p[1],p[2])

end

function cmsg(msg)

	if msg.cmd=="say" then
	
		insert(msg,msg.frm .. ": " ..msg.txt)
		
	elseif msg.cmd=="act" then
		
		insert(msg,"**"..msg.frm .. " " ..msg.txt.."**")

	elseif msg.cmd=="mux" then

		insert(msg,msg.txt)
		
	elseif msg.cmd=="note" then
		
		if msg.note=="feat" then
		
		elseif msg.note=="act" then
			insert(msg,"**"..msg.arg1.."**")
		elseif msg.note=="error" then
			insert(msg,"error: "..msg.arg1)
		elseif msg.note=="warning" then
			insert(msg,"warning: "..msg.arg1)
		elseif msg.note=="notice" then
			insert(msg,"notice: "..msg.arg1)
		elseif msg.note=="welcome" then
			insert(msg,msg.arg1)
		elseif msg.note=="ranking" then
			insert(msg,msg.arg1..":"..msg.arg2.." "..msg.arg3.." is now #"..msg.arg4)
		elseif msg.note=="rename" then
			insert(msg,msg.arg1.." is now known as "..msg.arg2.." "..msg.arg3)
		elseif msg.note=="join" then
			insert(msg,msg.arg1.." has joined room "..msg.arg2.." playing "..msg.arg3)
		elseif msg.note=="part" then
			insert(msg,msg.arg1.." has left room "..msg.arg2)
		elseif msg.note=="ban" then
			if tonumber(msg.arg3)>0 then
				insert(msg,msg.arg2.." has been banned by "..msg.arg1)
			else
				insert(msg,msg.arg2.." has been unbanned by "..msg.arg1)
			end
		elseif msg.note=="gag" then
			if tonumber(msg.arg3)>0 then
				insert(msg,msg.arg2.." has been gagged by "..msg.arg1)
			else
				insert(msg,msg.arg2.." has been ungagged by "..msg.arg1)
			end
		elseif msg.note=="dis" then
			if tonumber(msg.arg3)>0 then
				insert(msg,msg.arg2.." has been disemvowled by "..msg.arg1)
			else
				insert(msg,msg.arg2.." has been revowled by "..msg.arg1)
			end
		end
	end


end

local esc_char=string.char(27)
local esc_pat="("..esc_char.."%[%A*%a)"

function strip_ansi(s)

	s=string.gsub(s,esc_pat,function(x) return "" end)

	return s
end


function rgbstr_to_number(s)

	if not s then return 0xffffff end -- default to white

	local r,g,b=0xff,0xff,0xff

	if #s==3 then

		r=s:sub(1,1)
		g=s:sub(2,2)
		b=s:sub(3,3)

		r=tonumber(r,16)*0x11
		g=tonumber(g,16)*0x11
		b=tonumber(b,16)*0x11

	elseif #s==6 then

		r=s:sub(1,2)
		g=s:sub(3,4)
		b=s:sub(5,6)

		r=tonumber(r,16)
		g=tonumber(g,16)
		b=tonumber(b,16)

	end


	return (r*0x10000) + (g*0x100) + b

end

function insert(msg,s)

	local t={}
	
	t.rgb=rgbstr_to_number(msg.rgb)
	t.txt=strip_ansi(s)
	t.lines=word_wrap(t.txt,40-1)
	t.yh=#t.lines*(fontsize+2)
	
	lines[#lines+1]=t
	
	total_lines=total_lines+#t.lines
	
	while #lines>200 do
		t=lines[1]
		total_lines=total_lines-#t.lines
		table.remove(lines,1)
	end

end



-- wrap a string to a given character width
function word_wrap(s,w)

	s=s or ""
	local t={}

	while s~="" do
	
		if not s or s=="" then break end -- end of input
		
		local r
		
		if #s<=w or s:byte(w+1)==32 then -- perfect split
		
			r=s:sub(1,w+1)
			s=s:sub(w+2)
			
		else
		
			local split_at=1
			
			for i=w,1,-1 do
				if s:byte(i)==32 then -- found last space on this line
					split_at=i
					break
				end
			end
			
			if split_at==1 then -- no space no split
				r=s:sub(1,w)
				s=s:sub(w+1)
			else
				r=s:sub(1,split_at)
				s=s:sub(split_at+1)
			end
			
		end
		
		table.insert(t,r) -- building a table of lines each one of w or less length
		
	end
	
	return t
end


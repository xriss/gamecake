-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(state,scores)

	scores=scores or {} 
	
	local cake=state.cake
	local canvas=cake.canvas
	
	function scores.setup(max_up)
		max_up=max_up or 1
		scores.up={}
		for i=1,max_up do
			scores.up[i]={score=0,high=0} -- 1up 2up etc
		end
		scores.high=0
		return scores -- so setup is chainable with a bake
	end

	function scores.reset()
		for i,v in ipairs(scores.up) do
			v.score=0
		end
	end

	function scores.add(num,up)
		up=up or 1
		local v=assert(scores.up[up])
		v.score=v.score+num
		if v.score>v.high then v.high=v.score end
		if v.score>scores.high then scores.high=v.score end
		return v.score
	end

	function scores.get(up)
		up=up or 1
		local v=assert(scores.up[up])
		return v.score
	end


	function scores.set(num,up)
		up=up or 1
		local v=assert(scores.up[up])
		v.score=num
		if v.score>v.high then v.high=v.score end
		if v.score>scores.high then scores.high=v.score end
		return v.score
	end

	function scores.clean()
	end

	function scores.update()	
	end
	
	function scores.draw(mode)
	
		local function draw_mid_text(x,y,s)
		
			local sw=canvas.font.width(s)
			
			cake.gl.Color(0,0,0,1)	
			canvas.font.set_xy( x-(sw/2)+1 , y+1 )
			canvas.font.draw(s)
		
			cake.gl.Color(1,1,1,1)	
			canvas.font.set_xy( x-(sw/2)-1 , y-1 )
			canvas.font.draw(s)
		
		end
		
-- display 1up Hi (2up) at top of screen in 8 bit font
-- with the scores on the line bellow
		if mode=="arcade2" then

			local xh=canvas.view_width
			local yh=canvas.view_height		
			local fy=math.floor(yh/32)

			canvas.font.set(cake.fonts.get(1))
			canvas.font.set_size(fy,0)

			local s=string.format("%d",scores.up[1].score)
			draw_mid_text( (xh*3/16) , fy*0.25 , "1up")
			draw_mid_text( (xh*3/16) , fy*1.50 , s)

			local s=string.format("%d",scores.high)
			draw_mid_text( (xh*8/16) , fy*0.25 , "Hi")
			draw_mid_text( (xh*8/16) , fy*1.50 , s)

			if scores.up[2] then
				local s=string.format("%d",scores.up[2].score)
				draw_mid_text( (xh*13/16) , fy*0.25 , "2up")
				draw_mid_text( (xh*13/16) , fy*1.50 , s)
			end
			
		end
		
	end
		
	function scores.msg(m)
	end

	return scores
end

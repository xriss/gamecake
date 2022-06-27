--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")


local wwin=require("wetgenes.win") -- system independent helpers
local wstr=require("wetgenes.string")
local wsbox=require("wetgenes.sandbox")
local lfs ; pcall( function() lfs=require("lfs") end ) -- may not have a filesystem

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,scores)

	scores=scores or {} 

	scores.filename=wwin.files_prefix.."scores.lua"
	scores.mode="none" -- default mode
	
	local cake=oven.cake
	local canvas=cake.canvas
	
	local profiles=oven.rebake("wetgenes.gamecake.spew.profiles")

-- very very simple local score data
	local ss

-- initialise scores data
	function scores.init()
		ss={}
		scores.mode="none"
		scores.level=1
		
		ss.list={}
		ss.list[scores.mode]={}
		ss.list[scores.mode][scores.level]={}
	end
	
-- load all scores data
	function scores.load()
		if lfs then
log("oven","Loading "..scores.filename)
			local fp=io.open(scores.filename,"r")
			if fp then
				local s=fp:read("*all")
				ss=wsbox.lson(s) -- safeish
				fp:close()
				return true
			end
		end

		return false
	end
	
-- save all scores data
	function scores.save()
		if lfs then
log("oven","Saving "..scores.filename)
			local fp=io.open(scores.filename,"w")
			if fp then
				fp:write(wstr.serialize(ss))
				fp:close()
			end
		end
	end
	
	function scores.setup(max_up)
	log("setup",M.modname,max_up)

		scores.show_high=true

		max_up=max_up or 1
		scores.up={}
		for i=1,max_up do
			scores.up[i]={score=0,high=0} -- 1up 2up etc
		end
		scores.high=0
		for i,v in pairs( ss.list[scores.mode][scores.level] ) do
			if v.score>scores.high then scores.high=v.score end
		end

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
			
			oven.gl.Color(0,0,0,1)	
			canvas.font.set_xy( x-(sw/2)+1 , y+1 )
			canvas.font.draw(s)
		
			oven.gl.Color(1,1,1,1)	
			canvas.font.set_xy( x-(sw/2)-1 , y-1 )
			canvas.font.draw(s)
		
		end
		
-- display 1up Hi (2up) at top of screen in 8 bit font
-- with the scores on the line bellow
		if mode=="arcade2" then
		
		
			local view=cake.views.get()

			local xh=view.hx
			local yh=view.hy
			local fy=math.floor(yh/32)

			canvas.font.set(cake.fonts.get(1))
			canvas.font.set(cake.fonts.get(1))
			canvas.font.set_size(fy,0)

			local s=wstr.str_insert_number_commas(scores.up[1].score)
			draw_mid_text( (xh*3/16) , fy*0.25 , "1up")
			draw_mid_text( (xh*3/16) , fy*1.50 , s)

			if scores.show_high then
				local s=wstr.str_insert_number_commas(scores.high)
				draw_mid_text( (xh*8/16) , fy*0.25 , "Hi")
				draw_mid_text( (xh*8/16) , fy*1.50 , s)
			end

			if scores.up[2] then
				local s=wstr.str_insert_number_commas(scores.up[2].score)
				draw_mid_text( (xh*13/16) , fy*0.25 , "2up")
				draw_mid_text( (xh*13/16) , fy*1.50 , s)
			end
			
		end
		
	end
		
	function scores.msg(m)
	end


	function scores.get_list(opts)
		local mode=opts.mode or scores.mode or "none"
		local level=opts.level or scores.level or 1
		if not ss.list then ss.list={} end
		if not ss.list[mode] then ss.list[mode]={} end
		if not ss.list[mode][level] then ss.list[mode][level]={} end	
		return ss.list[mode][level]
	end

	function scores.get_high(opts)
		local r={}
		for i,v in ipairs( scores.get_list(opts) ) do
			local t=r[v.name] or v
			if (v.score>t.score) or ( (v.score==t.score) and (v.time<=t.time) ) then
				r[v.name]=v
			end
		end
		return r
	end

-- get a list of scores data
	function scores.list(opts)
		
		
		local ret={}

		local order=opts.order or "high"
		local offset=opts.offset or 1
		local limit=opts.limit or 10

		local t
		
		if order=="high" then -- normal high score list, one score per name
		
			t=scores.high_to_list(scores.get_high(opts))			

		else

			t=scores.high_to_list(scores.get_list(opts))			
		
		end
		
		if t then
			for i=offset,offset+limit do
				if t[i] then
					t[i].idx=i -- refresh idx
					ret[#ret+1]=t[i]
				end
			end
		end
		
		return ret	
	end
	
	function scores.high_to_list(t)
		local k={}
		for i,v in pairs(t) do k[#k+1]=v end		
		table.sort(k,function(a,b) -- sort by score then time
			if a.score==b.score then
				return a.time<b.time
			end
			return a.score>b.score
		end)
		return k
	end	

	function scores.get_best_score(opts)
		local name=opts.name or profiles.get("name")
		local h=scores.get_high(opts)
		return h[name]
	end
	
	function scores.final_score(opts)

		local score=opts.score or scores.up[1].score
	
		local scr={}
		
		scr.idx=0
		scr.time=opts.time or os.time()
		scr.name=opts.name or profiles.get("name")
		scr.score=opts.score or scores.up[1].score
		scr.pid=opts.pid or profiles.pid -- which profile slot this is associated with
		
		local t=scores.get_list(opts)
		t[#t+1]=scr -- insert
		
		local s=scores.high_to_list(t)
		for i=1,#s do
			s[i].idx=i
		end

		scores.save() -- always save new scores to disk
		
		if wwin.smell=="gamestick" then
			if wwin.hardcore.smell_score_send then wwin.hardcore.smell_score_send(score) end
		end

		return scr
	end


--make sure we have a dir to load/save profiles into
if lfs then
lfs.mkdir(wwin.files_prefix:sub(1,-2)) -- skip trailing slash
end

-- try autoload
scores.init()
if not scores.load() then
-- or create and save a default file
	scores.init()
	scores.save()
end

	return scores
end

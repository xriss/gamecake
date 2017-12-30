--
-- (C) 2016 kriss@wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")

--[[#lua.wetgenes.grdsvg

	wgrdsvg=require("wetgenes.grdsvg")

Build svg files from grd data (bitmaps)

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
-- short name
local wgrdsvg=M

--[[#lua.wetgenes.grdsvg.string

	local svgstring=wgrdsvg.string(grd,opts)

Return an svg formated string that represents the input grd which must 
be an indexed (8bit) format. Each pixel will be converted into a filled 
svg rectangle element. Opts can contain the following options to 
control the generation of the svg file.

	opts.skip_transparent_pixels=true

Do not export a rectangle for fully transparent pixels.

	opts.scalex=1

Width of each exported pixel.

	opts.scaley=1

Height of each exported pixel.

]]
wgrdsvg.string=function(grd,opts)
	opts=opts or {}
	
	opts.scalex=opts.scalex or 1
	opts.scaley=opts.scaley or 1
	
	local tab=	{
					push=function(it,s,d) if s then it[#it+1]=wstr.replace(s,d) end end ,
					tostring=function(it) return table.concat(it,"") end
				}
	
	tab:push([[
<svg
	xmlns="http://www.w3.org/2000/svg"
	xmlns:xlink="http://www.w3.org/1999/xlink"
	width="{width}"
	height="{height}"
	>
]],{width=grd.width*opts.scalex,height=grd.height*opts.scaley})
	
	tab:push([[
<style type="text/css" ><![CDATA[
]])

	local skipidx={}
	for i=0,255 do
		local p=grd:palette(i,1)
		if p[1]==0 and p[2]==0 and p[3]==0 and p[4]==0 then
			skipidx[i]=true
		end
		
		tab:push([[
	.{class} { fill:{color}; fill-opacity:{alpha}; }
]],{class=string.format("C%02X",i),color=string.format("#%02X%02X%02X",p[1],p[2],p[3]),alpha=string.format("%.3f",p[4]/255)})
	end
	tab:push([=[
]]></style>
]=])	

	if not opts.skip_transparent_pixels then skipidx={} end
	
	for y=0,grd.height-1 do
		for x=0,grd.width-1 do
			local idx=grd:pixels(x,y,1,1)[1]
			if not skipidx[idx] then

				tab:push([[
	<rect x="{x}" y="{y}" width="{w}" height="{h}" class="{class}"/>
]],{x=x*opts.scalex,y=y*opts.scaley,w=opts.scalex,h=opts.scaley,class=string.format("C%02X",idx) })

			end
		end
	end

	tab:push([[
</svg>
]])

	return tab:tostring()

end


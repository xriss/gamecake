
local grd=require("wetgenes.grd")
local wstr=require("wetgenes.string")


-- compare two rgba colors and return a distance value
local cdist=function( ar,ag,ab,aa , br,bg,bb,ba )
	return --[[math.sqrt]]( (ar-br)*(ar-br) + (ag-bg)*(ag-bg) + (ab-bb)*(ab-bb) + (aa-ba)*(aa-ba) )
end


-- sort the palette order
local sort_pal=function(g,colors)
	local pc=g:palette(0,colors)	
	local best_length=math.huge
	local best_list={}
	for istart=0,colors-1 do -- try each possible start point
		local pool={} ; for i=0,colors-1 do pool[i+1]=i end
		local list={}
		list[#list+1]=table.remove(pool,istart+1) -- remove/add the start color
		local list_length=0
		while #pool>0 do
			local pi=list[#list] -- color at end of list
			local best_step=math.huge
			local best_idx=0
			for i=1,#pool do -- 
				local pt=pool[i] -- color to test
				local d=cdist( pc[pi*4+1] , pc[pi*4+2] , pc[pi*4+3] , pc[pi*4+4] , pc[pt*4+1] , pc[pt*4+2] , pc[pt*4+3] , pc[pt*4+4] )
				if d<best_step then
					best_step=d
					best_idx=i
				end
			end
			list[#list+1]=table.remove(pool,best_idx) -- remove/add the next color
			list_length=list_length+best_step
		end
		if list_length<best_length then
			best_length=list_length
			best_list=list
		end
	end
	
	local po={}
	local map={}
	for i=1,#best_list do
		local c=best_list[i]
		map[ c ]=i-1
		po[ (i-1)*4 + 1 ]= pc[ c*4 + 1 ]
		po[ (i-1)*4 + 2 ]= pc[ c*4 + 2 ]
		po[ (i-1)*4 + 3 ]= pc[ c*4 + 3 ]
		po[ (i-1)*4 + 4 ]= pc[ c*4 + 4 ]
	end
	g:palette(0,colors,po)

--[[
	local d=g:pixels(0,0,0,g.width,g.height,1)
	for i=1,g.width*g.height do
		d[i]=map[ d[i] ] -- remap
	end
	g:pixels(0,0,0,g.width,g.height,1,d)
]]

end

local test=function(fname,colors)



-- save an easy to view palette
local save_pal=function(g,fname)
	local p=assert( grd.create("U8_INDEXED",16*8,16*8,1) )
	p:palette(0,16*16,g:palette(0,16*16,""))
	for i=0,255 do
		local x=i%16
		local y=math.floor(i/16)
		p:pixels(x*8,y*8,0,8,8,1,string.rep(string.char(i),8*8))
	end
	assert( p:save(fname) )
end


-- test neoquant as base
--[[
local g=assert(grd.create("gi/"..fname..".png"))
assert( g:quant(colors) )
assert( g:save("go/"..fname.."."..colors..".nq.png") )
save_pal(g,"go/"..fname.."."..colors..".nq.pal.png")
]]

local gi=assert(grd.create("gi/"..fname..".png")) -- input
assert( gi:convert("U8_RGBA") )
local go=assert(grd.create(grd.U8_INDEXED,gi.width,gi.height,1)) -- output

-- get input pixels

local pix_rgb=gi:pixels(0,0,0,gi.width,gi.height,1)

-- build blank pal

local pal_idx={}
local pal_rgb={}
local pal_dat={}

for i=0,colors-1 do
	pal_rgb[ i*4 + 1 ]=i
	pal_rgb[ i*4 + 2 ]=i
	pal_rgb[ i*4 + 3 ]=i
	pal_rgb[ i*4 + 4 ]=i
	
	pal_dat[ i*5 + 1 ]=0
	pal_dat[ i*5 + 2 ]=0
	pal_dat[ i*5 + 3 ]=0
	pal_dat[ i*5 + 4 ]=0
	pal_dat[ i*5 + 5 ]=0
end

-- assign equal pixels to each bucket as a starting state
for i=0,go.width*go.height-1 do
	local r=pix_rgb[ i*4 + 1 ] -- original rgb
	local g=pix_rgb[ i*4 + 2 ]
	local b=pix_rgb[ i*4 + 3 ]
	local a=pix_rgb[ i*4 + 4 ]
	local p=math.floor(0.5+(colors-1)*((r/255 + g/255 + b/255 )/3))
	pal_idx[ i + 1 ] = p
	pal_dat[ p*5 + 1 ]=pal_dat[ p*5 + 1 ] + pix_rgb[ i*4 + 1 ]
	pal_dat[ p*5 + 2 ]=pal_dat[ p*5 + 2 ] + pix_rgb[ i*4 + 2 ]
	pal_dat[ p*5 + 3 ]=pal_dat[ p*5 + 3 ] + pix_rgb[ i*4 + 3 ]
	pal_dat[ p*5 + 4 ]=pal_dat[ p*5 + 4 ] + pix_rgb[ i*4 + 4 ]
	pal_dat[ p*5 + 5 ]=pal_dat[ p*5 + 5 ] + 1
end

-- save current state
local save=function(colors)
	for i=0,colors-1 do
		local d=pal_dat[ i*5 + 5 ] ; if d==0 then d=1 end
		pal_rgb[ i*4 + 1 ]=math.floor(pal_dat[ i*5 + 1 ] / d + 0.5)
		pal_rgb[ i*4 + 2 ]=math.floor(pal_dat[ i*5 + 2 ] / d + 0.5)
		pal_rgb[ i*4 + 3 ]=math.floor(pal_dat[ i*5 + 3 ] / d + 0.5)
		pal_rgb[ i*4 + 4 ]=math.floor(pal_dat[ i*5 + 4 ] / d + 0.5)
	end
	go:palette(0,colors,pal_rgb)
	go:pixels(0,0,0,go.width,go.height,1,pal_idx)
	sort_pal(go,colors)
	assert( go:save("go/"..fname.."."..colors..".sq.png") )
	save_pal(go,"go/"..fname.."."..colors..".sq.pal.png")
end


-- perform a single quant pass
local quant=function(w)

	for i=0,go.width*go.height-1 do

		local r=pix_rgb[ i*4 + 1 ] -- original rgb
		local g=pix_rgb[ i*4 + 2 ]
		local b=pix_rgb[ i*4 + 3 ]
		local a=pix_rgb[ i*4 + 4 ]

		local p=pal_idx[ i + 1 ] -- original index

		pal_dat[ p*5 + 1 ]=pal_dat[ p*5 + 1 ] - r -- remove from bucket
		pal_dat[ p*5 + 2 ]=pal_dat[ p*5 + 2 ] - g
		pal_dat[ p*5 + 3 ]=pal_dat[ p*5 + 3 ] - b
		pal_dat[ p*5 + 4 ]=pal_dat[ p*5 + 4 ] - a
		pal_dat[ p*5 + 5 ]=pal_dat[ p*5 + 5 ] - 1

		local best_d=math.huge
		for t=0,colors-1 do -- test each bucket
--			local w=256
			local d=pal_dat[ t*5 + 5 ]+w ; if d==0 then d=1 end
			local tr=math.floor( (pal_dat[ t*5 + 1 ]+r*w) / d + 0.5)
			local tg=math.floor( (pal_dat[ t*5 + 2 ]+g*w) / d + 0.5)
			local tb=math.floor( (pal_dat[ t*5 + 3 ]+b*w) / d + 0.5)
			local ta=math.floor( (pal_dat[ t*5 + 4 ]+a*w) / d + 0.5)
			
			local d=cdist( r,g,b,a , tr,tg,tb,ta )
			if d<best_d then
				best_d=d
				p=t
			end
		end

		pal_dat[ p*5 + 1 ]=pal_dat[ p*5 + 1 ] + r -- put back in a bucket
		pal_dat[ p*5 + 2 ]=pal_dat[ p*5 + 2 ] + g
		pal_dat[ p*5 + 3 ]=pal_dat[ p*5 + 3 ] + b
		pal_dat[ p*5 + 4 ]=pal_dat[ p*5 + 4 ] + a
		pal_dat[ p*5 + 5 ]=pal_dat[ p*5 + 5 ] + 1
		
		pal_idx[ i + 1 ]=p -- and remapped image data
	end

end

-- work out error
local diff=function()
	local e,c=0,0

	for i=0,go.width*go.height-1 do
		local r=pix_rgb[ i*4 + 1 ] -- original rgb
		local g=pix_rgb[ i*4 + 2 ]
		local b=pix_rgb[ i*4 + 3 ]
		local a=pix_rgb[ i*4 + 4 ]

		local p=pal_idx[ i + 1 ] -- original index

		local pr=pix_rgb[ p*4 + 1 ] -- original rgb
		local pg=pix_rgb[ p*4 + 2 ]
		local pb=pix_rgb[ p*4 + 3 ]
		local pa=pix_rgb[ p*4 + 4 ]

		local d=cdist( r,g,b,a , pr,pg,pb,pa )
		
		e=e+d
		c=c+1

	end
	
	return e/c
	
end

-- loop update and output
local st=os.time()
local c=5
for i=1,c do
--	quant(1)
	quant( (go.width*go.height/(colors*2)) / i*i ) -- weight by average colors per bucket given image size
	local d=diff()
	print(fname,go.width*go.height,colors,i,os.time()-st,d)
	io.flush()
end
save(colors)

end

local i=2
while i<=256 do
--	test("rgb",i)
--	test("baboon",i)
--	test("monarch",i)
--	test("parrot",i)
	i=i*2
end

local remap=function(gi,go,colors)

local dith={
22,38,26,42,23,39,27,43,
54, 6,58,10,55, 7,59,11,
30,46,18,34,31,47,19,35,
62,14,50, 2,63,15,51, 3,
24,40,28,44,21,37,25,41,
56, 8,60,12,53, 5,57, 9,
32,48,20,36,29,45,17,33,
64,16,52, 4,61,13,49, 1,
}
	sort_pal(go,colors)

	local pc=go:palette(0,colors) -- output will already have a palette

	local e={0,0,0,0}
	for y=0,gi.height-1 do
		for x=0,gi.width-1 do
			local c=gi:pixels(x,y,1,1)
			local x8=x%8
			local y8=y%8
			local df=dith[x8+8*y8+1] -- dither flip point 1-64			
			local best={0,math.huge,0,math.huge} -- find best 2 indexs
			for i=0,colors-1 do
				local d=cdist( pc[i*4+1],pc[i*4+2],pc[i*4+3],pc[i*4+4] , c[1],c[2],c[3],c[4] )
				if d<best[2] then -- find the best color
					best[4]=best[2]
					best[3]=best[1]
					best[2]=d
					best[1]=i
				elseif d<best[4] then -- find the 2nd best color
					best[4]=d
					best[3]=i
				end
			end
			
-- limit to adjacent colors only?
--[[
			local b1=(best[1]-1)%colors
			local b2=(best[1]+2)%colors
			best[3]=b1
			best[4]=cdist( pc[b1*4+1],pc[b1*4+2],pc[b1*4+3],pc[b1*4+4] , c[1],c[2],c[3],c[4] )
			local d=cdist( pc[b2*4+1],pc[b2*4+2],pc[b2*4+3],pc[b2*4+4] , c[1],c[2],c[3],c[4] )
			if d<best[4] then best[3]=b2 end
]]

-- work out the mix level
			local c1={ pc[best[1]*4+1],pc[best[1]*4+2],pc[best[1]*4+3],pc[best[1]*4+4] , best[1] }
			local c2={ pc[best[3]*4+1],pc[best[3]*4+2],pc[best[3]*4+3],pc[best[3]*4+4] , best[3] }
			if c1[5] < c2[5] then c1,c2=c2,c1 end -- keep the two colors in the same order
			local cc={0,0,0,0}
			local best_df={0,math.huge}
			for i=0,64,4 do
				local a,b=i/64,(64-i)/64
				cc[1]= c1[1]*a + c2[1]*b
				cc[2]= c1[2]*a + c2[2]*b
				cc[3]= c1[3]*a + c2[3]*b
				cc[4]= c1[4]*a + c2[4]*b
				local d=cdist( cc[1],cc[2],cc[3],cc[4] , c[1],c[2],c[3],c[4] )
				if d<best_df[2] then -- find the best dither 
					best_df[1]=i
					best_df[2]=d
				end
			end
			local flip=best_df[1]
			
--			flip=math.floor(0.5+flip/8)*8

			if df<=flip then
				go:pixels(x,y,1,1,{c1[5]})
			else
				go:pixels(x,y,1,1,{c2[5]})
			end
		end
	end
end


local st=os.time()
local colors=2
while colors<=256 do

	print(colors,os.time()-st)
	local fname="rgb"
	local gi=assert(grd.create("gi/"..fname..".png"))
	assert( gi:convert("U8_RGBA") )

	local go=grd.create(gi)
	assert( go:quant(colors) )
--	remap(gi,go,colors)
	assert( go:save("go/"..fname.."."..colors..".sq.png") )
--	save_pal(go,"go/"..fname.."."..colors..".sq.pal.png")

	colors=colors*2
end





local grd=require("wetgenes.grd")
local wstr=require("wetgenes.string")


local test=function(fname,colors)

-- test neoquant as base

local g=assert(grd.create("gi/"..fname..".png"))
assert( g:convert("U8_INDEXED") )
assert( g:save("go/"..fname..".nq.png") )


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
local save=function(idx)
	for i=0,colors-1 do
		local d=pal_dat[ i*5 + 5 ] ; if d==0 then d=1 end
		pal_rgb[ i*4 + 1 ]=math.floor(pal_dat[ i*5 + 1 ] / d + 0.5)
		pal_rgb[ i*4 + 2 ]=math.floor(pal_dat[ i*5 + 2 ] / d + 0.5)
		pal_rgb[ i*4 + 3 ]=math.floor(pal_dat[ i*5 + 3 ] / d + 0.5)
		pal_rgb[ i*4 + 4 ]=math.floor(pal_dat[ i*5 + 4 ] / d + 0.5)
	end
	go:palette(0,colors,pal_rgb)
	go:pixels(0,0,0,go.width,go.height,1,pal_idx)
	assert( go:save("go/"..fname..".sq."..idx..".png") )
end

-- compare two rgba colors and return a distance value
local cdist=function( ar,ag,ab,aa , br,bg,bb,ba )
--	ar=255-ar ag=255-ag ab=255-ab aa=255-aa
--	br=255-br bg=255-bg bb=255-bb ba=255-ba
--	return (ar*ar-br*br)*(ar*ar-br*br) + (ag*ag-bg*bg)*(ag*ag-bg*bg) + (ab*ab-bb*bb)*(ab*ab-bb*bb) + (aa*aa-ba*ba)*(aa*aa-ba*ba)
	return math.sqrt( (ar-br)*(ar-br) + (ag-bg)*(ag-bg) + (ab-bb)*(ab-bb) + (aa-ba)*(aa-ba) )
--	return math.abs(ar-br) + math.abs(ag-bg) + math.abs(ab-bb) + math.abs(aa-ba)
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
save(0)
local st=os.time()
local c=8
for i=1,c do
	io.flush()
	quant(256*((c+1-i)/c))
	save(i)
	local d=diff()
	print(fname,i,os.time()-st,d)
end

end

test("rgb",4)
test("baboon",4)
test("monarch",4)

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module(...)

function build(tab)

tab.arg=tab.arg or {}


local bake=require("wetgenes.bake")

local pp=require("wetgenes.pp")
local lfs=require("lfs")

local no_art=false

opts=opts or {} -- can pass in an opts of default options

local opts_changed=false
local func

	opts.VERSION_NUMBER=opts.VERSION_NUMBER or 0

	func=loadfile("src/opts.lua")
	if func then
		setfenv(func,opts)
		pcall(func)
	else
		opts_changed=true
	end
		
	for i=1,#tab.arg do

		if tab.arg[i]=="bump" then
		
			opts.VERSION_NUMBER=opts.VERSION_NUMBER+0.001
		
			opts_changed=true
		end
		
		if tab.arg[i]=="debug" then
		
			opts.VERSION_BUILD="debug"
		
			opts_changed=true
		end
		
		if tab.arg[i]=="release" then
		
			opts.VERSION_BUILD="release"
		
			opts_changed=true
		end
		
		if tab.arg[i]=="noart" then
			no_art=true
		end
		
	end
	
	if opts_changed then -- write out changed file
		local fp=io.open("src/opts.lua","w")
		for i,v in pairs(opts) do
		
			if type(v)=="number" then
				fp:write(i.."="..v.."\n")
			elseif type(v)=="string" then
				fp:write(i.."="..string.format("%q",v).."\n")
			end
		end
		fp:close()
	end
	
	
	
	
-- where we are building from

bake.cd_base		=	bake.get_cd()

-- where we are building to

bake.cd_out		=	'out'

lfs.mkdir('out')
lfs.mkdir('out/src')
lfs.mkdir('out/art')



-- go up a dir from base cd and remember as main CD for building commands

bake.set_cd(bake.get_cd()..'/..')
bake.cd=bake.get_cd()

print('cd','=',bake.cd)


bake.cmd.xpp		=	bake.path_clean_exe( bake.cd , '/bin/xpp' )
bake.cmd.mtasc		=	bake.path_clean_exe( bake.cd , '/_mtasc/mtasc' )
bake.cmd.swfmill	=	bake.path_clean_exe( bake.cd , '/_swfmill/swfmill' )
bake.cmd.mtasc_path1=	bake.path_clean( bake.cd , '/_mtasc/std' )
bake.cmd.mtasc_path2=	bake.path_clean( bake.cd , '/_mtasc/std8' )
bake.cmd.mtasc_path3=	bake.path_clean( bake.cd , '/_mtasc/std9' )

if bake.osflavour=="nix" then -- expected to be installed...
--	bake.cmd.mtasc="mtasc"
	bake.cmd.swfmill="swfmill"
end



bake.set_cd(bake.cd_base)

bake.files_as={}
for v in lfs.dir("src") do -- do all files in the src dir

	if string.find(v,"%.as$") then -- a .as file

		v=string.gsub( v , "%.as$" , "") -- remove .as
		table.insert(bake.files_as,v)
		
	end
	
end

bake.files_xml=
{
	tab.swf_name,
}

bake.files_swf=
{
	tab.swf_name,
}

bake.set_cd(bake.cd_base)
bake.copyfile( 'art/index.html' , 'out/index.html' )
bake.copyfile( 'art/swfobject.js' , 'out/swfobject.js' )


for i,v in ipairs(bake.files_as) do

	pp.loadsave( 'src/'..v..'.as' , bake.cd_out..'/src/'..v..'.as' )

end

io.flush()

if no_art then
print('****')
print('**SKIPING**SWFMILL**BUILD**STEP**')
print('****')
else
for i,v in ipairs( bake.files_xml ) do

	pp.loadsave( 'art/'..v..'.xml' , bake.cd_out..'/art/'..v..'.xml' )
	
	bake.execute( bake.cd_base , bake.cmd.swfmill , '-v simple '..bake.cd_out..'/art/'..v..'.xml '..bake.cd_out..'/'..v..'.swf' )

end
end

for i,v in ipairs( bake.files_swf ) do

	bake.execute( bake.cd_base , bake.cmd.mtasc , '-main -version 10 -v -cp '..bake.cmd.mtasc_path3..' -cp '..bake.cmd.mtasc_path2..' -cp '..bake.cmd.mtasc_path1..' -cp '..bake.cd_out..'/src -cp '..bake.cd..'/base/src -swf '..bake.cd_out..'/'..v..'.swf '..bake.cd_out..'/src/'..v..'.as' )

end


--xtra.copyfile( 'out/'..tab.swf_name..".swf" , '../../www/wetgenes/subs/data/swf/'..tab.swf_name..".swf" )

end

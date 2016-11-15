--
-- (C) 2013 Kriss@XIXs.com
--

local require=require
local loadfile=loadfile
local setfenv=setfenv
local pcall=pcall
local print=print
local pairs=pairs
local ipairs=ipairs
local type=type

local string=string
local table=table
local io=io
local os=os

module(...)

function build(tab)

local bake=require("wetgenes.bake")

local pp=require("wetgenes.pp")
local lfs=require("lfs")

local no_art=false

local opts=tab.opts or {} -- can pass in an opts of default options

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
bake.cd_base	=	bake.cd_base or bake.get_cd()

-- where we are building to
bake.cd_out		=	bake.cd_out or 'out'

lfs.mkdir(bake.cd_out)
lfs.mkdir(bake.cd_out..'/art')
lfs.mkdir(bake.cd_out..'/cache')





-- go up a dir from base cd and remember as main CD for building commands

bake.set_cd(bake.get_cd()..'/..')
bake.cd=bake.get_cd()

print('cd','=',bake.cd)


bake.cmd.java="java" -- needed for minifier
bake.cmd.zip="zip" -- needed to zip stuffs

bake.cmd.lua		=	bake.path_clean_exe( bake.cd , '/exe/lua' )

if bake.osflavour=="nix" then -- expected to be installed...
--	bake.cmd.mtasc="mtasc"
--	bake.cmd.swfmill="swfmill"
end


bake.set_cd(bake.cd_base)

bake.files_min_js=bake.files_min_js or {}
bake.files_min_js[ #bake.files_min_js + 1 ]=tab.name

bake.files_js={}
for v in lfs.dir("src") do -- add all .js files in the src dir
	if string.find(v,"%.js$") then
		v=string.gsub( v , "%.js$" , "")
		table.insert(bake.files_js,v)
	end
end

bake.files_css={}
for v in lfs.dir("src") do -- add all .css files in the src dir
	if string.find(v,"%.css$") then
		v=string.gsub( v , "%.css$" , "")
		table.insert(bake.files_css,v)
	end
end

bake.files_html={}
for v in lfs.dir("src") do -- and all .html files in the src dir
	if string.find(v,"%.html$") then
		v=string.gsub( v , "%.html$" , "")
		table.insert(bake.files_html,v)
	end
end





for i,v in ipairs(bake.files_js) do
	pp.loadsave( 'src/'..v..'.js' , bake.cd_out..'/'..v..'.js' )
end
for i,v in ipairs(bake.files_css) do
	pp.loadsave( 'src/'..v..'.css' , bake.cd_out..'/'..v..'.css' )
end

for i,v in ipairs(bake.files_html) do
	pp.loadsave( 'src/'..v..'.html' , bake.cd_out..'/'..v..'.html' )
end

-- generic pp of files,full paths from -> to
bake.files_pp=bake.files_pp or {}
for i,v in ipairs(bake.files_pp) do
	pp.loadsave( v[1] , v[2] )
end


io.flush()

if no_art then
print('****')
print('**SKIPPING**ART**BUILD**STEP**')
print('****')
else


opts.compiler=opts.compiler or "../../js/class/compiler.jar" -- need a compiler jar to minify

for i,v in ipairs(bake.files_min_js) do
print('compressing '..v)
	bake.execute( bake.cd_base , bake.cmd.java ,
"-jar "..opts.compiler.." --js_output_file "..bake.cd_out.."/"..v..".min.js --js "..bake.cd_out.."/"..v..".js")

end

	local r=bake.findfiles{basedir=".",dir="art",filter="%.png$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end

	local r=bake.findfiles{basedir=".",dir="art",filter="%.jpg$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end

	local r=bake.findfiles{basedir=".",dir="art",filter="%.swf$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end

	local r=bake.findfiles{basedir=".",dir="art",filter="%.css$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end

	local r=bake.findfiles{basedir=".",dir="art",filter="%.mp3$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end
	local r=bake.findfiles{basedir=".",dir="art",filter="%.ogg$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end
	local r=bake.findfiles{basedir=".",dir="art",filter="%.wav$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end

	local r=bake.findfiles{basedir=".",dir="jslib",filter="%.js$"}
	for i,v in ipairs(r.ret) do
		bake.create_dir_for_file(bake.cd_out.."/"..v)
		bake.copyfile(v,bake.cd_out.."/"..v)
	end
	
-- build an application cache

	for i,v in ipairs(tab.cache_files or {}) do
		local v1,v2
		if type(v)=="table" then
			v1=v[1]
			v2=v[2]
		else
			v1=v
			v2=v
		end
print('caching out/'..v1.." as "..v2)
		bake.create_dir_for_file(bake.cd_out.."/cache/"..v2)
		bake.copyfile("out/"..v1,bake.cd_out.."/cache/"..v2)
	end

	local r=bake.findfiles{basedir=bake.cd_out.."/cache",dir=".",filter=""}
	local mc=table.concat(r.ret,"\n").."\n"
	local fp=io.open(bake.cd_out.."/cache/cache.manifest","w")
	fp:write("CACHE MANIFEST\n")
	fp:write("#Updated on "..os.date().."\n")
	fp:write(mc)
	fp:close()
	
-- zip everything in the cache, this is our distribution package
	bake.execute(bake.cd_out.."/cache",bake.cmd.zip,"../"..tab.name..".zip * -r")

end


	
end




#! /usr/bin/env lua

--[[
	Why use Makefiles when there is Lua available?

	Usage: cd module_dir && ./build.lua absolute_out_path [AMD64]
--]]
SEP = package.config:sub(1,1)
local SEP = SEP

local exe_path
if arg[0]:sub(1,1) == '.' then
   exe_path = os.getenv'PWD'
   exe_path = exe_path .. SEP .. arg[0]
else
   exe_path = arg[0]
end
_,_, exe_path = exe_path:find('^(.+)' .. SEP .. '.+$')
package.path = exe_path .. '/?.lua;' .. package.path
if SEP == '/' then
	package.cpath = exe_path .. '/?.so;' .. package.cpath
else
	package.cpath = exe_path .. '/?.dll;' .. package.cpath
end

local base = _G
local _expandStr_mt = { __index = base }
function expandStr(str, t)
	if not t then t = { } end
	str = str:gsub('%$(%b())', function (s)
								   s = s:sub(2, #s-1)
								   local func = loadstring('return ' .. s)
								   local env = setmetatable(t, _expandStr_mt)
								   setfenv(func, env)
								   return tostring(func())
						   end)
	str = str:gsub('%$([%w_]+)', function (s)
								  return tostring(t[s] or base[s])
						  end)
	return str
end

sf = string.format
local sf = sf
es = expandStr
local es = es

local usage = es'Usage: $(arg[0]) absolute_dest [AMD64]'

DEST   = assert(arg[1], usage)
AMD64  = arg[2] == 'AMD64'
GEN    = DEST .. '/bin/lgob-generator'

function shell(cmd)
	local f = assert(io.popen(cmd), sf("Couldn't open the pipe for %s!", cmd))

	local a = f:read('*a')
	f:close()
	a = a:gsub('\n', '')

	return a
end
sh = shell
local sh = sh

require('config')

function pkg(arg, pkgs)
	local p = table.concat(pkgs, ' ')
	local t = { p = p, arg = arg }
	return sh( es('$PKG $arg $p', t) )
end

function gen_iface(mod)
	local t = { name = mod.name }
	t.input   = es('src/$name.ovr' , t)
	t.output  = es('src/iface.c', t)
	t.log     = es('src/log', t)
	t.version = pkg('--modversion', {mod.pkg})

	sh(es('$GEN -i $input -o $output -l $log -v $version', t))
end

function compile(mod)
	local t = { name = mod.name, src = mod.src or 'iface.c' }
	t.input    = es('src/$src', t)
	t.output   = es('src/$name.$EXT', t)
	t.pkgflags = pkg('--cflags --libs', {LUA_PKG, mod.pkg})

	sh(es('$CC $COMPILE_FLAGS -I$DEST/include $input -o $output $pkgflags', t))
end

function install(mod, dest)
	local t = { name = mod.name, dest = dest }
	t.dir   = 'src'
	local inst = mod.inst or {es('$name.$EXT', t)}

	for _, f in ipairs(inst) do
		t.f = f
		sh(es('$INST $dir/$f $DEST/$dest/$f', t))
	end
end

function clean(mod)
	local t = {}
	t.dir   = 'src'
	local garb  = mod.garb or {'iface.c', 'log', mod.name .. '.' .. EXT}

	for _, f in ipairs(garb) do
		t.f = f
		sh(es('$RM $dir/$f', t))
	end
end

function init()
	print( es'** Building module $MODULE with $COMPILE_FLAGS **' )
end

require( 'mod' )

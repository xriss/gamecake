--[[#lua.wetgenes.txt.lex

(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT



Some useful lex files for other editors to be used as starting points 
and checking we did not miss anything.

	https://github.com/vim/vim/tree/master/runtime/syntax
	https://github.com/sublimehq/Packages


]]


local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.list={}

M.list.lua=require(M.modname.."_lua")

local keyval=function(t) for i=1,#t do t[ t[i] ]=i end return t end

M.token_numeric      = keyval{	"0","1","2","3","4","5","6","7","8","9"}

M.token_alpha        = keyval{	"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
								"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"}

M.token_alphanumeric = keyval{	"0","1","2","3","4","5","6","7","8","9",
								"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
								"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"}


local deepcopy ; deepcopy=function(orig)
	if type(orig) ~= 'table' then return orig end
	local copy={}
	for k,v in next,orig,nil do
		copy[ deepcopy(k) ] = deepcopy(v)
	end
	return copy
end

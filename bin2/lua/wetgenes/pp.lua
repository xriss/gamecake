--+-----------------------------------------------------------------------------------------------------------------+--
--
-- (C) Kriss Daniels 2005 http://www.XIXs.com
--
-- This file made available under the terms of The MIT License : http://www.opensource.org/licenses/mit-license.php
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--
--+-----------------------------------------------------------------------------------------------------------------+--

local up_G=_G

local print=print
local io=io
local table=table
local string=string
local loadfile=loadfile
local loadstring=loadstring
local assert=assert
local pcall=pcall
local xpcall=xpcall

local setmetatable=setmetatable
local getfenv=getfenv
local setfenv=setfenv

module("wetgenes.pp")



--
--
-- Create a lua program from an input file.
--
-- This can then be parsed and run to produce a preprocesed outputfile
--
--

-- #() can be used anywhere except at the start of a line to inject the text between () as a small chunk of lua
-- # can be used at the start of the line to inject the rest of the line as lua
-- #include "fname" is a piece of magic that injects the given file into the output
-- #( at the start of the line denotes the begining of a pure lua block, rest of line is ignored
-- #) at the start of the line denotes the end of a pure lua block, rest of line is ignored
-- a pure lua block is needed for multiline table definitions otherwise the line numbering system breaks things
--
-- returns a table of strings which can be table.concat into a string result

function ppload(filename,chunk)

local included

local file
local temp
local line_num
local new_filename
local lua_block_insert


	lua_block_insert=false

	if chunk then -- we are a sub build

		included=true

	else

		included=false

		chunk = {n=0}

	end



	file=io.open(filename,"r")
	
	assert(file,"failed to load "..filename)

	if not included then

		table.insert(chunk,"-- A lua file that builds a preprocesed file as a table\n")
		table.insert(chunk,"\n")
		table.insert(chunk,"pp_output_table = {n=0}\n")
		table.insert(chunk,"local _out_ti=table.insert\n")
		table.insert(chunk,"local _out_tt=pp_output_table\n")
		table.insert(chunk,"local _out_do=function(s)\n")
		table.insert(chunk,"	_out_ti(_out_tt,s)\n")
		table.insert(chunk,"	for i in string.gfind(s,'\\n') do -- count outputlines\n")
		table.insert(chunk,"		_lo=_lo+1\n")
		table.insert(chunk,"		if _lo_break then -- so we can break on line\n")
		table.insert(chunk,"			assert(_lo<_lo_break,'user line break')\n")
		table.insert(chunk,"		end\n")
		table.insert(chunk,"	end\n")
		table.insert(chunk,"end\n")
		table.insert(chunk,"_lo=0\n")
		table.insert(chunk,"local _out=function(...) for i,v in ipairs(arg) do _out_do(v) end end\n")

	end

	table.insert(chunk,"\n")
	table.insert(chunk,"_file='"..filename.."'\n")
	table.insert(chunk,"\n")

	line_num=1
	for line in file:lines() do

		if lua_block_insert==false then
		
			table.insert(chunk, string.format('_l=%4d; ', line_num))
		end
		
		
		if string.find(line, "^#%(") then
		
			lua_block_insert=true
			table.insert(chunk, "\n")
			
		elseif string.find(line, "^#%)") then
		
			lua_block_insert=false
			table.insert(chunk, "\n")
			
		elseif string.find(line, "^#dofile") then

			_,_,new_filename=string.find(line, "^#dofile%s\"([%w%p]+)\"")

			new_filename=pp_input_cd..new_filename

			table.insert(chunk, "--Including "..new_filename.."\n")
			
			local fp=io.open(new_filename,"r")
			local d=fp:read("*all")
			table.insert(chunk,d)

			table.insert(chunk,"\n")
			table.insert(chunk,"_file='"..filename.."'\n")
			table.insert(chunk,"\n")

		elseif string.find(line, "^#include") then

			_,_,new_filename=string.find(line, "^#include%s\"([%w%p]+)\"")

			new_filename=pp_input_cd..new_filename

			table.insert(chunk, "--Including "..new_filename.."\n")

			ppload(new_filename,chunk)

			table.insert(chunk,"\n")
			table.insert(chunk,"_file='"..filename.."'\n")
			table.insert(chunk,"\n")

		elseif string.find(line, "^#") then

			table.insert(chunk, string.sub(line, 2) .. "\n")

		else
		
			if lua_block_insert then
			
				table.insert(chunk, line .. "\n")
			
			else

			local last = 1

				for text, expr, index in string.gfind(line, "(.-)#(%b())()") do 

					last = index

					if text ~= "" then

						table.insert(chunk, string.format('_out %q ', text))

					end

					table.insert(chunk, string.format('_out%s ', expr))

				end

-- gsub to remove ugly line ends and replace with \n to make file more readable
temp=string.gsub(string.sub(line, last),"\n","\\n")
temp=string.gsub(temp,"\r","")
temp=string.format('_out %q\n',temp.."\n")

				table.insert(chunk, temp)
				
			end

		end

		line_num=line_num+1
	end

	file:close()

	if not included then
	
		table.insert(chunk,"\nreturn pp_output_table\n")
	end

  return chunk
end


function loadsave(fname_in,fname_out)

pp_input_cd=""


pp_input_table={}
pp_input_string=""
pp_input_lua=function() end

pp_output_table={}
pp_output_string=""



print ( "Lua pre processing " .. fname_in .. " into " .. fname_out .. "" )


-- create input table, and its string

pp_input_table=ppload(fname_in)

pp_input_string=table.concat(pp_input_table)


-- output the lua program string to a file along with the output to aid with debuging,
-- any lua errors will hopefully be more meaningfull this way


--[[
local fp
	fp=io.open(fname_out..".lua","w")
	fp:write(pp_input_string)
	fp:close()
	fp=null

-- load that string back in and run it to produce an output table
pp_input_lua,_msg=loadfile(fname_out..".lua")
]]


pp_input_lua,_msg=loadstring(pp_input_string)


assert(pp_input_lua,".\n.\n"..( _msg or "" ).."\n.\n.\n")

local newfenv = {}        -- create new environment just for this file
setmetatable(newfenv, {__index = up_G}) -- allow global access we are not trying to sandbox
setfenv(pp_input_lua, newfenv)    -- change this functions environment from the global one


local ret,_msg,_ret


	_lo_break=null
	
	ret,_ret=pcall(pp_input_lua)	-- not sure about scope here...
			

	if ret~=true then

		print('.\n.\n')
		print(string.format('%s(%d):PP file location\n',_file or "unknown",_l or -1))
		assert(null,_ret..'\n.\n.\n')

	end

-- write that table out as a string to the destination file


pp_output_string=table.concat(_ret)

local fp
	fp=io.open(fname_out,"w")
	fp:write(pp_output_string)
	fp:close()
	fp=null

end


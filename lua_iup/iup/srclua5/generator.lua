
function dofile(f)
  pcall(loadfile(f))
end

-- compatibility functions (with iuplua.lua)
iup = {}
function iup.SetClass(ctrl, name)
  element = ctrl
end

-- dummy functions
iupDoNothing = function() end
iupSetMethod = iupDoNothing
iup.RegisterWidget = iupDoNothing

-- TODO: This is different from iupClassRegisterCallback, must use the same standard
c_types = {
  n = "int",               -- should be i
  s = "char *",
  i = "Ihandle *",         -- should be h
  c = "unsigned char ",    -- should be b
  d = "double",
  f = "float",
  v = "Ihandle **",        -- should be g
}

-- Adjust the callbacks table
function adjustcallbacktable(c)
   d = {}
   for i,j in pairs(c) do
      if type(j) == "string" then
         d[i] = { j, "IUP_".. string.upper(i)}
      elseif type(j) == "table" then
         d[i] = j
      else
         print("ERROR IN CALLBACK TABLE FORMAT")
      end
   end
   return d
end


function header(o,i)
   io.write [[
/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
]]
  if i then io.write('#include "',i,'"\n') end
  io.write('#include "il.h"\n\n\n')
end

function firstupper(name)
   return string.upper(string.sub(name,1,1)) .. string.sub(name,2,-1)
end

function write_creation(o, t)
   local aux = {n = 1}
   local u = firstupper(o)
   local v = t.creation
   local c = t.callback
   if t.funcname then
      u = t.funcname
   end
   io.write ("static int ",u,"(lua_State *L)\n")
   io.write ("{\n") 
   if t.rettype == nil then io.write("  Ihandle *ih = Iup",u,"(")
   elseif t.rettype == "n" then io.write("  int n = (Iup",u,"(")
   elseif t.rettype == "s" then io.write("  char *s = (Iup",u,"(")
   end
   local max = string.len(v)
   string.gsub(v, "(.)", function(p)
      if p == "n" then io.write("luaL_checkint(L, ",aux.n,")")
      elseif p == "N" then io.write("luaL_optinteger(L, ",aux.n,", 0)")
      elseif p == "d" then io.write("luaL_number(L, ",aux.n,")")
      elseif p == "s" then io.write("(char *)luaL_checkstring(L, ",aux.n,")")
      elseif p == "S" then io.write("(char *)luaL_optstring(L, ",aux.n,", NULL)")
      elseif p == "i" then io.write("iuplua_checkihandle(L, ",aux.n,")")
      elseif p == "I" then io.write("iuplua_checkihandleornil(L, ",aux.n,")")
      elseif p == "-" then io.write("NULL")
      elseif p == "a" then io.write("iuplua_checkstring_array(L, ",aux.n,",0)")
      elseif p == "t" then io.write("iuplua_checkint_array(L, ",aux.n,",0)")
      elseif p == "v" then io.write("iuplua_checkihandle_array(L, ",aux.n,",0)")
      else io.write("FORMAT '", p, "' NOT SUPPORTED\n")
      end
      if aux.n < max then io.write(", ") end
      aux.n = aux.n + 1
   end)
   io.write(");\n")
   
   io.write("  iuplua_plugstate(L, ih);\n")
   io.write("  iuplua_pushihandle_raw(L, ih);\n")
   io.write("  return 1;\n")
   io.write("}\n\n")
end

function write_callbacks(o, c)
   local aux = { }
   for i,v in pairs(c) do
      local s = v[1]
      local max = string.len(s)
      aux.n = 0
      io.write("static ")
      if v.ret then
         if v.ret == "s" then
            io.write("char * ")
         end
      else
         io.write("int ")
      end
      io.write(o, "_", i, "(Ihandle *self")
      if max > 0 then io.write(", ") end
      string.gsub(s, "(.)", function(p)
         io.write(c_types[p], " p", aux.n)
         aux.n = aux.n + 1
         if aux.n < max then io.write(", ") end
      end)
      io.write(")\n{\n")
      io.write('  lua_State *L = iuplua_call_start(self, "', i, '");')
      aux.n = 0
      string.gsub(s, "(.)", function(p)
         if p == "n" or p == "c" then
            io.write("\n  lua_pushinteger(L, p"..aux.n..");")
         elseif p == "f" or p == "d" then
            io.write("\n  lua_pushnumber(L, p"..aux.n..");")
         elseif p == "s" then
            io.write("\n  lua_pushstring(L, p"..aux.n..");")
         elseif p == "i" then
            io.write("\n  iuplua_pushihandle(L, p"..aux.n..");")
         else
            io.write("\n ERROR !! ")
         end
         aux.n = aux.n + 1
      end)
      if v.ret and v.ret == "s" then
        io.write("\n  return iuplua_call_rs(L, " .. max .. ");")
      else   
        io.write("\n  return iuplua_call(L, " .. max .. ");")
      end
      io.write("\n}\n\n")
   end
end

function write_initialization(o,t)
   local aux= {n=1}
   local c = t.callback
   local u = firstupper(o)
   if t.extrafuncs then
      io.write('void iuplua_', o,'funcs_open(lua_State *L);\n\n')
   end
   if t.openfuncname then
      io.write("void ", t.openfuncname, "(lua_State * L)\n")
   else
      io.write("int iup", o,"lua_open(lua_State * L)\n")
   end
   io.write("{\n")
   io.write("  iuplua_register(L, ")
   if t.funcname then
      u = t.funcname
   end
   io.write(u, ', "', u,'");\n\n')
   
   for i,v in pairs(c) do
      local type = "NULL"
      -- Handle callbacks that have same names but different parameters
      if i == "action" or 
         i == "action_cb" or 
         i == "edit_cb" or 
         i == "mousemove_cb" then
        type = '"'..string.lower(o)..'"'
      end
      io.write('  iuplua_register_cb(L, "',string.upper(i),'", (lua_CFunction)',o,'_',i,', ',type,');\n')
      first = 0
   end
   io.write('\n')
   
   if t.extrafuncs then
      io.write('  iuplua_', o,'funcs_open(L);\n\n')
   end
   io.write('#ifdef IUPLUA_USELOH\n')
   io.write('#include "', o,'.loh"\n')
   io.write('#else\n')
   io.write('#ifdef IUPLUA_USELZH\n')
   io.write('#include "', o,'.lzh"\n')
   io.write('#else\n')
   io.write('  iuplua_dofile(L, "', o,'.lua");\n')
   io.write('#endif\n')  
   io.write('#endif\n\n')  
   io.write('  return 0;\n')
   io.write("}\n\n")
end

dofile(arg[1])
element.callback = adjustcallbacktable(element.callback)

io.output("il_"..element.nick..".c")
header(element.nick, element.include)
write_callbacks(element.nick, element.callback)
if element.createfunc == nil then 
   write_creation(element.nick, element)
else 
   io.write(element.createfunc) 
end
write_initialization(element.nick, element)
if element.extracode then 
   io.write(element.extracode) 
end

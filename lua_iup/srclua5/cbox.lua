------------------------------------------------------------------------------
-- Cbox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "cbox",
  parent = iup.WIDGET,
  creation = "v",
  callback = {},
  funcname = "Cboxv",
  createfunc = [[
static int Cboxv(lua_State *L)
{
  Ihandle **hlist = iuplua_checkihandle_array(L, 1, 0);
  Ihandle *h = IupCboxv(hlist);
  iuplua_plugstate(L, h);
  iuplua_pushihandle_raw(L, h);
  free(hlist);
  return 1;
}
  ]],
}

function ctrl.createElement(class, param)
  return iup.Cboxv(param)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")

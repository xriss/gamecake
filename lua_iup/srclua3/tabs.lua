IUPTABS = {parent = WIDGET}

function IUPTABS:CreateIUPelement (obj)
  return iupCreateTabs (obj, getn(obj))
end

function iuptabs (o)
  return IUPTABS:Constructor (o)
end
iup.tabs = iuptabs

iup_callbacks.tabchange = {"TABCHANGE_CB", iup_tabchange_cb}
iup_callbacks.tabchange_cb = iup_callbacks.tabchange
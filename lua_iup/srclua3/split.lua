IUPSPLIT = {parent = WIDGET}

function IUPSPLIT:CreateIUPelement (obj)
  return iupCreateSplit(obj[1], obj[2])
end

function iupsplit (o)
  return IUPSPLIT:Constructor (o)
end
iup.split = iupplit

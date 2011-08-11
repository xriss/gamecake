IUPSBOX = {parent = WIDGET}

function IUPSBOX:CreateIUPelement (obj)
  return iupCreateSbox(obj[1])
end

function iupsbox (o)
  return IUPSBOX:Constructor (o)
end
iup.sbox = iupsbox

#!../../../dbg/gamecake.nix



print("test")

local socket = require("socket")
local ssl = require("ssl")


local params = {
   mode = "client",
   protocol = "any",
  verify   = "none",
   options = {"all", "no_sslv2", "no_sslv3", "no_tlsv1", "no_tlsv1_3"},
--curveslist="P-521:P-384:P-256",
--curveslist="secp224r1:prime239v1:prime256v1:secp384r1:secp521r1",

   key = "./clientAkey.pem",
   certificate = "./clientA.pem",
   cafile = "./rootA.pem",
--[[
   verify = {"peer", "fail_if_no_peer_cert"},
]]
}

local function wait(peer, err)
   if err == "wantread" then
      socket.select({peer}, nil)
   elseif err == "timeout" or err == "wantwrite" then
      socket.select(nil, {peer})
   else
      peer:close()
      os.exit(1)
   end
end


local peer = socket.tcp()
--assert( peer:connect("127.0.0.1", 1443) )
assert( peer:connect("google.com", 443) )


peer = assert( ssl.wrap(peer, params) )
assert( peer:dohandshake() )


peer:settimeout(0.3)

local str = "a rose is a rose is a rose is a...\n"
--while true do
   print("Sending...")
   local succ, err = peer:send(str)
--   while succ do
--      succ, err = peer:send(str)
--   end
   print("Waiting...", err)
   wait(peer, err)
--end
peer:close()



print("done")

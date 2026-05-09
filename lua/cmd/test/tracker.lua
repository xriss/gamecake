

local bitsynth_tracker=require("wetgenes.gamecake.fun.bitsynth_tracker")

local wstr=require("wetgenes.string")
local djon=require("djon")


local fp=assert(io.open("/home/kriss/Desktop/test.it","rb"))
local data=fp:read("*a")
fp:close()


local mod=bitsynth_tracker.IT_to_mod(data)

--print( wstr.dump(mod) )

print( djon.save(mod,"djon") )

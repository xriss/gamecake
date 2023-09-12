#!/usr/local/bin/gamecake

local lanes=require("lanes")

local linda=lanes.linda()

local start=lanes.gen( "*" , {} ,
	function(linda)
		for idx=1,2 do
			print("start",idx)
		
			local ok,err=xpcall(function()
				print("wait")
				linda:receive(nil,"msg")
				error("test")
			end,function(e) print("error handler") return( debug.traceback() ) end)

			print(ok,err)
		end
	end
)

local handle=start(linda)

local ret=linda:send(nil,"msg","something") -- 1 force a test error for traceback

local ret=linda:receive(1,"forever") -- wait a bit so thread can print error

handle:cancel() -- cancel thread but error handler be called but traceback is lost

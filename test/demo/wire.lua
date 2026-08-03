
local wire=require("wire")

print(  )
print( "wire starting" )
print( "we are " , wire.threads.us.name , wire.threads.us.handle )

wire.tasks("http",4,"require('wiretasks').http_code()")

wire.thread({
	start=[[
	
local wire=require("wire")

print(  )
print( "thread starting" , wire.threads.us.name , wire.threads.us.handle )

local result=wire.memo({
	fifo=wire.threads.house,
	data={
		action="handle",
		name="test",
	},
}):resolve()

print( "thread ending" , wire.threads.us.name , wire.threads.us.handle )
	
]],
})

local result=wire.memo({
	fifo=wire.threads.house,
	data={
		action="handle",
		name="test",
	},
}):resolve()

-- check http
local memo_get=wire.memo({
	fifo=wire.manifest("http"),
	data={
		url="http://google.com/",
	},
}):send()
print("sent http request")


for i=3,1,-1 do
	print("sleep" , i , wire.time() )
	wire.sleep(1)
	wire.update() -- check for reply and run callbacks
end

-- http fetch will have happened in the background by now unless its very slow
if memo_get.result then print("got http",memo_get.result.code,memo_get.result.status) end


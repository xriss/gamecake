
local wire=require("wire")

print(  )
print( "wire starting" )
print( "we are " , wire.threads.us.name , wire.threads.us.handle )
DUMP( package.path )

wire.tasks("http",4,"require('wire').http_code()")

wire.thread({
	start=[[
	
local wire=require("wire")

print(  )
print( "thread starting" , wire.threads.us.name , wire.threads.us.handle )
DUMP( package.path )

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

for i=3,1,-1 do
	print("sleep" , i , wire.time() )
	wire.sleep(1)
end


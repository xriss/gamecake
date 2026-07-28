
local wire=require("wire")

print(  )
print( "wire starting" )
print( "we are " , wire.threads.us.name , wire.threads.us.handle )


wire.thread({
	start=[[
	
local wire=require("wire")

print(  )
print( "thread starting" , wire.threads.us.name , wire.threads.us.handle )

local memo=wire.memo({
	fifo=wire.threads.house,
	data={
		action="handle",
		name="test",
	},
})
memo:send()
memo:resolve()

print( "thread ending" , wire.threads.us.name , wire.threads.us.handle )
	
]],
})

local memo=wire.memo({
	fifo=wire.threads.house,
	data={
		action="handle",
		name="test",
	},
})
memo:send()
memo:resolve()

for i=3,1,-1 do
	print("sleep" , i , wire.time() )
	wire.sleep(1)
end


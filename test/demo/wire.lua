
local gamecake=require("wetgenes.gamecake.core")

local core=require("wire.core")

print(core)

print(gamecake.preloadlibs)

T1=core.thread_create( 0 , [[

print("thread",...)

local gamecake=require("wetgenes.gamecake.core")
print(gamecake.preloadlibs)

]],gamecake.preloadlibs )


T2=core.thread_create( 0 , [[

print("thread",...)

]],gamecake.preloadlibs )


T3=core.thread_create( 0 , [[

print("thread",...)

]],gamecake.preloadlibs )


core.thread_sleep(3);

core.thread_destroy(T1)
core.thread_destroy(T2)
core.thread_destroy(T3)

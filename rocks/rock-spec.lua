
local sandbox=require("wetgenes.sandbox")

ROCK_DIR      = assert(os.getenv("ROCK_DIR"))
ROCK_NAME     = assert(os.getenv("ROCK_NAME"))
ROCK_VERSION  = assert(os.getenv("ROCK_VERSION"))
ROCK_REVISION = assert(os.getenv("ROCK_REVISION"))
ROCK_FILENAME = assert(os.getenv("ROCK_FILENAME"))


print( "updating" , ROCK_FILENAME )

-- load base file
local fp=assert(io.open("./base.rockspec","rb"))
local text=assert(fp:read("*all"))
fp:close()

-- pare 
local spec=sandbox.load_ini(text)

-- adjust value
spec.version=ROCK_VERSION.."-"..ROCK_REVISION
spec.package=ROCK_NAME
spec.source={url=ROCK_DIR.."/src.zip"}


--save
local fp=assert(io.open( ROCK_FILENAME ,"wb"))
assert(fp:write( sandbox.save_ini(spec) ))
fp:close()

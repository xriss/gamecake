
local sandbox=require("wetgenes.sandbox")

ROCK_DIR      = assert(os.getenv("ROCK_DIR"))
ROCK_NAME     = assert(os.getenv("ROCK_NAME"))
ROCK_VERSION  = assert(os.getenv("ROCK_VERSION"))
ROCK_REVISION = assert(os.getenv("ROCK_REVISION"))
ROCK_BASENAME = assert(os.getenv("ROCK_BASENAME"))
ROCK_README   = assert(os.getenv("ROCK_README"))


print( "updating" , ROCK_BASENAME..".rockspec" )

-- load base file
local fp=assert(io.open("./base.rockspec","rb"))
local text=assert(fp:read("*all"))
fp:close()

local readme
do
	local fp=io.open(ROCK_README,"rb")
	if fp then
		readme=assert(fp:read("*all"))
		fp:close()
	end
end

-- pare 
local spec=sandbox.load_ini(text)

-- adjust value
spec.version=ROCK_VERSION.."-"..ROCK_REVISION
spec.package=ROCK_NAME
spec.source={url="file://"..ROCK_DIR.."/src.zip",dir="src"}

if readme then -- append readme
	spec.description.detailed=spec.description.detailed..readme
end

--save
local fp=assert(io.open( ROCK_BASENAME..".rockspec" ,"wb"))
assert(fp:write( sandbox.save_ini(spec) ))
fp:close()

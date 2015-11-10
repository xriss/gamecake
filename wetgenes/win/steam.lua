--
-- (C) 2015 wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local dprint=function(a) print(wstr.dump(a)) end

local steam={}

-- disable as this is all borken right now...
--do return steam end

-- we really need jit and ffi, exit here if we do not have them
local jit pcall(function() jit=require("jit") end) if not jit then return steam end
local ffi pcall(function() ffi=require("ffi") end) if not ffi then return steam end

local pragma_pack_size=1
local libdirname=""
local libname="steam_api.dll"
if jit.os=="Windows" and jit.arch=="x64" then libdirname=""     libname="steam_api64.dll"		pragma_pack_size=16		end
if jit.os=="Linux"   and jit.arch=="x86" then libdirname="x32/" libname="libsteam_api.so"								end
if jit.os=="Linux"   and jit.arch=="x64" then libdirname="x64/" libname="libsteam_api.so"								end
if jit.os=="OSX"                         then libdirname="osx/" libname="libsteam_api.dylib"							end

-- search for the steam_api library, libdirname is probably not needed due to rpath linker config but just in case we mucked that up
local libpath
local lib		pcall(function() libpath=libdirname..libname		lib=ffi.load(libpath) end)
if not lib then	pcall(function() libpath=libname					lib=ffi.load(libpath) end) end


--print("STEAM",libpath,tostring(lib))

-- no steam lib found
if not lib then return steam end

print("STEAM API -> "..libpath.." pack("..pragma_pack_size..")")


ffi.cdef( [[

void *SteamApps();
void *SteamUtils();
void *SteamUser();
void *SteamUserStats();

const char * SteamAPI_ISteamApps_GetCurrentGameLanguage(void *_p);
const char * SteamAPI_ISteamUtils_GetSteamUILanguage(void *_p);


uint64_t SteamAPI_ISteamUser_GetSteamID(void *_p);

uint64_t SteamAPI_ISteamUserStats_FindLeaderboard(void *_p, const char * pchLeaderboardName);


bool SteamAPI_ISteamUtils_GetAPICallResult(void *_p, uint64_t hSteamAPICall, void* ptr, int cubCallback, int iCallbackExpected, bool * pbFailed);

uint64_t SteamAPI_ISteamUserStats_UploadLeaderboardScore(void *_p, uint64_t hSteamLeaderboard, int eLeaderboardUploadScoreMethod, int32_t nScore, const int32_t *pScoreDetails, int cScoreDetailsCount);


bool SteamAPI_ISteamUserStats_RequestCurrentStats(void *_p);
bool SteamAPI_ISteamUserStats_StoreStats(void *_p);

bool SteamAPI_ISteamUserStats_GetAchievement(void *_p, const char * pchName, bool * pbAchieved);
bool SteamAPI_ISteamUserStats_SetAchievement(void *_p, const char * pchName);
bool SteamAPI_ISteamUserStats_ClearAchievement(void *_p, const char * pchName);
bool SteamAPI_ISteamUserStats_IndicateAchievementProgress(void *_p, const char * pchName, uint32_t nCurProgress, uint32_t nMaxProgress);



bool SteamAPI_Init();
bool InitSafe();

#pragma pack( push, ]]..pragma_pack_size..[[)

struct LeaderboardFindResult_t
{
	uint64_t m_hSteamLeaderboard;	// handle to the leaderboard serarched for, 0 if no leaderboard found
	uint8_t m_bLeaderboardFound;				// 0 if no leaderboard found
};

struct LeaderboardScoreUploaded_t
{
	uint8_t		m_bSuccess;				// 1 if the call was successful
	uint64_t	m_hSteamLeaderboard;	// the leaderboard handle that was
	int32_t		m_nScore;				// the score that was attempted to set
	uint8_t		m_bScoreChanged;		// true if the score in the leaderboard change, false if the existing score was better
	int			m_nGlobalRankNew;		// the new global rank of the user in this leaderboard
	int			m_nGlobalRankPrevious;	// the previous global rank of the user in this leaderboard; 0 if the user had no existing entry in the leaderboard
};

#pragma pack( pop )

]])

-- try a few ways to initalise
local steam_active=false
if not steam_active then pcall(function() if lib.InitSafe      and lib.InitSafe()      then steam_active=true end end) end
if not steam_active then pcall(function() if lib.SteamAPI_Init and lib.SteamAPI_Init() then steam_active=true end end) end
if not steam_active then return steam end

--print("STEAM","Init")

local lib_SteamApps      = lib.SteamApps()
local lib_SteamUtils     = lib.SteamUtils()
local lib_SteamUser      = lib.SteamUser()
local lib_SteamUserStats = lib.SteamUserStats()


-- get language of game, not sure how the game language gets set, so use UILanguage only?
local a
--a=lib.SteamAPI_ISteamApps_GetCurrentGameLanguage(lib_SteamApps)			-- check game language
--a=a and ffi.string(a) ; if a=="" then a=nil end							-- maybe ""?

if not a then a=lib.SteamAPI_ISteamUtils_GetSteamUILanguage(lib_SteamUtils) end	-- check steam UI language
a=a and ffi.string(a) ; if a=="" then a=nil end									-- maybe ""?

steam.language=a or "english"

print("STEAM language ",tostring(steam.language))

local a=lib.SteamAPI_ISteamUser_GetSteamID(lib_SteamUser)
if a then
	steam.userid=tostring(a):gsub("[^%d]*","") -- remove ULL from the end of the unsigned long long number string
end
print("STEAM userid",steam.userid)

-- table of [functions]=true to call until they return true
steam.results={}

-- call every function in results until it returns true, returns total number of functions called, so 0 when done
-- you can wrap a result function with a callback that would then complete here
steam.results_call=function()
	local count=0
	for result in pairs(steam.results) do
		count=count+1
		if result() then steam.results[result]=nil end -- call and forget if it returns true
	end
	return count
end

steam.leaderboards={}

-- *busywait* to get the board of the given name (probably preloaded by leaderboards_prepare function)
steam.leaderboards_board=function(name)
	if not steam.leaderboards[name] then
		steam.results[steam.leaderboards_setup(name)]=true
		repeat until steam.results_call()==0 -- busy wait here for everything we are waiting on
	end
	if type(steam.leaderboards[name])=="table" then	return steam.leaderboards[name] end
end

-- initial leaderboard setup, fetch id and setup board table
-- return a result function that needs to be polled until it returns a result
steam.leaderboards_setup=function(name)
	local req=lib.SteamAPI_ISteamUserStats_FindLeaderboard(lib_SteamUserStats,name)
	local ret=ffi.new("struct LeaderboardFindResult_t")
	local dropped=ffi.new("bool[1]")

	local done
	return function() -- result
		if done then return done end
		if lib.SteamAPI_ISteamUtils_GetAPICallResult(lib_SteamUtils, req, ret, ffi.sizeof(ret), 1104, dropped) then
			done=ret
			if ret.m_bLeaderboardFound~=0 then -- success
				local board=steam.leaderboards[name] or {} -- reuse table if called multiple times
				steam.leaderboards[name]=board
				board.name=name
				board.id=ret.m_hSteamLeaderboard
			end
			return done
		end
	end
end

-- submit a new leaderboard score to steam
-- return a result function that needs to be polled until it returns a result
steam.leaderboards_score=function(name,score)
	local board=steam.leaderboards_board(name)
	if not board then return end
	
	local req=lib.SteamAPI_ISteamUserStats_UploadLeaderboardScore(lib_SteamUserStats,board.id,1,score,nil,0)
	local ret=ffi.new("struct LeaderboardScoreUploaded_t")
	local dropped=ffi.new("bool[1]")
	
	local done
	return function() -- result
		if done then return done end
		if lib.SteamAPI_ISteamUtils_GetAPICallResult(lib_SteamUtils, req, ret, ffi.sizeof(ret), 1106, dropped) then
			done=ret
			board.rank=ret.m_nGlobalRankNew
			return done
		end
	end
end

-- local unlocked cache
steam.achievements={}

-- you don't get a achievment popup *until* you call SteamAPI_ISteamUserStats_StoreStats
steam.achievement_store_locked=false
steam.achievement_store=function(forced)
	if (not steam.achievement_store_locked) or forced then
		lib.SteamAPI_ISteamUserStats_StoreStats(lib_SteamUserStats)
	end
end

-- unlock or lock a named achievement
steam.achievement_set=function(name,value)
	if type(value)=="boolean" and (not value) then -- clear if value is false
		steam.achievements[name]=false
		lib.SteamAPI_ISteamUserStats_ClearAchievement(lib_SteamUserStats,name)
		steam.achievement_store()
	else -- set if value is missing or is true
		steam.achievements[name]=true
		lib.SteamAPI_ISteamUserStats_SetAchievement(lib_SteamUserStats,name)
		steam.achievement_store()
	end
end

-- get state of an achievement (using local cache)
steam.achievement_get=function(name)
	if type(steam.achievements[name])=="boolean" then return steam.achievements[name] end -- use cache
	local ret=ffi.new("bool[1]") ret[0]=0
	lib.SteamAPI_ISteamUserStats_GetAchievement(lib_SteamUserStats,name,ret)
	local value=ret[0] and true or false
	steam.achievements[name]=value
	return value
end


-- test codes
--[[

-- request leaderboards
for index,name in ipairs{"scores","levels","cookies"} do
	steam.results[steam.leaderboards_setup(name)]=true
end

-- wait for leaderboards results
repeat until steam.results_call()==0

-- print leaderboards info
for n,board in pairs(steam.leaderboards) do
	print(" STEAM leaderboard "..board.name.." : "..tostring(board.id) )
end


print( "achievement unlocked " .. tostring(steam.achievement_get("level")) )
steam.achievement_set("level",false)
print( "achievement unlocked " .. tostring(steam.achievement_get("level")) )
steam.achievement_set("level",true)
print( "achievement unlocked " .. tostring(steam.achievement_get("level")) )
steam.achievement_store()


os.exit(0)

]]

steam.loaded=true -- flag that steam is loaded and setup has been done


return steam

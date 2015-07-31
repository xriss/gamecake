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

local libdirname=""
local libname="steam_api.dll"
if jit.os=="Windows" and jit.arch=="x64" then libdirname=""     libname="steam_api64.dll"		end
if jit.os=="Linux"   and jit.arch=="x86" then libdirname="x32/" libname="libsteam_api_wrap.so"	end
if jit.os=="Linux"   and jit.arch=="x64" then libdirname="x64/" libname="libsteam_api_wrap.so"	end
if jit.os=="OSX"                         then libdirname="osx/" libname="libsteam_api.dylib"	end

-- search for the steam_api library
local libpath
local lib		pcall(function() libpath=libdirname..libname		lib=ffi.load(libpath) end)
if not lib then	pcall(function() libpath=libname					lib=ffi.load(libpath) end) end


print("STEAM",libpath,tostring(lib))

-- no steam lib found
if not lib then return steam end


ffi.cdef[[

void *SteamApps();
void *SteamUtils();
void *SteamUser();
void *SteamUserStats();

const char * SteamAPI_ISteamApps_GetCurrentGameLanguage(void *_p);


uint64_t SteamAPI_ISteamUser_GetSteamID(void *_p);

uint64_t SteamAPI_ISteamUserStats_FindLeaderboard(void *_p, const char * pchLeaderboardName);


bool SteamAPI_ISteamUtils_GetAPICallResult(void *_p, uint64_t hSteamAPICall, void* ptr, int cubCallback, int iCallbackExpected, bool * pbFailed);

uint64_t SteamAPI_ISteamUserStats_UploadLeaderboardScore(void *_p, uint64_t hSteamLeaderboard, int eLeaderboardUploadScoreMethod, int32_t nScore, const int32_t *pScoreDetails, int cScoreDetailsCount);


bool SteamAPI_ISteamUserStats_RequestCurrentStats(void *_p);
bool SteamAPI_ISteamUserStats_StoreStats(void *_p);

bool SteamAPI_ISteamUserStats_GetAchievement(void *_p, const char * pchName, bool * pbAchieved);
bool SteamAPI_ISteamUserStats_SetAchievement(void *_p, const char * pchName);
bool SteamAPI_ISteamUserStats_ClearAchievement(void *_p, const char * pchName);
bool SteamAPI_ISteamUserStats_IndicateAchievementProgress(void *_p, const char * pchName, uint32 nCurProgress, uint32 nMaxProgress);



bool SteamAPI_Init();
bool InitSafe();

#pragma pack( push, ]]..( jit.os=="Linux" or jit.os=="OSX" ) and 4 or 8)..[[

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


]]

-- failed to init
if lib.InitSafe then -- use wrapper
	if not lib.InitSafe() then return steam end
else
	if not lib.SteamAPI_Init() then return steam end
end

--print("STEAM","Init")


local lib_SteamApps      = lib.SteamApps()
local lib_SteamUtils     = lib.SteamUtils()
local lib_SteamUser      = lib.SteamUser()
local lib_SteamUserStats = lib.SteamUserStats()



local a=lib.SteamAPI_ISteamApps_GetCurrentGameLanguage(lib_SteamApps)
if a then
	steam.language=a and ffi.string(a)
end
print("STEAM language",steam.language)

local a=lib.SteamAPI_ISteamUser_GetSteamID(lib_SteamUser)
if a then
	steam.userid=tostring(a):gsub("[^%d]*","") -- remove ULL from the end of the unsigned long long number string
end
print("STEAM userid",steam.userid)

steam.leaderboards={}

steam.leaderboards_setup=function(name)
	local req=lib.SteamAPI_ISteamUserStats_FindLeaderboard(lib_SteamUserStats,name)
	local ret=ffi.new("struct LeaderboardFindResult_t")
	local dropped=ffi.new("bool[1]")
	
	return function()
		repeat until lib.SteamAPI_ISteamUtils_GetAPICallResult(lb_SteamUtils, req, ret, ffi.sizeof(ret), 1104, dropped)
--print(ffi.sizeof(ret),dropped[0],ret.m_hSteamLeaderboard,ret.m_bLeaderboardFound)

		steam.leaderboards[name]={}
		steam.leaderboards[name].id=ret.m_hSteamLeaderboard

		return ret.m_hSteamLeaderboard
	end
end

steam.leaderboards_score=function(name,score)
	local req=lib.SteamAPI_ISteamUserStats_UploadLeaderboardScore(lib_SteamUserStats,steam.leaderboards[name].id,1,score,nil,0)
	local ret=ffi.new("struct LeaderboardScoreUploaded_t")
	local dropped=ffi.new("bool[1]")
	
	return function()
	repeat until lib.SteamAPI_ISteamUtils_GetAPICallResult(lb_SteamUtils, req, ret, ffi.sizeof(ret), 1106, dropped)

		steam.leaderboards[name].rank=m_nGlobalRankNew
		return ret.m_bSuccess

	end
end



local ns={"scores","levels","cookies"}
local bs={}
for i,n in ipairs(ns) do
	bs[n]=steam.leaderboards_find(n) -- request leader board ids
end
for i,n in ipairs(ns) do
	bs[n]() -- block for return values
	print("STEAM leaderboards."..n,steam.leaderboards[n].id)
end

--[[
local req=lib.SteamAPI_ISteamUserStats_UploadLeaderboardScore(lib_SteamUserStats,steam.leaderboard_scores,1,13,nil,0)
local ret=ffi.new("struct LeaderboardScoreUploaded_t")
local dropped=ffi.new("bool[1]")
repeat until lib.SteamAPI_ISteamUtils_GetAPICallResult(lb_SteamUtils, req, ret, ffi.sizeof(ret), 1106, dropped)
print(ffi.sizeof(ret),dropped[0],ret.m_hSteamLeaderboard,ret.m_bSuccess)
]]

--os.exit()

steam.loaded=true -- flag that steam is loaded and setup has been done

return steam

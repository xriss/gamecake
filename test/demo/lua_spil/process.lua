#!/usr/bin/env gamecake


local wstr=require("wetgenes.string")

print("OK")

local maxw=8
local threshold=10
local weight=10

-- frequency of each group of letters
local freqs={}
for i=1,maxw do freqs[i]={} end

local words={}

local fsplit_one=function(word)

	if #word==0 then   return "","",""
	elseif #word==1 then  return "",word,""
	end
	
	local best=0
	local best_idx=0
	local best_width=0
	for w=maxw,3,-1 do
		local b=freqs[w]
		for i=1,1+#word-w do
			local f=word:sub(i,i+w-1)
			if b[f] then
				if b[f]*(weight^w)>best then -- better
					best=b[f]*(weight^w)
					best_idx=i
					best_width=w
				end
			end
		end
	end
	if best>0 then -- found it
		return word:sub(1,best_idx-1) , word:sub(best_idx,best_idx+best_width-1) , word:sub(best_idx+best_width)
	end

	return word:sub(1,1) , word:sub(2,2) , word:sub(3)
	
end

local fsplit=function(word)

	local split;split=function(s)
		local a,b,c = fsplit_one(s)
		if #a>1 then a=split(a) end
		if #c>1 then c=split(c) end
		return {a,b,c}
	end
	local ret={}
	local pick;pick=function(t)
		for i=1,#t do
			if type(t[i])=="table" then
				pick(t[i])
			else
				if t[i]~="" then
					ret[#ret+1]=t[i]
				end
			end
		end
	end
	pick( split(word) )
	return ret
end

local fp = io.open("input.txt", "r")
for line in fp:lines() do
	local word=wstr.split(line,"/")[1] or ""
	
	if word:find("%d") then word="" end -- ignore numbers
	if word:find("%u") then word="" end -- ignore capitols
	if word:find("%A") then word="" end -- ignore non letters

	if word~="" then
	
		words[#words+1]=word
	
		for w=2,maxw do
			local b=freqs[w]
			for i=1,1+#word-w do
				local f=word:sub(i,i+w-1)
				b[f]=(b[f] or 0)+1
			end
		end
	end
end

-- remove singles
for w=2,maxw do
	local b=freqs[w]	
	local t={}
	for k,v in pairs(b) do
		if v<threshold then
			b[k]=nil -- kill it
		end
	end
end

local parts
repeat
	local done=true
	parts={}
	for _,word in ipairs(words) do
		local t=fsplit(word)
		for i,v in ipairs(t) do
			parts[v]=( parts[v] or 0 ) + 1
		end
	end

	for n,c in pairs(parts) do
		if c<threshold then -- not used enough to bother
			freqs[#n][n]=nil
			done=false -- and try again
		end
	end
until done

for _,word in ipairs(words) do
	print( table.concat(fsplit(word)," ") )
end

local list={}
for n,c in pairs(parts) do
	list[#list+1]={n,c}
end
table.sort(list,function(a,b) if #a[1]==#b[1] then return a[2]>b[2] else return #a[1]<#b[1] end end)

local cmap={}
for i=1,maxw do cmap[i]={} end
for i,v in ipairs(list) do
	local w=#v[1]
	cmap[w][#cmap[w]+1]=v[1]
end
for i,v in ipairs(cmap) do
print(i,#v,table.concat(v))
end

--print(wstr.dump(freqs))

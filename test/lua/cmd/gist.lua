#!/bin/env gamecake

local global=_G ; pcall(function() global=require("global") end)
local logs=require("wetgenes.logs")
local djon=require("djon")

require("apps").default_paths() -- default search paths so things can easily be found

local tasks=require("wetgenes.tasks").create()
tasks:add_global_thread({
	count=8,
	id="http",
	code=tasks.http_code,
	globals={
		TASK_NAME="#HTTP"
	}
})


local gist=require("wetgenes.tasks_gist")

local opts={}
opts.tasks=tasks
opts.token=os.getenv("GIST_TOKEN") -- must provide a token
opts.per_page=100
opts.page=1

local result=gist.list(opts)
logs.dump(result)
for i,v in ipairs(result) do
	print(i,v.id,v.description)
	for n,f in pairs(v.files or {}) do
		print("","",n)
	end
end

os.exit(0)


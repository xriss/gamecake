--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- keep track of all the open documents

M.bake=function(oven,docs)

	local gui=oven.rebake(oven.modname..".gui")


	local doc={}
	local docmeta={__index=doc}
	local docs=docs or {}
	docs.oven=oven
	
	docs.modname=M.modname


	docs.list={} -- list of open documents

	docs.refresh_item=function(it,depth,opts)
			if it.name then

				local loaded=docs.find(it.path)

				local prefix="- "
				if loaded then prefix="+ " end

				if it.mode=="directory" then
					if it[1] then
						prefix="= "
					else
						prefix="> "
					end
					it.text=prefix..it.name.."/"
				else
					it.text=prefix..it.name
				end
			end
	end
	
	docs.refresh_items=function(widget,items)

--		local items=gui.master.ids.treefile.tree_widget.items

		local rec ; rec=function(it)
			for _,v in ipairs(it) do
				rec(v)
			end
			docs.refresh_item(it)
		end
		rec(items)
--		gui.master.ids.treefile:refresh()
--dprint(items)
--[[
		for i,v in ipairs(tree) do
			v.prefix="  "
			for _,doc in ipairs( docs.list ) do
				if doc.filename==v.path then
					v.prefix="* "
				end
			end
		end
]]
	end

	docs.find=function(filename)

		for _,it in ipairs(docs.list) do
			if it.filename==filename then
				return it
			end
		end

	end
	
	docs.manifest=function(filename)

		return docs.find(filename) or docs.create(filename)
	end

	docs.create=function(filename)

		local it={}

		docs.list[#docs.list+1]=it

		setmetatable(it,docmeta)

		
		it.docs=docs

		it.txt=require("wetgenes.txt").construct()
		

		it:load(filename)

--		docs.refresh()

		return it
	end


-- show this document in the main text editor window
	doc.show=function(it)
	
		it.docs.doc=it -- remember current doc
	
		it.txt.hooks={}
		gui.master.ids.texteditor.txt.hooks={}
		gui.master.ids.texteditor.set_txt(it.txt)

		return it
	end

	doc.save=function(it,filename)
	
		if filename then it.filename=filename end
		local s=it.txt.get_text()
		local f=io.open(it.filename,"wb")
		if f then
			local d=f:write(s)
			f:close()
		end

	end

	doc.load=function(it,filename)


		if not filename then
			filename=os.date("swanky-%Y%m%d-%H%M%S.txt")
		end
		it.filename=filename
		it.txt.set_text("\n",filename)


		local f=io.open(filename,"rb")
		if f then
			local d=f:read("*a")
			if d then
				it.txt.set_text(d,filename)
			end
			f:close()
		end

		it.txt.set_lexer()

		return it
	end


	return docs
end

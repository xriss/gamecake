--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local wpath=require("wetgenes.path")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- keep track of all the open documents

M.bake=function(oven,docs)

	local gui=oven.rebake(oven.modname..".gui")
	local collect=oven.rebake(oven.modname..".collect")

	local wtreefile=oven.rebake("wetgenes.gamecake.widgets.treefile")


	local doc={}
	local docmeta={__index=doc}
	local docs=docs or {}
	docs.oven=oven
	
	docs.modname=M.modname


	docs.list={} -- list of open documents

	docs.item_refresh=function(treefile,item)

		wtreefile.item_refresh(treefile,item)

		if item.mode~="directory" then
			local loaded=docs.find(item.path)
			item.line_prefix.text_color=nil
			if loaded then
				item.line_prefix.text_color=0xff00cc00
				if loaded.modified_index~=loaded.txt.undo.index then
					item.line_prefix.text_color=0xffcc0000
					item.line_prefix.text="* "
				else
					item.line_prefix.text="+ "
				end
			end
		end
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

	docs.search={} -- share this search info with all txts

	docs.create=function(filename)
	
		local it={}

		docs.list[#docs.list+1]=it

		setmetatable(it,docmeta)

		
		it.docs=docs

		it.txt=require("wetgenes.txt").construct({search=docs.search})
		it.txt.doc=it

		it:load(filename)

--		docs.refresh()

		it.modified_index=it.txt.undo.index

		return it
	end

	docs.update=function()

		local doc=docs.doc
		if not doc then return end

		if doc.modified_index==doc.txt.undo.index then
			if doc.modified_show==true then
				gui.refresh_tree()
				doc.modified_show=false
				gui.master.ids.infobar.text_color=nil
				gui.master.ids.infobar:set_dirty()
			end
		else
			if doc.modified_show~=true then
				gui.refresh_tree()
				doc.modified_show=true
				gui.master.ids.infobar.text_color=0xffcc0000
				gui.master.ids.infobar:set_dirty()
			end
		end
	end


-- show this document in the main text editor window
	doc.show=function(it)
	
		it.docs.doc=it -- remember current doc
	
		it.txt.hooks={}
		gui.master.ids.texteditor.txt.hooks={}
		gui.master.ids.texteditor.set_txt(it.txt)

		gui.master.ids.infobar.text=it.filename
		if it.modified_index==it.txt.undo.index then
			gui.master.ids.infobar.text_color=nil
		else
			gui.master.ids.infobar.text_color=0xffcc0000
		end
		gui.master.ids.infobar:set_dirty()


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
		it.modified_index=it.txt.undo.index

	end

	doc.load=function(it,filename)


		if not filename then
			filename=wpath.currentdir()..os.date("swanky-%Y%m%d-%H%M%S.txt")
		end
		it.filename=filename
		it.txt.set_text("\n",filename)

		gui.master.ids.treefile:add_file_item(it.filename)
		
		collect.load(filename)

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

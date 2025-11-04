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


	-- simple list of open documents
	docs.list={}

	docs.item_refresh=function(treefile,item)
--[[
		wtreefile.item_refresh(treefile,item)

		if item.mode~="directory" then
			local loaded=docs.find(item.path)
			item.line_prefix.text_color=nil
			if loaded then
				item.line_prefix.text_color=0xff00cc00
				if loaded.meta.undo~=loaded.txt.undo.index then
					item.line_prefix.text_color=0xffcc0000
					item.line_prefix.text="* "
				else
					item.line_prefix.text="+ "
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
		if not filename then
			return docs.create()
		end
		if filename:sub(1,2)~="//" then -- do nothing with special // paths
			filename=wpath.resolve(filename) -- fix path ( eg use cd )
		end
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
		it.txt.undo.list_hook=function(undo,mode,index,data)
			local it=undo.txt.doc
			if     mode=="trim" then	collect.undo_trim(it,index)
			elseif mode=="set"  then	collect.undo_update(it,index,data)
			else	error("unknown undo.list_hook mode : "..mode )
			end
		end

		it:load(filename)


		return it
	end

	docs.update=function()

		local doc=docs.doc
		if not doc then return end

		if doc.meta.undo==doc.txt.undo.index then
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


	docs.config_load=function()
		if not collect.config.docs      then return end
		if not collect.config.docs.open then return end

		-- close any open docs
		for idx=#docs.list,1,-1 do
			local v=docs.list[idx]
			v:close()
		end

		-- open saved docs
		for i,v in ipairs( collect.config.docs.open ) do
		end

	end

	docs.config_save=function()
		if not collect.config.docs      then collect.config.docs={}      end
		if not collect.config.docs.open then collect.config.docs.open={} end
		for i,v in ipairs( docs.list ) do
			local t={}
			collect.config.docs.open[ v.filename ] = t
		end

	end

	docs.save_all=function()
		for i,v in ipairs( docs.list ) do
			if v:is_modified() then
				v:save()
			end
		end
	end

	-- show or disable document display
	docs.show=function(doc)
		if not doc then
			docs.doc=nil
			gui.master.ids.texteditor.hidden=true
			gui.master.ids.infobar.text=""
			gui.master.ids.infobar.text_color=nil
			gui.master.ids.infobar:set_dirty()
		else
			docs.doc=doc
			doc:show()			
		end
	end

	doc.is_modified=function(it)
		return it.meta.undo~=it.txt.undo.index
	end
-- show this document in the main text editor window
	doc.show=function(it)
	
		it.docs.doc=it -- remember current doc
	
		it.txt.hooks={}
		gui.master.ids.texteditor.txt.hooks={}
		gui.master.ids.texteditor.set_txt(it.txt)
		gui.master.ids.texteditor.hidden=false

		gui.master.ids.infobar.text=it.filename
		if it.meta.undo==it.txt.undo.index then
			gui.master.ids.infobar.text_color=nil
		else
			gui.master.ids.infobar.text_color=0xffcc0000
		end
		gui.master.ids.infobar:set_dirty()


		return it
	end
	doc.save=function(it,filename)

-- trim on save ( so you can undo then save and know that the data is gone gone gone )	
		it.txt.undo.list_trim()
	
		if filename and it.filename~=filename then

-- TODO stop us saving over the name of another opened doc

			it.filename=filename
-- save with a new filename is basically a dupe or file copy.
			collect.save(it,filename)
		else
			collect.save(it)
		end


	end

	doc.load=function(it,filename)


		if not filename then
			filename=wpath.currentdir()..os.date("swed-%Y%m%d.txt")
		end
		it.filename=filename
		it.txt.set_text("\n",filename)
		
		collect.manifest_path(filename)
		collect.load(it,filename)
		it.txt.set_lexer()

		return it
	end
	
	doc.reload=function(it)
		collect.load(it,filename)		
		it.txt.set_lexer()
	end

	doc.close=function(it)

		-- remove from docs list
		for idx=#docs.list,1,-1 do
			local v=docs.list[idx]
			if v==it then
				table.remove(docs.list,idx)
			end
		end
		
		if docs.doc==it then -- currently viewing this doc
			docs.show( docs.list[1] ) -- show another doc or no doc if list is empty
		end

	end
	
	return docs
end

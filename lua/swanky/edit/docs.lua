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
	docs.list_modified=false

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
	
	docs.get_keepers=function()
		local t={}
		t["/"]=true -- force roots
		t["//"]=true
		
		local addpath;addpath=function(path)
			t[path]=true
			if path:sub(-1)=="/" then path=path:sub(1,-2) end
			local s,f=path:find("[^/]*$")
			if s and s>1 then
				addpath( path:sub(1,s-1) )
			end
		end
		
		for _,it in ipairs(docs.list) do
			addpath(it.filename)
		end
		
		return t
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
		docs.list_modified=true

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

		local get_text_color=function(tint)
			local c=gui.master.get_color(nil,tint)
			local cc=        bit.band(c[3]*0xff,0xff)
			cc=cc+bit.lshift(bit.band(c[2]*0xff,0xff),8)
			cc=cc+bit.lshift(bit.band(c[1]*0xff,0xff),16)
			cc=cc+bit.lshift(bit.band(c[4]*0xff,0xff),24)
			return cc
		end

		if doc.meta.undo~=doc.txt.undo.index then
			if gui.master.ids.infobar.text_color~=get_text_color(0x80ff0000) then
				gui.master.ids.infobar.text_color=get_text_color(0x80ff0000)
				gui.master.ids.infobar:set_dirty()
				gui.refresh_tree()
			end
		else
			if gui.master.ids.infobar.text_color~=get_text_color(0x8000ff00) then
				gui.master.ids.infobar.text_color=get_text_color(0x8000ff00)
				gui.master.ids.infobar:set_dirty()
				gui.refresh_tree()
			end
		end
		
		if docs.list_modified then
			docs.config_save()
			collect.save_config("docs")
			gui.refresh_tree()
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
		for idx=#collect.config.docs.open,1,-1 do
			local v=collect.config.docs.open[idx]
			if v then -- skip falses
				docs.manifest(v.filename):show()
			end
		end

		docs.list_modified=false
	end

	docs.config_save=function()
		if not collect.config.docs      then collect.config.docs={}      end
		collect.config.docs.open={}
		for i,v in ipairs( docs.list ) do
			local t={
				filename=v.filename
			}
			table.insert( collect.config.docs.open , t )
		end

		docs.list_modified=false
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
	
		for i,v in ipairs(docs.list) do
			if v==it then
				if i~=1 then -- move to front
					table.insert(docs.list,1, table.remove(docs.list,i) )
					docs.list_modified=true
				end
				break
			end
		end

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

	doc.load=function(it,filename,reload)

		if filename then
			it.filename=filename
		end
		if not it.filename then
			it.filename=wpath.currentdir()..os.date("swed-%Y%m%d.txt")
		end
		it.txt.set_text("\n",it.filename)
		
		collect.manifest_path(it.filename)
		collect.load(it,it.filename,reload)
		it.txt.set_lexer()

		return it
	end
	
	doc.reload=function(it)
		local text_file=collect.mounts:read_file(it.filename)
		local text_doc=it.txt.get_text()
		
		if text_file~=text_doc then -- text in file is not the same as text in memory
			it.txt.mark(0,0,it.txt.hy+1,0)
			it.txt.undo.replace(text_file)
		end
	end

	doc.close=function(it)

		-- remove from docs list
		for idx=#docs.list,1,-1 do
			local v=docs.list[idx]
			if v==it then
				table.remove(docs.list,idx)
			end
		end
		docs.list_modified=true
		
		if docs.doc==it then -- currently viewing this doc
			docs.show( docs.list[1] ) -- show another doc or no doc if list is empty
		end

	end
	
	return docs
end

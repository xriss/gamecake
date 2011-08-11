#! /usr/bin/env lua

--[[
	Dynamically builded trees that reflects the library contents.
--]]

require("lgob.gtk")

Navigator = {}

function Navigator.new()
	local self = {}
	setmetatable(self, {__index = Navigator})
	
	self.window = gtk.Window.new()
	self.scroll1 = gtk.ScrolledWindow.new()
	self.scroll2 = gtk.ScrolledWindow.new()
	
	self.scroll1:set("hscrollbar-policy", gtk.POLICY_AUTOMATIC,
		"vscrollbar-policy", gtk.POLICY_AUTOMATIC)
		
	self.scroll2:set("hscrollbar-policy", gtk.POLICY_AUTOMATIC,
		"vscrollbar-policy", gtk.POLICY_AUTOMATIC)
	
	self.libLabel = gtk.Label.new("Library")
	self.libEntry = gtk.Entry.new()
	self.libEntry:set("text", "lgob.gtk")
	self.libButton = gtk.Button.new_with_mnemonic("Load")
	
	self.libButton:connect("clicked", 
		function(self)
			self:populate(self.libEntry:get("text"))
		end
	, self
	)
	
	self.libHbox = gtk.HBox.new(false, 0)
	self.libHbox:pack_start(self.libLabel, false, false, 10)
	self.libHbox:pack_start(self.libEntry, false, false, 10)
	self.libHbox:pack_start(self.libButton, false, false, 0)

	self.classesModel = gtk.TreeStore.new("gchararray")
	self.classesView = gtk.TreeView.new_with_model(self.classesModel)
	self.classesCol = gtk.TreeViewColumn.new_with_attributes("Members", gtk.CellRendererText.new(), "text", 0)
	self.classesCol:set_sort_column_id(0)
	
	self.iter = gtk.TreeIter.new()
	self.dad = gtk.TreeIter.new()
	
	self.constsModel = gtk.TreeStore.new("gchararray")
	self.constsView = gtk.TreeView.new_with_model(self.constsModel)
	self.constsCol = gtk.TreeViewColumn.new_with_attributes("Constants", gtk.CellRendererText.new(), "text", 0)
	self.constsCol:set_sort_column_id(0)
	
	self.classesView:append_column(self.classesCol)
	self.classesCol:set("sort-indicator", false, "sort-order", gtk.SORT_DESCENDING)
	self.constsView:append_column(self.constsCol)

	self.scroll1:add(self.classesView)
	self.scroll2:add(self.constsView)
	
	self.hbox = gtk.HBox.new(true, 0)
	self.vbox = gtk.VBox.new(false, 5)
	
	self.status = gtk.Statusbar.new()
	self.sctx = self.status:get_context_id("info")

	self.hbox:add(self.scroll1)
	self.hbox:add(self.scroll2)
	self.vbox:pack_start(self.libHbox, false, true, 0)
	self.vbox:pack_start(self.hbox, true, true, 0)
	self.vbox:pack_start(self.status, false, true, 0)
	self.window:add(self.vbox)

	self.window:set("window-position", gtk.WIN_POS_CENTER, "title", "Navigator")
	self.window:set("width-request", 700, "height-request", 450)
	self.window:connect("delete-event", gtk.main_quit)
	self.window:show_all()
	
	self:populate()
	
	return self
end

function Navigator:setStatus(res, arg1, arg2, arg3)
	self.status:pop(self.sctx)
	local msg
	
	if res then
		msg = string.format("Classes: %d    Functions: %d    Constants: %d",
			arg1, arg2, arg3)
	else
		msg = arg1
	end
		
	self.status:push(self.sctx, msg)
end 

function Navigator:populate(libname)
	local iter, dad = self.iter, self.dad
	local libname = libname or "lgob.gtk"
	
	-- Reset the models
	self.classesView:set("model", nil)
	self.classesModel:clear()
	self.constsView:set("model", nil)
	self.constsModel:clear()
	
	-- Disable the sorting
	self.classesModel:set_sort_column_id(gtk.TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, gtk.SORT_ASCENDING)
	self.constsModel:set_sort_column_id(gtk.TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, gtk.SORT_ASCENDING)
	
	-- Load the library
	local res, lib = pcall(require, libname)
	
	if not res then
		self:setStatus(false, "Couldn't load the '" .. libname .. "' library")
		return
	elseif type(lib) ~= "table" then
        self:setStatus(false, "Couldn't list the exported symbols of the '" .. libname .. "' library" )
		return
    end
	
	local numClasses, numFunctions, numConstants = 0, 0, 0
	
	for i, j in pairs(lib) do
		-- Classes
		if type(j) == "table" then
			self.classesModel:append(dad)
			self.classesModel:set(dad, 0, i)
			numClasses = numClasses + 1
		
			-- Methods
			for o, k in pairs(j) do
				self.classesModel:append(iter, dad)
				self.classesModel:set(iter, 0, o)
				numFunctions = numFunctions + 1
			end
		-- Constants
		elseif type(j) == "number" then
			self.constsModel:append(iter)
			self.constsModel:set(iter, 0, i)
			numConstants = numConstants + 1
		-- Functions
		elseif type(j) == "function" then
			self.classesModel:append(iter)
			self.classesModel:set(iter, 0, i)
			numFunctions = numFunctions + 1
		end
	end
	
	-- Give some info to the user
	self:setStatus(true, numClasses, numFunctions, numConstants)
	
	-- Enable the Sort
	self.classesModel:set_sort_column_id(0, gtk.SORT_ASCENDING)
	self.constsModel:set_sort_column_id(0, gtk.SORT_ASCENDING)
	
	-- Set the models again
	self.classesView:set("model", self.classesModel)
	self.constsView:set("model", self.constsModel)
end

local nav = Navigator.new()
gtk.main()

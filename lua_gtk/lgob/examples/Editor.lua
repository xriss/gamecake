#! /usr/bin/env lua

require("lgob.gtkspell")
require("lgob.gdk")

---
-- Simple text editor.
Editor = {}
local __BACK, __FORWARD, __CLOSE = 0, 1, 2

---
-- Constructor
--
-- @return A new Editor instance
function Editor.new()
	local self = {}
	setmetatable(self, {__index = Editor})

	self.window = gtk.Window.new()
	self.vbox = gtk.VBox.new(false, 0)
	self.scroll = gtk.ScrolledWindow.new()
	self.scroll:set("hscrollbar-policy", gtk.POLICY_AUTOMATIC,
		"vscrollbar-policy", gtk.POLICY_AUTOMATIC)
	self.statusbar = gtk.Statusbar.new()
	self.context = self.statusbar:get_context_id("default")
	self.statusbar:push(self.context, "Untitled document")

	-- Find dialog
	self.findDialog, self.findEntry = self:buildFindDialog()

	-- Text
	self.view = gtk.TextView.new()
	self.spell = gtkspell.Spell.new_attach(self.view)
	self.buffer = self.view:get("buffer")

	-- Clipboard
	self.clip = gtk.clipboard_get(gdk.Atom.intern("CLIPBOARD"))

	-- File dialog
	self.dialog = gtk.FileChooserDialog.new("Select the file", self.window,
		gtk.FILE_CHOOSER_ACTION_OPEN, "gtk-cancel", gtk.RESPONSE_CANCEL,
		"gtk-ok", gtk.RESPONSE_OK)

	-- AccelGroup
	self.accelGroup = gtk.AccelGroup.new()
	self.window:add_accel_group(self.accelGroup)

	-- ActionGroup
	self.actionGroup = gtk.ActionGroup.new("Default")

	-- Actions
	self.aNewFile = gtk.Action.new("New file", nil, "Create a new file", "gtk-new")
	self.aNewFile:connect("activate", self.newFile, self)
	self.actionGroup:add_action_with_accel(self.aNewFile)
	self.aNewFile:set_accel_group(self.accelGroup)

	self.aLoadFile = gtk.Action.new("Open file", nil, "Open a file", "gtk-open")
	self.aLoadFile:connect("activate", self.loadFile, self)
	self.actionGroup:add_action_with_accel(self.aLoadFile)
	self.aLoadFile:set_accel_group(self.accelGroup)

	self.aSaveFile = gtk.Action.new("Save file", nil, "Save the current file", "gtk-save")
	self.aSaveFile:connect("activate", self.saveFile, self)
	self.actionGroup:add_action_with_accel(self.aSaveFile)
	self.aSaveFile:set_accel_group(self.accelGroup)

	self.aFind = gtk.Action.new("Find", nil, "Searchs the text", "gtk-find")
	self.aFind:connect("activate", self.showFind, self)
	self.actionGroup:add_action_with_accel(self.aFind)
	self.aFind:set_accel_group(self.accelGroup)

	self.aQuit = gtk.Action.new("Quit", nil, "Quit from the application", "gtk-quit")
	self.aQuit:connect("activate", gtk.main_quit)
	self.actionGroup:add_action_with_accel(self.aQuit)
	self.aQuit:set_accel_group(self.accelGroup)

	self.aCut = gtk.Action.new("Cut", nil, "Cut the selection to the clipboard", "gtk-cut")
	self.aCut:connect("activate", self.cut, self)
	self.actionGroup:add_action_with_accel(self.aCut)
	self.aCut:set_accel_group(self.accelGroup)

	self.aCopy = gtk.Action.new("Copy", nil, "Copy the selection to the clipboard", "gtk-copy")
	self.aCopy:connect("activate", self.copy, self)
	self.actionGroup:add_action_with_accel(self.aCopy)
	self.aCopy:set_accel_group(self.accelGroup)

	self.aPaste = gtk.Action.new("Paste", nil, "Paste the clipboard info to the buffer", "gtk-paste")
	self.aPaste:connect("activate", self.paste, self)
	self.actionGroup:add_action_with_accel(self.aPaste)
	self.aPaste:set_accel_group(self.accelGroup)

	self.aDelete = gtk.Action.new("Delete", nil, "Delete the selection", "gtk-delete")
	self.aDelete:connect("activate", self.delete, self)
	self.actionGroup:add_action_with_accel(self.aDelete)
	self.aDelete:set_accel_group(self.accelGroup)

	self.aAbout = gtk.Action.new("About", nil, "About this application...", "gtk-about")
	self.aAbout:connect("activate", self.showAbout, self)
	self.actionGroup:add_action_with_accel(self.aAbout, "F1")
	self.aAbout:set_accel_group(self.accelGroup)

	-- Toolbar
	self.toolbar = gtk.Toolbar.new()
	self.toolbar:set("toolbar-style", gtk.TOOLBAR_ICONS)
	
	self.toolbar:add(
		self.aNewFile:create_tool_item(),
		gtk.SeparatorToolItem.new(),
		self.aLoadFile:create_tool_item(),
		self.aSaveFile:create_tool_item(),
		self.aFind:create_tool_item(),
		gtk.SeparatorToolItem.new(),
		self.aAbout:create_tool_item()
	)

	-- Menu
	self.menubar = gtk.MenuBar.new()
	self.file = gtk.Menu.new()
	self.fileItem = gtk.MenuItem.new_with_mnemonic("_File")
	
	self.file:add(
		self.aNewFile:create_menu_item(),
		gtk.SeparatorMenuItem.new(),
		self.aLoadFile:create_menu_item(),
		self.aSaveFile:create_menu_item(),
		self.aFind:create_menu_item(),
		gtk.SeparatorMenuItem.new(),
		self.aQuit:create_menu_item()
	)
	
	self.fileItem:set_submenu(self.file)

	self.edit = gtk.Menu.new()
	self.editItem = gtk.MenuItem.new_with_mnemonic("_Edit")
	self.editItem:connect("activate", self.updateActions, self)

	self.edit:add(self.aCut:create_menu_item())
	self.edit:add(self.aCopy:create_menu_item())
	self.edit:add(self.aPaste:create_menu_item())
	self.edit:add(self.aDelete:create_menu_item())
	self.editItem:set_submenu(self.edit)

	self.help = gtk.Menu.new()
	self.helpItem = gtk.MenuItem.new_with_mnemonic("_Help")
	gtk.MenuShell.append(self.help, self.aAbout:create_menu_item())
	self.helpItem:set_submenu(self.help)

	self.menubar:add(self.fileItem, self.editItem, self.helpItem)
	
	-- Packing it!
	self.vbox:pack_start(self.menubar, false, false, 0)
	self.vbox:pack_start(self.toolbar, false, false, 0)
	self.scroll:add(self.view)
	self.vbox:pack_start(self.scroll, true, true, 0)
	self.vbox:pack_start(self.statusbar, false, false, 0)
	self.window:add(self.vbox)

	-- Set the hooks
	gtk.about_dialog_set_email_hook(self.openUri, {self, "mailto:"})
	gtk.about_dialog_set_url_hook(self.openUri, {self})

	-- About dialog

	self.about = gtk.AboutDialog.new()
	self.logo = gdk.Pixbuf.new_from_file("icon.png")
	self.about:set("program-name", "Simple editor", "authors",
		{"Lucas Hermann Negri <kkndrox@gmail.com>"}, "comments", "Example app!",
		"license", "LGPL 3+", "logo", self.logo, "title", "About...",
		"website", "http://oproj.tuxfamily.org", "window-position",
		gtk.WIN_POS_CENTER)

	self.window:set("title", "Simple editor", "width-request", 500,
		"height-request", 520, "window-position", gtk.WIN_POS_CENTER,
		"icon-name", "gtk-edit")

	self.window:connect("delete-event", gtk.main_quit)
	self.view:grab_focus()

	return self
end

---
-- Builds a find dialog.
--
-- @return The dialog, the entry widget
function Editor:buildFindDialog()
	local dialog = gtk.Dialog.new()
	local vbox = dialog:get_content_area()
	local entry = gtk.Entry.new()
	vbox:pack_start(gtk.Label.new("Search for:"), true, false, 5)
	vbox:pack_start(entry, true, true, 5)
	vbox:show_all()

	dialog:set("title", "Find", "window-position", gtk.WIN_POS_CENTER)
	dialog:add_buttons("gtk-go-back", __BACK, "gtk-go-forward", __FORWARD,
		"gtk-close", __CLOSE)

	return dialog, entry
end

---
-- Shows the about dialog
function Editor:showAbout()
	self.about:run()
	self.about:hide()
end

---
-- Logs a message in the statusbar
--
-- @param msn Message to log
function Editor:log(msg)
	self.statusbar:pop(self.context)
	self.statusbar:push(self.context, msg)
end

---
-- Loads the contents of a file and shows it in the editor
function Editor:loadFile()
	self.dialog:set("action", gtk.FILE_CHOOSER_ACTION_OPEN)
	local res = self.dialog:run()
	self.dialog:hide()

	if res == gtk.RESPONSE_OK then
		local fileName = self.dialog:get_filename()
		local file = io.open(fileName)
		self.buffer:set("text", file:read("*a"))
		file:close()

		self.openedFile = fileName
		fileName = glib.Path.get_basename(fileName)
		self:log("Loaded from " .. fileName)
	end
end

---
-- Saves the content of the editor into a file
function Editor:saveFile()
	local destination

	if self.openedFile then
		destination = self.openedFile
	else
		self.dialog:set("action", gtk.FILE_CHOOSER_ACTION_SAVE)
		local res = self.dialog:run()
		self.dialog:hide()

		if res == gtk.RESPONSE_OK then
			destination = self.dialog:get_filename()
		end
	end

	if destination then
		self:doSave(destination)
	end
end

---
-- Do the real save.
--
-- @param destination File that will receive the buffer text
function Editor:doSave(destination)
	local file = io.open(destination, "w")
	file:write(self.buffer:get("text"))
	file:close()
	self.openedFile = destination
	self:log("Saved to " .. glib.Path.get_basename(destination))
end

---
-- Creates a new document
function Editor:newFile()
	self.buffer:set("text", "")
	self.openedFile = nil
	self:log("Untitled document")
end

---
-- Copy the selected info to the clipboard
function Editor:copy()
	self.buffer:copy_clipboard(self.clip)
end

---
-- Cut the selected info to the clipboard
function Editor:cut()
	self.buffer:cut_clipboard(self.clip, true)
end

---
-- Paste the clipboard info to the buffer
function Editor:paste()
	self.buffer:paste_clipboard(self.clip, nil, true)
end

---
-- Delete the selected info from the buffer
function Editor:delete()
	self.buffer:delete_selection(true, true)
end

---
-- Shows the find dialog
function Editor:showFind()
	local iter, istart, iend = gtk.TextIter.new(), gtk.TextIter.new(), gtk.TextIter.new()
	self.buffer:get_start_iter(iter)

	-- Run until closed
	while true do
		local res = self.findDialog:run()

		if res == __BACK or res == __FORWARD then
			local found, text, li = false, self.findEntry:get("text")

			if res == __BACK then
				if mark1 then self.buffer:get_iter_at_mark(iter, mark1) end
				found = gtk.TextIter.backward_search(iter, text, 0, istart, iend)
			else
				if mark2 then self.buffer:get_iter_at_mark(iter, mark2) end
				found = gtk.TextIter.forward_search(iter, text, 0, istart, iend)
			end

			-- Found the text?
			if found then
				self.buffer:select_range(istart, iend)
				mark1 = self.buffer:create_mark("last_start", istart, false)
				mark2 = self.buffer:create_mark("last_end", iend, false)
				self.view:scroll_to_mark(mark1)
				found = false
			end
		else
			break
		end
	end

	self.findDialog:hide()
end

---
-- Opens the default web browser / email client
function Editor:openUri(uri)
	local self, extra = unpack(self)
	local total = (extra or "") .. uri
	
	print("Opening:", total)
	gtk.show_uri(nil, total, gdk.CURRENT_TIME)
end

---
-- Updates the actions.
function Editor:updateActions()
	local selected = self.buffer:get_selection_bounds()
	local paste = self.clip:wait_is_text_available()

	self.aCut:set("sensitive", selected)
	self.aDelete:set("sensitive", selected)
	self.aCopy:set("sensitive", selected)
	self.aPaste:set("sensitive", paste)
end

-- * Main *
local inst = Editor.new()
inst.window:show_all()
gtk.main()

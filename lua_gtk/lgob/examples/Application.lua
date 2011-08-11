#! /usr/bin/env lua

-- Sample application, in an OO way

require("lgob.gdk")
require("lgob.gtk")

App = {}

---
-- Constructor
--
-- @return A new App instance
function App.new()
	local self = {}
	setmetatable(self, {__index = App})

	self.window = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
	self.vbox = gtk.VBox.new(false, 0)
	self.content = gtk.Label.new("Here goes the content")
	self.statusbar = gtk.Statusbar.new()
	self.progress = gtk.ProgressBar.new()
	self.progress:set("fraction", 0.5)
	self.statusbar:pack_start(self.progress, false, false, 0)
	
	self.context = self.statusbar:get_context_id("default")
	self.statusbar:push(self.context, "Statusbar message")

	-- Actions
	self.a_open_file = gtk.Action.new("Open File", nil, "Open a file", "gtk-open")
	self.a_open_file:connect("activate", function() print("File opened!") end)
	self.a_copy = gtk.Action.new("Coapy", nil, "Copy selected text", "gtk-copy")
	self.a_paste = gtk.Action.new("Paste", nil, "Paste selected text", "gtk-paste")
	self.a_quit = gtk.Action.new("Quit", nil, "Quit from the application", "gtk-quit")
	self.a_quit:connect("activate", gtk.main_quit)
	self.a_about = gtk.Action.new("About", nil, "About this application...", "gtk-about")
	self.a_about:connect("activate", self.show_about, self)

	-- AccelGroup
	self.accel_group = gtk.AccelGroup.new()
	self.window:add_accel_group(self.accel_group)

	-- Action groups
	self.copy_paste_group = gtk.ActionGroup.new("Copy paste")
	self.copy_paste_group:add_action(self.a_copy)
	self.copy_paste_group:add_action(self.a_paste)
	self.copy_paste_group:set("sensitive", false)
	
	self.file_group = gtk.ActionGroup.new("File")

	self.file_group:add_action_with_accel(self.a_open_file)
	self.a_open_file:set_accel_group(self.accel_group)

	self.file_group:add_action_with_accel(self.a_quit)
	self.a_quit:set_accel_group(self.accel_group)

	self.file_group:add_action_with_accel(self.a_about, "F1")
	self.a_about:set_accel_group(self.accel_group)

	-- Toolbar
	self.toolbar = gtk.Toolbar.new()
	self.toolbar:add(self.a_open_file:create_tool_item())
	self.toolbar:add(gtk.SeparatorToolItem.new())
	self.toolbar:add(self.a_copy:create_tool_item())
	self.toolbar:add(self.a_paste:create_tool_item())
	self.toolbar:add(gtk.SeparatorToolItem.new())
	self.toolbar:add(self.a_about:create_tool_item())

	-- Menu
	self.menubar = gtk.MenuBar.new()
	self.file = gtk.Menu.new()
	self.file_item = gtk.MenuItem.new_with_mnemonic("_File")
	self.open_item = self.a_open_file:create_menu_item()
	self.quit_item = self.a_quit:create_menu_item()
	self.copy_item = self.a_copy:create_menu_item()
	self.paste_item = self.a_paste:create_menu_item()
	
	self.file:append(self.open_item)
	self.file:append(gtk.SeparatorMenuItem.new())
	self.file:append(self.copy_item)
	self.file:append(self.paste_item)
	self.file:append(gtk.SeparatorMenuItem.new())
	self.file:append(self.quit_item)
	
	self.file_item:set_submenu(self.file)

	self.help = gtk.Menu.new()
	self.help_item = gtk.MenuItem.new_with_mnemonic("_Help")
	self.about_item = self.a_about:create_menu_item()
	self.help:append(self.about_item)
	self.help_item:set_submenu(self.help)

	self.menubar:append(self.file_item)
	self.menubar:append(self.help_item)
	
	-- Packing it!
	self.vbox:pack_start(self.menubar, false, false, 0)
	self.vbox:pack_start(self.toolbar, false, false, 0)
	self.vbox:pack_start(self.content, true, true, 0)
	self.vbox:pack_start(self.statusbar, false, false, 0)
	self.window:add(self.vbox)

	-- About dialog
	self.about = gtk.AboutDialog.new()
	self.logo = gdk.Pixbuf.new_from_file("icon.png")
	self.about:set("program-name", "Sample Application", "authors", {"Lucas Hermann Negri <kkndrox@gmail.com>"},
	"comments", "No comments!", "license", "LGPL 3+", "logo", self.logo, "title", "About...")

	self.window:set("title", "Sample Application", "width-request", 300,
		"height-request", 300, "window-position", gtk.WIN_POS_CENTER,
		"icon-name", "gtk-about")

	self.window:connect("delete-event", gtk.main_quit)

	return self
end

---
-- Shows the about dialog
function App:show_about()
	self.about:run()
	self.about:hide()
end

-- * Main *
local inst = App.new()
inst.window:show_all()

gtk.main()

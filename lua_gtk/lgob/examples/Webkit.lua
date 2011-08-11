#! /usr/bin/env lua

require('lgob.loader')
require('lgob.webkit')

Browser = {}

function Browser.new()
	local self = {}
	setmetatable(self, {__index = Browser})
	
	self.window = gtk.Window.new()
	self.vbox   = gtk.VBox.new(false, 5)
	self.hbox   = gtk.HBox.new(false, 5)
	self.entry  = gtk.Entry.new()
	self.entry:set('text', 'http://oproj.tuxfamily.org')
	self.model  = gtk.ListStore.new('gchararray')
	self.comp   = gtk.EntryCompletion.new()
	self.comp:set_model(self.model)
	self.comp:set_text_column(0)
	self.compHash = {}
	self.iter   = gtk.TreeIter.new()
	self.entry:set_completion(self.comp)
	self.button = gtk.Button.new_from_stock('gtk-jump-to')
	
	self.button:connect('clicked', function() 
		local url = self.entry:get('text')
		self:navigate(url)
	end)
	
	self.hbox:pack_start(self.entry, true, true, 5)
	self.hbox:pack_start(self.button, false, true, 0)
	
	self.scrolled = gtk.ScrolledWindow.new()
	self.webview = webkit.WebView.new()
	self.settings = self.webview:get('settings')
	
	self.scrolled:set('hscrollbar-policy', gtk.POLICY_AUTOMATIC,
		'vscrollbar-policy', gtk.POLICY_AUTOMATIC)
	self.scrolled:add(self.webview)
	
	self.vbox:pack_start(self.hbox, false, true, 5)
	self.vbox:pack_start(self.scrolled, true, true, 0)
	
	self.window:add(self.vbox)
	self.window:set('title', 'Browser demo')
	self.window:maximize()
	self.window:show_all()
	self.window:connect('delete-event', gtk.main_quit)
	
	return self
end

function Browser:navigate(url)
	local url = (url and #url > 0) and url or 'http://oproj.tuxfamily.org'
	if url:sub(1, 7) ~= 'http://' then url = 'http://' .. url end
	webkit.WebView.open(self.webview, url)
	self.entry:set('text', url)
	
	if not self.compHash[url] then 
		self.compHash[url] = true
		self.model:append(self.iter)
		self.model:seto(self.iter, url)
	end
end

glib.thread_init()

local obj = Browser.new()
obj:navigate('http://oproj.tuxfamily.org')
gtk.main()

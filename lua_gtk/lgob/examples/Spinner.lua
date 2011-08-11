#! /usr/bin/env lua
require('lgob.gtk')

local win       = gtk.Window.new()
local vbox      = gtk.VBox.new(true, 5)
local start     = gtk.Button.new_with_mnemonic("Start")
local stop      = gtk.Button.new_with_mnemonic("Stop")
local spinner   = gtk.Spinner.new()

vbox:add(spinner, start, stop)
win:add(vbox)

win:set('title', "Spinner", 'window-position', gtk.WIN_POS_CENTER,
    'width-request', 100)
win:connect('delete-event', gtk.main_quit)

start:connect('clicked', function() spinner:start() end)
stop:connect('clicked',  function() spinner:stop()  end)

win:show_all()
gtk.main()

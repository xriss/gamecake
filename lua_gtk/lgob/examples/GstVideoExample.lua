#! /usr/bin/env lua

-- Shows how to play a video in a window

require("lgob.gtk")
require("lgob.gdk")
require("lgob.gst")

-- Configure the source and sink
local src = gst.ElementFactory.make("videotestsrc", "videosrc")
local sink = gst.ElementFactory.make("xvimagesink", "videosink")
sink:set("sync", false, "force-aspect-ratio", true)

-- Create the pipeline and link the source to the sink
local pipeline = gst.Pipeline.new("my_pipeline")
pipeline:add(src, sink)
src:link(sink)

-- Callback
local callback_id
function window_exposed(event, data)
	local handle = window:get("window"):get_handle()
	gst.XOverlay.set_xwindow_id(sink, handle)
	pipeline:set_state(gst.STATE_PLAYING)
	
	-- No more need for it
	window:disconnect(callback_id)
	
	return true
end

-- Clean (gstreamer module needs "manual" ref handling)
function quit()
	pipeline:set_state(gst.STATE_NULL)
	pipeline:unref()
	gtk.main_quit()
	
	return true
end

-- Create and configure the window
window = gtk.Window.new()
window:set("window-position", gtk.WIN_POS_CENTER, "title", "Video test!")
callback_id = window:connect("expose-event", window_exposed)
window:connect("delete-event", quit)
window:modify_bg(gtk.STATE_NORMAL, gdk.color_parse("black"))
window:show_all()

-- Run it
gtk.main()

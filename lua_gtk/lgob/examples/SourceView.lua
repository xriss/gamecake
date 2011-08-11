#! /usr/bin/env lua

require("lgob.loader")          -- completion seems to be a module now
require("lgob.gtksourceview")

local window = gtk.Window.new()
window:set("title", "GtkSourceView demo", "window-position", gtk.WIN_POS_CENTER,
	"width-request", 400, "height-request", 400)
window:connect("delete-event", gtk.main_quit)

local view = gtk.SourceView.new()
local buffer = view:get("buffer")
local manager = gtk.source_language_manager_get_default()
local lang = manager:get_language("c")

view:set("show-line-numbers", true, "highlight-current-line", true,
	"auto-indent", true)

buffer:set("text", [[
#include <stdio.h>

int main(int argc, char* argv[])
{
	printf("Hello world!\n");
	return 0;
}
]], "language", lang)

window:add(view)

window:show_all()
gtk.main()

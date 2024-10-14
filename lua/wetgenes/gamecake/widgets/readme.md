


---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.gamecake.widgets


	local widgets=oven.rebake("wetgenes.gamecake.widgets")

A collection of widgets, rendered using gles2 code and controlled 
using the mouse, keyboard or a joystick. EG click fire and move 
left/right to adjust a slider value.

Widgets must be created and bound to an oven, using the 
oven.rebake function.

This has undergone a number of rewrites as we try to simplify the 
widget creation and layout process. Eventually we ended up with a 
fixed size system of widget placement so every widget must have a 
known size in advance, however we allow scaling to occur so for 
instance building a 256x256 widget does not mean that it has to be 
displayed at 256x256 it just means it will be square.

The basic layout just lets you place these widgets in other widgets 
as left to right lines. So as long as you get your sizes right you 
can easily place things just using a list and without keeping track 
of the current position.

Other layout options are available, such as absolute positioning for 
full control and we have simple custom skin versions of the buttons 
as well rather than the built in skins.

All value data is handled by data structures that contain ranges and 
resolutions for data allowing the same data to be bound to multiple 
display widgets. For instance the same data can be linked to the 
position of a slider as well as the content of a text display. I 
think the kids call this sort of thing an MVC pattern but that's a 
terribly dull name.

Swanky paint is probably the most advanced used of the widgets so far
but I suspect we will be making a simple text editor in the near 
future. Designed for advanced in game tweaking of text files.




## lua.wetgenes.gamecake.widgets.button


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="button",...}

A button for pressing and such.



## lua.wetgenes.gamecake.widgets.button.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

As a button we always need to be solid so this is forced to true.

Also cursor will be set to "hand" if it is not already set so you can 
tell it is a button when you hover over it.



## lua.wetgenes.gamecake.widgets.button.update


	this function will also call lua.wetgenes.gamecake.widgets.meta.update

If we have a data assigned to this button then make sure that the 
displayed text is up to date. We should really hook into the data so 
any change there is copied here instead?



## lua.wetgenes.gamecake.widgets.center


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="center",...}

A layout class for very centered children.



## lua.wetgenes.gamecake.widgets.center.layout


	this function will also call lua.wetgenes.gamecake.widgets.meta.layout

Place any children in the center of this widget. Multiple children will 
overlap so probably best to only have one child.



## lua.wetgenes.gamecake.widgets.center.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

If not explicetly set we will use a size of "full" ie the size of 
parent as that is probably how this class will always be used.



## lua.wetgenes.gamecake.widgets.checkbox


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="checkbox",...}

A button that can be used to display and toggle a single bit of data.



## lua.wetgenes.gamecake.widgets.checkbox.class_hooks


We catch and react to the click hook as we can toggle the data_mask bit 
in the data.



## lua.wetgenes.gamecake.widgets.checkbox.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

As a button we always need to be solid so this is forced to true.

cursor will be set to "hand" if it is not already set so you can 
tell it is a button when you hover over it.

data_mask defaults to 1 and represents the bit (or bits) that should be 
tested and toggled in the data. The default of 1 and assuming your data 
starts at 0 means that the data will toggle between 0 and 1 using this 
checkbox.

text_false defaults to " " and is the text that will be displayed when 
a data_mask test is false.

text_true defaults to "X" and is the text that will be displayed when 
a data_mask test is true.



## lua.wetgenes.gamecake.widgets.checkbox.update


	this function will also call lua.wetgenes.gamecake.widgets.meta.update

If we have a data assigned to this checkbox then make sure that the 
displayed text is up to date. We should really hook into the data so 
any change there is copied here instead?

We use data_mask to check a single bit and then set the text to either 
text_false or text_true depending on the result.




## lua.wetgenes.gamecake.widgets.data


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local data=master.new_data{}

A number or string that can be shared between multiple widgets given 
basic limits and watched for changes.

This alows the same data to be linked and displayed in multiple widgets 
simultaneously.



## lua.wetgenes.gamecake.widgets.data.add_class_hook




## lua.wetgenes.gamecake.widgets.data.call_hook




## lua.wetgenes.gamecake.widgets.data.call_hook_later




## lua.wetgenes.gamecake.widgets.data.data_dec


adjust number (may trigger hook)



## lua.wetgenes.gamecake.widgets.data.data_get_pos


get display pos, given the size of the parent and our size?



## lua.wetgenes.gamecake.widgets.data.data_get_size


how wide or tall should the handle be given the size of the parent?



## lua.wetgenes.gamecake.widgets.data.data_inc


adjust number (may trigger hook)



## lua.wetgenes.gamecake.widgets.data.data_set


set a number value and min/max range probably without any triggers



## lua.wetgenes.gamecake.widgets.data.data_snap


given the parents size and our relative position/size within it update 
dat.num and return a new position (for snapping)



## lua.wetgenes.gamecake.widgets.data.data_tonumber


get a number from the string



## lua.wetgenes.gamecake.widgets.data.data_tonumber_from_list




## lua.wetgenes.gamecake.widgets.data.data_tostring


get a string from the number



## lua.wetgenes.gamecake.widgets.data.data_tostring_from_list




## lua.wetgenes.gamecake.widgets.data.data_value


set number (may trigger hook unless nohook is set)



## lua.wetgenes.gamecake.widgets.data.del_class_hook




## lua.wetgenes.gamecake.widgets.data.new_data




## lua.wetgenes.gamecake.widgets.datas


Handle a collection of data (IE in the master widget)



## lua.wetgenes.gamecake.widgets.datas.del




## lua.wetgenes.gamecake.widgets.datas.get




## lua.wetgenes.gamecake.widgets.datas.get_number




## lua.wetgenes.gamecake.widgets.datas.get_string




## lua.wetgenes.gamecake.widgets.datas.get_value




## lua.wetgenes.gamecake.widgets.datas.new




## lua.wetgenes.gamecake.widgets.datas.new_datas




## lua.wetgenes.gamecake.widgets.datas.set




## lua.wetgenes.gamecake.widgets.datas.set_infos




## lua.wetgenes.gamecake.widgets.datas.set_string




## lua.wetgenes.gamecake.widgets.datas.set_value




## lua.wetgenes.gamecake.widgets.defs


Helpers to define defaults for each class of widget 



## lua.wetgenes.gamecake.widgets.defs.add




## lua.wetgenes.gamecake.widgets.defs.add_border




## lua.wetgenes.gamecake.widgets.defs.copy




## lua.wetgenes.gamecake.widgets.defs.create




## lua.wetgenes.gamecake.widgets.defs.reset




## lua.wetgenes.gamecake.widgets.defs.set




## lua.wetgenes.gamecake.widgets.dialogs


handle a collection of dialogs that all live in the same place



## lua.wetgenes.gamecake.widgets.dialogs.hide_overlay




## lua.wetgenes.gamecake.widgets.dialogs.setup




## lua.wetgenes.gamecake.widgets.dialogs.show




## lua.wetgenes.gamecake.widgets.dialogs.show_overlay




## lua.wetgenes.gamecake.widgets.drag


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="drag",...}

A button to drag arround.



## lua.wetgenes.gamecake.widgets.drag.drag




## lua.wetgenes.gamecake.widgets.drag.setup




## lua.wetgenes.gamecake.widgets.drag.update




## lua.wetgenes.gamecake.widgets.paragraph


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="paragraph",...}

A layout class for a paragraph of wordwrapped text that ignores 
quantity of whitespace and possibly child widgets insereted into the 
text and apropriate control points.

We will be the full width of our parent and as tall as we need to be to 
fit the given text.



## lua.wetgenes.gamecake.widgets.paragraph.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options



## lua.wetgenes.gamecake.widgets.setup


	master=oven.rebake("wetgenes.gamecake.widgets").setup()

	master=oven.rebake("wetgenes.gamecake.widgets").setup(
		{font="Vera",text_size=16,grid_size=32,skin=0} )

Create a master widget, this widget which is considered the root of 
your GUI. It will be filled with functions/data and should contain all 
the functions you need to add data and widgets.

You can pass in these configuration values in a table, the example 
shown above has soom good defaults.

	font="Vera"

The default font to use, this must have already been loaded via 
wetgenes.gamecake.fonts functions.

	text_size=16
	
The default pixel height to render text at.

	grid_size=32
	
The size in pixels that we try and create buttons at.


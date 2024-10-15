

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## draw


	draw()

Draw called every frame, there may be any number of updates between 
each draw but hopefully we are looking at one update followed by a 
draw, if you have an exceptionally fast computer then we may even get 0 
updates between some draws.



## entities


	entities.reset()
	
empty the list of entites to update and draw

	entities.caste(caste)

get the list of entities of a given caste, eg "bullets" or "enemies"

	entities.add(it,caste)
	entities.add(it)

add a new entity of caste or it.caste to the list of things to update 

	entities.call(fname,...)

for every entity call the function named fname like so it[fname](it,...)



	entities.get(name)

get a value previously saved, this is an easy way to find a unique 
entity, eg the global space but it can be used to save any values you 
wish not just to bookmark unique entities.

	entities.set(name,value)

save a value by a unique name

	entities.manifest(name,value)

get a value previously saved, or initalize it to the given value if it 
does not already exist. The default value is {} as this is intended for 
lists.



	entities.systems

A table to register or find a global system, these are not cleared by 
reset and should not contain any state data, just functions to create 
the actual entity or initialise data.



	entities.tiles

These functions are called as we generate a level from ascii, every 
value in the tile legend data is checked against all the strings in 
entities.tiles and if it matches it calls that function which is then 
responsible for adding the appropriate collision and drawing code to 
make that tile actually add something to the level.

The basic values of tile.tile and tile.back are used to write graphics 
into the two tile layers but could still be caught here if you need to.

Multiple hooks may get called for a single tile, think of each string 
as a flag to signal that something happens and its value describes what 
happens.




## entities.systems


A global table for entity systems to live in.

	entities.systems_call(fname,...)
	
Call the named function on any systems that currently exist. For 
instance entities.systems_call("load") is used at the bottom 
of this file to prepare graphics of registered systems.



## entities.systems.bang


a bang



## entities.systems.donut


	donut = entities.systems.donut.add(opts)

Add an donut.



## entities.systems.horde


The invading horde



## entities.systems.invader


an invader



## entities.systems.item


	item = entities.systems.item.add()

items, can be used for general things, EG physics shapes with no special actions



## entities.systems.level


	entities.systems.level.setup(level)

reset and setup everything for this level idx



## entities.systems.menu


	menu = entities.systems.menu.setup()

Create a displayable and controllable menu system that can be fed chat 
data for user display.

After setup, provide it with menu items to display using 
menu.show(items) then call update and draw each frame.




## entities.systems.missile


a missile



## entities.systems.npc


	npc = entities.systems.npc.add(opts)

Add an npc.



## entities.systems.player


	entities.systems.player.controls(it,fast)

Handle player style movement, so we can reuse this code for player 
style monsters. it is a player or monster, fast lets us tweak the speed 
and defaults to 1

movement controls are set in it

it.move which is "left" or "right" to move left or right
it.jump which is true if we should jump



## entities.systems.score


	score = entities.systems.score.setup()

Create entity that handles the score hud update and display



## entities.systems.space


	space = entities.systems.space.setup()

Create the space that simulates all of the physics.



## entities.systems.stars


The stars



## entities.systems.tile


setup background tile graphics



## entities.systems.yarn


Handle the main yarn setup update and draw.



## entities.tiles.npc


Display a npc



## entities.tiles.sprite


Display a sprite



## entities.tiles.start


The player start point, just save the x,y



## graphics



define all graphics in this global, we will convert and upload to tiles 
at setup although you can change tiles during a game, we try and only 
upload graphics during initial setup so we have a nice looking sprite 
sheet to be edited by artists



## hardware


select the hardware we will need to run this code, eg layers of 
graphics, colors to use, sprites, text, sound, etc etc.

Here we have chosen the default 320x240 setup.



## levels


Design levels here



## prefabs


The yarn building blocks, recipes to build items from, multiple rules 
can be assigned to each item.



## rules


How the yarn engine should behave



## setup


Called once to setup things in the first update loop after hardware has 
been initialised.



## setup_menu


	menu = setup_menu()

Create a displayable and controllable menu system that can be fed chat 
data for user display.

After setup, provide it with menu items to display using 
menu.show(items) then call update and draw each frame.




## update


	update()

Update and draw loop, called every frame.

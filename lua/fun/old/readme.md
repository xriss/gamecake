

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## hardware


select the hardware we will need to run this code, eg layers of 
graphics, colors to use, sprites, text, sound, etc etc.

Here we have chosen the default 320x240 setup.

This also provides a default main function that will upload the 
graphics and call the provided update/draw callbacks.



## setup_menu


	menu = setup_menu()

Create a displayable and controllable menu system that can be fed chat 
data for user display.

After setup, provide it with menu items to display using 
menu.show(items) then call update and draw each frame.




## update


	update()

Update and draw loop, called every frame.

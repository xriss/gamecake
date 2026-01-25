
not sure why but have disable force resets on update in cpBody.c as that seems like, uhm, bad?


	https://chipmunk-physics.net/forum/viewtopic.php?t=484
	Re: Force vs Impulse?
	Post by slembcke » Tue Jun 16, 2009 5:19 pm
	
	Yes, it will work better to spread the force out over several frames. 
	Forces are not reset every step like they are in some other physics 
	engines. If you set a force on an object, it will continue to be 
	affected by it until you change or reset the force.

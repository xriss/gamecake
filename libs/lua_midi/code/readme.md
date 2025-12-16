

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.midi.clients


	m:clients()

fetch table of clients




## lua.wetgenes.midi.create


	m=wmidi.create()

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns a midi object.



## lua.wetgenes.midi.destroy


	m:destroy()

Free the midi and associated memory. This will of course be done 
automatically by garbage collection but you can force it explicitly 
using this function.



## lua.wetgenes.midi.event_to_string


	str = m:event_to_string(event)

Convert an event to a single line string for printing to the console.



## lua.wetgenes.midi.get


	m:get()

get all values for this connection and store them in m




## lua.wetgenes.midi.peek


	m:peek(it)

Returns a event if there is one or null if none are currently 
available.



## lua.wetgenes.midi.port_create


	p = m:port_create(name,caps,type)

Create a port with the given name and capability bits and type.

bits and type are either a number or a table of bitnames.

Returns the number of the port created which should be used in 
port_destroy or nil if something went wrong.



## lua.wetgenes.midi.port_destroy


	m:port_destroy(num)

Destroy a previously created port. Returns nil on failure, true on 
success.



## lua.wetgenes.midi.pull


	m:pull(it)

Receive an input midi event, blocking until there is one.

Occasionally, for "reasons" this may return nil.



## lua.wetgenes.midi.push


	m:push(it)

Send an output midi event.



## lua.wetgenes.midi.set


	m:set()

set all values for this connection from values found in m




## lua.wetgenes.midi.string_to_clientport


	client,port = m:string_to_clientport(str)

Convert a "client:port" string to two numbers client,port this can 
either be two decimal numbers or, if a m:scan() has been performed, 
then a partial case insensitive matching to the name of existing 
clients and ports may get a port number.

Will return a nil if we can not work out which client or port you mean.



## lua.wetgenes.midi.subscribe


	m:subscribe{
		source_client=0,	source_port=0,
		dest_client=1,		dest_port=0,
	}

	m:subscribe{
		source="0:0",
		dest="1:0",
	}

Creates a persistent subscription between two ports.



## lua.wetgenes.midi.unsubscribe


	m:unsubscribe{
		source_client=0,	source_port=0,
		dest_client=1,		dest_port=0,
	}

	m:unsubscribe{
		source="0:0",
		dest="1:0",
	}

Removes a persistent subscription from between two ports.

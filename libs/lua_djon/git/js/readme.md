DJON is a UTF8 only relaxed superset of JSON. DJON supports round trip 
comments, numbers are explicitly 64bit floats and strings can contain 
raw binary data.

This is implimented as a c module using wasm so must be imported rather 
than required as it needs async to load the wasm.

A valid utf8 json file is always a valid djon file.

Djon is half man half machine:

Pretty djon makes for more human readable and editable json style 
configuration files and compact djon is a machine readable format that 
can store binary data.

	import djon from "@xriss/djon"

	let data = djon.load_file("filename.json") // load in djon/json format
	
	djon.save_file("filename.json",data) // pretty json
	djon.save_file("filename.compact.json",data,"compact") // compact json
	djon.save_file("filename.djon",data,"djon") // pretty djon
	djon.save_file("filename.compact.djon",data,"djon","compact") // compact djon

	let text = djon.save(data,"djon") // save to string

	let datb = djon.load(text) // load from string

	// resave a djon file, pre-existing comments will be preserved
	// can be used to round trip comments in config files
	djon.save_comments("filename.djon",data)


if you are stuck in cjs then you may also use the nodejs import 

	let djon=(await import("@xriss/djon")).default

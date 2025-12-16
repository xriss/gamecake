// must async so we can await
(async function()
{

	// force relative path so we always test current version
	let djon=(await import("../../js/djon.js")).default /* but you should use...
	let djon=(await import("@xriss/djon")).default // npm install @xriss/djon 
	*/

	console.log("roundtrip data example")

	// normal json data structure but no comments
	let data=djon.load_file("test.djon")

	// can modify data here
	data.this_is_new="not old"
	data.show="old comments are kept even if we change data type"

	// save djon with comments ( loaded from output file )
	djon.save_comments("test.djon",data)

})()

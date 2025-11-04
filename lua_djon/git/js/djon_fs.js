

import fs from "fs"

import djon from "./djon.js"

const djon_fs={}
export default djon_fs

// dupe all base djon function
for ( let n in djon )
{
	djon_fs[n]=djon[n]
}

djon_fs.load_file=function(fname,...args)
{
	let it={}
	it.data = fs.readFileSync(fname, 'utf8')
	return djon.load_core(it,...args)
}

djon_fs.save_file_comments=function(fname,tree,...args)
{
	let com ; try{ // ignore errors (probably missing file)
		com=djon_fs.load_file(fname,"comments",...args)
	}catch(e){}
	
	let it={}
	it.tree=djon.merge_comments( tree , com ) // merge comments
	let data=djon.save_core(it,"comments","djon",...args)
	fs.writeFileSync(fname, data, 'utf8')
}

djon_fs.save_file=function(fname,tree,...args)
{
	let it={}
	it.tree=tree
	let data=djon.save_core(it,...args)
	fs.writeFileSync(fname, data, 'utf8')
}


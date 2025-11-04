

// ideally we would import the wasm but currently this dumb base64 wrapper is the only way
import { djoncore } from "./djon_core.wasm.js"

const djon={}
export default djon

/*

merge comments in com into the data in dat. Can also just be used to 
convert dat into a comments format if com is null.

*/
djon.merge_comments=function( dat , com )
{
	let out=dat
	if( Array.isArray(dat) || Array.isArray(com) ) // output must be array
	{
		out=[dat] // must be array
		if( Array.isArray(com) ) // copy comments
		{
			for( let i=1 ; i<com.length ; i++ ){ out[i]=com[i] }
			com=com[0]
		}
		if( Array.isArray(dat) ) // recurse
		{
			let o=[]
			out[0]=o
			for( let i=0 ; i<dat.length ; i++ )
			{
				let d=dat[i]
				let c = (Array.isArray(com))&&(com ? com[i] : null)
				o[i]=djon.merge_comments(d,c)
			}
		}
	}
	else
	if( typeof(dat)=="object" ) // need to recurse
	{
		let o={}
		out=o
		for( let n in dat )
		{
			let d=dat[n]
			let c = (typeof(com)=="object")&&(com ? com[n] : null)
			o[n]=djon.merge_comments(d,c)
		}
	}
	return out
}

/*

Remove comments converting com back into standard json data and 
returning it.

*/
djon.remove_comments=function( com )
{
	let out=com
	if( Array.isArray(out) ) // strip comments
	{
		out=out[0]
	}
	if( typeof(out)=="object" ) // need to copy and recurse
	{
		let o={}
		for( let n in out ){ o[n]=djon.remove_comments(out[n]) }
		out=o
	}
	return out
}


djon.load=function(data,...args)
{
	let it={}
	it.data=data
	return djon.load_core(it,...args)
}

djon.load_core=function(it,...args)
{
	it.core=new djoncore()
	it.core.load(it.data,...args)
	djon.check_error(it)
	it.tree=it.core.get(...args)
	djon.check_error(it)
	return it.tree
}

djon.save=function(tree,...args)
{
	let it={}
	it.tree=tree
	return djon.save_core(it,...args)
}

djon.save_core=function(it,...args)
{
	it.core=new djoncore()
	it.core.set(it.tree,...args)
	djon.check_error(it)
	it.data=it.core.save(...args)
	djon.check_error(it)
	return it.data
}

djon.check_error=function(it)
{
	it.err=it.core.locate()
	if(it.err[0])
	{
		throw new Error(it.err[0]+" at line "+it.err[1]+" char "+it.err[2]+" byte "+it.err[3])
	}
}

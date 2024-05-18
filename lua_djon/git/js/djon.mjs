

import fs from 'fs';
import { Environment, napi } from 'napi-wasm';

const wdat = fs.readFileSync( new URL('./djon_core.wasm', import.meta.url) )

const mod = await WebAssembly.instantiate( wdat , {napi:napi } )
let djoncore_env = new Environment(mod.instance);
let djoncore=djoncore_env.exports.djoncore

let djon={}
export default djon

djon.load_file=function(fname,...args)
{
	let it={}
	it.data = fs.readFileSync(fname, 'utf8')
	return djon.load_core(it,...args)
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

djon.save_file=function(fname,tree,...args)
{
	let it={}
	it.tree=tree
	let data=djon.save_core(it,...args)
	fs.writeFileSync(fname, data, 'utf8')
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
